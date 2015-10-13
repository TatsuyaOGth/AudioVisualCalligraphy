#include "ofMain.h"
#include "utils.h"
#include "constants.h"
#include "InputImageController.h"
#include "BlobDataController.h"
#include "VisualBlobs.h"
#include "ImageProcessing.hpp"
#include "ofxGui.h"
#include "MidiSenderController.hpp"

class mainApp : public ofBaseApp
{
#ifdef USE_CAMERA
    InputImageController<ofVideoGrabber>   *mInputImage;
#else
    InputImageController<ofVideoPlayer>    *mInputImage;
#endif
    
    BlobsDataController     *mBlobDataController;
    VisualBlobs             *mVisualBlob;
    
    enum mode { ON_SCREEN, PRE_PROCESS, BLOB_CONTROLL, } mMode;
    
    // parameter for imageprocessing
    ofxPanel gui;
    ofParameterGroup    mParamGroup;
    ofParameter<float>  mBlobThreshold;
    ofParameter<int>    mMaxNumBlobs;
    bool bDrawGui;
    
public:
    void setup();
    void update();
    void draw();
    void drawOnScreen();
    void drawPreProcess();
    void drawBlobControll();
    void drawInfomationText(float x, float y);
    
    void exit();
    
    void blobNoteEvent(BlobNoteEvent& e);
    
    void keyPressed(int key);
    void mousePressed(int x, int y, int mouse);
    
    void changedMasterThreshold(float& e);
    void addBlobAtPoint(ofxCvContourFinder& contourFinder, float x, float y, float w, float h, float offsetW);
};
