#include "ofMain.h"
#include "utils.h"
#include "Constants.h"
#include "gui.h"
#include "InputImageController.hpp"
#include "BlobsDataController.hpp"
#include "ImageProcessing.hpp"

class mainApp : public ofBaseApp
{
    InputImageController*    mIIC;
    BlobsDataController      mBDC;
    
    ofTexture mTexCrop, mTexTiltWarp, mTexThreshold;
    
    enum mode { ON_SCREEN, PRE_PROCESS, BLOB_CONTROLL, } mMode;
    
public:
    
    void setup()
    {
        ofSetFrameRate(60);
        ofSetVerticalSync(true);
        
        //----------
        // setup source image
        //----------
#ifdef USE_CAMERA
        mIIC = new InputCameraController();
#else
        mIIC = new InputVideoController();
#endif
        mIIC->setup();
        gui::setup(mIIC->getWidth(), mIIC->getHeight());
        
        gui::cropXY1.addListener(this, &mainApp::onGuiEvent);
        gui::cropXY2.addListener(this, &mainApp::onGuiEvent);
        
        //----------
        // init values
        //----------
        mMode = ON_SCREEN;
    }
    
    void update()
    {
        mIIC->update();
        if (!mIIC->isFrameNew()) return;
        //----------
        // image processing
        //----------
        ofPixels& pix = mIIC->getPixelsRef();
        
        mIIC->setFlip(gui::flipH, gui::flipV);
        
        imp::crop(pix, gui::cropXY1.get(), gui::cropXY2.get());
        mTexCrop.loadData(pix);
        
//        imp::tiltWarp(pix, gui::warpTiltV);
        imp::warpPerspective(pix, gui::warpX, gui::warpY);
        mTexTiltWarp.loadData(pix);
        
        imp::rgbToGray(pix);
        
        imp::threshold(pix, gui::blobThreshold);
        mTexThreshold.loadData(pix);
        
        imp::findContours(pix, gui::maxNumBlobs);
        mBDC.setSize(imp::cvContourFinder.getWidth(), imp::cvContourFinder.getHeight());
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
        
        gui::draw();
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
        
        // source image
        ofTexture& tex = mIIC->getTextureRef();
        float w = tex.getWidth();
        float h = tex.getHeight();
        ofSetColor(255, 255, 255);
        tex.draw(0, 0);
        
        // crop window
        ofNoFill();
        ofSetLineWidth(1);
        ofSetColor(0, 255, 0);
        ofRect(gui::cropXY1.get().x,  gui::cropXY1.get().y,
               gui::cropXY2.get().x - gui::cropXY1.get().x,
               gui::cropXY2.get().y - gui::cropXY1.get().y);
        
        // tilt warp
        ofSetColor(255, 255, 255);
        mTexTiltWarp.draw(0, h + 20);
        mTexThreshold.draw(0, h + 20 + mTexTiltWarp.getHeight());
        
        ofPopStyle();
    }
    
    void drawBlobControll()
    {
        ofBackground(30);
        
        float w = ofGetWidth();
        float h = ofGetHeight() * 0.5;
        
        // blob image
        ofSetColor(255, 255, 255);
        mTexThreshold.draw(0, 0, w, h);
        imp::cvContourFinder.draw(0, 0, w, h);
        
        // detected blobs
        mBDC.draw(0, h, w, h);
        
        ofSetColor(0, 255, 0);
        ofFill();
        ofCircle(ofGetMouseX(), ofGetMouseY(), 3);
    }
    
    void exit()
    {
        gui::save();
    }
    
    
    
    void keyPressed(int key)
    {
        switch (key)
        {
            case 'o': mIIC->togglePlay(); break;
            case 'f': ofToggleFullscreen(); break;
                
            case '1': mMode = ON_SCREEN; break;
            case '2': mMode = PRE_PROCESS; break;
            case '3': mMode = BLOB_CONTROLL; break;
                
            case '0': gui::toggleDraw(); break;
                
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
            float targetX = ofMap(x, 0, ofGetWidth(), 0, imp::cvContourFinder.getWidth(), true);
            float targetY = ofMap(y, 0, ofGetHeight()*0.5, 0, imp::cvContourFinder.getHeight(), true);
            addBlobAtPoint(targetX, targetY);
        }
    }
    
    void onGuiEvent(ofVec2f& e)
    {
        float w = gui::cropXY2.get().x - gui::cropXY1.get().x;
        float h = gui::cropXY2.get().y - gui::cropXY1.get().y;
        allocateTextures(w, h);
    }
    
    
    void allocateTextures(float w, float h)
    {
        mTexCrop.allocate(w, h, GL_RGB);
        mTexTiltWarp.allocate(w, h, GL_RGB);
        mTexThreshold.allocate(w, h, GL_RGB);
    }
    
    void addBlobAtPoint(float x, float y)
    {
        for (auto& e : imp::cvContourFinder.blobs)
        {
            if (e.boundingRect.inside(x, y))
            {
                mBDC.addBlob(e);
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
