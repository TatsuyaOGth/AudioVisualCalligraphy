#pragma once

#include "ofMain.h"
#include "../../common/utils.h"
#include "Blob.h"
#include "ofxAnimationPrimitives.h"
#include "InputImageController.hpp"


typedef const vector<BaseImagesInterface*> baseimages_type;

namespace vbu
{
    enum TargetTexture { GRAY, BINARY };
    
    static ofFbo fbo;
    static float width;
    static float height;
    static ofImage washiImage;
    
    static void setupFbo(float w, float h)
    {
        if (!fbo.isAllocated() || fbo.getWidth() != w || fbo.getHeight() != h)
        {
            fbo.allocate(w, h, GL_RGBA);
        }
    }

    static ofTexture& getJoinedTexture(baseimages_type& images, float width, float height, TargetTexture targetTexture)
    {
        setupFbo(width, height);
        glClearColor(0, 0, 0, 0);
        fbo.begin();
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
        fbo.end();
        return fbo.getTextureReference();
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
    BaseAnimation(const BLOB_TYPE* blob) : mBlob(blob)
    {
        
    }
};

//-----------------------------------------------------------------------------------------------
class TwinkBlob : public BaseAnimation
{
    ofColor mCol;
    
public:
    TwinkBlob(const BLOB_TYPE* blob) : BaseAnimation(blob)
    {
        mCol = ofColor::fromHsb(ofRandom(255), 255, 200);
    }
    void draw()
    {
        ofSetColor(mCol, getLife() * 255);
        
        ofFill();
        vbu::washiImage.bind();
        ofBeginShape();
        for (const auto& p : mBlob->pts)
        {
            ofVertex(p.x * vbu::width, p.y * vbu::height);
        }
        vbu::washiImage.unbind();
        ofEndShape();
    }
};

//-----------------------------------------------------------------------------------------------
class ParticleBlobEdge : public BaseAnimation
{
    ofColor mCol;
    int mValiation;
    
public:
    ParticleBlobEdge(const BLOB_TYPE* blob) : BaseAnimation(blob)
    {
        mCol = ofColor(255, 255, 255);
        mValiation = 0;
    }
    void draw()
    {
        ofSetColor(mCol, getLife() * 255);
        
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
        
    }
    
    void draw()
    {
        ofSetColor(255, getFadeAlpha());
        vbu::getJoinedTexture(mImages, mWidth, mHeight, vbu::GRAY).draw(0, 0, mWidth, mHeight);
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
        
    }
    
    void draw()
    {
        ofSetColor(255, getFadeAlpha());
        vbu::getJoinedTexture(mImages, mWidth, mHeight, vbu::BINARY).draw(0, 0, mWidth, mHeight);
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
    
    void update()
    {
    }
    
    void draw()
    {
    }
    
    OFX_ANIMATION_PRIMITIVES_DEFINE_SCENE(SceneVfx);
};



/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  MAIN VISUAL CLASS
//
/////////////////////////////////////////////////////////////////////////////////////////////////

class VisualBlobs
{
    baseimages_type mImages;
    const float mWidth;
    const float mHeight;
    ofFbo mFbo;
    
    // scenes
    ofxAnimationPrimitives::SceneManager mScenes;
    vector<string> mSceneNames;
    int mCurrentNumScene;
    
    // animations
    ofxAnimationPrimitives::InstanceManager mAnimations;
    
public:
    VisualBlobs(baseimages_type& baseImageInterfacePtr, const float width, const float height)
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
        
        vbu::width = width;
        vbu::height = height;
        vbu::washiImage.loadImage("washi.png");
    }
    
    void update()
    {
        mScenes.update();
        mAnimations.update();
    }
    
    void rendering()
    {
        ofBackground(0, 0, 0, 0);
        ofSetColor(255, 255, 255, 255);
        
        glClearColor(0, 0, 0, 0);
        mFbo.begin();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        mScenes.draw();
        mAnimations.draw();
        
        mFbo.end();
    }
    
    ofTexture& getTextureRef()
    {
        return mFbo.getTextureReference();
    }
    
    
    
    void changeScene(float fadeduration = 2)
    {
        mCurrentNumScene++;
        if (mCurrentNumScene >= mSceneNames.size())
        {
            mCurrentNumScene = 0;
        }
        mScenes.changeScene(mSceneNames[mCurrentNumScene], fadeduration);
    }
    
    ofxAnimationPrimitives::InstanceManager& getAnimationManager()
    {
        return mAnimations;
    }
};
