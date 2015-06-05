#pragma once

#include "ofMain.h"
#include "utils.h"
#include "ofxOpenCv.h"
#include "Blob.h"
#include "VisualBlobs.hpp"
#include "ofxAnimationPrimitives.h"
#include "MidiSenderController.hpp"
#include "MIdiReceiverController.hpp"

class BlobsDataController;

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  SEQUENCER
//
/////////////////////////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------------------------
namespace sequencerAnimation
{
    class BlobDrawr : public ofxAnimationPrimitives::Instance
    {
        const BLOB_TYPE  *mBlob;
        ofColor mCol;
    public:
        BlobDrawr(const BLOB_TYPE *blob, ofColor col) : mBlob(blob), mCol(col) {}
        void draw()
        {
            // FIXME: tessaration bug
//            assert(mBlob != NULL);
//            if (mBlob == NULL) return;
//            ofSetColor(mCol, getLife() * 255);
//            ofFill();
//            ofBeginShape();
//            for (const auto& p : mBlob->pts)
//            {
//                // TODO: define width and height
//                ofVertex(p.x * ofGetWidth(), p.y * ofGetHeight() * 0.5);
//            }
//            ofEndShape();
        }
    };
    
    static ofxAnimationPrimitives::InstanceManager manager;
}

//-----------------------------------------------------------------------------------------------
class Sequencer
{
    friend class BlobsDataController;
    
protected:
    bool    bPlaying;
    float   mTempo;
    int     mTime;
    float   mWidth, mHeight;
    
    ofEvent<BlobNoteEvent> mBlobNoteEvent;

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
    
    static void sendNote(const BLOB_TYPE& blob, float duration, int channel)
    {
        int note = ofMap(blob.area, 0, 0.01, 64, 24, true);
        int velo = ofRandom(90, 110);
        // option
        int pan  = ofMap(blob.centroid.x, 0, 1, 0, 127, true);
        int area = ofMap(blob.area, 0, 0.01, 0, 127, true);
        int length = ofMap(blob.length, 0, 200, 1, 16, true);
        
//        LOG_DEBUG << length;
        
        MIDI_SENDER->makeNote(note, velo, channel, duration * length);
        MIDI_SENDER->ctlOut(10, pan, channel);
        MIDI_SENDER->ctlOut(102, area, channel);
//        MIDI_SENDER->ctlOut(103, length, channel);
    }
};

//-----------------------------------------------------------------------------------------------
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
            if (e.hole) continue;
            const ofPoint& pos = e.centroid;
            if (pos.y > y1 && pos.y <= y2)
            {
                Sequencer::sendNote(e, 0.5, mChannel);
                sequencerAnimation::manager.createInstance<sequencerAnimation::BlobDrawr>(&e, mCol)->play(0.5);
                BlobNoteEvent event(&e, mChannel);
                ofNotifyEvent(mBlobNoteEvent, event, this);
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

//-----------------------------------------------------------------------------------------------
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
    bool bLoop;
    
public:
    OrdinalSequencer(float maxDurationToNext, bool loop, int channel, ofColor col)
    {
        mMaxDurationToNext = maxDurationToNext;
        mChannel = channel;
        mCol.set(col);
        bLoop = loop;
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
            // send midi
            Sequencer::sendNote(blobs[mCurrentIndex], mMaxDurationToNext, mChannel);
            sequencerAnimation::manager.createInstance<sequencerAnimation::BlobDrawr>(&blobs[mCurrentIndex], mCol)->play(0.5);
            // notify event
            BlobNoteEvent event(&blobs[mCurrentIndex], mChannel);
            ofNotifyEvent(mBlobNoteEvent, event, this);
            
            // set duration to next
            if (mCurrentIndex + 1 >= blobs.size())
            {
                if (bLoop)
                {
                    // loop
                    setup();
                    mDurationToNext = mMaxDurationToNext;
                    bPlay = false;
                    return;
                }
                else {
                    // stop sequence
                    setup();
                    bPlaying = false;
                    return;
                }
            }
            else {
                mLastPos.set(blobs[mCurrentIndex].centroid);
                mTargetPos.set(blobs[mCurrentIndex + 1].centroid);
                float dist = ofDist(blobs[mCurrentIndex  ].centroid.x, blobs[mCurrentIndex  ].centroid.y,
                                    blobs[mCurrentIndex+1].centroid.x, blobs[mCurrentIndex+1].centroid.y);
                mDurationToNext = mMaxDurationToNext;
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
        ofPushMatrix();
        ofTranslate(x, y);
        ofSetColor(mCol, 127);
        ofFill();
        ofVec2f pos = mLastPos.interpolate(mTargetPos, ofMap(mCount, 0, mDurationToNext, 0, 1));
        ofCircle(pos.x * w, pos.y * h, 5);
        ofPopMatrix();
    }
};

//-----------------------------------------------------------------------------------------------
class RandomSequencer : public Sequencer
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
    RandomSequencer(float maxDurationToNext, int channel, ofColor col)
    {
        mMaxDurationToNext = maxDurationToNext;
        mChannel = channel;
        mCol.set(col);
        setup();
    }
    
    void setup()
    {
        mCount = 0;
        mCurrentIndex = 0;
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
            int nextIndex = ofRandom(blobs.size());
            if (blobs.empty() || mCurrentIndex >= blobs.size()) return;
            
            // send midi
            Sequencer::sendNote(blobs[mCurrentIndex], mMaxDurationToNext, mChannel);
            sequencerAnimation::manager.createInstance<sequencerAnimation::BlobDrawr>(&blobs[mCurrentIndex], mCol)->play(0.5);
            
            // notify event
            BlobNoteEvent event(&blobs[mCurrentIndex], mChannel);
            ofNotifyEvent(mBlobNoteEvent, event, this);
            
            // set duration to next
            mLastPos.set(blobs[mCurrentIndex].centroid);
            mTargetPos.set(blobs[nextIndex].centroid);
            float dist = ofDist(blobs[mCurrentIndex  ].centroid.x, blobs[mCurrentIndex  ].centroid.y,
                                blobs[nextIndex].centroid.x, blobs[nextIndex].centroid.y);
            mDurationToNext = mMaxDurationToNext;
            
            // reset
            bPlay = false;
            mCurrentIndex = nextIndex;
            mCount = 0;
        }
    }
    
    void draw(int x, int y, int w, int h)
    {
        if (mLastPos.match(mTargetPos) || mDurationToNext == 0) return;
        ofPushMatrix();
        ofTranslate(x, y);
        ofSetColor(mCol, 127);
        ofFill();
        ofVec2f pos = mLastPos.interpolate(mTargetPos, ofMap(mCount, 0, mDurationToNext, 0, 1));
        ofCircle(pos.x * w, pos.y * h, 5);
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
    
    VerticalSequencer*  mVertSeq;
    OrdinalSequencer*   mOrdinalSeq;
    vector<Sequencer*> mSeq;
    
public:
    ofEvent<BlobNoteEvent> mBlobNoteEvent;
    
public:
    BlobsDataController()
    {
        mSeq.push_back(new VerticalSequencer(4, 1, ofColor(0, 255, 255)));
        mSeq.push_back(new VerticalSequencer(1, 2, ofColor(255, 0, 255)));
        mSeq.push_back(new OrdinalSequencer(0.25, true, 3, ofColor(127, 255, 0)));
        mSeq.push_back(new OrdinalSequencer(1.00, true, 4, ofColor(255, 127, 0)));
        mSeq.push_back(new RandomSequencer(0.125, 5, ofColor(255, 127, 255)));
        mSeq.push_back(new OrdinalSequencer(2.00, true, 6, ofColor(0, 0, 255)));
        
        mSeq.push_back(new OrdinalSequencer(0.50, false, 9, ofColor(255, 255, 0)));
        
        for (auto& e : mSeq)
        {
            ofAddListener(e->mBlobNoteEvent, this, &BlobsDataController::sequencerCallback);
        }
    }
    
    void setupMidi(const string& senderPoitName, const string& receiverPortName)
    {
        MIDI_SENDER->listPorts();
        MIDI_SENDER->openPort(senderPoitName);
        MIDI_RECEIVER->openPort(receiverPortName);
//        ofAddListener(MIDI_RECEIVER->receivedMidiEvent, this, &BlobsDataController::receivedMidiMessage);
    }
    
    void update()
    {
        const float tick = ofGetLastFrameTime();
        for (auto& e : mSeq)
        {
            if (e->isPlaying())
            {
                e->setSize(1, 1);
                e->update(tick);
                e->emit(mBlobs);
            }
        }
        sequencerAnimation::manager.update();
    }
    
    void draw(int x, int y, int w, int h)
    {
        ofPushStyle();
        ofSetColor(255, 0, 0);
        ofPushMatrix();
        ofTranslate(x, y);
        
        ofNoFill();
        for( int i=0; i<(int)mBlobs.size(); i++ )
        {
            ofRect(mBlobs[i].boundingRect.x * w,
                   mBlobs[i].boundingRect.y * h,
                   mBlobs[i].boundingRect.width * w,
                   mBlobs[i].boundingRect.height * h);
        }
        
        
        for( int i=0; i<(int)mBlobs.size(); i++ )
        {
            ofNoFill();
            mBlobs[i].hole ? ofSetColor(0, 0, 255) : ofSetColor(0, 255, 0);
            ofBeginShape();
            for( int j=0; j<mBlobs[i].nPts; j++ )
            {
                ofVertex( mBlobs[i].pts[j].x * w, mBlobs[i].pts[j].y * h );
            }
            ofEndShape();
            
        }
        
        sequencerAnimation::manager.draw();
        
        ofPopMatrix();
        
        for (auto& e : mSeq)
        {
            e->draw(x, y, w, h);
        }
        
        ofPopStyle();
    }
    
    void sequencerCallback(BlobNoteEvent& e)
    {
        ofNotifyEvent(mBlobNoteEvent, e, this);
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
    
    void addBlob(ofxCvBlob& cvBlob, float w, float h, float offsetW)
    {
        mBlobs.push_back(Blob(cvBlob, w, h, offsetW));
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
