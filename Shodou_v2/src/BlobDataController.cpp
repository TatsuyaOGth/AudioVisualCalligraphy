#include "BlobDataController.h"

VerticalSequencer::VerticalSequencer(float loopTime, int channel, ofColor col)
{
    mChannel = channel;
    mCol.set(col);
    mLoopTime = loopTime;
    setup();
}

void VerticalSequencer::setup()
{
    mPos = mLastPos = 0;
}

void VerticalSequencer::update(float tick)
{
    mLastPos = mPos;
    if (mLastPos > mLoopTime)
    {
        mPos = 0;
        mLastPos = 0;
    }
    mPos += tick;
}

void VerticalSequencer::emit(const BLOBS_TYPE& blobs)
{
    float y1 = ofMap(mLastPos, 0, mLoopTime, 0, mHeight);
    float y2 = ofMap(mPos,     0, mLoopTime, 0, mHeight);
    for (const auto& e : blobs)
    {
        if (e->hole) continue;
        const ofPoint& pos = e->centroid;
        if (pos.y > y1 && pos.y <= y2)
        {
            Sequencer::sendNote(e, 0.5, mChannel);
            sequencerAnimation::manager.createInstance<sequencerAnimation::BlobDrawr>(&e, mCol)->play(0.5);
            BlobNoteEvent event(e, mChannel);
            ofNotifyEvent(mBlobNoteEvent, event, this);
        }
    }
}

void VerticalSequencer::draw(int x, int y, int w, int h)
{
    ofPushStyle();
    ofSetColor(mCol);
    ofFill();
    ofSetLineWidth(10);
    float posY1 = ofMap(mLastPos, 0, mLoopTime, y, y + h);
    float posY2 = ofMap(mPos, 0, mLoopTime, y, y + h);
    ofRect(x, posY1, x + w, posY2 - posY1);
    ofPopStyle();
}

//-----------------------------------------------------------------------------------------------

OrdinalSequencer::OrdinalSequencer(float maxDurationToNext, bool loop, int channel, ofColor col)
{
    mMaxDurationToNext = maxDurationToNext;
    mChannel = channel;
    mCol.set(col);
    bLoop = loop;
    setup();
}

void OrdinalSequencer::setup()
{
    mCurrentIndex = 0;
    mCount = 0;
    mDurationToNext = 0;
    bPlay = true;
    mLastPos.set(0, 0);
    mTargetPos.set(0, 0);
}

void OrdinalSequencer::update(float tick)
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

void OrdinalSequencer::emit(const BLOBS_TYPE& blobs)
{
    if (bPlay)
    {
        if (blobs.empty() || mCurrentIndex >= blobs.size()) return;
        // send midi
        Sequencer::sendNote(blobs[mCurrentIndex], mMaxDurationToNext, mChannel);
        sequencerAnimation::manager.createInstance<sequencerAnimation::BlobDrawr>(&blobs[mCurrentIndex], mCol)->play(0.5);
        // notify event
        BlobNoteEvent event(blobs[mCurrentIndex], mChannel);
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
            mLastPos.set(blobs[mCurrentIndex]->centroid);
            mTargetPos.set(blobs[mCurrentIndex + 1]->centroid);
            float dist = ofDist(blobs[mCurrentIndex  ]->centroid.x, blobs[mCurrentIndex  ]->centroid.y,
                                blobs[mCurrentIndex+1]->centroid.x, blobs[mCurrentIndex+1]->centroid.y);
            mDurationToNext = mMaxDurationToNext;
        }
        
        // reset
        bPlay = false;
        mCurrentIndex++;
        mCount = 0;
    }
}

void OrdinalSequencer::draw(int x, int y, int w, int h)
{
    if (mLastPos.match(mTargetPos) || mDurationToNext == 0) return;
    ofPushMatrix();
    ofTranslate(x, y);
    ofSetColor(mCol);
    ofFill();
    ofVec2f pos = mLastPos.interpolate(mTargetPos, ofMap(mCount, 0, mDurationToNext, 0, 1));
    ofCircle(pos.x * w, pos.y * h, 5);
    ofPopMatrix();
}


//-----------------------------------------------------------------------------------------------

RandomSequencer::RandomSequencer(float maxDurationToNext, int channel, ofColor col)
{
    mMaxDurationToNext = maxDurationToNext;
    mChannel = channel;
    mCol.set(col);
    setup();
}

void RandomSequencer::setup()
{
    mCount = 0;
    mCurrentIndex = 0;
    mDurationToNext = 0;
    bPlay = true;
    mLastPos.set(0, 0);
    mTargetPos.set(0, 0);
}

void RandomSequencer::update(float tick)
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

void RandomSequencer::emit(const BLOBS_TYPE& blobs)
{
    if (bPlay)
    {
        int nextIndex = ofRandom(blobs.size());
        if (blobs.empty() || mCurrentIndex >= blobs.size()) return;
        
        // send midi
        Sequencer::sendNote(blobs[mCurrentIndex], mMaxDurationToNext, mChannel);
        sequencerAnimation::manager.createInstance<sequencerAnimation::BlobDrawr>(&blobs[mCurrentIndex], mCol)->play(0.5);
        
        // notify event
        BlobNoteEvent event(blobs[mCurrentIndex], mChannel);
        ofNotifyEvent(mBlobNoteEvent, event, this);
        
        // set duration to next
        mLastPos.set(blobs[mCurrentIndex]->centroid);
        mTargetPos.set(blobs[nextIndex]->centroid);
        float dist = ofDist(blobs[mCurrentIndex  ]->centroid.x, blobs[mCurrentIndex  ]->centroid.y,
                            blobs[nextIndex]->centroid.x, blobs[nextIndex]->centroid.y);
        mDurationToNext = mMaxDurationToNext;
        
        // reset
        bPlay = false;
        mCurrentIndex = nextIndex;
        mCount = 0;
    }
}

void RandomSequencer::draw(int x, int y, int w, int h)
{
    if (mLastPos.match(mTargetPos) || mDurationToNext == 0) return;
    ofPushMatrix();
    ofTranslate(x, y);
    ofSetColor(mCol);
    ofFill();
    ofVec2f pos = mLastPos.interpolate(mTargetPos, ofMap(mCount, 0, mDurationToNext, 0, 1));
    ofCircle(pos.x * w, pos.y * h, 5);
    ofPopMatrix();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  BLOBS DATA CONTROLLER
//
/////////////////////////////////////////////////////////////////////////////////////////////////

BlobsDataController::BlobsDataController()
{
    float pct = 1.8;
    mSeq.push_back(new VerticalSequencer(4 * pct, 1, ofColor(0, 255, 255)));
    mSeq.push_back(new VerticalSequencer(2 * pct, 2, ofColor(255, 0, 255)));
    mSeq.push_back(new OrdinalSequencer(0.25  * pct, true, 3, ofColor(127, 255, 0)));
    mSeq.push_back(new OrdinalSequencer(1.00 * pct, true, 4, ofColor(255, 127, 0)));
    mSeq.push_back(new RandomSequencer(0.125 * pct, 5, ofColor(255, 127, 255)));
    mSeq.push_back(new OrdinalSequencer(2.00 * pct, true, 6, ofColor(0, 0, 255)));
    
    mSeq.push_back(new OrdinalSequencer(0.50, false, 9, ofColor(255, 255, 0)));
    
    for (auto& e : mSeq)
    {
        ofAddListener(e->mBlobNoteEvent, this, &BlobsDataController::sequencerCallback);
    }
}

void BlobsDataController::setupMidi(const string& senderPoitName, const string& receiverPortName)
{
    MIDI_SENDER->listPorts();
    MIDI_SENDER->openPort(senderPoitName);
    MIDI_RECEIVER->openPort(receiverPortName);
}

void BlobsDataController::update()
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

void BlobsDataController::draw(int x, int y, int w, int h)
{
    ofPushStyle();
    ofSetColor(255, 0, 0);
    ofPushMatrix();
    ofTranslate(x, y);
    
    ofNoFill();
    for( int i=0; i<(int)mBlobs.size(); i++ )
    {
        ofRect(mBlobs[i]->boundingRect.x * w,
               mBlobs[i]->boundingRect.y * h,
               mBlobs[i]->boundingRect.width * w,
               mBlobs[i]->boundingRect.height * h);
    }
    
    
    for( int i=0; i<(int)mBlobs.size(); i++ )
    {
        ofNoFill();
        mBlobs[i]->hole ? ofSetColor(0, 0, 255) : ofSetColor(0, 255, 0);
        ofBeginShape();
        for( int j=0; j<mBlobs[i]->nPts; j++ )
        {
            ofVertex( mBlobs[i]->pts[j].x * w, mBlobs[i]->pts[j].y * h );
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

void BlobsDataController::sequencerCallback(BlobNoteEvent& e)
{
    ofNotifyEvent(mBlobNoteEvent, e, this);
}

void BlobsDataController::sequencerPlay(int sequencerIndex)
{
    if (sequencerIndex < 0 || sequencerIndex >= mSeq.size()) return;
    mSeq[sequencerIndex]->setup();
    mSeq[sequencerIndex]->play();
}

void BlobsDataController::sequencerStop(int sequencerIndex)
{
    if (sequencerIndex < 0 || sequencerIndex >= mSeq.size()) return;
    mSeq[sequencerIndex]->setup();
    mSeq[sequencerIndex]->stop();
}

void BlobsDataController::sequencerTogglePlay(int sequencerIndex)
{
    if (sequencerIndex < 0 || sequencerIndex >= mSeq.size()) return;
    mSeq[sequencerIndex]->setup();
    mSeq[sequencerIndex]->togglePlay();
}

void BlobsDataController::addBlob(ofxCvBlob& cvBlob, float w, float h, float offsetW)
{
    mBlobs.push_back(BLOB_TYPE(new Blob(cvBlob, w, h, offsetW)));
}

void BlobsDataController::removeBlob()
{
    if (!mBlobs.empty())
    {
        mBlobs.pop_back();
    }
}

void BlobsDataController::clearBlobs()
{
    if (!mBlobs.empty())
    {
        mBlobs.clear();
    }
}

const BLOBS_TYPE& BlobsDataController::getBlobsRef() const
{
    return mBlobs;
}

void BlobsDataController::drawSeq(int index, int x, int y, int w, int h)
{
    if (index >= 0 || index < mSeq.size())
    {
        mSeq[index]->draw(x, y, w, h);
    }
}

void BlobsDataController::drawSeqAll(int x, int y, int w, int h)
{
    for (auto e : mSeq)
    {
        e->draw(x, y, w, h);
    }
}
