#include "InputImageController.h"

template<typename ofxCvImageType>
void setCvImageFromPixels(ofxCvImageType& cvImage, ofPixels& pix)
{
    if (cvImage.getWidth() != pix.getWidth() || cvImage.getHeight() != pix.getHeight())
    {
        cvImage.allocate(pix.getWidth(), pix.getHeight());
    }
    cvImage.setFromPixels(pix);
}

template<typename ofxCvImageType>
void warpPerspective(ofxCvImageType& cvImage, const double vecX, const double vecY)
{
    const double w = cvImage.getWidth();
    const double h = cvImage.getHeight();
    ofPoint dst_pt[4];
    dst_pt[0] = ofPoint( (vecX > 0 ? 0.0  : -vecX     ), (vecY > 0 ? 0.0  : -vecY   ) );
    dst_pt[1] = ofPoint( (vecX > 0 ? w : w+vecX       ), (vecY > 0 ? vecY    : 0.0  ) );
    dst_pt[2] = ofPoint( (vecX > 0 ? w-vecX     : w  ), (vecY > 0 ? h-vecY      : h) );
    dst_pt[3] = ofPoint( (vecX > 0 ? vecX    : 0.0    ), (vecY > 0 ? h : h+vecY     ) );
    cvImage.warpPerspective(dst_pt[0], dst_pt[1], dst_pt[2], dst_pt[3]);
}



InputVideoController::InputVideoController(const string& videoPath)
: mVideoPath(videoPath)
{
    setup();
}

void InputVideoController::setup()
{
    if (loadMovie(mVideoPath) == false)
    {
        LOG_ERROR << "failed load movie: " << mVideoPath;
        return;
    }
    basePlayer::setVolume(0);
    allocateAllPixelsAndTextures(basePlayer::getWidth(), basePlayer::getHeight());
}

void InputVideoController::update()
{
    basePlayer::update();
    if (isFrameNew())
    {
        preProcess(basePlayer::getPixelsRef());
    }
}

void InputVideoController::play()
{
    basePlayer::play();
}

void InputVideoController::stop()
{
    basePlayer::stop();
}

void InputVideoController::togglePlay()
{
    basePlayer::isPlaying() ? stop() : play();
}



InputCameraController::InputCameraController(int w, int h, int deviceId)
: mWidth(w)
, mHeight(h)
, mDeviceId(deviceId)
{
    setup();
}

void InputCameraController::setup()
{
    vector<ofVideoDevice> devices = listDevices();
    
    cout << "~~~~~~~~~~ CAMERA DEVICE ~~~~~~~~~~" << endl;
    for(int i = 0; i < devices.size(); i++){
        cout << devices[i].id << ": " << devices[i].deviceName;
        if( devices[i].bAvailable ){
            cout << endl;
        }else{
            cout << " - unavailable " << endl;
        }
    }
    cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    baseGrabber::setDeviceID(mDeviceId);
    baseGrabber::initGrabber(mWidth, mHeight, true);
    allocateAllPixelsAndTextures(baseGrabber::getWidth(), baseGrabber::getHeight());
}

void InputCameraController::update()
{
    baseGrabber::update();
    if (isFrameNew())
    {
        preProcess(baseGrabber::getPixelsRef());
    }
}

void InputCameraController::play()
{
    if (isInitialized() == false)
    {
        setup();
    }
}

void InputCameraController::stop()
{
    baseGrabber::close();
}

void InputCameraController::togglePlay()
{
    isInitialized() ? stop() : play();
}
