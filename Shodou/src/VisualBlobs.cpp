#include "VisualBlobs.h"


//---------
// static shared value and function
//---------

ofFbo    VisualBlobs::smFbo;
float    VisualBlobs::smWidth;
float    VisualBlobs::smHeight;
ofRectangle VisualBlobs::smRemapedRect;
ofImage  VisualBlobs::smWashiImage;
FlowTools* VisualBlobs::smFlowTools;


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

void VisualBlobs::getJoinedContourseBlob(baseimages_type& images, vector<ofxCvBlob>& dst, float width, float height)
{
    dst.clear();
    int n = images.size();
    for (int i = 0; i < n; ++i)
    {
        int x = (width / n) * i;
        int y = 0;
        int w =  width / n;
        int h = height;
        float orgW = images[i]->getCvContourFinder().getWidth();
        float orgH = images[i]->getCvContourFinder().getHeight();

        for (auto& e : images[i]->getCvContourFinder().blobs)
        {
            dst.push_back(e);
            for (int j = 0; j < e.pts.size(); ++j)
            {
                dst.back().pts[j].x = ofMap(e.pts[j].x, 0, orgW, x, x + w);
                dst.back().pts[j].y = ofMap(e.pts[j].y, 0, orgH, y, y + h);
//                dst.back().boundingRect.x = ofMap(dst.back().boundingRect.x, 0, orgW, x, x + w);
//                dst.back().boundingRect.y = ofMap(dst.back().boundingRect.y, 0, orgH, y, y + h);
//                dst.back().boundingRect.width  = ofMap(dst.back().boundingRect.width,  0, orgW, x, x + w);
//                dst.back().boundingRect.height = ofMap(dst.back().boundingRect.height, 0, orgH, y, y + h);
            }
        }
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
    const BLOB_TYPE* mBlob;
    
public:
    BaseAnimation(const BLOB_TYPE* blob) : mBlob(blob) {}
    
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
    TwinkBlob(const BLOB_TYPE* blob, ofColor col)
    : BaseAnimation(blob)
    , mCol(col)
    {
    }
    void draw()
    {
        ofSetColor(mCol, getAlpha() * 255);
        
        ofFill();
        ofPushMatrix();
        ofTranslate(VisualBlobs::smRemapedRect.x, VisualBlobs::smRemapedRect.y);
        VisualBlobs::smWashiImage.bind();
        ofBeginShape();
        for (const auto& p : mBlob->pts)
        {
            ofVertex(p.x * VisualBlobs::smRemapedRect.width  + ofRandom(-1, 1),
                     p.y * VisualBlobs::smRemapedRect.height + ofRandom(-1, 1));
        }
        VisualBlobs::smWashiImage.unbind();
        ofEndShape();
        ofPopMatrix();
    }
};

//-----------------------------------------------------------------------------------------------
class BlobEdge : public BaseAnimation
{
    ofColor mCol;
    ofVboMesh mMesh;
    
public:
    BlobEdge(const BLOB_TYPE* blob, ofColor col)
    : BaseAnimation(blob)
    , mCol(col)
    {
        for (const auto& p : mBlob->pts)
        {
            mMesh.addColor(ofColor::fromHsb(mCol.getHue() + ofRandom(-20, 20), mCol.getSaturation(), mCol.getBrightness()));
            mMesh.addVertex(ofPoint(p.x * VisualBlobs::smRemapedRect.width, p.y * VisualBlobs::smRemapedRect.height));
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
        ofTranslate(VisualBlobs::smRemapedRect.x, VisualBlobs::smRemapedRect.y);
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
    ParticleBlobEdge(const BLOB_TYPE* blob, ofColor col)
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
            mPos.push_back(ofPoint(e.x * VisualBlobs::smRemapedRect.width,
                                   e.y * VisualBlobs::smRemapedRect.height));
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
            ofTranslate(VisualBlobs::smRemapedRect.x, VisualBlobs::smRemapedRect.y);
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


class ZoomCamera : public ofxAnimationPrimitives::Instance
{
    baseimages_type mImages;
    ofRectangle mSrcRect, mDstRect;
    
public:
    ZoomCamera(baseimages_type& images) : mImages(images)
    {
        
    }
    void update()
    {
        
    }
    
    void draw()
    {
        ofSetColor(255, getAlpha() * 90);
        const ofTexture& tex = VisualBlobs::getJoinedTexture(mImages,
                                      VisualBlobs::smRemapedRect.width,
                                      VisualBlobs::smRemapedRect.height,
                                      VisualBlobs::GRAY);
        
    }
    
    float getAlpha()
    {
        return ofxAnimationPrimitives::Easing::Cubic::easeOut(getLife());
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
    vector<ofxCvBlob> mBlobs;
    
    
public:
    SceneVfx(baseimages_type& baseImageInterfacePtr, const float width, const float height)
    : BaseScene(baseImageInterfacePtr, width, height)
    {
    }
    void update()
    {
    }
    void draw()
    {
        ofSetColor(255, getFadeAlpha());
        
        VisualBlobs::getJoinedContourseBlob(mImages, mBlobs, VisualBlobs::smRemapedRect.width, VisualBlobs::smRemapedRect.height);
        ofPushMatrix();
        ofTranslate(VisualBlobs::smRemapedRect.x, VisualBlobs::smRemapedRect.y);
        for (auto e : mBlobs)
        {
            ofNoFill();
            ofBeginShape();
            for (int i = 0; i < e.nPts; i++){
                ofVertex(e.pts[i].x, e.pts[i].y);
            }
            ofEndShape(true);
        }
        ofPopMatrix();
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
    mScenes.addScene<EmptyScene>();

    mSceneNames = mScenes.getSceneNames();
    mCurrentNumScene = 0;
    mScenes.changeScene(mSceneNames[mCurrentNumScene], 2);
    
    // setup static velues
    smWidth = width;
    smHeight = height;
    smWashiImage.loadImage("washi.png");
    
    smFlowTools = new FlowTools();
//    smFlowTools->setTexture();
    smFlowTools->setup();
}

void VisualBlobs::update()
{
    mScenes.update();
    mAnimations.update();
    
//    smFlowTools->setTexture(VisualBlobs::getJoinedTexture(mImages,
//                                  VisualBlobs::smRemapedRect.width,
//                                  VisualBlobs::smRemapedRect.height,
//                                  VisualBlobs::BINARY));
    smFlowTools->update();
}

void VisualBlobs::rendering()
{
    glClearColor(0, 0, 0, 0);
    mFbo.begin();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    ofBackground(0);
    ofSetColor(255);
    
    smFlowTools->draw();
    mScenes.draw();
    mAnimations.draw();

    mBlobData->drawSeqAll(VisualBlobs::smRemapedRect.x,
                          VisualBlobs::smRemapedRect.y,
                          VisualBlobs::smRemapedRect.width,
                          VisualBlobs::smRemapedRect.height);

    
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
//        VisualBlobs::smFlowTools->emit(e.blobPtr->centroid.x * smRemapedRect.width + smRemapedRect.x,
//                                       e.blobPtr->centroid.x * smRemapedRect.height + smRemapedRect.y);
        VisualBlobs::smFlowTools->emit(ofRandomWidth(), ofRandomHeight());

    }
    if (e.channel == 4)
    {
        mAnimations.createInstance<BlobEdge>(e.blobPtr, ofColor::fromHsb(ofRandom(180, 200), 255, 255))->play(3);
        mAnimations.createInstance<BlobEdge>(e.blobPtr, ofColor::fromHsb(ofRandom(180, 200), 255, 255))->play(4, 0.5);
        
//        VisualBlobs::smFlowTools->emit(e.blobPtr->centroid.x * smRemapedRect.width + smRemapedRect.x,
//                                       e.blobPtr->centroid.x * smRemapedRect.height + smRemapedRect.y);
        VisualBlobs::smFlowTools->emit(ofRandomWidth(), ofRandomHeight());
    }
    if (e.channel == 5)
    {
        mAnimations.createInstance<BlobEdge>(e.blobPtr, ofColor(255, 255, 255))->play(6);
//        VisualBlobs::smFlowTools->emit(e.blobPtr->centroid.x * smRemapedRect.width + smRemapedRect.x,
//                                       e.blobPtr->centroid.x * smRemapedRect.height + smRemapedRect.y);
    }
    if (e.channel == 6)
    {
        mAnimations.createInstance<TwinkBlob>(e.blobPtr, ofColor::fromHsb(ofRandom(255), 120, 255))->play(9);
        mAnimations.createInstance<BlobEdge>(e.blobPtr, ofColor(255, 255, 255))->play(9);
//        VisualBlobs::smFlowTools->emit(e.blobPtr->centroid.x * smRemapedRect.width + smRemapedRect.x,
//                                       e.blobPtr->centroid.x * smRemapedRect.height + smRemapedRect.y);
        VisualBlobs::smFlowTools->emit(ofRandomWidth(), ofRandomHeight());
    }
    
    VisualBlobs::smFlowTools->emit(ofRandomWidth(), ofRandomHeight());

}
