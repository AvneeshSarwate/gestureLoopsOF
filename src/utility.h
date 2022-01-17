//
//  utility.h
//  gestureLoops
//
//  Created by Avneesh Sarwate on 12/27/21.
//
#include "libs/json.hpp"
#include "ofxVoronoi.h"

using json = nlohmann::json;

#ifndef utility_h
#define utility_h

vector<TimePoint> extractLoopData(json &loop);

map<string, vector<TimePoint>> restructureJson(json &loops);

ofRectangle getPathBbox(ofPath &path);

ofRectangle getPolylineBbox(ofPolyline &polyline);

ofRectangle getPlaneBbox(ofPlanePrimitive &plane);

ofPolyline makeCellPolyline(ofxVoronoiCell &cell);

void drawClosedPolyline(ofPolyline &poly);

glm::vec3 col2vec(ofColor col);

glm::vec3 float2randColVec(float seed);

ofColor float2randCol(float seed);

ofRectangle getCircleBbox(float x, float y, float rad);

void setBBoxUniform(ofRectangle bbox, ofShader shader);

void setResolutionUniform(ofShader shader);

GestureRunner gestureFromJson(const map<string, vector<TimePoint>> &loopBank, const json& gestureData);

void initializeFBO(ofFbo &fbo);

#endif /* utility_h */
