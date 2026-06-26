/*
  ==============================================================================

    Pages.h
    Created: 24 Jun 2026 8:08:29am
    Author:  Luke Amos

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "DraggableComponent.h"

//Multiple Pages that the user can flick through
/* Pages Avialable:
 3D rendering of the device and its movements
 choosing synthesis and ordering the effects
 selecting how the controls map to the device

 */

//**-----------------------------------------------

//Moved the original rendering structure to this block of code where the model rendering screen can be switched between
class ModelPage :public juce::OpenGLAppComponent , public juce::MidiInputCallback {
    
public:
    
    ModelPage(){
        
        openGLContext.setOpenGLVersionRequired(juce::OpenGLContext::openGL3_2);
        
        openGLContext.attachTo(*this);
        
        
    }
    
    ~ModelPage(){
        
        openGLContext.detach();
        
    }
    //OpenGL stuff
    void initialise() override {
        // Enable depth testing
        juce::gl::glEnable(juce::gl::GL_DEPTH_TEST);
        juce::gl::glDepthFunc(juce::gl::GL_LESS);

        
        //In the final project this data needs to be read from binary so that it can be used across devices
        tinyobj::ObjReader reader;
        
        if(!reader.ParseFromFile("/Users/lukeamos/Desktop/SquishCUB.obj")){
            DBG("ERROR " + reader.Error());
            return;
        } else {
            meshAttrib = reader.GetAttrib();
            shapes = reader.GetShapes();
        }
        
        // Variables to track the boundaries of the model
        float minX = std::numeric_limits<float>::max();
        float minY = std::numeric_limits<float>::max();
        float minZ = std::numeric_limits<float>::max();
        
        float maxX = -std::numeric_limits<float>::max();
        float maxY = -std::numeric_limits<float>::max();
        float maxZ = -std::numeric_limits<float>::max();

        // First pass: Populate flat vertices and find boundaries
        for (auto& shape : shapes)
        {
            for (auto& index : shape.mesh.indices)
            {
                float x = meshAttrib.vertices[3 * index.vertex_index + 0];
                float y = meshAttrib.vertices[3 * index.vertex_index + 1];
                float z = meshAttrib.vertices[3 * index.vertex_index + 2];

                minX = std::min(minX, x); maxX = std::max(maxX, x);
                minY = std::min(minY, y); maxY = std::max(maxY, y);
                minZ = std::min(minZ, z); maxZ = std::max(maxZ, z);

                flatVertices.push_back(x);
                flatVertices.push_back(y);
                flatVertices.push_back(z);
            }
        }

        // Calculate the geometric center of the object
        float centerX = (minX + maxX) / 2.0f;
        float centerY = (minY + maxY) / 2.0f;
        float centerZ = (minZ + maxZ) / 2.0f;

        // Second pass: Re-center every vertex around (0, 0, 0)
        for (size_t i = 0; i < flatVertices.size(); i += 3) //Stepping through each of the points and re-positioning them so that the object is centred at the origin , after doing this the rotation is now more natural and spins around its centre positiion as wanted
        {
            flatVertices[i + 0] -= centerX; // Shift X to center
            flatVertices[i + 1] -= centerY; // Shift Y to center
            flatVertices[i + 2] -= centerZ; // Shift Z to center
        }

        DBG("Model auto-centered. Offset removed: X=" + juce::String(centerX) + " Y=" + juce::String(centerY) + " Z=" + juce::String(centerZ)); // returns the amount offseted which allow

        // Shader setup , the variables inside the script are filled with values that the proj uniform try to find
        const char* vertexShader = R"(
            #version 150
            in vec3 position;
            uniform mat4 viewMatrix;
            uniform mat4 projectionMatrix;
            void main()
            {
                gl_Position = projectionMatrix * viewMatrix * vec4(position, 1.0);
            }
        )";

        //the fragment shader controls the colour of the object using a vec4 value , (RGBA)
        const char* fragmentShader = R"(
            #version 150
            out vec4 fragColor;
            uniform vec4 objectColour;
            void main()
            {
                fragColor = objectColour;
            }
        )";
        
        shaderProgram = std::make_unique<juce::OpenGLShaderProgram>(openGLContext);
        shaderProgram->addVertexShader(vertexShader);
        shaderProgram->addFragmentShader(fragmentShader);
        
        juce::gl::glBindAttribLocation(shaderProgram->getProgramID(), 0, "position");
        shaderProgram->link();
        
        // Generate and bind graphics buffers
        juce::gl::glGenVertexArrays(1, &vao); //Raw data positions, "reserve spots in gpu " the refernce is where it writes back the spot assigned
        juce::gl::glGenBuffers(1, &vbo); //How to read them
        
        //Binding the BUffers/ Arrays , is stating to work with this unit
        juce::gl::glBindVertexArray(vao);
        juce::gl::glBindBuffer(juce::gl::GL_ARRAY_BUFFER, vbo);
        
        //Send the data as well as infomation about the data
        juce::gl::glBufferData(juce::gl::GL_ARRAY_BUFFER, flatVertices.size() * sizeof(float), flatVertices.data(), juce::gl::GL_STATIC_DRAW); //Static draw tells the gpu that the data wont change for these positional points
        
        //When reading the previously sent data , it has 3 peices of data per point , as well as stating the size of each point , void* 0 is the data offset our data starts at the start of the buffer so there is no offset
        juce::gl::glVertexAttribPointer(0, 3, juce::gl::GL_FLOAT, juce::gl::GL_FALSE, 3 * sizeof(float), (void*)0);
        //Switch on the attrib
        juce::gl::glEnableVertexAttribArray(0);
        
    }
    
    
    void render() override{
        
        
        //Rendering the 3D model
        juce::gl::glClear(juce::gl::GL_COLOR_BUFFER_BIT | juce::gl::GL_DEPTH_BUFFER_BIT);
        
        shaderProgram->use();
        
        float aspectRatio = getWidth() / (float)getHeight();
        auto projectionMatrix = juce::Matrix3D<float>::fromFrustum(-aspectRatio, aspectRatio, -1.0f, 1.0f, 1.0f, 100.f);
        

        // Move the object back into the camera screen (-10.0f z) and center it (0.0f x, 0.0f y)
        auto translationMatrix = juce::Matrix3D<float>::fromTranslation({ 0.0f, 0.0f, -10.0f });

        // Compute local rotations
        auto rotationX = juce::Matrix3D<float>::rotation({ rotationAngleUD, 0.0f, 0.0f });
        auto rotationY = juce::Matrix3D<float>::rotation({ 0.0f, rotationAngleLR, 0.0f });
        auto rotationZ = juce::Matrix3D<float>::rotation({0.0f , 0.0f , rotationAngleCW});
        
        // Multiply right-to-left: Rotate first, then Translate.
        auto modelViewMatrix = translationMatrix * rotationX * rotationY * rotationZ;
        
        //Send matrixes down to GPU uniforms, using the modelViewMatrix instead of the view matrix
        juce::OpenGLShaderProgram::Uniform viewUniform(*shaderProgram, "viewMatrix");
        viewUniform.setMatrix4(modelViewMatrix.mat, 1, false);
        
        
        //Send the projection matrix
        juce::OpenGLShaderProgram::Uniform projUniform(*shaderProgram, "projectionMatrix");
        projUniform.setMatrix4(projectionMatrix.mat, 1, false);
        
        //Update the colour
        // Send the updated colour array to the fragment shader
        juce::OpenGLShaderProgram::Uniform colorUniform(*shaderProgram, "objectColour");
        colorUniform.set(cubeColor[0], cubeColor[1], cubeColor[2], cubeColor[3]);

        
        juce::gl::glBindVertexArray(vao);
        juce::gl::glDrawArrays(juce::gl::GL_TRIANGLES, 0, flatVertices.size() / 3);
        
        repaint();
        
    }
    
    void shutdown() override{
        
        juce::gl::glDeleteVertexArrays(1, &vao);
        juce::gl::glDeleteBuffers(1, &vbo);
        
        shaderProgram.reset();
        
    }
    
    
private:
    
    CustomLookAndFeel customLookAndFeel ;
    
    //Rendering 3D Models
    tinyobj::attrib_t  meshAttrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<float> flatVertices;
    
    float rotationAngleLR = 0.0f;
    float rotationAngleUD = 0.0f;
    float rotationAngleCW = 0.0f;
    
    GLuint vbo = 0;
    GLuint vao = 0;
    
    std::unique_ptr<juce::OpenGLShaderProgram> shaderProgram;

    float cubeColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    
};



//**-----------------------------------------------

class SynthEffectsPage : public juce::Component{

public:
    
    SynthEffectsPage();
    
private:
    
    CustomLookAndFeel customLookAndFeel ;
    
}




//**-----------------------------------------------

class MappingPage : public juce::Component{
    
public:
    
    MappingPage(); 
    
private:
    
    CustomLookAndFeel customLookAndFeel ;
    
}
