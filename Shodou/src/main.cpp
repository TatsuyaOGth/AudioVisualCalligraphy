#include "ofMain.h"
#include "utils.h"
#include "InputImageController.hpp"
#include "ImageProcessing.hpp"

class mainApp : public ofBaseApp
{
    InputVideoController mIVC;
    ofTexture mTex;
    
public:
    
    void setup()
    {
        mIVC.load("movie/test.mov");
        mIVC.setFlip(true, true);
    }
    
    void update()
    {
        mIVC.update();
        
        ofPixels& pix = mIVC.getPixelsRef();
        
        
        imp::crop(pix, 44, 147, 628, 285);
        imp::tiltWarp(pix, -90);
        imp::rgbToGray(pix);
        imp::threshold(pix, ofMap(ofGetMouseX(), 0, ofGetWidth(), 0, 255, true));
        
        
        mTex.loadData(pix);
        
        ofSetWindowTitle(ofToString(ofGetFrameRate()));
    }
    
    void draw()
    {
//        mIVC.getTextureRef().draw(0, 0);
        mTex.draw(0, 0);
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
