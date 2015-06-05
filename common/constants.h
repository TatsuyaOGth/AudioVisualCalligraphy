#pragma once


// GENERAL
//------------------------------------------------------------------------------
//#define USE_CAMERA

static const int NUM_INPUT = 2;

static const int VISUAL_WINDOW_WIDTH  = 1280;
static const int VISUAL_WINDOR_HEIGHT = 720;


// CAMERA
//------------------------------------------------------------------------------
static const int NUM_CAMERA = NUM_INPUT;
static const int CAMERA_DEVISE_ID[] = {0, 1};
static const int CAMERA_WIDTH     = 1280;
static const int CAMERA_HEIGHT    = 720;



// VIDEO
//------------------------------------------------------------------------------
static const string SOURCE_VIDEO[] = {"movie/test.mov", "movie/test_mini.mov"};
static const int START_POSITION = 0.5;


// MIDI
//------------------------------------------------------------------------------
static const string MIDI_SENDER_PORT_NAME   = "IAC Driver buss 1";
static const string MIDI_RECEIVER_PORT_NAME = "IAC Driver buss 2";


// GUU
//------------------------------------------------------------------------------
static const string GUI_FILENAME = "settings.xml";

