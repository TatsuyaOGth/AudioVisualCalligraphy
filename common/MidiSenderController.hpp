#pragma once

#include "ofMain.h"
#include "ofxMidi.h"
#include "ofxAnimationPrimitives.h"

#define MIDI_SENDER MidiSenderController::getInstance()

class MidiSenderController
{
    class Note : public ofxAnimationPrimitives::Instance
    {
        ofxMidiOut * mMidi;
        const int mCh;
        const int mPich;
    public:
        Note(ofxMidiOut * midi, int pich, int velo, int channel):
        mMidi(midi), mCh(channel), mPich(pich)
        {
            mMidi->sendNoteOn(channel, pich, velo);
        }
        void willDelete()
        {
            mMidi->sendNoteOff(mCh, mPich);
        }
    };
    

    // singleton
    MidiSenderController(){ init(); };
    
    // midi controll instances
    ofxAnimationPrimitives::InstanceManager mMidi;

    
private:
    ofxMidiOut      mMidiOut;
    int             mChannel;
    unsigned int    mCurrentPgm;
    
public:
    
    static MidiSenderController * getInstance()
    {
        static MidiSenderController * instance = new MidiSenderController();
        return instance;
    }
    
    void init()
    {
        mChannel = 1;
        mCurrentPgm = 0;
        ofAddListener(ofEvents().update, this, &MidiSenderController::update);
    }
    
    void update(ofEventArgs& e)
    {
        mMidi.update();
    }
    
    void openPort(const string& deviceName)
    {
        mMidiOut.openPort(deviceName);
    }
    
    void setCurrentChannel(int ch)
    {
        mChannel = ch;
    }
    
    void setCurrentProgram(unsigned int pgm)
    {
        mCurrentPgm = pgm;
    }
    
    void makeNote(int note, int velo, int channel, float duration)
    {
        if (channel != 0) mChannel = channel;
        mMidi.createInstance<Note>(&mMidiOut, note, velo, mChannel)->play(duration);
    }
    
    void ctlOut(int cc, int value, int channel = 0)
    {
        if (channel != 0) mChannel = channel;
        mMidiOut.sendControlChange(mChannel, cc, value);
    }
    
    void listPorts()
    {
        mMidiOut.listPorts();
    }

};
