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


struct TimePoint {
    glm::vec2 pos;
    double ts;
    TimePoint(glm::vec2 pos, double ts) : pos(pos), ts(ts) {};
};
typedef struct TimePoint TimePoint;


class GestureRunner {
public:
    GestureRunner(const vector<TimePoint> &jsonLoop, string loop_key) : loop(jsonLoop) {
        ind = 0;
        origin = glm::vec2(loop[0].pos.x, loop[0].pos.y);
        pos = origin;
        duration = 2;
        loopKey = loop_key;
        
        //
        startTime = ofGetElapsedTimef();
        lastAge = 0;
        lastPos = origin;
    }
    
    void naiveStep() {
        // if(ind < loop.size()) {
        //     auto posJson = loop[ind]["pos"];
        //     if(!posJson.is_string()) {
        //         auto debugJson = posJson.dump();
        //         pos = glm::vec2(posJson["x"], posJson["y"]);
        //     }
        //     ind++;
        // }
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
        lastPos = pos;
        if(deltaLoop) deltaStep();
        else absoluteStep();
        lastAge = ofGetElapsedTimef() - startTime;
    }
    
    //todo - this only works if the delta time is less than the duration of the loop
    glm::vec2 getDelta() {
        double age = ofGetElapsedTimef() - startTime;
        double lastPhase = std::fmod(lastAge, duration) / duration;
        double phase = std::fmod(age, duration) / duration;
        
        auto deltaCheck = [&](glm::vec2 delta, double phase, double lastPhase){
            if(glm::length(delta) > 0.2) {
                bool flipped = phase < lastPhase;
                int x = 5;
            }
        };
        
        glm::vec2 delta;
        if(phase >= lastPhase) {
            auto lastPos = getPos(lastPhase);
            auto pos = getPos(phase);
            delta = pos - lastPos;
            deltaCheck(delta, phase, lastPhase);
        } else { //if the phase has rolled over in the interval
            auto endStart = getPos(lastPhase);
            auto endEnd = getPos(1);
            auto startStart = getPos(0);
            auto startEnd = getPos(phase);
            delta = (endEnd-endStart) + (startEnd-startStart);
            deltaCheck(delta, phase, lastPhase);
        }
        
        return delta;
    }
    
    glm::vec2 getPos(double normTime) {
        int len = loop.size();
        int ind = len / 2;
        double stepSize = loop.size() / 4;
        
        if(normTime == 0) return glm::vec2(loop[0].pos.x, loop[0].pos.y);
        if(normTime == 1) return glm::vec2(loop[len-1].pos.x, loop[len-1].pos.y);
        
        auto continueSearch = [&] () {
            bool bounded = 0 < ind && ind < loop.size();
            if(!bounded) return false;
            int i = ind;
            double st = loop[ind-1].ts;
            double end = loop[ind].ts;
            bool found = st <= normTime && normTime <= end;
            return !found;
        };
        
        while(continueSearch()) {
            if(normTime == loop[ind].ts) break;
            if(normTime < loop[ind].ts) {
                ind -= std::floor(std::fmax(1.0, stepSize));
            } else {
                ind += std::floor(std::fmax(1.0, stepSize));
            }
            stepSize /= 2.;
        }
        
        double interHitTime = loop[ind].ts - loop[ind-1].ts;
        double hitProgressTime = normTime - loop[ind-1].ts;
        double lerp_a = hitProgressTime / interHitTime;
        
        auto last = glm::vec2(loop[ind-1].pos.x, loop[ind-1].pos.y);
        auto next = glm::vec2(loop[ind].pos.x, loop[ind].pos.y);
        auto pos = last*(1-lerp_a) + next*lerp_a;
        
        if(pos.x < 0 || pos.y < 0) {
            int x = 5;
        }
        
        return pos;
    }
    
    bool isDone() {
        return ind == loop.size();
    }
    
    //necessary in constructor
    string loopKey;
    const vector<TimePoint>& loop;
    
    //"private" - could use externally, but need to be careful
    int ind;
    glm::vec2 lastPos;
    glm::vec2 origin;
    double lastAge;
    double startTime;
    
    //pulic - can play with initialization
    glm::vec2 pos;
    double duration;
    string group;
    string key;
    bool deltaLoop = true;
    bool deltaAccumulate = true;
};

#endif /* GestureRunner_h */
