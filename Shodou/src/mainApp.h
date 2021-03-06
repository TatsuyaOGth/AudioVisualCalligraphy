#include "ofMain.h"
#include "../../common/utils.h"
#include "../../common/constants.h"
#include "InputImageController.h"
#include "BlobDataController.h"
#include "VisualBlobs.h"
#include "ImageProcessing.hpp"
#include "ofxGui.h"
#include "ofxSyphon.h"

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
    
    ofxSyphonServer mSyponeServer;
    
public:
    void setup();
    void update();
    void draw();
    void drawOnScreen();
    void drawPreProcess();
    void drawBlobControll();
    
    void exit();
    
    void blobNoteEvent(BlobNoteEvent& e);
    
    void keyPressed(int key);
    void mousePressed(int x, int y, int mouse);
    
    void changedMasterThreshold(float& e);
    void addBlobAtPoint(ofxCvContourFinder& contourFinder, float x, float y, float w, float h, float offsetW);
};
