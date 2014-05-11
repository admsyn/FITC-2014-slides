#pragma once

#include "ofBaseApp.h"
#include "ofVideoGrabber.h"
#include "ofxPresentSlide.h"
#include "ofxPresentTransition.h"

#include <vector>
#include <string>

#import <CoreServices/CoreServices.h>

#ifdef __OBJC__
#import <WebKit/WebKit.h>
#endif

class ofxPresentApp : public ofBaseApp {
	
public:
	
	ofxPresentApp();
	virtual ~ofxPresentApp();
	ofxPresentApp( const ofxPresentApp& other );
	
	// updateSlide / drawSlide will be called automatically by ofEvents, but you can
	// disable this with disableEvents() and then call them yourself if you like
	void updateSlide(ofEventArgs & e);
	void drawSlide(ofEventArgs & e);
	void resizeSlide(ofResizeEventArgs & e);
	
	void setEventsEnabled(bool enabled); // true by default
	void setAutorefreshEnabled(bool enabled); // false by default
	
	template<typename T> void addSlide(ofxPresentTransition transition = ofxPresentTransition::None) {
		addSlide(ofPtr<ofxPresentSlide>(new T), transition);
	}
	
	void addSlide(ofPtr<ofxPresentSlide> slide, ofxPresentTransition transition = ofxPresentTransition::None);
	
	bool nextSlide();
	bool previousSlide();
	bool goToSlide(size_t targetSlideIndex);
	
	void clearHTML();
	void refreshHTML();
	
	ofPtr<ofxPresentSlide> getCurrentSlide();
	
protected:
	typedef std::pair<ofPtr<ofxPresentSlide>, ofxPresentTransition> SlideData;
	std::vector<SlideData> slides;
	size_t slideIndex;
	
private:
	
#ifdef __OBJC__
	WebView * webView;
	NSWindow * window;
#else
	void * webView;
	void * window;
#endif
	
	bool eventStreamRunning;
	FSEventStreamRef eventStream;

	void setupWebkit();
	void cleanupWebkit();
	void setWebViewPageName(const std::string &pageName);
	void setWebViewHidden(bool hidden);
	void setWebViewFrame(ofRectangle frame);
	void setWebViewKeyFocus(bool focus);
	
	void setVideoPlayerVideoName(const std::string &videoName);
	void setVideoPlayerHidden(bool hidden);
	
	void initSlide(ofPtr<ofxPresentSlide> slide);
	
	ofPtr<ofVideoGrabber> _sharedVideoGrabber;
	bool _didInitFirstSlide;
	bool _allowKeyCapture;
	bool _eventsEnabled;
	bool _listeningToEvents;
};
