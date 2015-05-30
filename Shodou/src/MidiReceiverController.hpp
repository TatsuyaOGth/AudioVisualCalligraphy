#pragma once

#include "ofMain.h"
#include "ofxMidi.h"

#define MIDI_RECEIVER MidiReceiverController::getInstance()

class MidiReceiverController : public ofxMidiListener
{
    //singleton
    MidiReceiverController(){ init(); }
    
    ofxMidiIn mMidiIn;
    ofxMidiMessage mMidiMessage;
    
public:
    ofEvent<ofxMidiMessage> receivedMidiEvent;
    
public:
    
    // get instance
    static MidiReceiverController * getInstance()
    {
        static MidiReceiverController * instance = new MidiReceiverController();
        return instance;
    }
    
    void init()
    {
        mMidiIn.addListener(this);
    }
    
    void openPort(int portid)
    {
        mMidiIn.openPort(portid);
    }
    
    void openPort(const string& deviceName)
    {
        mMidiIn.openPort(deviceName);
    }
    
    void listPorts()
    {
        mMidiIn.listPorts();
    }
    
    ofxMidiIn & getMidiInRef()
    {
        return mMidiIn;
    }
    
    ofxMidiMessage & getMidiMessageRef()
    {
        return mMidiMessage;
    }
    
    void newMidiMessage(ofxMidiMessage& e)
    {
        mMidiMessage = e;
        ofNotifyEvent(receivedMidiEvent, mMidiMessage, this);
    }
    
    void draw(float x = 0, float y = 0, float w = 400)
    {
        ofPushStyle();
        ofPushMatrix();
        
        stringstream text;
        ofSetColor(0);
        
        ofTranslate(x, y);
        
        // draw the last recieved message contents to the screen
        text << "Received: " << ofxMidiMessage::getStatusString(mMidiMessage.status);
        ofDrawBitmapString(text.str(), 20, 20);
        text.str(""); // clear
        
        text << "channel: " << mMidiMessage.channel;
        ofDrawBitmapString(text.str(), 20, 34);
        text.str(""); // clear
        
        text << "pitch: " << mMidiMessage.pitch;
        ofDrawBitmapString(text.str(), 20, 48);
        text.str(""); // clear
        ofNoFill(); ofRect(20, 58, w, 20);
        ofFill();   ofRect(20, 58, ofMap(mMidiMessage.pitch, 0, 127, 0, w), 20);
        
        text << "velocity: " << mMidiMessage.velocity;
        ofDrawBitmapString(text.str(), 20, 96);
        text.str(""); // clear
        ofNoFill(); ofRect(20, 105, w, 20);
        ofFill();   ofRect(20, 105, ofMap(mMidiMessage.velocity, 0, 127, 0, w), 20);
        
        text << "control: " << mMidiMessage.control;
        ofDrawBitmapString(text.str(), 20, 144);
        text.str(""); // clear
        ofNoFill(); ofRect(20, 154, w, 20);
        ofFill();   ofRect(20, 154, ofMap(mMidiMessage.control, 0, 127, 0, w), 20);
        
        text << "value: " << mMidiMessage.value;
        ofDrawBitmapString(text.str(), 20, 192);
        text.str(""); // clear
        if(mMidiMessage.status == MIDI_PITCH_BEND) {
            ofRect(20, 202, ofMap(mMidiMessage.value, 0, MIDI_MAX_BEND, 0, w), 20);
        }
        else {
            ofRect(20, 202, ofMap(mMidiMessage.value, 0, 127, 0, w), 20);
        }
        
        text << "delta: " << mMidiMessage.deltatime;
        ofDrawBitmapString(text.str(), 20, 240);
        
        ofPopMatrix();
        ofPopStyle();
    }
};
