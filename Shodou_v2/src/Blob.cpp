#include "Blob.h"


Blob::Blob(const ofxCvBlob& blob, float w, float h, float offsetW)
{
    setup(blob, w, h, offsetW);
}

Blob::Blob(const Blob* o)
{
    setup(*o, o->width, o->height, o->offsetW);
}

void Blob::setup(const ofxCvBlob& blob, float w, float h, float offsetW)
{
    ofxCvBlob::hole          = blob.hole;
    ofxCvBlob::nPts          = blob.nPts;
    ofxCvBlob::length        = blob.length;
    
    // set value with normalize
    for (const auto& e : blob.pts)
    {
        ofxCvBlob::pts.push_back(ofPoint((e.x + offsetW) / w, (e.y) / h));
    }
    ofxCvBlob::boundingRect.setX((blob.boundingRect.getX() + offsetW) / w);
    ofxCvBlob::boundingRect.setY(blob.boundingRect.getY() / h);
    ofxCvBlob::boundingRect.setWidth(blob.boundingRect.getWidth() / w);
    ofxCvBlob::boundingRect.setHeight(blob.boundingRect.getHeight() / h);
    ofxCvBlob::centroid = ofPoint((blob.centroid.x + offsetW) / w, blob.centroid.y / h);
    ofxCvBlob::area = blob.area / (w * h);
    this->width = w;
    this->height = h;
    this->offsetW = offsetW;
}

void Blob::draw(float x, float y)
{
    ofNoFill();
    ofSetColor(255, 255, 0);
    ofBeginShape();
    for (int i = 0; i < nPts; i++)
    {
        ofVertex(x + pts[i].x, y + pts[i].y);
    }
    ofEndShape(true);
    ofSetHexColor(0xff0099);
    ofRect(x + boundingRect.x, y + boundingRect.y, boundingRect.width, boundingRect.height);
}
