#pragma once

#include "ofMain.h"
#include "utils.h"
#include "ofxOpenCv.h"
#include "Blob.h"
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
        int note = ofMap(blob->area, 0, 0.01, 64, 24, true);
        int velo = ofRandom(90, 110);
        // option
        int pan  = ofMap(blob->centroid.x, 0, 1, 0, 127, true);
        int area = ofMap(blob->area, 0, 0.01, 0, 127, true);
                
        MIDI_SENDER->makeNote(note, velo, channel, duration);
        MIDI_SENDER->ctlOut(10, pan, channel);
        MIDI_SENDER->ctlOut(102, area, channel);
        //MIDI_SENDER->ctlOut(103, length, channel);
    }
    
    static void sendNote(int note, int velo, float duration, int channel, int pan = -1)
    {
        MIDI_SENDER->makeNote(note, velo, channel, duration);
        if (pan != -1)
        {
            MIDI_SENDER->ctlOut(10, pan, channel);
        }
    }
};

//-----------------------------------------------------------------------------------------------
class VerticalSequencer : public Sequencer
{
    float mPos, mLastPos, mLoopTime;
    int mChannel;
    ofColor mCol;
    
public:
    VerticalSequencer(float loopTime, int channel, ofColor col);
    void setup();
    void update(float tick);
    void emit(const BLOBS_TYPE& blobs);
    void draw(int x, int y, int w, int h);
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
    OrdinalSequencer(float maxDurationToNext, bool loop, int channel, ofColor col);
    void setup();
    void update(float tick);
    void emit(const BLOBS_TYPE& blobs);
    void draw(int x, int y, int w, int h);
};

//-----------------------------------------------------------------------------------------------
class RandomSequencer : public Sequencer
{
    float mCount;
    float mDurationToNext;
    int mCurrentIndex;
    bool bPlay;
    bool bLoop;
    float mMaxDurationToNext;
    int mChannel;
    ofColor mCol;
    ofPoint mLastPos, mTargetPos;
    
public:
    RandomSequencer(float maxDurationToNext, bool loop, int channel, ofColor col);
    void setup();
    void update(float tick);
    void emit(const BLOBS_TYPE& blobs);
    void draw(int x, int y, int w, int h);
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
    BlobsDataController();
    
    void setupMidi(const string& senderPoitName, const string& receiverPortName);
    void update();
    void draw(int x, int y, int w, int h);
    void sequencerCallback(BlobNoteEvent& e);
    void sequencerPlay(int sequencerIndex);
    void sequencerStop(int sequencerIndex);
    void sequencerTogglePlay(int sequencerIndex);
    void addBlob(ofxCvBlob& cvBlob, float w, float h, float offsetW);
    void removeBlob();
    void clearBlobs();
    const BLOBS_TYPE& getBlobsRef() const;
    
    void drawSeq(int index, int x, int y, int w, int h);
    void drawSeqAll(int x, int y, int w, int h);
    
    string getSequencerInfomationText();
};
