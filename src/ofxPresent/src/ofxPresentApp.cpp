#include "ofxPresentApp.h"
#include "ofMain.h"
#include "ofEvents.h"

typedef ofPtr<ofxPresentSlide> SlidePtr;
SlidePtr deadSlide = SlidePtr(new ofxPresentSlide);

ofxPresentApp::ofxPresentApp()
: webView(NULL)
, _sharedVideoGrabber(ofPtr<ofVideoGrabber>())
, _didInitFirstSlide(false)
, _allowKeyCapture(false)
, _eventsEnabled(true)
, _listeningToEvents(false) {
	
}

ofxPresentApp::~ofxPresentApp() {
	cleanupWebkit();
	
	if(_listeningToEvents) {
		setEventsEnabled(false);
	}
}

ofxPresentApp::ofxPresentApp(const ofxPresentApp& other) {
	this->slides = other.slides;
	this->slideIndex = other.slideIndex;
	this->webView = other.webView;
	this->window = other.window;
	this->_didInitFirstSlide = other._didInitFirstSlide;
	this->_sharedVideoGrabber = other._sharedVideoGrabber;
}

void ofxPresentApp::updateSlide(ofEventArgs &e) {
	getCurrentSlide()->update();
}

void ofxPresentApp::drawSlide(ofEventArgs &e) {
	getCurrentSlide()->draw();
}

void ofxPresentApp::resizeSlide(ofResizeEventArgs &e) {
	getCurrentSlide()->windowResized(e.width, e.height);
}

void ofxPresentApp::setEventsEnabled(bool enabled) {
	_eventsEnabled = enabled;
	
	if(_eventsEnabled && !_listeningToEvents) {
		ofAddListener(ofEvents().update, this, &ofxPresentApp::updateSlide, OF_EVENT_ORDER_BEFORE_APP);
		ofAddListener(ofEvents().draw, this, &ofxPresentApp::drawSlide, OF_EVENT_ORDER_BEFORE_APP);
		ofAddListener(ofEvents().windowResized, this, &ofxPresentApp::resizeSlide, OF_EVENT_ORDER_BEFORE_APP);
		_listeningToEvents = true;
	} else if(!_eventsEnabled && _listeningToEvents) {
		ofRemoveListener(ofEvents().update, this, &ofxPresentApp::updateSlide);
		ofRemoveListener(ofEvents().draw, this, &ofxPresentApp::drawSlide);
		ofAddListener(ofEvents().windowResized, this, &ofxPresentApp::resizeSlide, OF_EVENT_ORDER_BEFORE_APP);
		_listeningToEvents = false;
	}
}

#pragma mark - Presentation Flow

bool ofxPresentApp::nextSlide() {
	return goToSlide(slideIndex + 1);
}

bool ofxPresentApp::previousSlide() {
	if(slideIndex == 0) {
		return false;
	} else {
		return goToSlide(slideIndex - 1);
	}
}

bool ofxPresentApp::goToSlide(size_t targetSlideIndex) {
	if(targetSlideIndex < slides.size()) {
		getCurrentSlide()->exit();
		slideIndex = targetSlideIndex;
		initSlide(getCurrentSlide());
		
		return true;
	} else {
		return false;
	}
}

void ofxPresentApp::initSlide(ofPtr<ofxPresentSlide> slide) {
	
	ofSetStyle(ofStyle());
	
	slide->setup();
	
	if(slide->wantsHTMLPage()) {
		
		if(!webView) {
			setupWebkit();
		}
		
		clearHTML();
		setWebViewHidden(true);
		
		if(slide->wantsCustomHTMLFrame()) {
			setWebViewFrame(slide->HTMLFrame());
		} else {
			setWebViewFrame(ofRectangle());
		}
		
		// since this event was likely triggered by a key press, we dispatch
		// async the web view's key focus (or else it'll get the key press that
		// triggered the transition in the first place)
		dispatch_async(dispatch_get_main_queue(), ^{
			_allowKeyCapture = slide->allowsHTMLPageKeyCapture();
			setWebViewKeyFocus(_allowKeyCapture);
		});
		
		setWebViewPageName(slide->HTMLPageName());
		setWebViewHidden(false);
		
	} else {
		setWebViewHidden(true);
	}
}

#pragma mark - Slide Access

SlidePtr ofxPresentApp::getCurrentSlide() {
	if(slideIndex < slides.size() && slides[slideIndex].first) {
		return slides[slideIndex].first;
	} else {
		return deadSlide;
	}
}

void ofxPresentApp::addSlide(ofPtr<ofxPresentSlide> slide, ofxPresentTransition transition) {
	slides.push_back( std::make_pair(slide, transition) );
	
	if(slide->wantsVideoGrabber()) {
		if(!_sharedVideoGrabber) {
			_sharedVideoGrabber = ofPtr<ofVideoGrabber>(new ofVideoGrabber);
			_sharedVideoGrabber->initGrabber(1280, 720);
		}
		slide->setSharedVideoGrabber(_sharedVideoGrabber);
	}
	
	if(!_didInitFirstSlide) {
		initSlide(slides.back().first);
		setEventsEnabled(_eventsEnabled);
		_didInitFirstSlide = true;
	}
}
