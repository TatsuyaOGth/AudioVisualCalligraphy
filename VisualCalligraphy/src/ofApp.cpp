#include "ofApp.h"

void ofApp::setup()
{
    ofSetFrameRate(60);
    ofSetVerticalSync(true);

    MIDI_RECEIVER->openPort(MIDI_SENDER_PORT_NAME);
    ofAddListener(MIDI_RECEIVER->receivedMidiEvent, this, &ofApp::receivedMidiMessage);
}

void ofApp::update()
{

}

void ofApp::draw()
{

}

void ofApp::keyPressed(int key)
{

}

void ofApp::keyReleased(int key)
{

}

void ofApp::receivedMidiMessage(ofxMidiMessage &e)
{
    cout << e.channel << endl;
}
