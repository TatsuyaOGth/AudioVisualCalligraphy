#pragma once

#include "ofMain.h"
#include "Constants.h"
#include "ImageProcessing.hpp"

class InputImageController
{
protected:
    ofPixels mPix;
    ofTexture mTex;
    bool bFlipHorizon, bFlipVertical;

protected:
    void allocatePixAndTex(int w, int h, int channel = 3)
    {
        mPix.allocate(w, h, channel);
        mTex.allocate(mPix);
    }
    
public:
    float mVerticalWarp;
    
    InputImageController() : bFlipHorizon(false), bFlipVertical(false) {}
    virtual ~InputImageController() {}
    
    virtual void setup()    = 0;
    virtual void update()   = 0;
    
    virtual void play()         = 0;
    virtual void stop()         = 0;
    virtual void togglePlay()   = 0;
    
    virtual bool isFrameNew(){}
    
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
        return mPix.getWidth();
    }
    
    int getHeight()
    {
        return mPix.getHeight();
    }
};



class InputVideoController : public InputImageController
{
    ofVideoPlayer mSource;
    
public:
    InputVideoController() {}
    
    void setup()
    {
        if (mSource.loadMovie(SOURCE_VIDEO) == false)
        {
            LOG_ERROR << "failed load movie: " << SOURCE_VIDEO;
            return;
        }
        mSource.setVolume(0);
        allocatePixAndTex(mSource.getWidth(), mSource.getHeight());
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
};



class InputCameraController : public InputImageController
{
    vector<ofVideoGrabber> mCam;
    
public:
    InputCameraController() {}
    
    void setup()
    {
        mVerticalWarp = 0;
        mCam.resize(NUM_CAMERA);
        
        vector<ofVideoDevice> devices = mCam[0].listDevices();
        for (const auto& e : devices)
        {
            cout << e.id << ": " << e.deviceName << (e.bAvailable ? "" : " - unavailable ") << endl;
        }
        
        int camWidth 		= CAMERA_WIDTH;
        int camHeight 		= CAMERA_HEIGHT;
        int i = 0;
        for (auto& e : mCam)
        {
            e.setDeviceID(CAMERA_DEVISE_ID[i]);
//            e.setDesiredFrameRate(60);
            e.initGrabber(camWidth, camHeight, false);
            ++i;
        }
        allocatePixAndTex(camWidth * mCam.size(), camHeight);
    }
    
    void update()
    {
        const int ch = 3;
        const int dstW = CAMERA_WIDTH * NUM_CAMERA;
        const int dstH = CAMERA_HEIGHT;
        unsigned char* margePix = new unsigned char[dstW * dstH * ch];
        mPix.allocate(dstW, dstH, ch);
        
        for (int i = 0; i < NUM_CAMERA; ++i)
        {
            auto& e = mCam[i];
            e.update();
        }
        
        for (int i = 0; i < NUM_CAMERA; ++i)
        {
            auto& e = mCam[i];
            const int w = CAMERA_WIDTH;
            const int h = CAMERA_HEIGHT;
            unsigned char* camPix = e.getPixelsRef().getPixels();
            for (int y = 0; y < h; ++y)
            {
                for (int x = 0; x < w; ++x)
                {
                    const int index   = w * y + x;
                    const int offsetx = w * y + (w * i);
                    const int dstIdx  = index + offsetx;
                    for (int j = 0; j < ch; ++j)
                    {
                        mPix[dstIdx*ch+j] = camPix[index*ch+j];
                    }
                }
            }
        }
        mPix.mirror(bFlipHorizon, bFlipVertical);
        mTex.loadData(mPix);
        delete[] margePix;
    }
    
    void play()
    {
        if (isInitialized() == false)
        {
            setup();
        }
    }
    
    void stop()
    {
        for (auto& e : mCam)
        {
            e.close();
        }
    }
    
    void togglePlay()
    {
        isInitialized() ? stop() : play();
    }
    
    void setFlip(bool horizon, bool vertical)
    {
        bFlipHorizon  = horizon;
        bFlipVertical = vertical;
    }
    
    bool isFrameNew()
    {
        for (auto& e : mCam)
        {
            if (e.isFrameNew() == false) return false;
        }
        return true;
    }
    
    bool isInitialized()
    {
        for (auto& e : mCam)
        {
            if (e.isInitialized() == false) return false;
        }
        return true;
    }
};
