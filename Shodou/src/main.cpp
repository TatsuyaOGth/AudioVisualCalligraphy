#include "ofMain.h"
#include "../../common/utils.h"
#include "../../common/constants.h"
#include "InputImageController.hpp"
#include "BlobsDataController.hpp"
#include "VisualBlobs.hpp"
#include "ImageProcessing.hpp"
#include "ofxGui.h"


class mainApp : public ofBaseApp
{
#ifdef USE_CAMERA
    vector<InputImageController<ofVideoGrabber>*>   mInputImage;
#else
    vector<InputImageController<ofVideoPlayer>*>    mInputImage;
#endif
    
    vector<BaseImagesInterface*> mBaseImages; // for visual
        
    BlobsDataController     mBDC;
    VisualBlobs             *mVisualBlob;
    
    enum mode { ON_SCREEN, PRE_PROCESS, BLOB_CONTROLL, } mMode;
    
    // parameter for imageprocessing
    ofxPanel gui;
    ofParameterGroup    mParamGroup;
    ofParameter<float>  mBlobThreshold;
    ofParameter<int>    mMaxNumBlobs;
    bool bDrawGui;
    
public:
    
    //-----------------------------------------------------------------------------------------------
    /*
        Main processing
     */
    //-----------------------------------------------------------------------------------------------
    
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
        ofAddListener(mBDC.mBlobNoteEvent, this, &mainApp::blobNoteEvent);
        
        //----------
        // setup visual
        //----------
        for (auto& e : mInputImage)
        {
            mBaseImages.push_back(e);
        }
        mVisualBlob = new VisualBlobs(mBaseImages, VISUAL_WINDOW_WIDTH, VISUAL_WINDOR_HEIGHT);
        
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
        gui.minimizeAll();
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
        
        //----------
        // update visual
        //----------
        mVisualBlob->update();
    }
    
    void draw()
    {
        //----------
        // render visual
        //----------
        mVisualBlob->rendering();
        
        
        //----------
        // draw moniter
        //----------
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
        mVisualBlob->getTextureRef().draw(0, 0, ofGetWidth(), ofGetHeight());
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
    
    
    //-----------------------------------------------------------------------------------------------
    /*
        Events
     */
    //-----------------------------------------------------------------------------------------------

    void blobNoteEvent(BlobNoteEvent& e)
    {
        // to visual
        if (e.channel == 3)
        {
            mVisualBlob->getAnimationManager().createInstance<TwinkBlob>(e.blobPtr)->play(1);
        }
    }
    
    void keyPressed(int key)
    {
        switch (key)
        {
            case 'o':
                for (auto& e : mInputImage) e->togglePlay();
                break;
                
            case 'F': ofToggleFullscreen(); break;
                
            case '1': mMode = ON_SCREEN; break;
            case '2': mMode = PRE_PROCESS; break;
            case '3': mMode = BLOB_CONTROLL; break;
                
            case '0': bDrawGui = !bDrawGui; break;
               
            // sequencer
            case 'q': mBDC.sequencerTogglePlay(0); break;
            case 'w': mBDC.sequencerTogglePlay(1); break;
            case 'e': mBDC.sequencerTogglePlay(2); break;
            case 'r': mBDC.sequencerTogglePlay(3); break;
            case 'a': mBDC.sequencerTogglePlay(4); break;
            case 's': mBDC.sequencerTogglePlay(5); break;
            case 'd': mBDC.sequencerTogglePlay(6); break;
            case 'f': mBDC.sequencerTogglePlay(7); break;
                
            // visual
            case '/': mVisualBlob->changeScene(); break;
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
    
    //-----------------------------------------------------------------------------------------------
    /*
        Sub Routine
     */
    //-----------------------------------------------------------------------------------------------
    
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




int main( )
{
	ofSetupOpenGL(1280,768,OF_WINDOW);
	ofRunApp(new mainApp());
}
