#include "ofApp_2.h"
#include "libs/json.hpp"
#include "ofxOscSubscriber.h"
#include "utility.h"

#define RECONNECT_TIME 400
#define BarInset 20
#define BarHeight 40

using json = nlohmann::json;

//--------------------------------------------------------------
void ofApp_2::setup(){

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
    ofxSubscribeOsc(7072, "/penForce", penForce);
    ofxSubscribeOsc(7072, "/penAzimuth", penAzimuth);

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
    
    // sketch specific vars below here =============================
    ofDisableAlphaBlending();
    
    
    auto fbo1 = new ofFbo();
    auto fbo2 = new ofFbo();
//    initializeFBO(fbo1);
//    initializeFBO(fbo2);
    pingpong.push_back(fbo1);
    pingpong.push_back(fbo2);
    initializeFBO(*pingpong[0]);
    initializeFBO(*pingpong[1]);
    initializeFBO(brush);
    initializeFBO(utility);
    
    shader.load(sketchId+"/feedback");
    
    plane.set(ofGetWidth(), ofGetHeight());
    plane.setPosition(0+ofGetWidth()/2., 0+ofGetHeight()/2., 0);
    
    ofxSubscribeOsc(7072, "/"+sketchId+"/schemeInd", schemeInd);
    ofxSubscribeOsc(7072, "/"+sketchId+"/delayTime", delayTime);
    ofxSubscribeOsc(7072, "/"+sketchId+"/loopPosition", loopRegion);
    
    circPix.setup(120);
    
    player.setLoopState(OF_LOOP_NORMAL);
    player.load("movies/gore.mov");
    
}

//--------------------------------------------------------------
void ofApp_2::update(){
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
    // Show or hide the cursor and position bar
    if (ofGetSystemTimeMillis() - lastMovement < 3000){
        drawBar = true;
    } else {
        drawBar = false;
    }
    
    ofRectangle window = ofGetWindowRect();
    if (!drawBar && window.inside(ofGetMouseX(), ofGetMouseY())) {
        ofHideCursor();
    } else {
        ofShowCursor();
    }
    auto pPos = player.getPosition();
    if(pPos < loopRegion.x  ||  loopRegion.y  < pPos  ) player.setPosition(loopRegion.x);
}

void ofApp_2::drawToFbo() {
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
//        if(player.isLoaded()) player.draw(0, 0, ofGetWidth(), ofGetHeight());
    } brush.end();
    
    auto delayedFrame = circPix.getDelayedPixels(delayTime);
    
    if(player.isLoaded()) {
        utility.begin(); {
            player.draw(0, 0, ofGetWidth(), ofGetHeight());
        } utility.end();
    }
    
    // ofEnableAlphaBlending();
    dest->begin(); {
        ofClear(0, 0, 0, 0);
        shader.begin(); {
            setResolutionUniform(shader);
            shader.setUniform1i("schemeInd", schemeInd);
            shader.setUniform1f("time", ofGetElapsedTimef());
            shader.setUniformTexture("brush", brush.getTexture(), 0);
            shader.setUniformTexture("backbuffer", src->getTexture(), 1);
            shader.setUniformTexture("delayedFrame", delayedFrame, 2);
            shader.setUniformTexture("video", utility.getTexture(), 3);
            auto ptvec = getGesturePoints(gestures, 3);
            float* flatPts = ptvec.data();
            shader.setUniform2fv("pts", flatPts, 3);
            auto bbox_plane = getPlaneBbox(plane);
            setBBoxUniform(bbox_plane, shader);
            
            plane.draw();
        } shader.end();
    } dest->end();
    // ofDisableAlphaBlending();
    
    circPix.pushPixels(*dest);
    
    targetFbo.begin(); {
        ofClear(0, 0, 0, 0);
        
        dest->draw(0, 0, ofGetWidth(), ofGetHeight());
        
        if (penTouching) {
            ofSetColor(255, penAzimuth * 255, 0);
            ofDrawCircle(touchPos.x * ofGetWidth(), (1-touchPos.y) * ofGetHeight(), 10 + penForce*10);
        }
        ofDrawBitmapString(std::to_string(penForce), 30, 30);
        ofDrawBitmapString(std::to_string(penAzimuth), 30, 500);
        // Draw the position bar if appropriate
        if (drawBar)
        {
            ofNoFill();
            ofRectangle bar = getBarRectangle();
            ofSetColor(244, 66, 234);
            ofDrawRectangle(bar);
            ofFill();
            ofSetColor(244, 66, 234, 180);
            bar.width *= player.getPosition();
            ofDrawRectangle(bar);
        }
        
    }; targetFbo.end();
    lastDrawTime = drawTime;
}

//--------------------------------------------------------------
void ofApp_2::draw(){
    ofClear(0, 0, 0, 0);
    drawToFbo();
    targetFbo.draw(0, 0, ofGetWidth(), ofGetHeight());
}


//--------------------------------------------------------------
void ofApp_2::keyPressed(ofKeyEventArgs & key){
    if (key.key == ' ')
   {
       player.setPaused(!player.isPaused());
   }
   else if (key.key == OF_KEY_UP)
   {
       player.setVolume(player.getVolume() + 0.1);
   }
   else if (key.key == OF_KEY_DOWN)
   {
       player.setVolume(player.getVolume() - 0.1);
   }

}

//--------------------------------------------------------------
void ofApp_2::keyReleased(int key){


}

//--------------------------------------------------------------
void ofApp_2::mouseMoved(int x, int y ){
    if (ofGetWindowRect().inside(x, y))
    {
        lastMovement = ofGetSystemTimeMillis();
    }
}

//--------------------------------------------------------------
void ofApp_2::mouseDragged(int x, int y, int button){
    if (inScrub)
    {
        float position = static_cast<float>(x - BarInset) / getBarRectangle().width;
        position = std::max(0.0f, std::min(position, 1.0f));
        player.setPosition(position);
        lastMovement = ofGetSystemTimeMillis();
    }
}

//--------------------------------------------------------------
void ofApp_2::mousePressed(int x, int y, int button){
    ofRectangle bar = getBarRectangle();
    if (bar.inside(x, y))
    {
        inScrub = true;
        wasPaused = player.isPaused() || player.getIsMovieDone();
        player.setPaused(true);
        mouseDragged(x, y, button);
    }
    lastMovement = ofGetSystemTimeMillis();
}

//--------------------------------------------------------------
void ofApp_2::mouseReleased(int x, int y, int button){
    if (inScrub)
    {
        inScrub = false;
        player.setPaused(wasPaused);
    }
}

//--------------------------------------------------------------
void ofApp_2::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp_2::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp_2::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp_2::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp_2::dragEvent(ofDragInfo dragInfo){

}

ofRectangle ofApp_2::getBarRectangle() const
{
    return ofRectangle(BarInset, ofGetWindowHeight() - BarInset - BarHeight, ofGetWindowWidth() - (2 * BarInset), BarHeight);
}
