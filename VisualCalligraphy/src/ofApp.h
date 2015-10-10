#pragma once

#include "ofMain.h"
#include "../../common/utils.h"
#include "../../common/constants.h"
#include "MidiReceiverController.hpp"

class ofApp : public ofBaseApp
{
    
    
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    
    void receivedMidiMessage(ofxMidiMessage& e);
    
};
