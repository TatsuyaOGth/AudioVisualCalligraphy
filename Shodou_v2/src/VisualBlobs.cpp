#include "VisualBlobs.h"


//---------
// static shared value and function
//---------

ofFbo    VisualBlobs::smFbo;
float    VisualBlobs::smWidth;
float    VisualBlobs::smHeight;
ofImage  VisualBlobs::smWashiImage;


void VisualBlobs::setupFbo(float w, float h)
{
    if (!smFbo.isAllocated() || smFbo.getWidth() != w || smFbo.getHeight() != h)
    {
        smFbo.allocate(w, h, GL_RGBA);
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ANIMATION CLASS
//
/////////////////////////////////////////////////////////////////////////////////////////////////

class BaseAnimation : public ofxAnimationPrimitives::Instance
{
protected:
    BLOB_TYPE mBlob;
    
public:
    BaseAnimation(const BLOB_TYPE& blob)
    {
        mBlob = BLOB_TYPE(new Blob(blob.get()));
    }
    
    float getAlpha()
    {
        return ofxAnimationPrimitives::Easing::Cubic::easeOut(getLife());
    }
};

//-----------------------------------------------------------------------------------------------
class TwinkBlob : public BaseAnimation
{
    ofColor mCol;
    
public:
    TwinkBlob(const BLOB_TYPE& blob, ofColor col)
    : BaseAnimation(blob)
    , mCol(col)
    {
    }
    void draw()
    {        
        ofSetColor(mCol, getAlpha() * 255);
        
        ofFill();
        ofBeginShape();
        for (const auto& p : mBlob->pts)
        {
            ofVertex(p.x * ofGetWidth()  + ofRandom(-1, 1),
                     p.y * ofGetHeight() + ofRandom(-1, 1));
        }
        ofEndShape();
    }
};

//-----------------------------------------------------------------------------------------------
class BlobEdge : public BaseAnimation
{
    ofColor mCol;
    ofVboMesh mMesh;
    
public:
    BlobEdge(const BLOB_TYPE& blob, ofColor col)
    : BaseAnimation(blob)
    , mCol(col)
    {
        for (const auto& p : mBlob->pts)
        {
            mMesh.addColor(ofColor::fromHsb(mCol.getHue() + ofRandom(-20, 20), mCol.getSaturation(), mCol.getBrightness()));
            mMesh.addVertex(ofPoint(p.x * ofGetWidth(), p.y * ofGetHeight()));
        }
        mMesh.setMode(OF_PRIMITIVE_LINE_LOOP);
    }
    void update()
    {
        for (int i = 0; i < mMesh.getNumVertices(); ++i)
        {
            ofPoint p = mMesh.getVertex(i);
            mMesh.setVertex(i, ofPoint(p.x + ofRandom(-1.5, 1.5),
                                       p.y + ofRandom(-1.5, 1.5)));
        }
        if (getLife() < 0.75)
        {
            for (int i = 0; i < mMesh.getNumVertices(); ++i)
            {
                ofColor c = mMesh.getColor(i);
                if (ofRandomf() > 0)
                {
                    mMesh.setColor(i, c);
                }
                else {
                    mMesh.setColor(i, ofColor(0, 0, 0, 0));
                }
            }
        }
    }
    void draw()
    {
        ofSetColor(mCol, getAlpha() * 255);
        ofNoFill();
        ofSetLineWidth(1);
        ofPushMatrix();
        ofTranslate(0, 0);
        ofBeginShape();
        mMesh.draw();
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
    vector<ofPoint> mPos;
    vector<float> mDeg;
    vector<float> mSpeed;
    int mNum;
    
public:
    ParticleBlobEdge(const BLOB_TYPE& blob, ofColor col)
    : BaseAnimation(blob)
    , mCol(col)
    {
        mValiation = 0;
        mSize = ofRandom(100, 150);
        mNum = 0;
        for (const auto& e : blob->pts)
        {
            mDeg.push_back(e.angle(blob->centroid));
            mSpeed.push_back(blob->centroid.distance(e) * 2);
            mPos.push_back(ofPoint(e.x * ofGetWidth(),
                                   e.y * ofGetHeight()));
            mNum++;
        }
    }
    void update()
    {
        for (int i = 0; i < mNum; ++i)
        {
            mPos[i].y -= mSpeed[i];
        }
    }
    void draw()
    {
        ofSetColor(mCol, getAlpha() * 255);
        for (int i = 0; i < mNum; ++i)
        {
            ofPushMatrix();
            ofTranslate(0, 0);
            ofTranslate(mPos[i]);
            ofRotateZ(mDeg[i]);
            ofCircle(0, 0, ofxAnimationPrimitives::Easing::Quad::easeOut(getLife()) * 2);
            ofPopMatrix();
        }
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
    BaseImagesInterface* mImages;
    const float mWidth;
    const float mHeight;
    ofFbo mFbo;
    
    int getFadeAlpha()
    {
        return ofxAnimationPrimitives::Easing::Quint::easeOut(getAlpha()) * 255;
    }
    
    void bufferBegin()
    {
        ofPushStyle();
        ofPushMatrix();
        mFbo.begin();
        ofClear(0, 0, 0, 0);
    }
    
    void bufferEnd()
    {
        mFbo.end();
        ofPopMatrix();
        ofPopStyle();
        
        ofSetColor(255, getFadeAlpha());
        mFbo.draw(0, 0, mWidth, mHeight);
    }
    
public:
    BaseScene(BaseImagesInterface* baseImageInterfacePtr, const float width, const float height)
    : mImages(baseImageInterfacePtr)
    , mWidth(width)
    , mHeight(height)
    {
        mFbo.allocate(mWidth, mHeight, GL_RGBA);
    }
};


//-----------------------------------------------------------------------------------------------
class SceneGray : public BaseScene
{
public:
    SceneGray(BaseImagesInterface* baseImageInterfacePtr, const float width, const float height)
    : BaseScene(baseImageInterfacePtr, width, height)
    {
    }
    void update()
    {
    }
    void draw()
    {
        bufferBegin();
        
        mImages->getWarpedTextureRef().draw(0, 0, mWidth, mHeight);
        mImages->getCvContourFinder().draw(0, 0, mWidth, mHeight);
        
        bufferEnd();
    }
    
    OFX_ANIMATION_PRIMITIVES_DEFINE_SCENE(SceneGray);
};


//-----------------------------------------------------------------------------------------------

class SceneBinary : public BaseScene
{
    
public:
    SceneBinary(BaseImagesInterface* baseImageInterfacePtr, const float width, const float height)
    : BaseScene(baseImageInterfacePtr, width, height)
    {
        
    }
    void update()
    {
    }
    void draw()
    {
        bufferBegin();
        
        mImages->getBinaryTextureRef().draw(0, 0, mWidth, mHeight);
        mImages->getCvContourFinder().draw(0, 0, mWidth, mHeight);
        
        bufferEnd();
    }
    
    OFX_ANIMATION_PRIMITIVES_DEFINE_SCENE(SceneBinary);
};

//-----------------------------------------------------------------------------------------------
class SceneVfx : public BaseScene
{
public:
    SceneVfx(BaseImagesInterface* baseImageInterfacePtr, const float width, const float height)
    : BaseScene(baseImageInterfacePtr, width, height)
    {
    }
    void update()
    {
    }
    void draw()
    {
        bufferBegin();
        
        float w = mImages->getCvContourFinder().getWidth();
        float h = mImages->getCvContourFinder().getHeight();
        for (auto e : mImages->getCvContourFinder().blobs)
        {
            ofNoFill();
            ofBeginShape();
            for (int i = 0; i < e.nPts; i++){
                float x = ofMap(e.pts[i].x, 0, w, 0, mWidth);
                float y = ofMap(e.pts[i].y, 0, h, 0, mHeight);
                ofVertex(x, y);
            }
            ofEndShape(true);
        }
        
        bufferEnd();
    }
    OFX_ANIMATION_PRIMITIVES_DEFINE_SCENE(SceneVfx);
};




class EmptyScene : public ofxAnimationPrimitives::Scene
{
    
public:
    EmptyScene()
    {
    }
    OFX_ANIMATION_PRIMITIVES_DEFINE_SCENE(EmptyScene);
};




VisualBlobs::VisualBlobs(BaseImagesInterface* baseImageInterfacePtr, const float width, const float height)
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
    mFbo.begin();
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    ofBackground(0);
    ofSetColor(255);
    
    mScenes.draw();
    mAnimations.draw();
    mBlobData->drawSeqAll(0, 0, smWidth, smHeight);
    
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
    if (e.channel == 1)
    {
        mAnimations.createInstance<TwinkBlob>(e.blobPtr, ofColor(255, 255, 255))->play(6);
    }
    
    if (e.channel == 2)
    {
        mAnimations.createInstance<BlobEdge>(e.blobPtr, ofColor(255, 255, 255))->play(6);
    }
    
    if (e.channel == 3)
    {
        mAnimations.createInstance<TwinkBlob>(e.blobPtr, ofColor::fromHsb(ofRandom(255), 255, 255))->play(6);
        
    }
    if (e.channel == 4)
    {
        mAnimations.createInstance<BlobEdge>(e.blobPtr, ofColor::fromHsb(ofRandom(180, 200), 255, 255))->play(3);
        mAnimations.createInstance<BlobEdge>(e.blobPtr, ofColor::fromHsb(ofRandom(180, 200), 255, 255))->play(4, 0.5);
    }
    if (e.channel == 5)
    {
        mAnimations.createInstance<BlobEdge>(e.blobPtr, ofColor(255, 255, 255))->play(6);
    }
    if (e.channel == 6)
    {
        mAnimations.createInstance<TwinkBlob>(e.blobPtr, ofColor::fromHsb(ofRandom(255), 120, 255))->play(9);
        mAnimations.createInstance<BlobEdge>(e.blobPtr, ofColor(255, 255, 255))->play(9);
    }
}
