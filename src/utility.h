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

ofRectangle getPathBbox(ofPath &path) {
    ofRectangle rect;
    for (int i=0; i<path.getOutline().size(); i++) {
        ofRectangle b = path.getOutline().at(i).getBoundingBox();
        if (i==0) rect = b;
        else rect.growToInclude(b);
    }
    return rect;
}

ofRectangle getPolylineBbox(ofPolyline &polyline) {
    ofRectangle rect;
//    for (int i = 0)
    for (auto vert : polyline.getVertices()) {
        rect.growToInclude(vert);
    }
    return rect;
}

ofRectangle getPlaneBbox(ofPlanePrimitive &plane) {
    ofRectangle rect;
    auto vertices = plane.getMesh().getVertices();
    for (int i=0; i< vertices.size(); i++) {
        auto pt = vertices.at(i);
        rect.growToInclude(glm::vec2(pt.x, pt.y));
    }
    return rect;
}

void drawClosedPolyline(ofPolyline &poly) {
    ofBeginShape();
    for( int i = 0; i < poly.getVertices().size(); i++) {
        ofVertex(poly.getVertices().at(i).x, poly.getVertices().at(i).y);
    }
    ofEndShape();
}

glm::vec3 col2vec(ofColor col) {
    return glm::vec3(col.r/255.0, col.g/255.0, col.b/255.0);
}

glm::vec3 float2randColVec(float seed) {
    auto rnd = [](float seed) { return (sin(seed*1000)+1)/2;};
    return glm::vec3(rnd(seed+1), rnd(seed+2), rnd(seed+3));
}

ofColor float2randCol(float seed) {
    auto colvec = float2randColVec(seed);
    return ofColor(colvec.r*255, colvec.g*255, colvec.b*255);
}

ofRectangle getCircleBbox(float x, float y, float rad) {
    ofRectangle rect = ofRectangle(x-rad, y-rad, rad*2, rad*2);
    return rect;
}

void setBBoxUniform(ofRectangle bbox, ofShader shader) {
    shader.setUniform4f("bbox", bbox.getMinX(), bbox.getMinY(), bbox.getWidth(), bbox.getHeight());
}

void setResolutionUniform(ofShader shader) {
    shader.setUniform2f("resolution", ofGetWidth(), ofGetHeight());
}

#endif /* timePoint_h */
