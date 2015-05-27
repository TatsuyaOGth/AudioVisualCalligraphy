#pragma once

#include "ofxGui.h"
#include "Constants.h"

namespace gui
{
    static const string GUI_FILENAME = "settings.xml";
    
    static ofxPanel panel;
    static ofParameterGroup group1;
    
    static ofParameter<bool>        flipH;
    static ofParameter<bool>        flipV;
    static ofParameter<ofVec2f>     cropXY1;
    static ofParameter<ofVec2f>     cropXY2;
    static ofParameter<float>       warpTiltV;
    static ofParameter<float>       blobThreshold;
    
    static void setup()
    {
        group1.setName("PRE-PROCESS");
        group1.add(flipH.set("FLIP_HORIZON", true));
        group1.add(flipV.set("FLIP_VERTICAL", true));
        group1.add(cropXY1.set("CROP_XY_1", ofVec2f(0, 0), ofVec2f(0, 0), ofVec2f(SOURCE_IMAGE_WIDTH, SOURCE_IMAGE_HEIGHT)));
        group1.add(cropXY2.set("CROP_XY_2", ofVec2f(SOURCE_IMAGE_WIDTH, SOURCE_IMAGE_HEIGHT), ofVec2f(0, 0), ofVec2f(SOURCE_IMAGE_WIDTH, SOURCE_IMAGE_HEIGHT)));
        group1.add(warpTiltV.set("TILT_WARP", 0, -180, 180));
        group1.add(blobThreshold.set("THRESHOLD", 127, 0, 255));
        
        panel.setup("PARAMETERS", GUI_FILENAME);
        panel.add(group1);
        panel.loadFromFile(GUI_FILENAME);
    }
    
    static void draw()
    {
        panel.draw();
    }
    
    static void save()
    {
        panel.saveToFile(GUI_FILENAME);
    }
}
