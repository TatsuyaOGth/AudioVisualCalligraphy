#pragma once

#include "ofMain.h"
#include "utils.h"
#include "ofxOpenCv.h"
#include "ofxAnimationPrimitives.h"
#include "MidiSenderController.hpp"
#include "MIdiReceiverController.hpp"


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
    Blob(ofxCvBlob& blob)
    {
        this->area          = blob.area;
        this->length        = blob.length;
        this->hole          = blob.hole;
        this->pts           = blob.pts;
        this->nPts          = blob.nPts;
        this->boundingRect  = blob.boundingRect;
        this->centroid      = blob.centroid;
    }
    
    void addOffsetX(float offsetX)
    {
        for (auto& e : pts)
        {
            e.x += offsetX;
        }
        centroid.x += offsetX;
        boundingRect.translate(offsetX, 0);
    }
    
    void draw(float x = 0, float y = 0)
    {
        ofNoFill();
        ofSetHexColor(0x00FFFF);
        ofBeginShape();
        for (int i = 0; i < nPts; i++){
            ofVertex(x + pts[i].x, y + pts[i].y);
        }
        ofEndShape(true);
        ofSetHexColor(0xff0099);
        ofRect(x + boundingRect.x, y + boundingRect.y, boundingRect.width, boundingRect.height);
    }
};


typedef Blob                BLOB_TYPE;
typedef deque<BLOB_TYPE>    BLOBS_TYPE;

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  SEQUENCER
//
/////////////////////////////////////////////////////////////////////////////////////////////////
namespace sequencerAnimation
{
    class BlobDrawr : public ofxAnimationPrimitives::Instance
    {
        const BLOB_TYPE& mBlob;
        ofColor mCol;
    public:
        BlobDrawr(const BLOB_TYPE& blob, ofColor col) : mBlob(blob), mCol(col) {}
        void draw()
        {
            ofSetColor(mCol, getLife() * 255);
            ofFill();
            ofBeginShape();
            for (const auto& p : mBlob.pts)
            {
                ofVertex(p.x, p.y);
            }
            ofEndShape();
        }
    };
    
    static ofxAnimationPrimitives::InstanceManager manager;
}


class Sequencer
{
protected:
    bool    bPlaying;
    float   mTempo;
    int     mTime;
    float   mWidth, mHeight;

public:
    Sequencer() : bPlaying(false), mTempo(120), mTime(5) {}
    
    virtual void setup(){};
    virtual void update(float tick){};
    virtual void emit(const BLOBS_TYPE& blobs){};
    virtual void draw(int x, int y, int w, int h){}
    
    void play(){ bPlaying = true; }
    void stop(){ bPlaying = false; }
    void togglePlay(){ bPlaying ? stop() : play(); }
    bool isPlaying(){ return bPlaying; }
    void setSequencer(float tempo, int time){ mTempo = tempo, mTime = time; }
    void setSize(float w, float h){ mWidth = w, mHeight = h; }
    
    
    // send midi messages
    
    static void sendNote(const BLOB_TYPE& blob, int channel)
    {
        int note = ofMap(blob.area, 10*10, 100*100, 86, 32, true);
        int velo = ofRandom(90, 110);
        MIDI_SENDER->makeNote(note, velo, channel, 1);
    }
};

class VerticalSequencer : public Sequencer
{
    float mPos, mLastPos, mLoopTime;
    int mChannel;
    ofColor mCol;
    
public:
    VerticalSequencer(float loopTime, int channel, ofColor col)
    {
        mChannel = channel;
        mCol.set(col);
        mLoopTime = loopTime;
        setup();
    }
    
    void setup()
    {
        mPos = mLastPos = 0;
    }
    
    void update(float tick)
    {
        mLastPos = mPos;
        if (mLastPos > mLoopTime)
        {
            mPos = 0;
            mLastPos = 0;
        }
        mPos += tick;
    }
    
    void emit(const BLOBS_TYPE& blobs)
    {
        float y1 = ofMap(mLastPos, 0, mLoopTime, 0, mHeight);
        float y2 = ofMap(mPos,     0, mLoopTime, 0, mHeight);
        for (const auto& e : blobs)
        {
            const ofPoint& pos = e.centroid;
            if (pos.y > y1 && pos.y <= y2)
            {
                Sequencer::sendNote(e, mChannel);
                sequencerAnimation::manager.createInstance<sequencerAnimation::BlobDrawr>(e, mCol)->play(0.5);
            }
        }
    }
    
    void draw(int x, int y, int w, int h)
    {
        ofPushStyle();
        ofSetColor(mCol, 127);
        ofFill();
        ofSetLineWidth(1);
        float posY1 = ofMap(mLastPos, 0, mLoopTime, y, y + h);
        float posY2 = ofMap(mPos, 0, mLoopTime, y, y + h);
        ofRect(x, posY1, x + w, posY2 - posY1);
        ofPopStyle();
    }
};

class OrdinalSequencer : public Sequencer
{
    float mCount;
    float mDurationToNext;
    int mCurrentIndex;
    bool bPlay;
    float mMaxDurationToNext;
    int mChannel;
    ofColor mCol;
    ofPoint mLastPos, mTargetPos;
    
public:
    OrdinalSequencer(float maxDurationToNext, int channel, ofColor col)
    {
        mMaxDurationToNext = maxDurationToNext;
        mChannel = channel;
        mCol.set(col);
        setup();
    }
    
    void setup()
    {
        mCurrentIndex = 0;
        mCount = 0;
        mDurationToNext = 0;
        bPlay = true;
        mLastPos.set(0, 0);
        mTargetPos.set(0, 0);
    }
    
    void update(float tick)
    {
        if (!bPlay)
        {
            mCount += tick;
            if (mDurationToNext < mCount)
            {
                bPlay = true;
            }
        }
    }
    
    void emit(const BLOBS_TYPE& blobs)
    {
        if (bPlay)
        {
            if (blobs.empty() || mCurrentIndex >= blobs.size()) return;
            Sequencer::sendNote(blobs[mCurrentIndex], mChannel);
            sequencerAnimation::manager.createInstance<sequencerAnimation::BlobDrawr>(blobs[mCurrentIndex], mCol)->play(0.5);
            
            // set duration to next
            if (mCurrentIndex + 1 >= blobs.size())
            {
                setup();
                bPlaying = false;
            }
            else {
                mLastPos.set(blobs[mCurrentIndex].centroid);
                mTargetPos.set(blobs[mCurrentIndex + 1].centroid);
                float dist = ofDist(blobs[mCurrentIndex  ].centroid.x, blobs[mCurrentIndex  ].centroid.y,
                                    blobs[mCurrentIndex+1].centroid.x, blobs[mCurrentIndex+1].centroid.y);
                mDurationToNext = (MIN(dist, 100) / 100) * mMaxDurationToNext;
            }
            
            // reset
            bPlay = false;
            mCurrentIndex++;
            mCount = 0;
        }
    }
    
    void draw(int x, int y, int w, int h)
    {
        if (mLastPos.match(mTargetPos) || mDurationToNext == 0) return;
        float scalex, scaley;
        if( mWidth != 0 ) { scalex = w/mWidth; } else { scalex = 1.0f; }
        if( mHeight != 0 ) { scaley = h/mHeight; } else { scaley = 1.0f; }
        ofPushMatrix();
        ofTranslate(x, y);
        ofScale(scalex, scaley);
        ofSetColor(mCol, 127);
        ofFill();
        ofCircle(mLastPos.interpolate(mTargetPos, ofMap(mCount, 0, mDurationToNext, 0, 1)), 3);
        ofPopMatrix();
    }

};


/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  BLOBS DATA CONTROLLER
//
/////////////////////////////////////////////////////////////////////////////////////////////////

class BlobsDataController
{
    BLOBS_TYPE mBlobs;
    float mWidth, mHeight;
    
    VerticalSequencer*  mVertSeq;
    OrdinalSequencer*   mOrdinalSeq;
    vector<Sequencer*> mSeq;
    
public:
    BlobsDataController()
    {
        mWidth = 0;
        mHeight = 0;
        mSeq.push_back(mVertSeq = new VerticalSequencer(2.5, 1, ofColor(0, 255, 255)));
        mSeq.push_back(mVertSeq = new VerticalSequencer(2.5/4, 2, ofColor(255, 0, 255)));
        mSeq.push_back(mOrdinalSeq = new OrdinalSequencer(1, 9, ofColor(255, 255, 0)));
    }
    
    void setupMidi(const string& senderPoitName, const string& receiverPortName)
    {
        MIDI_SENDER->listPorts();
        MIDI_SENDER->openPort(senderPoitName);
        MIDI_RECEIVER->openPort(receiverPortName);
        ofAddListener(MIDI_RECEIVER->receivedMidiEvent, this, &BlobsDataController::receivedMidiMessage);
    }
    
    void update()
    {
        const float tick = ofGetLastFrameTime();
        for (auto& e : mSeq)
        {
            if (e->isPlaying())
            {
                e->setSize(mWidth, mHeight);
                e->update(tick);
                e->emit(mBlobs);
            }
        }
        sequencerAnimation::manager.update();
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
        for( int i=0; i<(int)mBlobs.size(); i++ )
        {
            ofRect( mBlobs[i].boundingRect.x, mBlobs[i].boundingRect.y,
                   mBlobs[i].boundingRect.width, mBlobs[i].boundingRect.height );
        }
        
        // ---------------------------- draw the blobs
        ofSetHexColor(0x00FFFF);
        
        for( int i=0; i<(int)mBlobs.size(); i++ )
        {
            ofNoFill();
            ofBeginShape();
            for( int j=0; j<mBlobs[i].nPts; j++ )
            {
                ofVertex( mBlobs[i].pts[j].x, mBlobs[i].pts[j].y );
            }
            ofEndShape();
            
        }
        
        sequencerAnimation::manager.draw();
        
        glPopMatrix();
        
        // ------------------------------ draw sequencer
        for (auto& e : mSeq)
        {
            e->draw(x, y, w, h);
        }
        
        ofPopStyle();
    }
    
    void receivedMidiMessage(ofxMidiMessage & e)
    {
        if (e.status == MIDI_NOTE_ON)
        {
            
            //MIDI_SENDER->makeNote(64, 100, 1, 20);
        }
    }
    
    void makeNoteRandom(int channel)
    {
        int shuffle = ofRandom(mBlobs.size());
        int note = ofMap(mBlobs[shuffle].area, 10*10, 100*100, 86, 32, true);
        int velo = ofRandom(90, 110);
        MIDI_SENDER->makeNote(note, velo, channel, 1);
    }
    
    void sequencerPlay(int sequencerIndex)
    {
        if (sequencerIndex < 0 || sequencerIndex >= mSeq.size()) return;
        mSeq[sequencerIndex]->setup();
        mSeq[sequencerIndex]->play();
    }
    
    void sequencerStop(int sequencerIndex)
    {
        if (sequencerIndex < 0 || sequencerIndex >= mSeq.size()) return;
        mSeq[sequencerIndex]->setup();
        mSeq[sequencerIndex]->stop();
    }
    
    void sequencerTogglePlay(int sequencerIndex)
    {
        if (sequencerIndex < 0 || sequencerIndex >= mSeq.size()) return;
        mSeq[sequencerIndex]->setup();
        mSeq[sequencerIndex]->togglePlay();
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
    
    void addBlob(BLOB_TYPE& blob)
    {
        mBlobs.push_back(blob);
    }
    
    void addBlob(ofxCvBlob& cvBlob)
    {
        mBlobs.push_back(Blob(cvBlob));
    }
    
    void addBlob(ofxCvBlob& cvBlob, float offsetX)
    {
        mBlobs.push_back(Blob(cvBlob));
        mBlobs.back().addOffsetX(offsetX);
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
