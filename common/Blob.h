#pragma once

#include "ofxCvContourFinder.h"

class Blob
{
public:
    float               area;
    float               length;
    ofRectangle         boundingRect;
    ofPoint             centroid;
    bool                hole;
    
    vector <ofPoint>    pts;    // the contour of the blob
    int                 nPts;   // number of pts;
    
    
public:
    Blob(ofxCvBlob& blob, float w, float h, float offsetW = 0)
    {
        this->hole          = blob.hole;
        this->nPts          = blob.nPts;
        this->length        = blob.length;
        
        // set value with normalize
        for (const auto& e : blob.pts)
        {
            this->pts.push_back(ofPoint((e.x + offsetW) / w, (e.y) / h));
        }
        this->boundingRect.setX((blob.boundingRect.getX() + offsetW) / w);
        this->boundingRect.setY(blob.boundingRect.getY() / h);
        this->boundingRect.setWidth(blob.boundingRect.getWidth() / w);
        this->boundingRect.setHeight(blob.boundingRect.getHeight() / h);
        this->centroid = ofPoint((blob.centroid.x + offsetW) / w, blob.centroid.y / h);
        this->area = blob.area / (w * h);
    }
};

typedef Blob                BLOB_TYPE;
typedef deque<BLOB_TYPE>    BLOBS_TYPE;



class BlobNoteEvent : public ofEventArgs
{
public:
    const BLOB_TYPE     *blobPtr;
    const int           channel;
    
    BlobNoteEvent(const BLOB_TYPE* blobPtr, const int channel)
    : blobPtr(blobPtr)
    , channel(channel)
    {}
};
