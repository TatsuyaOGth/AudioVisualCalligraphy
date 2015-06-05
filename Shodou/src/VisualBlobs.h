#pragma once

#include "ofMain.h"
#include "../../common/utils.h"
#include "Blob.h"
#include "ofxAnimationPrimitives.h"
#include "InputImageController.h"


typedef const vector<BaseImagesInterface*> baseimages_type;




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
    VisualBlobs(baseimages_type& baseImageInterfacePtr, const float width, const float height);
    void update();
    void rendering();
    
    void changeScene(float fadeduration = 2);
    void blobNoteEvent(BlobNoteEvent& e);
    
    inline ofTexture& getTextureRef()
    {
        return mFbo.getTextureReference();
    }
    
    inline ofxAnimationPrimitives::InstanceManager& getAnimationManager()
    {
        return mAnimations;
    }
    
    //---------
    // static shared value and function
    //---------
    
    enum TargetTexture { GRAY, BINARY };
    
    static ofFbo    smFbo;
    static float    smWidth;
    static float    smHeight;
    static ofRectangle smRemapedRect;
    static ofImage  smWashiImage;
    
    static void setupFbo(float w, float h);
    static ofTexture& getJoinedTexture(baseimages_type& images, float width, float height, TargetTexture targetTexture);
};
