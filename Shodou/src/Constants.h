#pragma once


// GENERAL
//------------------------------------------------------------------------------
//#define USE_CAMERA



// CAMERA
//------------------------------------------------------------------------------
static const int NUM_CAMERA = 2;
static const int CAMERA_DEVISE_ID[] = {0, 1};
static const int CAMERA_WIDTH     = 1280/2;
static const int CAMERA_HEIGHT    = 720/2;



// VIDEO
//------------------------------------------------------------------------------
static const string SOURCE_VIDEO = "movie/test.mov";



// MIDI
//------------------------------------------------------------------------------
static const string MIDI_SENDER_PORT_NAME   = "IAC Driver buss 1";
static const string MIDI_RECEIVER_PORT_NAME = "IAC Driver buss 2";
