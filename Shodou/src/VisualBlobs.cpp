#include "VisualBlobs.h"


//---------
// static shared value and function
//---------

ofFbo    VisualBlobs::smFbo;
float    VisualBlobs::smWidth;
float    VisualBlobs::smHeight;
ofRectangle VisualBlobs::smRemapedRect;
ofImage  VisualBlobs::smWashiImage;


void VisualBlobs::setupFbo(float w, float h)
{
    if (!smFbo.isAllocated() || smFbo.getWidth() != w || smFbo.getHeight() != h)
    {
        smFbo.allocate(w, h, GL_RGBA);
    }
}

ofTexture& VisualBlobs::getJoinedTexture(baseimages_type& images, float width, float height, TargetTexture targetTexture)
{
    setupFbo(width, height);
    glClearColor(0, 0, 0, 0);
    smFbo.begin();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    int n = images.size();
    int i= 0;
    for (int i = 0; i < n; ++i)
    {
        int x = (width / n) * i;
        int w =  width / n;
        switch (targetTexture)
        {
            case GRAY:   images[i]->getWarpedTextureRef().draw(x, 0, w, height); break;
            case BINARY: images[i]->getBinaryTextureRef().draw(x, 0, w, height); break;
        }
    }
    smFbo.end();
    return smFbo.getTextureReference();
}



/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ANIMATION CLASS
//
/////////////////////////////////////////////////////////////////////////////////////////////////

class BaseAnimation : public ofxAnimationPrimitives::Instance
{
protected:
    const BLOB_TYPE* mBlob;
    
public:
    BaseAnimation(const BLOB_TYPE* blob) : mBlob(blob) {}
};

//-----------------------------------------------------------------------------------------------
class TwinkBlob : public BaseAnimation
{
    ofColor mCol;
    float mVec;
    
public:
    TwinkBlob(const BLOB_TYPE* blob) : BaseAnimation(blob)
    {
        mCol = ofColor::fromHsb(ofRandom(255), 200, 255);
        mVec = 0;
    }
    void draw()
    {
        ofSetColor(mCol, getLife() * 255);
        mVec += 0.5;
        
        ofFill();
        ofPushMatrix();
        ofTranslate(VisualBlobs::smRemapedRect.x, VisualBlobs::smRemapedRect.y);
        VisualBlobs::smWashiImage.bind();
        ofBeginShape();
        for (const auto& p : mBlob->pts)
        {
            ofVertex(p.x * VisualBlobs::smRemapedRect.width, p.y * VisualBlobs::smRemapedRect.height);
        }
        VisualBlobs::smWashiImage.unbind();
        ofEndShape();
        ofPopMatrix();
    }
};

//-----------------------------------------------------------------------------------------------
class ParticleBlobEdge : public BaseAnimation
{
    ofColor mCol;
    int mValiation;
    float mSize;
    
public:
    ParticleBlobEdge(const BLOB_TYPE* blob) : BaseAnimation(blob)
    {
        mCol = ofColor(255, 255, 255);
        mValiation = 0;
        mSize = ofRandom(100, 150);
    }
    void draw()
    {
        ofSetColor(mCol, getLife() * 255);
        
        switch (mValiation)
        {
            case 0: drawPoints(); break;
                
        }
    }
    
    void drawPoints()
    {
        const ofPoint& p = mBlob->centroid;
        for (auto& e : mBlob->pts)
        {
            
        }
    }
    
    void drawPointLines()
    {
        
    }
};



class RippleBlob : public ofxAnimationPrimitives::Instance
{
    ofVec2f mPos;
    float mBeginSize, mEndSize;
    float mFadeinTime, mFadeoutTime;
    
public:
    RippleBlob(float w, float h, float beginSize, float endSize, float fadeinTime, float fadeoutTime)
    : mPos(ofVec2f(w, h))
    , mBeginSize(beginSize)
    , mEndSize(endSize)
    , mFadeinTime(fadeinTime)
    , mFadeoutTime(fadeoutTime)
    {}
    
    void draw()
    {
        float r = mBeginSize + mEndSize * getOneMinusLife();
        int a = getFadeAlpha(mFadeinTime, mFadeoutTime) * 80;
        ofPushStyle();
        ofNoFill();
        ofSetColor(0, a);
        ofSetLineWidth(4);
        ofCircle(mPos, r);
        ofPopStyle();
    }
    
    float getFadeAlpha(float fadeinTime, float fadeoutTime)
    {
        float duration = getDuration();
        float life = getLife() * duration;
        
        if (life > duration - fadeinTime)
        {
            return ofMap(life, duration - fadeinTime, duration, 1, 0);
        }
        else if (life < fadeoutTime) {
            return ofMap(life, 0, fadeoutTime, 0, 1);
        }
        return 1;
    }
};


/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  SCENE CLASS
//
/////////////////////////////////////////////////////////////////////////////////////////////////

class BaseScene : public ofxAnimationPrimitives::Scene
{
protected:
    baseimages_type mImages;
    const float mWidth;
    const float mHeight;
    
    int getFadeAlpha()
    {
        return ofxAnimationPrimitives::Easing::Quint::easeOut(getAlpha()) * 255;
    }
    
public:
    BaseScene(baseimages_type& baseImageInterfacePtr, const float width, const float height)
    : mImages(baseImageInterfacePtr)
    , mWidth(width)
    , mHeight(height)
    {}
};


//-----------------------------------------------------------------------------------------------
class SceneGray : public BaseScene
{
public:
    SceneGray(baseimages_type& baseImageInterfacePtr, const float width, const float height)
    : BaseScene(baseImageInterfacePtr, width, height)
    {
    }
    void update()
    {
        float w = 0;
        float h = 0;
        for (const auto& e : mImages)
        {
            w += e->getWarpedPixelsRef().getWidth();
            if (h < e->getWarpedPixelsRef().getHeight()) h = e->getWarpedPixelsRef().getHeight();
        }
        
        float pDifW = 1 - ((mWidth - w) / mWidth);
        float dstH = h * pDifW;
        
        VisualBlobs::smRemapedRect.x = 0;
        VisualBlobs::smRemapedRect.y = mHeight * 0.5 - dstH * 0.5;
        VisualBlobs::smRemapedRect.width  = mWidth;
        VisualBlobs::smRemapedRect.height = dstH;
    }
    void draw()
    {
        ofSetColor(255, getFadeAlpha());
        VisualBlobs::getJoinedTexture(mImages,
                                      VisualBlobs::smRemapedRect.width,
                                      VisualBlobs::smRemapedRect.height,
                                      VisualBlobs::GRAY).draw(VisualBlobs::smRemapedRect);
    }
    
    OFX_ANIMATION_PRIMITIVES_DEFINE_SCENE(SceneGray);
};


//-----------------------------------------------------------------------------------------------

class SceneBinary : public BaseScene
{
    
public:
    SceneBinary(baseimages_type& baseImageInterfacePtr, const float width, const float height)
    : BaseScene(baseImageInterfacePtr, width, height)
    {
        
    }
    void update()
    {
        float w = 0;
        float h = 0;
        for (const auto& e : mImages)
        {
            w += e->getBinaryPixelsRef().getWidth();
            if (h < e->getBinaryPixelsRef().getHeight()) h = e->getBinaryPixelsRef().getHeight();
        }
        
        float pDifW = 1 - ((mWidth - w) / mWidth);
        float dstH = h * pDifW;
        
        VisualBlobs::smRemapedRect.x = 0;
        VisualBlobs::smRemapedRect.y = mHeight * 0.5 - dstH * 0.5;
        VisualBlobs::smRemapedRect.width  = mWidth;
        VisualBlobs::smRemapedRect.height = dstH;
    }
    void draw()
    {
        ofSetColor(255, getFadeAlpha());
        VisualBlobs::getJoinedTexture(mImages,
                                      VisualBlobs::smRemapedRect.width,
                                      VisualBlobs::smRemapedRect.height,
                                      VisualBlobs::BINARY).draw(VisualBlobs::smRemapedRect);
    }
    
    OFX_ANIMATION_PRIMITIVES_DEFINE_SCENE(SceneBinary);
};

//-----------------------------------------------------------------------------------------------
class SceneVfx : public BaseScene
{
public:
    SceneVfx(baseimages_type& baseImageInterfacePtr, const float width, const float height)
    : BaseScene(baseImageInterfacePtr, width, height)
    {
    }    
    OFX_ANIMATION_PRIMITIVES_DEFINE_SCENE(SceneVfx);
};




VisualBlobs::VisualBlobs(baseimages_type& baseImageInterfacePtr, const float width, const float height)
: mImages(baseImageInterfacePtr)
, mWidth(width)
, mHeight(height)
{
    mFbo.allocate(width, height, GL_RGBA);
    
    // setup scene
    mScenes.addScene<SceneGray>(baseImageInterfacePtr, width, height);
    mScenes.addScene<SceneBinary>(baseImageInterfacePtr, width, height);
    mScenes.addScene<SceneVfx>(baseImageInterfacePtr, width, height);
    
    mSceneNames = mScenes.getSceneNames();
    mCurrentNumScene = 0;
    mScenes.changeScene(mSceneNames[mCurrentNumScene], 2);
    
    // setup static velues
    smWidth = width;
    smHeight = height;
    smWashiImage.loadImage("washi.png");
}

void VisualBlobs::update()
{
    mScenes.update();
    mAnimations.update();
}

void VisualBlobs::rendering()
{
    ofBackground(0, 0, 0, 0);
    ofSetColor(255, 255, 255);
    
    glClearColor(0, 0, 0, 0);
    mFbo.begin();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    mScenes.draw();
    mAnimations.draw();
    
    mFbo.end();
}

void VisualBlobs::changeScene(float fadeduration)
{
    mCurrentNumScene++;
    if (mCurrentNumScene >= mSceneNames.size())
    {
        mCurrentNumScene = 0;
    }
    mScenes.changeScene(mSceneNames[mCurrentNumScene], fadeduration);
}

void VisualBlobs::blobNoteEvent(BlobNoteEvent &e)
{
    if (e.channel == 3)
    {
        mAnimations.createInstance<TwinkBlob>(e.blobPtr)->play(1);
//        mAnimations.createInstance<RippleBlob>(e.blobPtr->centroid.x * smWidth,
//                                               e.blobPtr->centroid.y * smHeight,
//                                               ofMap(e.blobPtr->length, 20, 200, 20, 60, true),
//                                               ofMap(e.blobPtr->length, 20, 200, 40, 100, true),
//                                               0.3, 1.0)->play(1.5);
    }
}
