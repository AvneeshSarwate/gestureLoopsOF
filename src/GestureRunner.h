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
    GestureRunner(json &loops, string loopKey) {
        loop = loops[loopKey];
        ind = 0;
        pos = glm::vec2(0, 0);
        startTime = ofGetElapsedTimef();
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
    
    glm::vec2 getPos(float normTime) {
        int ind = loop.size() / 2;
        float stepSize = loop.size() / 4;
        
        auto continueSearch = [&] () {
            bool bounded = 0 <= ind && ind < loop.size();
            bool found = loop[ind-1]["ts"] <= normTime && normTime < loop[ind]["ts"];
            return bounded && !found;
        };
        
        while(continueSearch()) {
            if(normTime == loop[ind]["ts"]) break;
            if(normTime < loop[ind]["ts"]) {
                ind -= floor(stepSize/2);
            } else {
                ind += floor(stepSize/2);
            }
            stepSize = std::fmax(1.0, stepSize/2);
        }
        
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
    float startTime;
    float duration;
};

#endif /* GestureRunner_h */
