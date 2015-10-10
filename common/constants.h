#pragma once


// GENERAL
//------------------------------------------------------------------------------
//#define USE_CAMERA

static const int        NUM_INPUT = 1;

static const int        VISUAL_WINDOW_WIDTH  = 1440;
static const int        VISUAL_WINDOW_HEIGHT = 900;
static const string     MAIN_DISP_SERVER_NAME = "syphone";


// CAMERA
//------------------------------------------------------------------------------
static const int NUM_CAMERA = NUM_INPUT;
static const int CAMERA_DEVISE_ID[] = {0};
static const int CAMERA_WIDTH     = 1280;
static const int CAMERA_HEIGHT    = 720;



// VIDEO
//------------------------------------------------------------------------------
static const string SOURCE_VIDEO[] = {"movie/test_mini.mov"};


// MIDI
//------------------------------------------------------------------------------
static const string MIDI_SENDER_PORT_NAME   = "IAC Driver buss 1";
static const string MIDI_RECEIVER_PORT_NAME = "IAC Driver buss 2";


// GUU
//------------------------------------------------------------------------------
static const string GUI_FILENAME = "settings.xml";

