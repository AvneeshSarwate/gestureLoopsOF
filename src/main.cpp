#include "ofMain.h"
#include "ofApp_2.h"

//========================================================================
int main( ){

    ofGLFWWindowSettings settings;
    settings.setGLVersion(3, 3); //we define the OpenGL version we want to use
    float fac = 1;
    settings.setSize(1024*fac, 768*fac);
    ofCreateWindow(settings);            // <-------- setup the GL context

    // this kicks off the running of my app
    // can be OF_WINDOW or OF_FULLSCREEN
    // pass in width and height too:
    ofRunApp(new ofApp_2());

}
