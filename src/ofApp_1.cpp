#include "ofApp_1.h"
#include "libs/json.hpp"
#include "ofxOscSubscriber.h"
#include "utility.h"

#define RECONNECT_TIME 400

using json = nlohmann::json;

//--------------------------------------------------------------
void ofApp_1::setup(){

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
        
    ofDisableAlphaBlending();
    }
    
    // sketch specific vars below here =============================
    auto fbo1 = new ofFbo();
    auto fbo2 = new ofFbo();
//    initializeFBO(fbo1);
//    initializeFBO(fbo2);
    pingpong.push_back(fbo1);
    pingpong.push_back(fbo2);
    initializeFBO(*pingpong[0]);
    initializeFBO(*pingpong[1]);
    initializeFBO(brush);
    
    shader.load("sketch1/feedback");
    
    plane.set(ofGetWidth(), ofGetHeight());
    plane.setPosition(0+ofGetWidth()/2., 0+ofGetHeight()/2., 0);
    
    ofxSubscribeOsc(7072, "/"+sketchId+"/schemeInd", schemeInd);
    ofxSubscribeOsc(7072, "/"+sketchId+"/delayTime", delayTime);
    
    circPix.setup(120);
}

//--------------------------------------------------------------
void ofApp_1::update(){
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

void ofApp_1::drawToFbo() {
    double drawTime = ofGetElapsedTimef();
    
    auto src = pingpong[pingpong_ind%2];
    auto dest = pingpong[(pingpong_ind+1)%2];
    pingpong_ind++;
    
    brush.begin(); {
        ofClear(0, 0, 0, 0);
        for(auto &g : gestures) {
            ofSetColor(255);
            g.step(drawTime-lastDrawTime);
            glm::vec3 pos = glm::vec3(g.pos.x*ofGetWidth(), g.pos.y*ofGetHeight(), 0);
            
            ofDrawCircle(pos.x, pos.y, 10);
        }
    } brush.end();
    
    auto delayedFrame = circPix.getDelayedPixels(delayTime);
    
    dest->begin(); {
        ofClear(0, 0, 0, 0);
        shader.begin(); {
            setResolutionUniform(shader);
            shader.setUniform1i("schemeInd", schemeInd);
            shader.setUniform1f("time", ofGetElapsedTimef());
            shader.setUniformTexture("brush", brush.getTexture(), 0);
            shader.setUniformTexture("backbuffer", src->getTexture(), 1);
            shader.setUniformTexture("delayedFrame", delayedFrame, 2);
            auto bbox_plane = getPlaneBbox(plane);
            setBBoxUniform(bbox_plane, shader);
            
            plane.draw();
        } shader.end();
    } dest->end();
    
    circPix.pushPixels(*dest);
    
    targetFbo.begin(); {
        ofClear(0, 0, 0, 0);
        
        dest->draw(0, 0, ofGetWidth(), ofGetHeight());
        
        if (penTouching) {
            ofSetColor(255, 0, 0);
            ofDrawCircle(touchPos.x * ofGetWidth(), (1-touchPos.y) * ofGetHeight(), 10);
        }
    }; targetFbo.end();
    lastDrawTime = drawTime;
}

//--------------------------------------------------------------
void ofApp_1::draw(){
    ofClear(0, 0, 0, 0);
    drawToFbo();
    targetFbo.draw(0, 0, ofGetWidth(), ofGetHeight());
}


//--------------------------------------------------------------
void ofApp_1::keyPressed(ofKeyEventArgs & key){

}

//--------------------------------------------------------------
void ofApp_1::keyReleased(int key){


}

//--------------------------------------------------------------
void ofApp_1::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp_1::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp_1::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp_1::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp_1::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp_1::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp_1::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp_1::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp_1::dragEvent(ofDragInfo dragInfo){

}
