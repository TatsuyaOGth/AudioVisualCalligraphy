#pragma once

#include "ofMain.h"
#include "Constants.h"

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
    
private:
    bool isInitialized()
    {
        for (auto& e : mCam)
        {
            if (e.isInitialized() == false) return false;
        }
        return true;
    }
    
public:
    InputCameraController() {}
    
    void setup()
    {
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
        int numLoop = 0;
        const int dstW = CAMERA_WIDTH * mCam.size();
        const int dstH = CAMERA_HEIGHT;
        unsigned char* margePix = new unsigned char[dstW * dstH * 3];
        for (auto& e : mCam)
        {
            e.update();

            const int w = e.getWidth();
            const int h = e.getHeight();
            const int c = e.getPixelsRef().getNumChannels();
            unsigned char* camPix = e.getPixelsRef().getPixels();
            for (int y = 0; y < h; ++y)
            {
                for (int x = 0; x < w; ++x)
                {
                    int i = w * y + x;
                    int j = w * y + x + (w * numLoop);
                    for (int ch = 0; ch < c; ++ch)
                    {
                        margePix[j*c+ch] = camPix[i*c+ch];
                    }
                }
            }
            numLoop++;
        }
        mPix.setFromPixels(margePix, dstW, dstH, 3);
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
};
