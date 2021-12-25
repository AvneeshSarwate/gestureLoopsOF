//
//  GestureRunner.h
//  gestureLoops
//
//  Created by Avneesh Sarwate on 12/25/21.
//

#include "libs/json.hpp"

using json = nlohmann::json;

#ifndef GestureRunner_h
#define GestureRunner_h

class GestureRunner {
public:
    GestureRunner(json &loops, string loopKey) {
        loop = loops[loopKey];
        ind = 0;
        pos = glm::vec2(0, 0);
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
    
    bool isDone() {
        return ind == loop.size();
    }
    
    json loop;
    int ind;
    glm::vec2 pos;
    string group;
    string key;
};

#endif /* GestureRunner_h */
