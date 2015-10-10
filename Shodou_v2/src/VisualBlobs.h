#pragma once

#include "ofMain.h"
#include "utils.h"
#include "Blob.h"
#include "ofxAnimationPrimitives.h"
#include "InputImageController.h"
#include "BlobDataController.h"

class VisualBlobs
{
    BaseImagesInterface* mImages;
    const float mWidth;
    const float mHeight;
    ofFbo mFbo;
    
    // scenes
    ofxAnimationPrimitives::SceneManager mScenes;
    vector<string> mSceneNames;
    int mCurrentNumScene;
    
    // animations
    ofxAnimationPrimitives::InstanceManager mAnimations;
    
    // sequencer
    BlobsDataController* mBlobData;
    
public:
    VisualBlobs(BaseImagesInterface* baseImageInterfacePtr, const float width, const float height);
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
    
    inline void setBlobDataController(BlobsDataController* bdc)
    {
        mBlobData = bdc;
    }
    
    //---------
    // static shared value and function
    //---------
    
    enum TargetTexture { GRAY, BINARY };
    
    static ofFbo    smFbo;
    static float    smWidth;
    static float    smHeight;
    static ofImage  smWashiImage;
    
    static void setupFbo(float w, float h);
};
