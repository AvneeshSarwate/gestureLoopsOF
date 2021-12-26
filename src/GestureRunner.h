//
//  GestureRunner.h
//  gestureLoops
//
//  Created by Avneesh Sarwate on 12/25/21.
//

#include "libs/json.hpp"
#include "ofMain.h"

using json = nlohmann::json;

#ifndef GestureRunner_h
#define GestureRunner_h

class GestureRunner {
public:
    GestureRunner(json &jsonLoop, string loop_key) {
        loop = jsonLoop;
        ind = 0;
        pos = glm::vec2(0, 0);
        startTime = ofGetElapsedTimef();
        duration = 2;
        loopKey = loop_key;
    }
    
    void step() {
        if(ind < loop.size()) {
            auto posJson = loop[ind]["pos"];
            if(!posJson.is_string()) {
                auto debugJson = posJson.dump();
                pos = glm::vec2(posJson["x"], posJson["y"]);
            }
            ind++;
        }
    }
    
    void step2() {
        float age = ofGetElapsedTimef() - startTime;
        float phase = std::fmod(age, duration) / duration;
        pos = getPos(phase);
    }
    
    glm::vec2 getPos(float normTime) {
        int ind = loop.size() / 2;
        float stepSize = loop.size() / 4;
        
        auto continueSearch = [&] () {
            bool bounded = 0 < ind && ind < loop.size();
            if(!bounded) return false;
            int i = ind;
            float st = loop[ind-1]["ts"];
            float end = loop[ind]["ts"];
            bool found = st <= normTime && normTime <= end;
            return !found;
        };
        
        while(continueSearch()) {
            if(normTime == loop[ind]["ts"]) break;
            if(normTime < loop[ind]["ts"]) {
                ind -= stepSize;
            } else {
                ind += stepSize;
            }
            stepSize = std::fmax(1.0, stepSize/2);
        }
        //todo - figure out this interpolation crap - is "close enough" for now
        //but never actually will progress pass this line?
        if(std::fmod(ind, 1.) == 0) return glm::vec2(loop[ind]["pos"]["x"], loop[ind]["pos"]["y"]);
        
        float interHitTime = loop[ind]["ts"].get<float>() - loop[ind-1]["ts"].get<float>();
        float hitProgressTime = normTime - loop[ind-1]["ts"].get<float>();
        float lerp_a = hitProgressTime / interHitTime;
        
        auto last = glm::vec2(loop[ind-1]["pos"]["x"], loop[ind-1]["pos"]["y"]);
        auto next = glm::vec2(loop[ind]["pos"]["x"], loop[ind]["pos"]["y"]);
        
        return last*(1-lerp_a) + next*lerp_a;
    }
    
    bool isDone() {
        return ind == loop.size();
    }
    
    json loop;
    int ind;
    glm::vec2 pos;
    string group;
    string key;
    string loopKey;
    float startTime;
    float duration;
};

#endif /* GestureRunner_h */
