#pragma once

#include "ofMain.h"
#include "../../common/utils.h"
#include "Blob.h"
#include "ofxAnimationPrimitives.h"
#include "InputImageController.hpp"

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ANIMATION CLASS
//
/////////////////////////////////////////////////////////////////////////////////////////////////






/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  SCENE CLASS
//
/////////////////////////////////////////////////////////////////////////////////////////////////

class SceneCamera : public ofxAnimationPrimitives::Scene
{
public:
    SceneCamera()
    {
        
    }
    
    void update()
    {
        
    }
    
    void draw()
    {
        
    }
    
    OFX_ANIMATION_PRIMITIVES_DEFINE_SCENE(SceneCamera);
};



/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  MAIN VISUAL CLASS
//
/////////////////////////////////////////////////////////////////////////////////////////////////

class VisualBlobs
{
    const vector<BaseImagesInterface*> mImages;
    const float mWidth;
    const float mHeight;
    ofFbo mFbo;
    
    
    
public:
    VisualBlobs(vector<BaseImagesInterface*>& baseImageInterfacePtr, const float width, const float height)
    : mImages(baseImageInterfacePtr)
    , mWidth(width)
    , mHeight(height)
    {
        mFbo.allocate(width, height, GL_RGBA);
    }
    
    void update()
    {
        
    }
    
    void rendering()
    {
        ofSetColor(255, 255, 255, 255);
        
        glClearColor(0, 0, 0, 0);
        mFbo.begin();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // test
        int n = mImages.size();
        int i= 0;
        for (auto& e : mImages)
        {
            int x = (mWidth / n) * i;
            int w = mWidth / n;
            e->getWarpedTextureRef().draw(x, 0, w, mHeight);
            ++i;
        }
        
        mFbo.end();
    }
    
    ofTexture& getTextureRef()
    {
        return mFbo.getTextureReference();
    }
};
