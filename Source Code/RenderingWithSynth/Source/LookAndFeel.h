


#pragma once
#include <JuceHeader.h>

using namespace juce ;

class CustomLookAndFeel : public juce::LookAndFeel_V4{
    
public:
    
    CustomLookAndFeel()
    {
        
        //Colour Scheme
        setColour(juce::ResizableWindow::backgroundColourId, juce::Colours::black);
        setColour(juce::Label::textColourId, juce::Colours::white);
        setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        setColour(juce::Slider::textBoxOutlineColourId , juce::Colours::black);
        setColour(juce::GroupComponent::outlineColourId, juce::Colours::white);
        setColour(juce::GroupComponent::textColourId, juce::Colours::white);

        
        setColour(juce::PopupMenu::backgroundColourId, juce::Colours::black);
        setColour(juce::PopupMenu::textColourId, juce::Colours::white);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colours::grey);
        setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::black);
        
        setColour(juce::ComboBox::backgroundColourId, juce::Colours::black);
        setColour(juce::ComboBox::outlineColourId, juce::Colours::white);
        setColour(juce::ComboBox::textColourId, juce::Colours::white);
        setColour(juce::ComboBox::arrowColourId, juce::Colours::white);
        
    }
    
    
    juce::Font getLabelFont(juce::Label&) override { return bitcount.withHeight(20.0f); }
    juce::Font getComboBoxFont(juce::ComboBox&) override { return bitcount.withHeight(14.0f); }
    juce::Font getTextButtonFont(juce::TextButton&, int) override { return bitcount.withHeight(25.0f); }
    
    void drawGroupComponentOutline (juce::Graphics& g, int width, int height, const juce::String& text, const juce::Justification& position, juce::GroupComponent& group) override{
        
        const float textH = 15.0f;
        const float indent = 3.0f;
        const float textEdgeGap = 4.0f;
        auto cs = 5.0f;

        Font f = bitcount.withHeight(textH);

        Path p;
        auto x = indent;
        auto y = f.getAscent() - 3.0f;
        auto w = jmax (0.0f, (float) width - x * 2.0f);
        auto h = jmax (0.0f, (float) height - y  - indent);
        cs = jmin (cs, w * 0.5f, h * 0.5f);
        auto cs2 = 2.0f * cs;

        auto textW = text.isEmpty() ? 0
                                    : jlimit (0.0f,
                                              jmax (0.0f, w - cs2 - textEdgeGap * 2),
                                              (float) GlyphArrangement::getStringWidthInt (f, text) + textEdgeGap * 2.0f);
        auto textX = cs + textEdgeGap;

        if (position.testFlags (Justification::horizontallyCentred))
            textX = cs + (w - cs2 - textW) * 0.5f;
        else if (position.testFlags (Justification::right))
            textX = w - cs - textW - textEdgeGap;

        p.startNewSubPath (x + textX + textW, y);
        p.lineTo (x + w - cs, y);

        p.addArc (x + w - cs2, y, cs2, cs2, 0, MathConstants<float>::halfPi);
        p.lineTo (x + w, y + h - cs);

        p.addArc (x + w - cs2, y + h - cs2, cs2, cs2, MathConstants<float>::halfPi, MathConstants<float>::pi);
        p.lineTo (x + cs, y + h);

        p.addArc (x, y + h - cs2, cs2, cs2, MathConstants<float>::pi, MathConstants<float>::pi * 1.5f);
        p.lineTo (x, y + cs);

        p.addArc (x, y, cs2, cs2, MathConstants<float>::pi * 1.5f, MathConstants<float>::twoPi);
        p.lineTo (x + textX, y);

        auto alpha = group.isEnabled() ? 1.0f : 0.5f;

        g.setColour (group.findColour (GroupComponent::outlineColourId)
                        .withMultipliedAlpha (alpha));

        g.strokePath (p, PathStrokeType (2.0f));

        g.setColour (group.findColour (GroupComponent::textColourId)
                        .withMultipliedAlpha (alpha));
        g.setFont (f);
        g.drawText (text,
                    roundToInt (x + textX), 0,
                    roundToInt (textW),
                    roundToInt (textH),
                    Justification::centred, true);
    }
    
    juce::Font getPopupMenuFont() override
    {
        return bitcount.withHeight(18.0f);
    }
    
    void drawLinearSlider (Graphics& g, int x, int y, int width, int height,
                                           float sliderPos,
                                           float minSliderPos,
                                           float maxSliderPos,
                                           const Slider::SliderStyle style, Slider& slider) override
    {
        if (slider.isBar())
        {
            g.setColour (slider.findColour (Slider::trackColourId));
            g.fillRect (slider.isHorizontal() ? Rectangle<float> (static_cast<float> (x), (float) y + 0.5f, sliderPos - (float) x, (float) height - 1.0f)
                                              : Rectangle<float> ((float) x + 0.5f, sliderPos, (float) width - 1.0f, (float) y + ((float) height - sliderPos)));

            drawLinearSliderOutline (g, x, y, width, height, style, slider);
        }
        else
        {
            auto isTwoVal   = (style == Slider::SliderStyle::TwoValueVertical   || style == Slider::SliderStyle::TwoValueHorizontal);
            auto isThreeVal = (style == Slider::SliderStyle::ThreeValueVertical || style == Slider::SliderStyle::ThreeValueHorizontal);

            auto trackWidth = jmin (6.0f, slider.isHorizontal() ? (float) height * 0.25f : (float) width * 0.25f);

            Point<float> startPoint (slider.isHorizontal() ? (float) x : (float) x + (float) width * 0.5f,
                                     slider.isHorizontal() ? (float) y + (float) height * 0.5f : (float) (height + y));

            Point<float> endPoint (slider.isHorizontal() ? (float) (width + x) : startPoint.x,
                                   slider.isHorizontal() ? startPoint.y : (float) y);
            
            auto xStep = (endPoint.x - startPoint.x)/ 10.0f;
            auto yStep = (endPoint.y - startPoint.y)/10.0f;
            int maxPos;
            
            if(slider.isHorizontal())
                maxPos = jmap(sliderPos, startPoint.x, endPoint.x, 0.0f, 10.0f);
            else
                maxPos = jmap(sliderPos, startPoint.y, endPoint.y, 0.0f, 10.0f);
            
            for(int i = 0 ; i < 10 ; i++){
                
                Point<float> dotPoint {startPoint.x + (i * xStep) , startPoint.y + (i * yStep)};
                
                if(i <= maxPos){
                    g.setColour (juce::Colours::grey.withBrightness(0.2 + (maxPos/10.0f)));
                    g.fillEllipse (Rectangle<float> (trackWidth, trackWidth).withCentre (dotPoint));
                }else {
                    g.setColour (juce::Colours::grey.withBrightness(0.15));
                    g.fillEllipse (Rectangle<float> (trackWidth, trackWidth).withCentre (dotPoint));
                }
                
            }

            //Two value and threeval value handling situations
            Path valueTrack;
            Point<float> minPoint, maxPoint, thumbPoint;

            if (isTwoVal || isThreeVal)
            {
                minPoint = { slider.isHorizontal() ? minSliderPos : (float) width * 0.5f,
                             slider.isHorizontal() ? (float) height * 0.5f : minSliderPos };

                if (isThreeVal)
                    thumbPoint = { slider.isHorizontal() ? sliderPos : (float) width * 0.5f,
                                   slider.isHorizontal() ? (float) height * 0.5f : sliderPos };

                maxPoint = { slider.isHorizontal() ? maxSliderPos : (float) width * 0.5f,
                             slider.isHorizontal() ? (float) height * 0.5f : maxSliderPos };
            }

            auto thumbWidth = getSliderThumbRadius (slider);

            valueTrack.startNewSubPath (minPoint);
            valueTrack.lineTo (isThreeVal ? thumbPoint : maxPoint);
            g.setColour (slider.findColour (Slider::trackColourId));
            g.strokePath (valueTrack, { trackWidth, PathStrokeType::curved, PathStrokeType::rounded });

            if (! isTwoVal && isThreeVal)
            {
                g.setColour (slider.findColour (Slider::thumbColourId));
                g.fillEllipse (Rectangle<float> (static_cast<float> (thumbWidth), static_cast<float> (thumbWidth)).withCentre (isThreeVal ? thumbPoint : maxPoint));
            }

            if (isTwoVal || isThreeVal)
            {
                auto sr = jmin (trackWidth, (slider.isHorizontal() ? (float) height : (float) width) * 0.4f);
                auto pointerColour = slider.findColour (Slider::thumbColourId);

                if (slider.isHorizontal())
                {
                    drawPointer (g, minSliderPos - sr,
                                 jmax (0.0f, (float) y + (float) height * 0.5f - trackWidth * 2.0f),
                                 trackWidth * 2.0f, pointerColour, 2);

                    drawPointer (g, maxSliderPos - trackWidth,
                                 jmin ((float) (y + height) - trackWidth * 2.0f, (float) y + (float) height * 0.5f),
                                 trackWidth * 2.0f, pointerColour, 4);
                }
            }

            if (slider.isBar())
                drawLinearSliderOutline (g, x, y, width, height, style, slider);
        }
    }
    
    void  drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos,
                                           const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider) override
    {

        auto bounds = Rectangle<int> (x, y, width, height).toFloat().reduced (10);

        auto radius = jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto angleStep = (rotaryEndAngle - rotaryStartAngle)/10.0f;
        
        auto lineW = jmin (8.0f, radius * 0.5f);
        auto arcRadius = radius - lineW * 0.5f;
        
        int maxPosition = (int)(sliderPos * 10);
        

        //drawing dots
        for(int i = 0 ; i < 10 ; i++){
            auto dotAngle = rotaryStartAngle + (i * angleStep);
            
            Point<float> dotPoint (bounds.getCentreX() + arcRadius * std::cos (dotAngle - MathConstants<float>::halfPi),
                                   bounds.getCentreY() + arcRadius * std::sin (dotAngle - MathConstants<float>::halfPi));
            
            if(i <= maxPosition){
                g.setColour (juce::Colours::grey.withBrightness(0.2 + sliderPos));
                g.fillEllipse (Rectangle<float> (lineW, lineW).withCentre (dotPoint));
            }else {
                g.setColour (juce::Colours::grey.withBrightness(0.15));
                g.fillEllipse (Rectangle<float> (lineW, lineW).withCentre (dotPoint));
            }
            
        }
        
    }

    
    
    
    void drawButtonBackground (Graphics& g,
                                               Button& button,
                                               const Colour& backgroundColour,
                                               bool shouldDrawButtonAsHighlighted,
                                               bool shouldDrawButtonAsDown) override
    {
        auto cornerSize = 6.0f;
        auto bounds = button.getLocalBounds().toFloat().reduced (0.5f, 0.5f);
        
        auto fillColour = shouldDrawButtonAsDown? juce::Colours::white : juce::Colours::black;
        
        auto baseColour = fillColour;
        
        if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
            baseColour = baseColour.contrasting (shouldDrawButtonAsDown ? 0.2f : 0.05f);

        g.setColour (baseColour);

        auto flatOnLeft   = button.isConnectedOnLeft();
        auto flatOnRight  = button.isConnectedOnRight();
        auto flatOnTop    = button.isConnectedOnTop();
        auto flatOnBottom = button.isConnectedOnBottom();

        if (flatOnLeft || flatOnRight || flatOnTop || flatOnBottom)
        {
            Path path;
            path.addRoundedRectangle (bounds.getX(), bounds.getY(),
                                      bounds.getWidth(), bounds.getHeight(),
                                      cornerSize, cornerSize,
                                      ! (flatOnLeft  || flatOnTop),
                                      ! (flatOnRight || flatOnTop),
                                      ! (flatOnLeft  || flatOnBottom),
                                      ! (flatOnRight || flatOnBottom));

            g.fillPath (path);

            g.setColour (button.findColour (ComboBox::outlineColourId));
            g.strokePath (path, PathStrokeType (1.0f));
        }
        else
        {
            g.fillRoundedRectangle (bounds, cornerSize);

            g.setColour (button.findColour (ComboBox::outlineColourId));
            g.drawRoundedRectangle (bounds, cornerSize, 1.0f);
        }
    }
    
    
private:
    
    juce::Font bitcount{ juce::FontOptions(juce::Typeface::createSystemTypefaceFor(BinaryData::BitcountPropSingle_ttf , BinaryData::BitcountPropSingle_ttfSize))};
};
