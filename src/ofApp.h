#pragma once

#include "ofMain.h"
#include "ofxNetwork.h"
#include "libs/json.hpp"
#include "GestureRunner.h"

using json = nlohmann::json;

class ofApp : public ofBaseApp{

	public:

		void setup();
		void update();
		void draw();
		
		void keyPressed(ofKeyEventArgs & key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);		
		
		ofxTCPClient tcpClient;
		string msgTx, msgRx;

		ofTrueTypeFont  mono;
		ofTrueTypeFont  monosm;

		float counter;
		int connectTime;
		int deltaTime;

        int size;
    
        GestureRunner *gest;
        vector<GestureRunner> gestures;
    
        json jsonLoops;
        map<string, vector<TimePoint>> stdLoops;
};

