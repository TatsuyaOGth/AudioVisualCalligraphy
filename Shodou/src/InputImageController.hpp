#pragma once

#include "ofMain.h"
#include "ImageProcessing.hpp"

class BaseImagesInterface
{
protected:
    ofPixels    mFlipedPix, mGrayPix, mResizedPix, mCropedPix, mWarpedPix, mBinaryPix;
    ofTexture   mFlipedTex, mGrayTex, mResizedTex, mCropedTex, mWarpedTex, mBinaryTex;
    
    ofxCvColorImage         mCvImage;
    ofxCvGrayscaleImage     mCvGrayImage;
    ofxCvContourFinder      mContourFinder;
    
public:
    
    ofPixels& getGrayPixelsRef()      { return mGrayPix;    }
    ofPixels& getResizedPixelsRef()   { return mResizedPix; }
    ofPixels& getCropedPixelsRef()    { return mCropedPix;  }
    ofPixels& getWarpedPixelsRef()    { return mWarpedPix;  }
    ofPixels& getBinaryPixelsRef()    { return mBinaryPix;  }
    
    ofTexture& getGrayTextureRef()    { return mGrayTex;    }
    ofTexture& getResizedTextureRef() { return mResizedTex; }
    ofTexture& getCropedTextureRef()  { return mCropedTex;  }
    ofTexture& getWarpedTextureRef()  { return mWarpedTex;  }
    ofTexture& getBinaryTextureRef()  { return mBinaryTex;  }
};



template<typename baseImageObject>
class InputImageController : public baseImageObject, public BaseImagesInterface
{
protected:
    
    ofParameterGroup        mParamGroup;
    ofParameter<int>        mResizeRatio;
    ofParameter<bool>       mFlipH;
    ofParameter<bool>       mFlipV;
    ofParameter<ofVec2f>    mCropXY1;
    ofParameter<ofVec2f>    mCropXY2;
    ofParameter<float>      mWarpX;
    ofParameter<float>      mWarpY;
    ofParameter<float>      mBlobThreshold;
    ofParameter<int>        mMaxNumBlobs;
    
    void setupGui()
    {
        static int idx = 1;
        string idxStr = "_" + ofToString(idx);
        mParamGroup.setName("INPUT_IMAGE" + idxStr);
        mParamGroup.add(mResizeRatio.set("RESIZE_RATIO" + idxStr, 2, 1, 4));
        mParamGroup.add(mFlipH.set("FLIP_HORIZON" + idxStr, true));
        mParamGroup.add(mFlipV.set("FLIP_VERTICAL" + idxStr, true));
        mParamGroup.add(mCropXY1.set("CROP_XY_1" + idxStr, ofVec2f(0, 0), ofVec2f(0, 0), ofVec2f(1, 1)));
        mParamGroup.add(mCropXY2.set("CROP_XY_2" + idxStr, ofVec2f(1, 1), ofVec2f(0, 0), ofVec2f(1, 1)));
        mParamGroup.add(mWarpX.set("WARP_X" + idxStr, 0, -180, 180));
        mParamGroup.add(mWarpY.set("WARP_Y" + idxStr, 0, -180, 180));
        mParamGroup.add(mBlobThreshold.set("THRESHOLD", 127, 0, 255));
        idx++;
    }
    
    void allocatePixels(ofPixels& pix, int w, int h, int ch)
    {
        pix.allocate(w, h, ch);
    }
    
    void allocateTexture(ofTexture& tex, int w, int h, int ch)
    {
        tex.allocate(w, h, ch == 1 ? GL_LUMINANCE : GL_RGB);
    }
    
    template<typename ofxCvImageType>
    void setCvImageFromPixels(ofxCvImageType& cvImage, ofPixels& pix)
    {
        if (cvImage.getWidth() != pix.getWidth() || cvImage.getHeight() != pix.getHeight())
        {
            cvImage.allocate(pix.getWidth(), pix.getHeight());
        }
        cvImage.setFromPixels(pix);
    }
    
    void textureLoadData(ofPixels& pix, ofTexture& tex)
    {
        if (pix.getWidth() != tex.getWidth() || pix.getHeight() != tex.getHeight())
        {
            allocateTexture(tex, pix.getWidth(), pix.getHeight(), pix.getNumChannels());
        }
        tex.loadData(pix);
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
    
    void allocateAllPixelsAndTextures(int w, int h)
    {
        allocatePixels(mFlipedPix, w, h, 3);
        allocatePixels(mGrayPix, w, h, 1);
        allocatePixels(mResizedPix, w, h, 1);
        allocatePixels(mCropedPix, w, h, 1);
        allocatePixels(mWarpedPix, w, h, 1);
        allocatePixels(mBinaryPix, w, h, 1);
        
        allocateTexture(mFlipedTex, w, h, 3);
        allocateTexture(mGrayTex, w, h, 1);
        allocateTexture(mResizedTex, w, h, 1);
        allocateTexture(mCropedTex, w, h, 1);
        allocateTexture(mWarpedTex, w, h, 1);
        allocateTexture(mBinaryTex, w, h, 1);
        
        mCvImage.allocate(w, h);
        mCvGrayImage.allocate(w, h);
    }
    
    void preProcess(const ofPixels& srcPix)
    {
        imp::flip(srcPix, mFlipedPix, mFlipH, mFlipV);
        imp::rgbToGray(mFlipedPix, mGrayPix);
        imp::resize(mGrayPix, mResizedPix, srcPix.getWidth() / mResizeRatio, srcPix.getHeight() / mResizeRatio);
        imp::crop(mResizedPix, mCropedPix,
                  mCropXY1->x * mResizedPix.getWidth(), mCropXY1->y * mResizedPix.getHeight(),
                  mCropXY2->x * mResizedPix.getWidth(), mCropXY2->y * mResizedPix.getHeight());
//        imp::warpPerspective(mCropedPix, mWarpedPix, mWarpX, mWarpY);
//        imp::threshold(mWarpedPix, mBinaryPix, mBlobThreshold);
        
        setCvImageFromPixels(mCvGrayImage, mCropedPix);
        warpPerspective(mCvGrayImage, mWarpX, mWarpY);
        mWarpedPix.setFromPixels(mCvGrayImage.getPixels(), mCvGrayImage.getWidth(), mCvGrayImage.getHeight(), 1);
        mCvGrayImage.threshold(mBlobThreshold);
        mCvGrayImage.invert();
        mBinaryPix.setFromPixels(mCvGrayImage.getPixels(), mCvGrayImage.getWidth(), mCvGrayImage.getHeight(), 1);
        
        mCvGrayImage.setFromPixels(mBinaryPix);
        mContourFinder.findContours(mCvGrayImage, 0, 800*800, 127, true, true);
        
        textureLoadData(mGrayPix,       mGrayTex);
        textureLoadData(mResizedPix,    mResizedTex);
        textureLoadData(mCropedPix,     mCropedTex);
        textureLoadData(mWarpedPix,     mWarpedTex);
        textureLoadData(mBinaryPix,     mBinaryTex);
    }
    

public:
    InputImageController()
    {
        setupGui();
    }
    virtual ~InputImageController() {}
    
    virtual void update()       = 0;
    virtual void play()         = 0;
    virtual void stop()         = 0;
    virtual void togglePlay()   = 0;
    
    void setThreshold(float th) { mBlobThreshold = th; }
    
    ofxCvContourFinder& getCvContourFinder() { return mContourFinder; }
    
    ofParameterGroup& getParameterGroup()
    {
        return mParamGroup;
    }
    
    void drawCropRect(int x, int y, int w, int h)
    {
        const float baseWidth  = mResizedPix.getWidth();
        const float baseHeight = mResizedPix.getHeight();

        ofPushMatrix();
        ofPushStyle();
        ofTranslate(x, y);
        ofNoFill();
        ofSetLineWidth(1);
        ofSetColor(0, 255, 0);
        float _x = mCropXY1->x * baseWidth;
        float _y = mCropXY1->y * baseHeight;
        float _w = (mCropXY2->x - mCropXY1->x) * baseWidth;
        float _h = (mCropXY2->y - mCropXY1->y) * baseHeight;
        ofRect(_x, _y, _w, _h);
        ofPopStyle();
        ofPopMatrix();
    }

};


class InputVideoController : public InputImageController<ofVideoPlayer>
{
    typedef ofVideoPlayer basePlayer;
    const string mVideoPath;
public:
    InputVideoController(const string& videoPath)
    : mVideoPath(videoPath)
    {
        setup();
    }
    
    void setup()
    {
        if (loadMovie(mVideoPath) == false)
        {
            LOG_ERROR << "failed load movie: " << mVideoPath;
            return;
        }
        basePlayer::setVolume(0);
        allocateAllPixelsAndTextures(basePlayer::getWidth(), basePlayer::getHeight());
    }
    
    void update()
    {
        basePlayer::update();
        if (isFrameNew())
        {
            preProcess(basePlayer::getPixelsRef());
        }
    }
    
    void play()
    {
        basePlayer::play();
    }
    
    void stop()
    {
        basePlayer::stop();
    }
    
    void togglePlay()
    {
        basePlayer::isPlaying() ? stop() : play();
    }
};


class InputCameraController : public InputImageController<ofVideoGrabber>
{
    typedef ofVideoGrabber baseGrabber;
    const int mWidth;
    const int mHeight;
    const int mDeviceId;
    
public:
    InputCameraController(int w, int h, int deviceId)
    : mWidth(w)
    , mHeight(h)
    , mDeviceId(deviceId)
    {
        setup();
    }
    
    void setup()
    {
        baseGrabber::setDeviceID(mDeviceId);
        baseGrabber::initGrabber(mWidth, mHeight, true);
        allocateAllPixelsAndTextures(baseGrabber::getWidth(), baseGrabber::getHeight());
    }
    
    void update()
    {
        baseGrabber::update();
        if (isFrameNew())
        {
            preProcess(baseGrabber::getPixelsRef());
        }
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
        baseGrabber::close();
    }
    
    void togglePlay()
    {
        isInitialized() ? stop() : play();
    }
};
