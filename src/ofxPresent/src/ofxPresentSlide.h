#pragma once

#include "ofBaseApp.h"
#include "ofVideoGrabber.h"
#include <string>

class ofxPresentSlide : public ofBaseApp {
	
public:
	virtual void draw();
	
	virtual bool wantsHTMLPage() { return false; }
	virtual bool wantsCustomHTMLFrame() { return false; }
	virtual bool wantsVideoGrabber() { return false; }
	virtual bool allowsHTMLPageKeyCapture() { return false; }
	
	virtual std::string HTMLPageName() { return ""; }
	virtual ofRectangle HTMLFrame() { return ofRectangle(); }
	
	virtual void didRefresh() { };
	
	void setSharedVideoGrabber(ofPtr<ofVideoGrabber> sharedVidGrabber);
	
protected:
	
	ofPtr<ofVideoGrabber> sharedVidGrabber;
};
