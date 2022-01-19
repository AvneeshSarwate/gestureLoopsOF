#pragma once

#include "ofMain.h"
#include "ofxNetwork.h"
#include "ofxVoronoi.h"
#include "libs/json.hpp"
#include "GestureRunner.h"
#include "ofxAutoReloadedShader.h"

using json = nlohmann::json;

class ofApp_1 : public ofBaseApp{

	public:
        string sketchId = "sketch1";

		void setup();
		void update();
		void draw();
    
        void drawToFbo();
        ofFbo targetFbo;
		
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
		int tcpConnectTime;
		int tcpConnectRetryDelta;
    
        vector<GestureRunner> gestures;
        map<string, vector<TimePoint>> stdLoops;
    
        bool penTouching;
        glm::vec2 touchPos;
    
        double lastDrawTime;
    
        // sketch specific vars below here =============================
        vector<ofFbo*> pingpong;
        ofFbo brush;
        int pingpong_ind = 0;
        ofxAutoReloadedShader shader;
        ofPlanePrimitive plane;
        
        
};

