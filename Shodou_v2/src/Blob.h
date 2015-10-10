#pragma once

#include "ofxCvContourFinder.h"

class Blob : public ofxCvBlob
{
public:
    float width;
    float height;
    float offsetW;
    
public:
    Blob(const ofxCvBlob& blob, float w, float h, float offsetW = 0);
    Blob(const Blob* o);
    void setup(const ofxCvBlob& blob, float w, float h, float offsetW = 0);
    void draw(float x = 0, float y = 0);
};

typedef ofPtr<Blob>         BLOB_TYPE;
typedef deque<BLOB_TYPE>    BLOBS_TYPE;



class BlobNoteEvent : public ofEventArgs
{
public:
    const BLOB_TYPE     &blobPtr;
    const int           channel;
    
    BlobNoteEvent(const BLOB_TYPE& blobPtr, const int channel)
    : blobPtr(blobPtr)
    , channel(channel)
    {}
};
