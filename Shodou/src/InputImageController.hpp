#pragma once

#include "ofMain.h"

template <typename sourceType>
class InputImageController
{
    sourceType mSource;
    ofPixels mPix;
    ofTexture mTex;
    bool bFlipHorizon, bFlipVertical;
    
public:
    InputImageController() : bFlipHorizon(false), bFlipVertical(false) {}
    
    bool load(const string& path)
    {
        if (mSource.loadMovie(path))
        {
            mSource.setVolume(0);
            return true;
        }
        else return false;
    }
    
    bool setup(int w, int h, int deviceID)
    {
        mSource.setDeviceID(deviceID);
        return mSource.initGrabber(w, h);
    }
    
    void update()
    {
        mSource.update();
        mPix = mSource.getPixelsRef();
        mPix.mirror(bFlipHorizon, bFlipVertical);
        mTex.loadData(mPix);
    }
    
    void play()
    {
        mSource.play();
    }
    
    void stop()
    {
        mSource.stop();
    }
    
    void togglePlay()
    {
        mSource.isPlaying() ? mSource.stop() : mSource.play();
    }
    
    void setFlip(bool horizon, bool vertical)
    {
        bFlipHorizon  = horizon;
        bFlipVertical = vertical;
    }
    
    ofPixels& getPixelsRef()
    {
        return mPix;
    }
    
    ofTexture& getTextureRef()
    {
        return mTex;
    }
    
    int getWidth()
    {
        return mSource.getWitdth();
    }
    
    int getHeight()
    {
        return mSource.getHeight();
    }
};

typedef InputImageController<ofVideoPlayer>     InputVideoController;
typedef InputImageController<ofVideoGrabber>    InputCameraController;
