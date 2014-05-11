#pragma once

#include "SlideUtils.h"
#include "ofxPresentSlide.h"
#include "ofxAudioUnit.h"

BEGIN_NAMESPACE(intro)

struct Hi : public ofxPresentSlide {
	virtual bool wantsHTMLPage() { return true; }
	virtual std::string HTMLPageName() { return "slides/intro-hi"; }
};

struct Bye : public ofxPresentSlide {
	virtual bool wantsHTMLPage() { return true; }
	virtual std::string HTMLPageName() { return "slides/intro-bye"; }
};

struct NoInspireOne : public ofxPresentSlide {
	bool wantsHTMLPage() { return true; }
	string HTMLPageName() { return "slides/intro-no-inspire-one"; }
};

struct NoInspireTwo : public ofxPresentSlide {
	bool wantsHTMLPage() { return true; }
	string HTMLPageName() { return "slides/intro-no-inspire-two"; }
};

struct RightQuestions : public ofxPresentSlide {
	bool wantsHTMLPage() { return true; }
	string HTMLPageName() { return "slides/intro-right-question"; }
};

struct SignalQuestion : public ofxPresentSlide {
	bool wantsHTMLPage() { return true; }
	string HTMLPageName() { return "slides/intro-signal-question"; }
};

struct TalkName : public ofxPresentSlide {
	bool wantsHTMLPage() { return true; }
	string HTMLPageName() { return "slides/intro-name"; }
	
	vector<ofPolyline> waveforms;
	
	void setup() {
		waveforms.resize(33);
		ofEnableAlphaBlending();
	}
	
	void exit() {
		waveforms.clear();
	}
	
	void update() {
		
		const float xStep = ofGetWidth() / 500.;
		const float w = ofGetWidth();
		const float t = ofGetElapsedTimef();
		const float offset = ofGetElapsedTimef() * 0.5;
		const float mag = ofGetHeight() / (float)waveforms.size() * 1.5;
		
		for(int i = 0; i < waveforms.size(); i++) {
			waveforms[i].clear();
			float y = ofGetHeight() / (float)(waveforms.size() + 1) * ((float)i + 1);
			
			for(float x = 0; x <= w; x += xStep) {
				const float signal = ofSignedNoise(x * 0.003 + offset, i * 0.1 + t * 0.3) * mag;
				waveforms[i].addVertex(x, y + signal);
			}
		}
	}
	
	void draw() {
		ofClear(ofColor::black);
		ofPushStyle();
		{
			ofSetLineWidth(ofMap(waveforms.size(), 1, 128, 6, 2, true));
			for(int i = 0; i < waveforms.size(); i++) {
				ofSetColor(palette(i), 155 + sin(i * 0.6 + ofGetElapsedTimef() * 3.) * 90.);
				waveforms[i].draw();
			}
		}
		ofPopStyle();
	}
	
	void keyPressed(int key) {
		if(key == '=') {
			waveforms.push_back(ofPolyline());
		} else if(key == '-') {
			waveforms.pop_back();
		}
	}
};

struct TimeDomainVis : public ofxPresentSlide {
	const GLint layering_mode = GL_LUMINANCE_ALPHA;
	vector<ofTexture> stereoTex;
	ofTexture diffTex, fftTex;
	ofxAudioUnitFilePlayer filePlayer;
	ofxAudioUnitOutput output;
	ofxAudioUnitTap tap;
	ofxAudioUnitTap::StereoSamples signal;
	ofxAudioUnitFftNode fft;
	float leftRMS, rightRMS;
	
	void setup() {
		windowResized(ofGetWidth(), ofGetHeight());
		fft.setNormalizeInput(false);
		fft.setNormalizeOutput(false);
		filePlayer.setFile(ofToDataPath("amon.m4a"));
		filePlayer.connectTo(tap).connectTo(fft).connectTo(output);
		output.start();
	}
	
	void exit() {
		output.stop();
		stereoTex.clear();
		diffTex = ofTexture();
		fftTex = ofTexture();
	}
	
	void update() {
		leftRMS = tap.getLeftChannelRMS();
		rightRMS = tap.getRightChannelRMS();
		tap.getSamples(signal);
		
		if(signal.size() >= stereoTex.back().getWidth() * 2) {
			vector<float> diffData(signal.size());
			vDSP_vsub(&signal.left.front(), 1, &signal.right.front(), 1, &diffData.front(), 1, diffData.size());
			vDSP_vabs(&diffData.front(), 1, &diffData.front(), 1, diffData.size());
			diffTex.loadData(&diffData.front(), diffTex.getWidth(), diffTex.getHeight(), layering_mode);
			
			const float zero = 0;
			const float one = 1;
			vDSP_vclip(&signal.left.front(), 1, &zero, &one, &signal.left.front(), 1, signal.left.size());
			vDSP_vclip(&signal.right.front(), 1, &zero, &one, &signal.right.front(), 1, signal.right.size());
			
			for(int i = 0; i < stereoTex.size(); i++) {
				vector<float> stereoData;
				stereoData.assign(signal.left.end() - stereoTex[i].getWidth() * 2, signal.left.end());
				stereoData.insert(stereoData.end(), signal.right.end() - stereoTex[i].getWidth() * 2, signal.right.end());
				float multi = 3 - (leftRMS + rightRMS);
				vDSP_vsmul(&stereoData.front(), 1, &multi, &stereoData.front(), 1, stereoData.size());
				stereoTex[i].loadData(&stereoData.front(), stereoTex[i].getWidth(), stereoTex[i].getHeight(), layering_mode);
			}
		}
		
		vector<float> amplitude;
		fft.getAmplitude(amplitude);
		if(amplitude.size() >= fftTex.getWidth() * 2) {
			fftTex.loadData(&amplitude.front(), fftTex.getWidth(), fftTex.getHeight(), layering_mode);
		}
	}
	
	void draw() {
		ofClear(0);
		ofEnableBlendMode(OF_BLENDMODE_ADD);
		for(int i = 0; i < stereoTex.size(); i++) {
			float perc = (i / (float)stereoTex.size());
			ofSetColor(ofColor::fromHsb(perc * 255, 20, 225), 125 - perc * 125.);
			stereoTex[i].draw(0, 0, ofGetWidth(), ofGetHeight());
		}
		
		ofEnableBlendMode(OF_BLENDMODE_SUBTRACT);
		float hue = ofMap(leftRMS + rightRMS, 0, 1, 0, 255);
		ofSetColor(ofColor::fromHsb(hue, 255, 255), ofMap(fabsf(leftRMS - rightRMS), 0, 0.1, 100, 250));
		diffTex.draw(0, 0, ofGetWidth(), ofGetHeight());
		
		ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
		ofSetColor(ofColor::fromHsb(255 - hue, ofMap(leftRMS + rightRMS, 0, 1, 0, 255, true), 255), ofMap(leftRMS, 0, 1, 0, 200));
		fftTex.draw(0, 0, ofGetWidth(), ofGetHeight());
	}
	
	void windowResized(int w, int h) {
		stereoTex.clear();
		stereoTex.resize(3);
		const float fundamental = 674;
		for(int i = 0; i < stereoTex.size(); i++) {
			const float fundamentalPercentage = (fundamental * (i / (float)stereoTex.size()));
			stereoTex[i].allocate(fundamental + fundamentalPercentage, 2, layering_mode);
		}
		diffTex.allocate(1, fundamental, layering_mode);
		fftTex.allocate(1, 1024, layering_mode);
		tap.setBufferLength(fundamental * 4);
		fft.setFftBufferSize(8192);
	}
	
	void keyPressed(int key) {
		if(key == 'p') {
			filePlayer.play();
		}
	}
};

struct Intuition : public ofxPresentSlide {
	bool wantsHTMLPage() { return true; }
	string HTMLPageName() { return "slides/intro-intuition"; }
};

struct Sin : public ofxPresentSlide {
	bool wantsHTMLPage() { return true; }
	string HTMLPageName() { return "slides/sin-simple"; }
};

struct SinDefinition : public ofxPresentSlide {
	bool wantsHTMLPage() { return true; }
	string HTMLPageName() { return "slides/sin-definition"; }
};

struct SinDefinitionPic : public ofxPresentSlide {
	bool wantsHTMLPage() { return true; }
	string HTMLPageName() { return "slides/sin-definition-pic"; }
};

struct SinVis : public ofxPresentSlide {
	
	float offset;
	
	void setup() {
		offset = 0;
	}
	
	void draw() {
		ofClear(ofColor::black);
		
		const int count = 40;
		const float s = ofGetWidth() / (float)count;
		const float r = s * 0.7;
		const float t = ofGetElapsedTimef() * 3.5;
		
		offset = ofLerp(offset, 0.01, 0.002);
		
		ofFill();
		ofSetColor(ofColor::white);
		for(int y = -s; y < ofGetHeight() + s; y += s) {
			for(int x = -s; x < ofGetWidth() + s; x += s) {
				float v = t - (x + y) * offset;
				ofVec2f p(sin(v) * r + x, cos(v) * r + y);
				ofCircle(p.x, p.y, r / 6.);
			}
		}
	}
	
};

} // end namespace intro
