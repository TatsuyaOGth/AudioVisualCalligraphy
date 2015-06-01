#pragma once

#include "ofMain.h"
#include "utils.h"
#include "ofxOpenCv.h"
#include "ofxAnimationPrimitives.h"
#include "MidiSenderController.hpp"
#include "MIdiReceiverController.hpp"

typedef ofxCvBlob           BLOB_TYPE;
typedef deque<BLOB_TYPE>    BLOBS_TYPE;

class BlobsDataController;

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
    public:
        BlobDrawr(const BLOB_TYPE& blob) : mBlob(blob) {}
        void draw()
        {
            ofSetColor(255, getLife() * 255);
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
    Sequencer() : bPlaying(true), mTempo(120), mTime(5) {}
    
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
        sequencerAnimation::manager.createInstance<sequencerAnimation::BlobDrawr>(blob)->play(0.5);
    }
};

class VerticalSequencer : public Sequencer
{
    float mPos, mLastPos, mLoopTime;
    
public:
    VerticalSequencer()
    {
        mPos = mLastPos = 0;
        mLoopTime = 60. / mTempo * mTime;
    }
    
    void update(float tick)
    {
        mPos += tick;
        if (mPos > mLoopTime)
        {
            mPos = 0;
        }
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
                Sequencer::sendNote(e, 1);
            }
        }
        mLastPos = mPos;
    }
    
    void draw(int x, int y, int w, int h)
    {
        ofPushStyle();
        ofSetColor(0, 255, 255);
        ofSetLineWidth(1);
        float posY = ofMap(mPos, 0, mLoopTime, y, y + h);
        ofLine(x, posY, x + w, posY);
        ofPopStyle();
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
    
    VerticalSequencer* mVertSeq;
    vector<Sequencer*> mSeq;
    
public:
    BlobsDataController()
    {
        mWidth = 0;
        mHeight = 0;
        setupMidi();
        mSeq.push_back(mVertSeq = new VerticalSequencer());
    }
    
    void setupMidi()
    {
        MIDI_SENDER->listPorts();
        MIDI_SENDER->openPort(MIDI_SENDER_PORT_NAME);
        MIDI_RECEIVER->openPort(MIDI_RECEIVER_PORT_NAME);
        ofAddListener(MIDI_RECEIVER->receivedMidiEvent, this, &BlobsDataController::receivedMidiMessage);
    }
    
    void update()
    {
        const float tick = ofGetLastFrameTime();
        for (auto& e : mSeq)
        {
            e->setSize(mWidth, mHeight);
            e->update(tick);
            e->emit(mBlobs);
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
