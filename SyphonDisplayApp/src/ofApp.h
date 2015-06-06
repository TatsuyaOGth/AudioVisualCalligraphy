#pragma once

#include "ofMain.h"
#include "../../common/constants.h"
#include "ofxSyphon.h"
#include "ofxSharedMemory.h"

class ofApp : public ofBaseApp
{
    ofxSyphonClient mClient;
    
    bool mfTestView;
    int mTestViewMode;
    ofImage mColorBar;
    
public:
    void setup()
    {
        ofSetFrameRate(60);
        ofSetVerticalSync(true);
        ofSetCircleResolution(180);
        ofDisableAntiAliasing();
        
        mClient.setup();
        mClient.set(MAIN_DISP_SERVER_NAME, "");
        
        mfTestView = false;
        mTestViewMode = 0;
        mColorBar.loadImage("cb.png");
    }
    
    void update()
    {
    }
    
    void draw()
    {
        ofBackground(0, 0, 0);
        ofSetColor(255, 255, 255);
        
        if (mfTestView)
        {
            switch (mTestViewMode)
            {
                case 0: testView(); break;
                case 1: mColorBar.draw(0, 0, ofGetWidth(), ofGetHeight()); break;
            }
        }
        else {
//            mClient.draw(0, 0);
            mClient.draw(0, 0, ofGetWidth(), ofGetHeight());
        }
    }
    
    void keyPressed(int key)
    {
        if (key == 'f') ofToggleFullscreen();
        if (key == 't') mfTestView = !mfTestView;
        
        if (key == '0') mTestViewMode = 0;
        if (key == '1') mTestViewMode = 1;
    }
    
    
    
    void testView()
    {
        double v = ofMap(sin(ofGetElapsedTimef()), -0.95, 0.95, 0, 255, true);
        ofBackground( v );
        ofSetColor( ofGetStyle().bgColor.getInverted() , 90);
        ofSetLineWidth(1);
        
        for (int i = 0; i < ofGetWidth(); i += 20) ofLine(i, 0, i, ofGetHeight());
        for (int i = 0; i < ofGetHeight(); i += 20) ofLine(0, i, ofGetWidth(), i);
        
        ofSetColor( ofGetStyle().bgColor.getInverted() ,255);
        ofSetLineWidth(2);
        
        ofLine(0, 0, ofGetWidth(), ofGetHeight());
        ofLine(ofGetWidth(), 0, 0, ofGetHeight());
        
        ofNoFill();
        ofCircle(ofGetWidth()*0.5, ofGetHeight()*0.5, min(ofGetWidth()*0.5, ofGetHeight()*0.5));
        
        
        stringstream s;
        s << "FPS: " << ofGetFrameRate() << endl;
        s << "SIZE: " << ofGetWidth() << " : " << ofGetHeight() << endl;
        s << "BG: " << v << endl;
        ofDrawBitmapString(s.str(), 20, 20);
        
        ofSetLineWidth(1);
        ofSetColor(0, 255, 0);
        ofLine(0, mouseY, ofGetWidth(), mouseY);
        ofLine(mouseX, 0, mouseX, ofGetHeight());
        ofDrawBitmapString(s.str(), mouseX + 5, mouseY + 20);
    }
};
