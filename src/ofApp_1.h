#pragma once

#include "ofMain.h"
#include "ofxNetwork.h"
#include "ofxVoronoi.h"
#include "ofxAutoReloadedShader.h"
#include "ofxHapPlayer.h"
#include "libs/json.hpp"
#include "GestureRunner.h"
#include "utility.h"

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
        double penForce;
        double penAzimuth;
    
        double lastDrawTime;
    
        // sketch specific vars below here =============================
        vector<ofFbo*> pingpong;
        ofFbo utility;
        ofFbo brush;
        int pingpong_ind = 0;
        ofxAutoReloadedShader shader;
        ofPlanePrimitive plane;
        int schemeInd;
        CircularPixelBuffer circPix;
        int delayTime = 60;
    
        ofxHapPlayer player;
        ofRectangle getBarRectangle() const;
        uint64_t lastMovement;
        bool wasPaused;
        bool drawBar;
        bool inScrub;
        glm::vec2 loopRegion = glm::vec2(0, 1);
};

