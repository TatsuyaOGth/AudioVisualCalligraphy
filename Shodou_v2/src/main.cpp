#include "ofMain.h"
#include "mainApp.h"

int main( )
{
	ofSetupOpenGL(1280,768,OF_WINDOW);
	ofRunApp(new mainApp());
}
