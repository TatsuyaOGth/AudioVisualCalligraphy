#pragma once

#include "ofMain.h"
#include "utils.h"
#include "ofxOpenCv.h"

typedef deque<ofxCvBlob> BLOBS_TYPE;

class BlobsDataController
{
    BLOBS_TYPE mBlobs;
    float mWidth, mHeight;
    
public:
    BlobsDataController()
    {
        mWidth = 0;
        mHeight = 0;
    }
    
    void update()
    {
        
    }
    
    void draw(int x, int y, int w, int h)
    {
        float scalex, scaley;
        if( mWidth != 0 ) { scalex = w/mWidth; } else { scalex = 1.0f; }
        if( mHeight != 0 ) { scaley = h/mHeight; } else { scaley = 1.0f; }
        
        ofPushStyle();
        // ---------------------------- draw the bounding rectangle
        ofSetHexColor(0xDD00CC);
        glPushMatrix();
        glTranslatef( x, y, 0.0 );
        glScalef( scalex, scaley, 0.0 );
        
        ofNoFill();
        for( int i=0; i<(int)mBlobs.size(); i++ ) {
            ofRect( mBlobs[i].boundingRect.x, mBlobs[i].boundingRect.y,
                   mBlobs[i].boundingRect.width, mBlobs[i].boundingRect.height );
        }
        
        // ---------------------------- draw the blobs
        ofSetHexColor(0x00FFFF);
        
        for( int i=0; i<(int)mBlobs.size(); i++ ) {
            ofNoFill();
            ofBeginShape();
            for( int j=0; j<mBlobs[i].nPts; j++ ) {
                ofVertex( mBlobs[i].pts[j].x, mBlobs[i].pts[j].y );
            }
            ofEndShape();
            
        }
        glPopMatrix();
        ofPopStyle();
    }
    
    void setSize(float w, float h)
    {
        mWidth = w;
        mHeight = h;
    }
    
    float getWidth()
    {
        return mWidth;
    }
    
    float getHeight()
    {
        return mHeight;
    }
    
    void addBlob(ofxCvBlob& blob)
    {
        mBlobs.push_back(blob);
    }
    
    void removeBlob()
    {
        if (!mBlobs.empty())
        {
            mBlobs.pop_back();
        }
    }
    
    const BLOBS_TYPE& getBlobsRef() const
    {
        return mBlobs;
    }
};
