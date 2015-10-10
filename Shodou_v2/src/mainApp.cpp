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
    mVisualBlob = new VisualBlobs(mBaseImages, VISUAL_WINDOW_WIDTH, VISUAL_WINDOW_HEIGHT);
    mVisualBlob->setBlobDataController(&mBDC);
    
    //----------
    // init values
    //----------
    mMode = ON_SCREEN;
    mScanMode = AUTO;
    
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

void mainApp::update()
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
    
    //----------
    // automatic scan
    //----------
    if (mScanMode == AUTO)
    {
        // TODO: support multic video/camera
        mBDC.clearBlobs();
        ofxCvContourFinder& cf = mInputImage[0]->getCvContourFinder();
        
        int w = cf.getWidth();
        int h = cf.getHeight();
        for (auto& e : cf.blobs)
        {
            mBDC.addBlob(e, w, h, 0);
        }

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
//    if (bDrawGui) VisualBlobs::smFlowTools->drawGui();
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

void mainApp::drawBlobControll()
{
    ofBackground(30);
    
    float w = ofGetWidth();
    float h = ofGetHeight() * 0.5;
    float srcW = 0;
    float srcH = 0;
    int   nSrc = mInputImage.size();
    for (const auto& e : mInputImage)
    {
        float w = e->getBinaryPixelsRef().getWidth();
        float h = e->getBinaryPixelsRef().getHeight();
        if (srcW < w) srcW = w;
        if (srcH < h) srcH = h;
    }
    
    for (int i = 0; i < nSrc; ++i)
    {
        // blob image
        ofSetColor(255, 255, 255);
        mInputImage[i]->getBinaryTextureRef().draw(w / nSrc * i, 0, w / nSrc, h);
        mInputImage[i]->getCvContourFinder().draw(w / nSrc * i, 0, w / nSrc, h);
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


void mainApp::drawInfomationText(float x, float y)
{
    ofPushStyle();
    stringstream s;
    s << "frame rate: " << ofGetFrameRate() << endl;
    s << "number of blobs: " << mBDC.getBlobsRef().size() << endl;
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

void mainApp::mousePressed(int x, int y, int mouse)
{
    if (mMode == BLOB_CONTROLL && mScanMode == MANUAL)
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
    
//    VisualBlobs::smFlowTools->emit(x, y);
}

//-----------------------------------------------------------------------------------------------
/*
 Sub Routine
 */
//-----------------------------------------------------------------------------------------------

void mainApp::changedMasterThreshold(float& e)
{
    for (auto& e : mInputImage)
    {
        e->setThreshold(mBlobThreshold);
    }
}

void mainApp::addBlobAtPoint(ofxCvContourFinder& contourFinder, float x, float y, float w, float h, float offsetW)
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
