#include "mainApp.h"

void mainApp::setup()
{
    ofSetFrameRate(60);
    ofSetVerticalSync(true);
    ofSetCircleResolution(64);
    
    //----------
    // setup source image
    //----------
#ifdef USE_CAMERA
    mInputImage = new InputCameraController(CAMERA_WIDTH, CAMERA_HEIGHT, CAMERA_DEVISE_ID);
#else
    mInputImage = new InputVideoController(SOURCE_VIDEO);
#endif
    
    //----------
    // setup blob controller
    //----------
    mBlobDataController = new BlobsDataController();
    mBlobDataController->setupMidi(MIDI_SENDER_PORT_NAME, MIDI_RECEIVER_PORT_NAME);
    ofAddListener(mBlobDataController->mBlobNoteEvent, this, &mainApp::blobNoteEvent);
    
    //----------
    // setup visual
    //----------
    mVisualBlob = new VisualBlobs(static_cast<BaseImagesInterface*>(mInputImage), VISUAL_WINDOW_WIDTH, VISUAL_WINDOW_HEIGHT);
    mVisualBlob->setBlobDataController(mBlobDataController);
    
    //----------
    // init values
    //----------
    mMode = ON_SCREEN;
    
    //----------
    // setup GUI parameter
    //----------
    mParamGroup.setName("PARAMETERS");
    mParamGroup.add(mInputImage->getParameterGroup());
    mParamGroup.add(mBlobThreshold.set("MASTER_THRESHOLD", 127, 0, 255));
    gui.setup(mParamGroup, GUI_FILENAME);
    gui.loadFromFile(GUI_FILENAME);
    gui.minimizeAll();
    bDrawGui = true;
    mBlobThreshold.addListener(this, &mainApp::changedMasterThreshold);

}

void mainApp::update()
{
    //----------
    // make marged input pixel
    //----------
    mInputImage->update();
    
    //----------
    // update blob data controller
    //----------
    mBlobDataController->update();
    
    //----------
    // update visual
    //----------
    mVisualBlob->update();
    
    //----------
    // automatic scan
    //----------
    mBlobDataController->clearBlobs();
    ofxCvContourFinder& cf = mInputImage->getCvContourFinder();
    
    int w = cf.getWidth();
    int h = cf.getHeight();
    for (auto& e : cf.blobs)
    {
        mBlobDataController->addBlob(e, w, h, 0);
    }
}

void mainApp::draw()
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
    
    if (bDrawGui)
    {
        gui.draw();
        drawInfomationText(gui.getPosition().x, gui.getPosition().y + gui.getHeight() + 20);
    }
    ofSetWindowTitle(ofToString(ofGetFrameRate()));
}

void mainApp::drawOnScreen()
{
    ofBackground(0);
    ofSetColor(255, 255, 255);
    mVisualBlob->getTextureRef().draw(0, 0, ofGetWidth(), ofGetHeight());
}

void mainApp::drawPreProcess()
{
    ofBackground(80);
    
    ofPushStyle();
    ofDisableAlphaBlending();
    ofDisableAntiAliasing();
    
    ofSetColor(255, 255, 255);
    
    float offsetY = 0;
    auto& e = mInputImage;
    float w = e->getLimitedPixRef().getWidth();
    float h = e->getLimitedPixRef().getHeight();
    float x = w * 0;
    e->getLimitedTexRef().draw(x, 0, w, h);
    e->drawCropRect(x, 0, w, h);
    offsetY += h;
    e->getCropedTextureRef().draw(x, offsetY);
    offsetY += e->getCropedPixelsRef().getHeight();
    e->getWarpedTextureRef().draw(x, offsetY);
    offsetY += e->getWarpedPixelsRef().getHeight();
    e->getBinaryTextureRef().draw(x, offsetY);
    
    ofPopStyle();
}

void mainApp::drawBlobControll()
{
    ofBackground(30);
    
    float w = ofGetWidth();
    float h = ofGetHeight() * 0.5;
    float srcW = 0;
    float srcH = 0;
    {
        float w = mInputImage->getBinaryPixelsRef().getWidth();
        float h = mInputImage->getBinaryPixelsRef().getHeight();
        if (srcW < w) srcW = w;
        if (srcH < h) srcH = h;
    }
    
    // blob image
    ofSetColor(255, 255, 255);
    mInputImage->getBinaryTextureRef().draw(0, 0, w, h);
    mInputImage->getCvContourFinder().draw(0, 0, w, h);
    
    // detected blobs
    mBlobDataController->draw(0, h, w, h);
    
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


void mainApp::drawInfomationText(float x, float y)
{
    ofPushStyle();
    stringstream s;
    s << "frame rate: " << ofGetFrameRate() << endl;
    s << "number of blobs: " << mBlobDataController->getBlobsRef().size() << endl;
    ofSetColor(0, 255, 0);
    ofDrawBitmapString(s.str(), x, y);
    ofPopStyle();
}

void mainApp::exit()
{
    gui.saveToFile(GUI_FILENAME);
}


//-----------------------------------------------------------------------------------------------
/*
 Events
 */
//-----------------------------------------------------------------------------------------------

void mainApp::blobNoteEvent(BlobNoteEvent& e)
{
    mVisualBlob->blobNoteEvent(e);
}

void mainApp::keyPressed(int key)
{
    switch (key)
    {
        case 'o':
            mInputImage->togglePlay();
            break;
            
        case 'F': ofToggleFullscreen(); break;
            
        case '1': mMode = ON_SCREEN; break;
        case '2': mMode = PRE_PROCESS; break;
        case '3': mMode = BLOB_CONTROLL; break;
            
        case ' ': bDrawGui = !bDrawGui; break;
            
            // sequencer
        case 'q': mBlobDataController->sequencerTogglePlay(0); break;
        case 'w': mBlobDataController->sequencerTogglePlay(1); break;
        case 'e': mBlobDataController->sequencerTogglePlay(2); break;
        case 'r': mBlobDataController->sequencerTogglePlay(3); break;
        case 'a': mBlobDataController->sequencerTogglePlay(4); break;
        case 's': mBlobDataController->sequencerTogglePlay(5); break;
        case 'd': mBlobDataController->sequencerTogglePlay(6); break;
        case 'f': mBlobDataController->sequencerTogglePlay(7); break;
            
            // visual
        case '/': mVisualBlob->changeScene(); break;
    }
    
    if (mMode == BLOB_CONTROLL)
    {
        switch (key)
        {
            case OF_KEY_BACKSPACE:
            case OF_KEY_DEL:
                mBlobDataController->removeBlob();
                break;
        }
    }
}

void mainApp::mousePressed(int x, int y, int mouse)
{
}

//-----------------------------------------------------------------------------------------------
/*
 Sub Routine
 */
//-----------------------------------------------------------------------------------------------

void mainApp::changedMasterThreshold(float& e)
{
    mInputImage->setThreshold(mBlobThreshold);
}

void mainApp::addBlobAtPoint(ofxCvContourFinder& contourFinder, float x, float y, float w, float h, float offsetW)
{
    for (auto& e : contourFinder.blobs)
    {
        if (e.boundingRect.inside(x, y))
        {
            // add blob
            mBlobDataController->addBlob(e, w, h, offsetW);
            // add inner blob
            for (auto& f : contourFinder.blobs)
            {
                if (f.centroid.match(e.centroid) == false)
                {
                    if (e.boundingRect.inside(f.boundingRect))
                    {
                        mBlobDataController->addBlob(f, w, h, offsetW);
                    }
                }
            }
        }
    }
}
