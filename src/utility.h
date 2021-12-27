//
//  timePoint.h
//  gestureLoops
//
//  Created by Avneesh Sarwate on 12/27/21.
//
#include "libs/json.hpp"

using json = nlohmann::json;

#ifndef utility_h
#define utility_h

vector<TimePoint> extractLoopData(json &loop) {
    int loopSize = loop.size();
    vector<TimePoint> loopData;
    for(int i = 0; i < loopSize; i++) {
        TimePoint tp(glm::vec2(loop[i]["pos"]["x"], loop[i]["pos"]["y"]), loop[i]["ts"]);
        loopData.push_back(tp);
    }
    return loopData;
}

map<string, vector<TimePoint>> restructureJson(json &loops) {
    map<string, vector<TimePoint>> loopMap;
    for (auto& el : loops.items()) {
//        std::cout << el.key() << " : " << el.value() << "\n";
        string key = el.key();
        vector<TimePoint> val = extractLoopData(el.value());
        loopMap.insert({key, val});
    }
    return loopMap;
}

#endif /* timePoint_h */
