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
    GestureRunner(json &jsonLoop, string loop_key) : loop(jsonLoop) {
        loop = jsonLoop;
        ind = 0;
        pos = glm::vec2(0, 0);
        startTime = ofGetElapsedTimef();
        duration = 2;
        loopKey = loop_key;
        lastAge = 0;
        origin = glm::vec2(loop[0]["pos"]["x"], loop[0]["pos"]["y"]);
        pos = origin;
    }
    
    void naiveStep() {
        if(ind < loop.size()) {
            auto posJson = loop[ind]["pos"];
            if(!posJson.is_string()) {
                auto debugJson = posJson.dump();
                pos = glm::vec2(posJson["x"], posJson["y"]);
            }
            ind++;
        }
    }
    
    void absoluteStep() {
        double age = ofGetElapsedTimef() - startTime;
        double phase = std::fmod(age, duration) / duration;
        pos = getPos(phase);
    }
    
    double modulo(double a, double b)
    {
        double r = std::fmod(a, b);
        return r < 0 ? r + b : r;
    }
    
    void deltaStep() {
        auto normalDelta = [&] {
            auto delta = getDelta();
            pos += delta;
            pos = glm::vec2(modulo(pos.x, 1), modulo(pos.y, 1));
        };
        if(deltaAccumulate) {
            normalDelta();
        } else {
            double age = ofGetElapsedTimef() - startTime;
            double lastPhase = std::fmod(lastAge, duration) / duration;
            double phase = std::fmod(age, duration) / duration;
            if(phase < lastPhase) {
                auto delta = getPos(phase) - getPos(0);
                pos = origin + delta;
            } else {
                normalDelta();
            }
        }
    }
    
    void step() {
        if(deltaLoop) deltaStep();
        else absoluteStep();
        lastAge = ofGetElapsedTimef() - startTime;
    }
    
    //todo - this only works if the delta time is less than the duration of the loop
    glm::vec2 getDelta() {
        double age = ofGetElapsedTimef() - startTime;
        double lastPhase = std::fmod(lastAge, duration) / duration;
        double phase = std::fmod(age, duration) / duration;
        
        if(phase >= lastPhase) {
            auto lastPos = getPos(lastPhase);
            auto pos = getPos(phase);
            return pos - lastPos;
        } else { //if the phase has rolled over in the interval
            auto endStart = getPos(lastPhase);
            auto endEnd = getPos(1);
            auto startStart = getPos(0);
            auto startEnd = getPos(phase);
            return (endEnd-endStart) + (startEnd-startStart);
        }
    }
    
    glm::vec2 getPos(double normTime) {
        int len = loop.size();
        int ind = len / 2;
        double stepSize = loop.size() / 4;
        
        if(normTime == 0) return glm::vec2(loop[0]["pos"]["x"], loop[0]["pos"]["y"]);
        if(normTime == 1) return glm::vec2(loop[len-1]["pos"]["x"], loop[len-1]["pos"]["y"]);
        
        auto continueSearch = [&] () {
            bool bounded = 0 < ind && ind < loop.size();
            if(!bounded) return false;
            int i = ind;
            double st = loop[ind-1]["ts"];
            double end = loop[ind]["ts"];
            bool found = st <= normTime && normTime <= end;
            return !found;
        };
        
        while(continueSearch()) {
            if(normTime == loop[ind]["ts"]) break;
            if(normTime < loop[ind]["ts"]) {
                ind -= std::floor(std::fmax(1.0, stepSize));
            } else {
                ind += std::floor(std::fmax(1.0, stepSize));
            }
            stepSize /= 2.;
        }
        //todo - figure out this interpolation crap - is "close enough" for now
        //but never actually will progress pass this line?
//        if(std::fmod(stepSize, 1.) == 0) return glm::vec2(loop[ind]["pos"]["x"], loop[ind]["pos"]["y"]);
        
        double interHitTime = loop[ind]["ts"].get<double>() - loop[ind-1]["ts"].get<double>();
        double hitProgressTime = normTime - loop[ind-1]["ts"].get<double>();
        double lerp_a = hitProgressTime / interHitTime;
        
        auto last = glm::vec2(loop[ind-1]["pos"]["x"], loop[ind-1]["pos"]["y"]);
        auto next = glm::vec2(loop[ind]["pos"]["x"], loop[ind]["pos"]["y"]);
        
        return last*(1-lerp_a) + next*lerp_a;
    }
    
    bool isDone() {
        return ind == loop.size();
    }
    
    json& loop;
    int ind;
    glm::vec2 pos;
    glm::vec2 origin;
    string group;
    string key;
    string loopKey;
    double startTime;
    double duration;
    double lastAge;
    bool deltaLoop = true;
    bool deltaAccumulate = true;
};

#endif /* GestureRunner_h */
