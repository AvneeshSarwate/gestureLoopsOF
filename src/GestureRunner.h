//
//  GestureRunner.h
//  gestureLoops
//
//  Created by Avneesh Sarwate on 12/25/21.
//

#include "libs/json.hpp"
#include "ofMain.h"
#include <glm/gtx/rotate_vector.hpp>

using json = nlohmann::json;

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL

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
        origin = glm::vec2(loop[0].pos.x, loop[0].pos.y);
        pos = origin;
        loopKey = loop_key;
        
        //
        startTime = ofGetElapsedTimef();
        lastPos = origin;
    }
    
    GestureRunner& operator=(GestureRunner other){
        return *this;
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
    
    void absoluteStep(double wallClockDelta) {
        double age = lastAge + wallClockDelta;
        double phase = std::fmod(age, duration) / duration;
        pos = getPos(phase);
    }
    
    double modulo(double a, double b)
    {
        double r = std::fmod(a, b);
        return r < 0 ? r + b : r;
    }
    
    void deltaStep(double wallClockDelta) {
        auto normalDelta = [&] {
            auto delta = getDelta(wallClockDelta);
            pos += glm::rotate(delta, rotation);
            pos = glm::vec2(modulo(pos.x, 1), modulo(pos.y, 1));
        };
        
        if(deltaAccumulate) {
            normalDelta();
        } else {
            double age = lastAge + wallClockDelta;
            double lastPhase = std::fmod(lastAge, duration) / duration;
            double phase = std::fmod(age, duration) / duration;
            if(phase < lastPhase) {
                auto delta = getPos(phase) - getPos(0);
                pos = origin + glm::rotate(delta, rotation);
            } else {
                normalDelta();
            }
        }
    }
    
    void step(double wallClockDelta) {
        lastPos = pos;
        if(deltaLoop) deltaStep(wallClockDelta);
        else absoluteStep(wallClockDelta);
        lastAge += wallClockDelta;
    }
    
    //todo - this only works if the delta time is less than the duration of the loop
    glm::vec2 getDelta(double wallClockDelta) {
        double age = lastAge + wallClockDelta;
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
        return lastAge > duration;
    }
    
    //necessary in constructor
    string loopKey;
    const vector<TimePoint>& loop;
    
    //"private" - could use externally, but need to be careful
    int ind = 0;
    glm::vec2 lastPos;
    glm::vec2 origin;
    double lastAge = 0;
    double startTime;
    
    //pulic - can play with initialization
    glm::vec2 pos;
    double duration = 5;
    string group;
    string key;
    bool looping = true;
    bool deltaLoop = true;
    bool deltaAccumulate = true;
    float rotation = 0;
};

#endif /* GestureRunner_h */
#endif //GLM_ENABLE_EXPERIMENTAL
