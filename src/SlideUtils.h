#pragma once

#include "ofMain.h"
#include "ofxPresentSlide.h"

// namespace macro, to keep Xcode from indenting everything
#define BEGIN_NAMESPACE(x) namespace x {

static const ofFloatColor p1 = ofFloatColor::fromHex(0x334D5C);
static const ofFloatColor p2 = ofFloatColor::fromHex(0x45B29D);
static const ofFloatColor p3 = ofFloatColor::fromHex(0xEFC94C);
static const ofFloatColor p4 = ofFloatColor::fromHex(0xE27A3F);
static const ofFloatColor p5 = ofFloatColor::fromHex(0xDF5A49);

static vector<ofFloatColor>& palette() {
	static dispatch_once_t token;
	static vector<ofFloatColor> palette;
	dispatch_once(&token, ^{
		palette.push_back(p1);
		palette.push_back(p2);
		palette.push_back(p3);
		palette.push_back(p4);
		palette.push_back(p5);
	});
	return palette;
}

static ofFloatColor randPalette() {
	return palette()[ (int)ofRandom(1000) % palette().size() ];
}

static ofFloatColor palette(int index) {
	return palette()[index % palette().size()];
}

float mapX(float min, float max) {
	return ofMap(ofGetMouseX(), 0, ofGetWidth(), min, max, true);
}

float mapY(float min, float max) {
	return ofMap(ofGetMouseY(), 0, ofGetHeight(), min, max, true);
}
