#include "ofMain.h"
#include "utils.h"
#include "InputImageController.hpp"
#include "BlobsDataController.hpp"
#include "ImageProcessing.hpp"
#include "ofxGui.h"

////////////////////////////////////////////////////////////////////////////////

// GENERAL
//------------------------------------------------------------------------------
//#define USE_CAMERA

static const int NUM_INPUT = 2;

// CAMERA
//------------------------------------------------------------------------------
static const int NUM_CAMERA = NUM_INPUT;
static const int CAMERA_DEVISE_ID[] = {0, 1};
static const int CAMERA_WIDTH     = 1280;
static const int CAMERA_HEIGHT    = 720;



// VIDEO
//------------------------------------------------------------------------------
static const string SOURCE_VIDEO[] = {"movie/test.mov", "movie/test_mini.mov"};
static const int START_POSITION = 0.5;


// MIDI
//------------------------------------------------------------------------------
static const string MIDI_SENDER_PORT_NAME   = "IAC Driver buss 1";
static const string MIDI_RECEIVER_PORT_NAME = "IAC Driver buss 2";


// GUU
//------------------------------------------------------------------------------
static const string GUI_FILENAME = "settings.xml";

////////////////////////////////////////////////////////////////////////////////



class mainApp : public ofBaseApp
{
#ifdef USE_CAMERA
    vector<InputImageController<ofVideoGrabber>*>   mInputImage;
#else
    vector<InputImageController<ofVideoPlayer>*>    mInputImage;
#endif
    
    BlobsDataController      mBDC;
    
    ofPixels    mPrePix, mBlobPix;
    ofTexture   mPreTex, mBlobTex;
    
    enum mode { ON_SCREEN, PRE_PROCESS, BLOB_CONTROLL, } mMode;
    
    // parameter for imageprocessing
    ofxPanel gui;
    ofParameterGroup mParamGroup;
    ofParameter<float>  mBlobThreshold;
    ofParameter<int>    mMaxNumBlobs;
    bool bDrawGui;
    
public:
    
    void setup()
    {
        ofSetFrameRate(60);
        ofSetVerticalSync(true);
        
        //----------
        // setup source image
        //----------
#ifdef USE_CAMERA
        for (int i = 0; i < NUM_INPUT; ++i)
        {
            mInputImage.push_back(new InputCameraController(CAMERA_WIDTH, CAMERA_HEIGHT, CAMERA_DEVISE_ID[i]));
        }
#else
        for (int i = 0; i < NUM_INPUT; ++i)
        {
            mInputImage.push_back(new InputVideoController(SOURCE_VIDEO[i]));
        }
#endif
        
        //----------
        // setup blob controller
        //----------
        mBDC.setupMidi(MIDI_SENDER_PORT_NAME, MIDI_RECEIVER_PORT_NAME);
        mBDC.setSize(CAMERA_WIDTH * mInputImage.size(), CAMERA_HEIGHT);
        
        //----------
        // init values
        //----------
        mMode = ON_SCREEN;
        
        //----------
        // setup GUI parameter
        //----------
        mParamGroup.setName("PARAMETERS");
        for (const auto& e : mInputImage)
        {
            mParamGroup.add(e->getParameterGroup());
        }
        mParamGroup.add(mBlobThreshold.set("MASTER_THRESHOLD", 127, 0, 255));
        gui.setup(mParamGroup, GUI_FILENAME);
        gui.loadFromFile(GUI_FILENAME);
        bDrawGui = true;
        
        mBlobThreshold.addListener(this, &mainApp::changedMasterThreshold);
    }
    
    void update()
    {
        //----------
        // make marged input pixel
        //----------
        for (auto& e : mInputImage)
        {
            e->update();
        }
        
        //----------
        // update blob data controller
        //----------
        mBDC.update();
    }
    
    
    
    void draw()
    {
        switch (mMode)
        {
            case ON_SCREEN:     drawOnScreen(); break;
            case PRE_PROCESS:   drawPreProcess(); break;
            case BLOB_CONTROLL: drawBlobControll(); break;
        }
        
        if (bDrawGui) gui.draw();
        ofSetWindowTitle(ofToString(ofGetFrameRate()));
    }
    
    void drawOnScreen()
    {
        ofBackground(0);
        // TODO: make visual
    }
    
    void drawPreProcess()
    {
        ofBackground(80);

        ofPushStyle();
        ofDisableAlphaBlending();
        ofDisableAntiAliasing();
        
        ofSetColor(255, 255, 255);
        for (int i = 0; i < mInputImage.size(); ++i)
        {
            float offsetY = 0;
            auto& e = mInputImage[i];
            float w = e->getResizedPixelsRef().getWidth();
            float h = e->getResizedPixelsRef().getHeight();
            float x = w * i;
            e->getResizedTextureRef().draw(x, 0, w, h);
            e->drawCropRect(x, 0, w, h);
            offsetY += h;
            e->getCropedTextureRef().draw(x, offsetY);
            offsetY += e->getCropedPixelsRef().getHeight();
            e->getWarpedTextureRef().draw(x, offsetY);
            offsetY += e->getWarpedPixelsRef().getHeight();
            e->getBinaryTextureRef().draw(x, offsetY);
        }
        ofPopStyle();
    }
    
    void drawBlobControll()
    {
        ofBackground(30);
        
        float w = ofGetWidth();
        float h = ofGetHeight() * 0.5;
        float srcW = 0;
        float srcH = 0;
        for (const auto& e : mInputImage)
        {
            float w = e->getBinaryPixelsRef().getWidth();
            float h = e->getBinaryPixelsRef().getHeight();
            if (srcW < w) srcW = w;
            if (srcH < h) srcH = h;
        }
        
        for (int i = 0; i < mInputImage.size(); ++i)
        {            
            // blob image
            ofSetColor(255, 255, 255);
            mInputImage[i]->getBinaryTextureRef().draw(w / 2 * i, 0, w / 2, h);
            mInputImage[i]->getCvContourFinder().draw(w / 2 * i, 0, w / 2, h);
        }
        
        // detected blobs
        mBDC.draw(0, h, w, h);
        
        // draw frame
        ofNoFill();
        ofSetColor(127);
        ofRect(0, 0, w, h);
        ofRect(0, h, w, h);
        
        // draw mouse pointer
        ofSetColor(0, 255, 0);
        ofFill();
        ofCircle(ofGetMouseX(), ofGetMouseY(), 3);
    }
    
    void exit()
    {
        gui.saveToFile(GUI_FILENAME);
    }
    
    
    
    void keyPressed(int key)
    {
        switch (key)
        {
            case 'o':
                for (auto& e : mInputImage) e->togglePlay();
                break;
                
            case 'f': ofToggleFullscreen(); break;
                
            case '1': mMode = ON_SCREEN; break;
            case '2': mMode = PRE_PROCESS; break;
            case '3': mMode = BLOB_CONTROLL; break;
                
            case '0': bDrawGui = !bDrawGui; break;
               
            // sequencer
            case 'z': mBDC.sequencerTogglePlay(0); break;
            case 'x': mBDC.sequencerTogglePlay(1); break;
            case 'c': mBDC.sequencerTogglePlay(2); break;
            case 'v': mBDC.sequencerTogglePlay(3); break;
                
            // test midi
            case 'm': mBDC.makeNoteRandom(1); break;
        }
        
        if (mMode == BLOB_CONTROLL)
        {
            switch (key)
            {
                case OF_KEY_BACKSPACE:
                case OF_KEY_DEL:
                    mBDC.removeBlob();
                    break;
            }
        }
    }
    
    void mousePressed(int x, int y, int mouse)
    {
        if (mMode == BLOB_CONTROLL)
        {
            int sizeIIC = mInputImage.size();
            int targetIIC = x / (ofGetWidth() / sizeIIC);
            ofClamp(targetIIC, 0, sizeIIC);
            float cfW = 0;
            float cfH = 0;
            for (const auto& e : mInputImage)
            {
                cfW += e->getCvContourFinder().getWidth();
                if (cfH < e->getCvContourFinder().getHeight()) cfH = e->getCvContourFinder().getHeight();
            }
            ofxCvContourFinder& cf = mInputImage[targetIIC]->getCvContourFinder();
            int pointX = ofGetWidth() / sizeIIC * targetIIC;
            int pointY = 0;
            int pointW = pointX + ofGetWidth() / sizeIIC;
            int pointH = ofGetHeight() / 2;
            float targetX = ofMap(x, pointX, pointW, 0, cf.getWidth(), true);
            float targetY = ofMap(y, pointY, pointH, 0, cf.getHeight(), true);
            
            float offsetW = 0;
            if (targetIIC > 0)
            {
                for (int i = 0; i < mInputImage.size() - 1; ++i)
                {
                    offsetW += mInputImage[i]->getCvContourFinder().getWidth() * targetIIC;
                }
            }
            addBlobAtPoint(cf, targetX, targetY, cfW, cfH, offsetW);
        }
    }
    
    void changedMasterThreshold(float& e)
    {
        for (auto& e : mInputImage)
        {
            e->setThreshold(mBlobThreshold);
        }
    }
    
    void addBlobAtPoint(ofxCvContourFinder& contourFinder, float x, float y, float w, float h, float offsetW)
    {
        for (auto& e : contourFinder.blobs)
        {
            if (e.boundingRect.inside(x, y))
            {
                // add blob
                mBDC.addBlob(e, w, h, offsetW);
                // add inner blob
                for (auto& f : contourFinder.blobs)
                {
                    if (f.centroid.match(e.centroid) == false)
                    {
                        if (e.boundingRect.inside(f.boundingRect))
                        {
                            mBDC.addBlob(f, w, h, offsetW);
                        }
                    }
                }
            }
        }
    }
};




//========================================================================
int main( )
{
	ofSetupOpenGL(1280,768,OF_WINDOW);
	ofRunApp(new mainApp());
}
