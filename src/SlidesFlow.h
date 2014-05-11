#pragma once

#include "SlideUtils.h"
#include "ofxPresentSlide.h"

BEGIN_NAMESPACE(flow)

struct Farneback : public ofxPresentSlide {
	bool wantsHTMLPage() { return true; }
	bool wantsVideoGrabber() { return true; }
	string HTMLPageName() { return "optical-flow-farneback"; }
	
	ofxCv::FlowFarneback flow;
	ofxCv::Mat downA, downB;
	ofPixels flowPix;
	
	void update() {
		sharedVidGrabber->update();
		if(sharedVidGrabber->isFrameNew()) {
			ofxCv::pyrDown(ofxCv::toCv(sharedVidGrabber->getPixelsRef()), downA);
			ofxCv::pyrDown(downA, downB);
			ofxCv::toOf(downB, flowPix);
			flow.calcOpticalFlow(flowPix);
		}
	}
	
	void draw() {
		sharedVidGrabber->draw(0, 0, ofGetWidth(), ofGetHeight());
		flow.draw(0, 0, ofGetWidth(), ofGetHeight());
	}
};

struct PyrLk : public ofxPresentSlide {
	bool wantsHTMLPage() { return true; }
	bool wantsVideoGrabber() { return true; }
	string HTMLPageName() { return "optical-flow-pyrlk"; }
	
	ofxCv::FlowPyrLK flow;
	ofxCv::Mat downA, downB;
	ofPixels flowPix;
	
	void update() {
		sharedVidGrabber->update();
		if(sharedVidGrabber->isFrameNew()) {
			ofxCv::pyrDown(ofxCv::toCv(sharedVidGrabber->getPixelsRef()), downA);
			ofxCv::pyrDown(downA, downB);
			ofxCv::equalizeHist(downB);
			ofxCv::toOf(downB, flowPix);
			flow.calcOpticalFlow(flowPix);
		}
	}
	
	void draw() {
		ofSetColor(ofColor::white);
		sharedVidGrabber->draw(0, 0, ofGetWidth(), ofGetHeight());
		flow.draw(0, 0, ofGetWidth(), ofGetHeight());
		
		if(flow.getWidth() > 0 && flow.getHeight() > 0) {
			ofVec2f mapVec(ofGetWidth() / flow.getWidth(), ofGetHeight() / flow.getHeight());
			vector<ofPoint> features = flow.getCurrent();
			ofNoFill();
			ofSetColor(palette(3));
			for(ofPoint p : features) {
				p *= mapVec;
				ofCircle(p.x, p.y, 5);
			}
		}
	}
	
	void keyPressed(int key) {
		flow.resetFeaturesToTrack();
	}
};

} // end namespace flow
