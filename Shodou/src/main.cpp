#include "ofMain.h"
#include "utils.h"
#include "gui.h"
#include "InputImageController.hpp"
#include "ImageProcessing.hpp"

class mainApp : public ofBaseApp
{
    InputVideoController mIVC;
    ofTexture mTex;
    
public:
    
    void setup()
    {
        gui::setup();
        mIVC.load("movie/test.mov");
    }
    
    void update()
    {
        mIVC.update();
        
        ofPixels& pix = mIVC.getPixelsRef();
        
        mIVC.setFlip(gui::flipH, gui::flipV);
        imp::crop(pix, gui::cropXY1.get(), gui::cropXY2.get());
        imp::tiltWarp(pix, gui::warpTiltV);
        imp::rgbToGray(pix);
        imp::threshold(pix, gui::blobThreshold);
        
        
        mTex.loadData(pix);
        
        ofSetWindowTitle(ofToString(ofGetFrameRate()));
    }
    
    void draw()
    {
        ofBackground(0, 0, 0);
//        mIVC.getTextureRef().draw(0, 0);
        mTex.draw(0, 0);
        
        gui::draw();
    }
    
    void exit()
    {
        gui::save();
    }
    
    
    
    void keyPressed(int key)
    {
        switch (key)
        {
            case ' ': mIVC.togglePlay(); break;
            
            case '[': mIVC.setFlip(true, true); break;
            case ']': mIVC.setFlip(false, false); break;
        }
    }
    
    void mousePressed(int x, int y, int mouse)
    {
        cout << x << " " << y << endl;
    }
};




//========================================================================
int main( )
{
	ofSetupOpenGL(1024,768,OF_WINDOW);
	ofRunApp(new mainApp());
}
