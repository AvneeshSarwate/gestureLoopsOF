#include "ofApp.h"
#include "libs/json.hpp"
#include "ofxOscSubscriber.h"

#define RECONNECT_TIME 400

using json = nlohmann::json;

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetBackgroundColor(230);

	// our send and recieve strings
	msgTx	= "";
	msgRx	= "";

	ofxTCPSettings settings("127.0.0.1", 11999);
    
    ofxSubscribeOsc(7072, "/launch", [&](std::string &loopName, int duration, bool loop, std::string &group, std::string &key){
        auto g = GestureRunner(loops[loopName], loopName);
        g.duration = duration;
        g.group = group;
        g.key = key;
        gestures.push_back(g);
    });

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
    int numLoops = loops.size();
	if(tcpClient.isConnected()){
		// we are connected - lets try to receive from the server
		string str = tcpClient.receive();
		if( str.length() > 0 ){
			msgRx = str;
            loops = json::parse(msgRx);
            gest = new GestureRunner(loops, "firstLoop");
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
//	ofSetColor(20);
//	ofDrawBitmapString("openFrameworks TCP Send Example", 15, 30);
//
//	if(tcpClient.isConnected()){
//		if(!msgTx.empty()){
//			ofDrawBitmapString("sending:", 15, 55);
//			ofDrawBitmapString(msgTx, 85, 55);
//		}else{
//			ofDrawBitmapString("status: type something to send data to port 11999", 15, 55);
//		}
//		ofDrawBitmapString("from server: \n" + msgRx, 15, 270);
//	}else{
//		ofDrawBitmapString("status: server not found. launch server app and check ports!\n\nreconnecting in "+ofToString( (5000 - deltaTime) / 1000 )+" seconds", 15, 55);
//	}
    ofSetColor(255);
//    if(loops.size() > 0) {
//        auto str = loops["firstLoop"][0]["pos"];
//        ofDrawBitmapString(str, 100, 100);
//        gest->step2();
//        ofDrawCircle(gest->pos.x*ofGetWidth(), gest->pos.y*ofGetHeight(), 10);
//    }
    for(auto &g : gestures) {
        g.step2();
        ofDrawCircle(g.pos.x*ofGetWidth(), g.pos.y*ofGetHeight(), 10);
    }
}


//--------------------------------------------------------------
void ofApp::keyPressed(ofKeyEventArgs & key){
	// you can only type if you're connected
	// we accumulate 1 line of text and send every typed character
	// on the next character after a breakline we clear the buffer
//	if(tcpClient.isConnected()){
//		if(key.key == OF_KEY_BACKSPACE || key.key == OF_KEY_DEL){
//			if( !msgTx.empty() ){
//				msgTx = msgTx.substr(0, msgTx.size()-1);
//			}
//		}else if (key.codepoint != 0){
//			ofUTF8Append(msgTx, key.codepoint);
//        }
//        if(key.key == OF_KEY_RETURN) {
//            tcpClient.send(msgTx);
//        }
//		if (!msgTx.empty() && msgTx.back() == '\n') {
//			msgTx.clear();
//		}
//	}
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
