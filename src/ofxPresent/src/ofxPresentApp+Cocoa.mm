#include "ofxPresentApp.h"
#include "ofAppGLFWWindow.h"

#import <Cocoa/Cocoa.h>
#import <CoreGraphics/CoreGraphics.h>
#import <AVFoundation/AVFoundation.h>

#pragma mark Webkit

void ofxPresentApp::setupWebkit() {
	ofAppGLFWWindow glfwWindow = *(ofAppGLFWWindow *)ofGetWindowPtr();
	window = (NSWindow *)glfwWindow.getCocoaWindow();
	
	webView = [[WebView alloc] initWithFrame:[window.contentView bounds] frameName:@"ofxPresentFrame" groupName:nil];
	
	[webView setDrawsBackground:NO];
	[webView setWantsLayer:YES];
	[webView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
	[webView setCanDrawConcurrently:YES];
	
	[webView setHostWindow:window];
	[window.contentView addSubview:webView];
	
	[NSEvent addLocalMonitorForEventsMatchingMask:NSKeyDownMask
										  handler:^ NSEvent * (NSEvent * event) {
											  if(!_allowKeyCapture) {
												  [window makeFirstResponder:window.contentView];
											  }
											  return event;
										  }];
}

void ofxPresentApp::cleanupWebkit() {
	[webView removeFromSuperview];
	[webView release];
}

void ofxPresentApp::setWebViewHidden(bool hidden) {
	if(hidden != webView.isHidden) {
		[webView.animator setHidden:hidden];
	}
}

void ofxPresentApp::setWebViewPageName(const std::string &pageName) {
	NSString * page = [NSString stringWithUTF8String:ofToDataPath(pageName).c_str()];
	NSURL * url = [[NSBundle mainBundle] URLForResource:page withExtension:@"html"];;
	NSURLRequest * urlRequest = [NSURLRequest requestWithURL:url];
	[[webView mainFrame] loadRequest:urlRequest];
}

void ofxPresentApp::setWebViewFrame(ofRectangle frame) {
	NSRect f;
	
	if(frame.isEmpty()) {
		f = [window.contentView bounds];
	} else {
		f = (NSRect){frame.x, frame.y, frame.width, frame.height};
	}
	
	[webView setFrame:f];
}

void ofxPresentApp::setWebViewKeyFocus(bool focus) {
	[window makeFirstResponder:focus ? webView : window.contentView];
}

void ofxPresentApp::clearHTML() {
	[[webView mainFrame] loadHTMLString:@"" baseURL:nil];
}

#pragma mark - Auto Refresh

void ofxPresentApp::refreshHTML() {
	[webView reload:nil];
	getCurrentSlide()->didRefresh();
}

void SlidesModifiedCallback(ConstFSEventStreamRef streamRef,
							void * clientCallBackInfo,
							size_t numEvents,
							void * eventPaths,
							const FSEventStreamEventFlags eventFlags[],
							const FSEventStreamEventId eventIds[])
{
	((ofxPresentApp *)clientCallBackInfo)->refreshHTML();
}

void ofxPresentApp::setAutorefreshEnabled(bool enable) {
	
	if(enable && !eventStreamRunning) {
		NSArray * paths = @[ [NSString stringWithUTF8String:ofToDataPath("").c_str()] ];
		FSEventStreamContext ctx = { .info = this };
		eventStream = FSEventStreamCreate(NULL, &SlidesModifiedCallback, &ctx, (CFArrayRef)paths, kFSEventStreamEventIdSinceNow, 0.25, kFSEventStreamCreateFlagNone);
		FSEventStreamScheduleWithRunLoop(eventStream, CFRunLoopGetMain(), kCFRunLoopDefaultMode);
		FSEventStreamStart(eventStream);
		eventStreamRunning = true;
	} else if(!enable &&eventStreamRunning) {
		FSEventStreamStop(eventStream);
		FSEventStreamInvalidate(eventStream);
		FSEventStreamRelease(eventStream);
		eventStreamRunning = false;
	}
}
