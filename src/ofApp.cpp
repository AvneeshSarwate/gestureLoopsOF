#include "ofApp.h"
#include "libs/json.hpp"
#include "ofxOscSubscriber.h"
#include "utility.h"

#define RECONNECT_TIME 400

using json = nlohmann::json;

//--------------------------------------------------------------
void ofApp::setup(){

    // generic setup stuff ====================================================
    {
	ofxTCPSettings settings("127.0.0.1", 11999);
    
    ofxSubscribeOsc(7072, "/"+sketchId+"/launch", [&](std::string &loopName, float duration, bool loop, std::string &group, std::string &key){
        auto g = GestureRunner(stdLoops[loopName], loopName);
        g.duration = duration;
        g.group = group;
        g.key = key;
        gestures.push_back(g);
    });
    
    ofxSubscribeOsc(7072, "/"+sketchId+"/clearAll", [&](){
        gestures.clear();
    });
    
    //todo - figure out whether this is sketch specific or general
    ofxSubscribeOsc(7072, "/touching", penTouching);
    ofxSubscribeOsc(7072, "/touchPos", touchPos);

	// set other options:
	//settings.blocking = false;
	//settings.messageDelimiter = "\n";

	// connect to the server - if this fails or disconnects
	// we'll check every few seconds to see if the server exists
	tcpClient.setup(settings);

	// optionally set the delimiter to something else.  The delimiter in the client and the server have to be the same
	tcpClient.setMessageDelimiter("\n");

	tcpConnectTime = 0;
	tcpConnectRetryDelta = 0;
    
    initializeFBO(targetFbo);
    }
    
    
    // sketch specific stuff below here ====================================================
    ofxSubscribeOsc(7072, "/"+sketchId+"/useVoronoi", renderWithVoronoi);
    ofxSubscribeOsc(7072, "/"+sketchId+"/gridSize", gridSize);
    
}

//--------------------------------------------------------------
void ofApp::update(){
    // generic setup stuff ====================================================
    {
	if(tcpClient.isConnected()){
		// we are connected - lets try to receive from the server
		string str = tcpClient.receive();
		if( str.length() > 0 ){
            auto jsonData = json::parse(str);
            string jsonSkechVal = jsonData.at("sketchId");
            bool forThisSketch = jsonSkechVal == sketchId;
            
            //todo: when combining sketches, remove this from sub-sketch update() functions
            bool forRunner = jsonSkechVal == "runner";
            if(jsonData.at("type") == "processedLoops" && (forThisSketch || forRunner)) {
                auto newLoops = restructureJson(jsonData.at("data"));
                for ( const auto &loopData : newLoops ) {
                    stdLoops[loopData.first] = loopData.second;
                }
                cout << "got loops" << endl;
            }
            
            if(jsonData.at("type") == "launchConfig" && forThisSketch) {
                auto config = jsonData.at("data");
                for(auto &conf : config) {
                    auto g = gestureFromJson(stdLoops, conf);
                    gestures.push_back(g);
                }
                cout << "got config" << endl;
            }
		}
	}else{
		// if we are not connected lets try and reconnect every 5 seconds
		tcpConnectRetryDelta = ofGetElapsedTimeMillis() - tcpConnectTime;

		if( tcpConnectRetryDelta > 5000 ){
			tcpClient.setup("127.0.0.1", 11999);
			tcpClient.setMessageDelimiter("\n");
			tcpConnectTime = ofGetElapsedTimeMillis();
		}
	}
    
    // todo using this because std::erase_if not yet supported by C++ version in compiler/project setup
    auto it = std::remove_if(gestures.begin(), gestures.end(), [](GestureRunner g) { return !g.looping && g.isDone(); });
    auto r = std::distance(it, gestures.end());
    gestures.erase(it, gestures.end());
        
    }
    
    
    // sketch specific stuff below here ====================================================
}

void ofApp::drawToFbo() {
    double drawTime = ofGetElapsedTimef();
    targetFbo.begin(); {
        ofClear(0, 0, 0, 0);
        
        vector<glm::vec3> gesturePoints;
        if(renderWithVoronoi) {
            for(int i = 0; i < gridSize; i++) {
                for(int j = 0; j < gridSize; j++) {
                    gesturePoints.push_back(glm::vec3((i+.5)/gridSize*ofGetWidth(), (j+.5)/gridSize*ofGetHeight(), 0));
                }
            }
        }
        
        for(auto &g : gestures) {
            ofSetColor(255);
            g.step(drawTime-lastDrawTime);
            glm::vec3 pos = glm::vec3(g.pos.x*ofGetWidth(), g.pos.y*ofGetHeight(), 0);
            
            if(!renderWithVoronoi) ofDrawCircle(pos.x, pos.y, 10);
            
            gesturePoints.push_back(pos);
        }
        
        if(renderWithVoronoi) {
            auto rect = ofRectangle(0, 0, ofGetWidth(), ofGetHeight());
            voronoi.setBounds(rect);
            voronoi.setPoints(gesturePoints);
            voronoi.generate(true); //cells in order of points - needed to only draw gestures instead of grid
            int relaxIterations = 1;
            while(relaxIterations--){
                voronoi.relax();
            }
            auto cells = voronoi.getCells();
            for(int i = gridSize*gridSize; i < cells.size(); i++) {
                auto polyline = makeCellPolyline(cells[i]);
                ofSetColor(float2randCol(i/10.));
                ofFill();
                drawClosedPolyline(polyline);
            }
        }
        
        if (penTouching) {
            ofSetColor(255, 0, 0);
            ofDrawCircle(touchPos.x * ofGetWidth(), (1-touchPos.y) * ofGetHeight(), 10);
        }
    }; targetFbo.end();
    lastDrawTime = drawTime;
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofClear(0, 0, 0, 0);
    drawToFbo();
    targetFbo.draw(0, 0, ofGetWidth(), ofGetHeight());
}


//--------------------------------------------------------------
void ofApp::keyPressed(ofKeyEventArgs & key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){


}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
