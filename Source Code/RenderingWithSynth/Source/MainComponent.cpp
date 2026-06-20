#include "MainComponent.h"
#include <cmath>

//==============================================================================
MainComponent::MainComponent()
{
    setSize (600, 400);
    setOpaque(true);
    
    for(auto& comp : draggableComponents){
        
        addAndMakeVisible(comp);
        
    }

    
    //Custom look and feel
    setLookAndFeel(&customLookAndFeel); 
    
    openGLContext.setOpenGLVersionRequired(juce::OpenGLContext::openGL3_2);
    
    openGLContext.attachTo(*this);
    
    setWantsKeyboardFocus(true);
    
    
    //Audio connections
    
    audioDeviceManager.initialiseWithDefaultDevices(0, 2); //Open Audio Hardware
    
    audioSourcePlayer.setSource(&synthAudioSource); //set the Audio Source in this case the synthesiser
    
    audioDeviceManager.addAudioCallback(&audioSourcePlayer); // pass the audioSource to the Audiodevice manager to be outputted through the hardware
    
    //Midi connections
    
    auto midiDevices = juce::MidiInput::getAvailableDevices();
    
    //loop through the list of devices and set them all to be enables , this means that all midi inputs are passed through
    for (auto& device : midiDevices){
        
        audioDeviceManager.setMidiInputDeviceEnabled(device.identifier, true);
        audioDeviceManager.addMidiInputDeviceCallback(device.identifier, this);
        
    }
}

void MainComponent::visibilityChanged()
{
    if (isShowing())
    {
        grabKeyboardFocus();
    }
}


void MainComponent::initialise()
{
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


void MainComponent::render()
{
    
    //Reading midi messages to update rotational values each time the render function is called , could this be made more efficient such as checking for changes first and then updating the values? 
    {
        juce::ScopedLock lock(midiMutex);
        
        for(const auto metadata : visMidiBuffer){
            
            auto message = metadata.getMessage();
            
            if(message.isController()){
                
                int ccNumber = message.getControllerNumber();
                int ccValue = message.getControllerValue(); // A value between 0 and 127 , we need to map this to be in radians between -pi and pi
                
                float radRotVal = ((ccValue / 127.0f) * juce::MathConstants<float>::twoPi) - juce::MathConstants<float>::pi;
                
                
                switch (ccNumber) {
                    case CCValUD:
                        rotationAngleUD = radRotVal;
                        break;
                        
                    case CCValLR:
                        rotationAngleLR = radRotVal;
                        break;
                        
                    case CCValCW:
                        rotationAngleCW = radRotVal;
                        break;
                        
                    default:
                        break;
                }
                
            }
            
        }
        
        visMidiBuffer.clear();
    }
    
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

void MainComponent::handleIncomingMidiMessage(juce::MidiInput* source , const juce::MidiMessage& message){
    
    //Midi messages are sent through straight to the synth as well as sending through to the visual midi buffer where it is used to update the 3D model of the device
    
    //Pass the midiMessages through to the synthesiser to be used
    synthAudioSource.addMidiMessage(message);
    
    //Pass midi messages through to main component to be used to control visualisation
    addVisMidiMessage(message); 
}


bool MainComponent::keyPressed (const juce::KeyPress& key){
    
    bool val = false;
    
    switch (key.getTextCharacter()) {
        case 'a':
            rotationAngleLR += 0.1;
            val = true;
            break;
        case 's':
            rotationAngleUD -=0.1;
            val = true;
            break;
        case 'd':
            rotationAngleLR -= 0.1;
            val = true;
            break;
        case 'w':
            rotationAngleUD += 0.1;
            val = true;
            break;
            
        default:
            break;
    }
    
    return val;
}

void MainComponent::mouseDown (const juce::MouseEvent& event)
{
    // Generate random values between 0.0f and 1.0f for Red, Green, and Blue
    auto& random = juce::Random::getSystemRandom();
    
    cubeColor[0] = random.nextFloat(); // Red
    cubeColor[1] = random.nextFloat(); // Green
    cubeColor[2] = random.nextFloat(); // Blue
    cubeColor[3] = 1.0f;               // Alpha (fully opaque)
    
    
    //On every cick update the arrangement of the effects
    
    synthAudioSource.arrangeEffects(); 
    
}


void MainComponent::shutdown(){
    
    juce::gl::glDeleteVertexArrays(1, &vao);
    juce::gl::glDeleteBuffers(1, &vbo);
    
    shaderProgram.reset();
}

MainComponent::~MainComponent()
{
    //Release all resources
    
    openGLContext.detach();
    audioDeviceManager.removeAudioCallback(&audioSourcePlayer);
    audioSourcePlayer.setSource(nullptr);
    
    for (auto& device : juce::MidiInput::getAvailableDevices())
    {
        audioDeviceManager.removeMidiInputDeviceCallback(device.identifier, this);
    }
    
    setLookAndFeel(nullptr);
    
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{


    g.setColour (juce::Colours::white);


    g.drawText ("Use W, A, S, D to rotate the cube",
                10, 10, 400, 30,
                juce::Justification::left);

    
    //Want to keep these values inbetween 0 and 1 ,
    float rotValLR = std::fmod(rotationAngleLR , 1.0f);
    float rotValUD = std::fmod(rotationAngleUD, 1.0f);
    
    if (rotValLR < 0.0f) rotValLR += 1.0f;
    if (rotValUD < 0.0f) rotValUD += 1.0f;
    
    juce::String debugInfo = "Rotation LR: " + juce::String ((rotValLR), 2)
                           + " | UD: " + juce::String (rotValUD, 2);
    
    g.drawText (debugInfo,
                10, getHeight() - 40, getWidth() - 20, 30,
                juce::Justification::left);
}


void MainComponent::resized()
{
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    
    for(auto& comp : draggableComponents){
        
        comp.setBounds(100, 100, 100, 100);
    }
    
    
}
