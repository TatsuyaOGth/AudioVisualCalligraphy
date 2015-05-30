#pragma once

#include "ofxGui.h"
#include "InputImageController.hpp"

namespace gui
{
    static const string GUI_FILENAME = "settings.xml";
    
    static ofxPanel panel;
    static ofParameterGroup mainParamGroup;
    static ofParameterGroup group1;
    
    static ofParameter<bool>        flipH;
    static ofParameter<bool>        flipV;
    static ofParameter<ofVec2f>     cropXY1;
    static ofParameter<ofVec2f>     cropXY2;
    static ofParameter<float>       warpTiltV;
    static ofParameter<float>       blobThreshold;
    static ofParameter<int>         maxNumBlobs;
    
    static bool bDraw;
    
    static void setup(int srcImgW, int srcImgH)
    {
        mainParamGroup.setName("PARAMETERS");
        
        group1.setName("PRE-PROCESS");
        group1.add(flipH.set("FLIP_HORIZON", true));
        group1.add(flipV.set("FLIP_VERTICAL", true));
        group1.add(cropXY1.set("CROP_XY_1", ofVec2f(0, 0), ofVec2f(0, 0), ofVec2f(srcImgW, srcImgH)));
        group1.add(cropXY2.set("CROP_XY_2", ofVec2f(srcImgW, srcImgH), ofVec2f(0, 0), ofVec2f(srcImgW, srcImgH)));
        group1.add(warpTiltV.set("TILT_WARP", 0, -180, 180));
        group1.add(blobThreshold.set("THRESHOLD", 127, 0, 255));
        group1.add(maxNumBlobs.set("MAX_NUM_BLOBS", 50, 1, 255));
        
        mainParamGroup.add(group1);
        
        panel.setup(mainParamGroup, GUI_FILENAME);
        panel.loadFromFile(GUI_FILENAME);
        
        bDraw = true;
    }
    
    static void draw()
    {
        if (bDraw) panel.draw();
    }
    
    static void save()
    {
        panel.saveToFile(GUI_FILENAME);
    }
    
    static bool toggleDraw()
    {
        bDraw = !bDraw;
        return bDraw;
    }
}
