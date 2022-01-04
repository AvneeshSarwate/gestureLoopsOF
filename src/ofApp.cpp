#include "ofApp.h"
#include "libs/json.hpp"
#include "ofxOscSubscriber.h"
#include "utility.h"

#define RECONNECT_TIME 400

using json = nlohmann::json;

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetBackgroundColor(230);

	// our send and recieve strings
	msgTx	= "";
	msgRx	= "";

	ofxTCPSettings settings("127.0.0.1", 11999);
    
    ofxSubscribeOsc(7072, "/launch", [&](std::string &loopName, float duration, bool loop, std::string &group, std::string &key){
        auto g = GestureRunner(stdLoops[loopName], loopName);
        g.duration = duration;
        g.group = group;
        g.key = key;
        gestures.push_back(g);
    });
    
    ofxSubscribeOsc(7072, "/launchConfig", [&](std::string &launchConfigStr) {
        auto config = json::parse(launchConfigStr);
        for(auto &conf : config) {
            auto g = gestureFromJson(stdLoops, conf);
            gestures.push_back(g);
        }
    });
    
    ofxSubscribeOsc(7072, "/clearAll", [&](){
        gestures.clear();
    });
    
    ofxSubscribeOsc(7072, "/useVoronoi", renderWithVoronoi);
    ofxSubscribeOsc(7072, "/gridSize", gridSize);
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

	connectTime = 0;
	deltaTime = 0;
}

//--------------------------------------------------------------
void ofApp::update(){
    int numLoops = jsonLoops.size();
	if(tcpClient.isConnected()){
		// we are connected - lets try to receive from the server
		string str = tcpClient.receive();
		if( str.length() > 0 ){
			msgRx = str;
            jsonLoops = json::parse(msgRx);
            auto newLoops = restructureJson(jsonLoops);
            for ( const auto &loopData : newLoops ) {
                stdLoops[loopData.first] = loopData.second;
            }
            
//            gest = new GestureRunner(loops, "firstLoop");
            cout << "got loops" << endl;
		}
	}else{
		msgTx = "";
		// if we are not connected lets try and reconnect every 5 seconds
		deltaTime = ofGetElapsedTimeMillis() - connectTime;

		if( deltaTime > 5000 ){
			tcpClient.setup("127.0.0.1", 11999);
			tcpClient.setMessageDelimiter("\n");
			connectTime = ofGetElapsedTimeMillis();
		}

	}
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofClear(0, 0, 0, 0);
    
//    renderWithVoronoi = true;
    
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
        g.step();
        glm::vec3 pos = glm::vec3(g.pos.x*ofGetWidth(), g.pos.y*ofGetHeight(), 0);
        
        if(!renderWithVoronoi) ofDrawCircle(pos.x, pos.y, 10);
        
        gesturePoints.push_back(pos);
//        ofSetColor(255, 0, 0);
//        ofDrawBitmapString(std::to_string(i), pos.x, pos.y);
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
//        int i = 0;
//        for(auto cell : voronoi.getCells()) {
//            auto cellPts = cell.points;
//            ofPolyline polyline;
//            polyline.addVertices(cellPts);
//            polyline.close();
//
//            ofSetColor(float2randCol(i/10.));
//            ofNoFill();
//            drawClosedPolyline(polyline);
//            i++;
//        }
    }
    
    if (penTouching) {
        ofSetColor(255, 0, 0);
        ofDrawCircle(touchPos.x * ofGetWidth(), (1-touchPos.y) * ofGetHeight(), 10);
    }
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
