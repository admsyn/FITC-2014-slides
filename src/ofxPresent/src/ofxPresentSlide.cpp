#include "ofxPresentSlide.h"
#include "ofGraphics.h"
#include "ofAppRunner.h"

void ofxPresentSlide::draw() {
	ofClear(ofColor::black);
}

void ofxPresentSlide::setSharedVideoGrabber(ofPtr<ofVideoGrabber> sharedVidGrabber) {
	this->sharedVidGrabber = sharedVidGrabber;
}
