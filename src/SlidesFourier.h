#pragma once

#include "SlideUtils.h"
#include "ofxPresentSlide.h"
#include "ofxCv.h"

BEGIN_NAMESPACE(fourier)

struct Title : public ofxPresentSlide {
	bool wantsHTMLPage() { return true; }
	string HTMLPageName() { return "slides/fourier-title"; }
};

struct Video : public ofxPresentSlide {
	bool wantsVideoGrabber() { return true; }
	bool wantsHTMLPage() { return true; }
	string HTMLPageName() { return "fft-intro"; }
	
	vector<ofTexture> displayTex;
	cv::Mat downA, downB, padded, output;
	vector<cv::Mat> channels;
	ofPixels outputPixels;
	bool showCam;
	
	void setup() {
		displayTex.resize(3);
		showCam = false;
	}
	
	void exit() {
		displayTex.clear();
		ofDisableBlendMode();
	}
	
	void update() {
		sharedVidGrabber->update();
		if(sharedVidGrabber->isFrameNew()) {
			ofxCv::pyrDown(ofxCv::toCv(sharedVidGrabber->getPixelsRef()), downA);
			cv::split(downA, channels);
			
			for(int i = 0; i < channels.size(); i++) {
				doFFT(channels[i], output);
				ofxCv::toOf(output, outputPixels);
				displayTex[i].loadData(outputPixels);
			}
		}
	}
	
	// adapted from http://docs.opencv.org/doc/tutorials/core/discrete_fourier_transform/discrete_fourier_transform.html
	void doFFT(const cv::Mat &I, cv::Mat &O) {
		int m = cv::getOptimalDFTSize(I.rows);
		int n = cv::getOptimalDFTSize(I.cols); // on the border add zero values
		cv::copyMakeBorder(I, padded, 0, m - I.rows, 0, n - I.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));
		
		cv::Mat planes[] = {cv::Mat_<float>(padded), cv::Mat::zeros(padded.size(), CV_32F)};
		cv::Mat complex;
		
		cv::merge(planes, 2, complex);
		cv::dft(complex, complex);
		cv::split(complex, planes);
		cv::magnitude(planes[0], planes[1], planes[0]); // planes[0] = magnitude
		cv::Mat magI = planes[0];
		
		magI += cv::Scalar::all(1); // switch to logarithmic scale
		log(magI, magI);
		
		// crop the spectrum, if it has an odd number of rows or columns
		magI = magI(cv::Rect(0, 0, magI.cols & -2, magI.rows & -2));
		
		// rearrange the quadrants of Fourier image  so that the origin is at the image center
		int cx = magI.cols/2;
		int cy = magI.rows/2;
		
		cv::Mat q0(magI, cv::Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant
		cv::Mat q1(magI, cv::Rect(cx, 0, cx, cy));  // Top-Right
		cv::Mat q2(magI, cv::Rect(0, cy, cx, cy));  // Bottom-Left
		cv::Mat q3(magI, cv::Rect(cx, cy, cx, cy)); // Bottom-Right
		
		cv::Mat tmp; // swap quadrants (Top-Left with Bottom-Right)
		q0.copyTo(tmp);
		q3.copyTo(q0);
		tmp.copyTo(q3);
		
		q1.copyTo(tmp); // swap quadrant (Top-Right with Bottom-Left)
		q2.copyTo(q1);
		tmp.copyTo(q2);
		
		// Transform the matrix with float values into a
		// viewable image form (float between values 0 and 1).
		cv::normalize(magI, magI, 0, 255, CV_MINMAX);

		magI.convertTo(O, I.type());
	}
	
	void draw() {
		ofClear(ofColor::black);
		ofEnableBlendMode(OF_BLENDMODE_ADD);
		
		ofSetColor(ofColor::blue);
		displayTex[0].draw(0, 0, ofGetWidth(), ofGetHeight());
		
		ofSetColor(ofColor::green);
		displayTex[1].draw(0, 0, ofGetWidth(), ofGetHeight());
		
		ofSetColor(ofColor::red);
		displayTex[2].draw(0, 0, ofGetWidth(), ofGetHeight());
		
		if(showCam) {
			ofSetColor(ofColor::white, 200);
			float w = sharedVidGrabber->getWidth() / 2.;
			float h = sharedVidGrabber->getHeight() / 2.;
			sharedVidGrabber->draw(ofGetWidth() - w, ofGetHeight() - h, w, h);
		}
	}
	
	void keyPressed(int key) {
		if(key == 'c') {
			showCam = !showCam;
		}
	}
};

struct Sound : public ofxPresentSlide {
	
	ofxAudioUnitInput input;
	ofxAudioUnitFilePlayer filePlayer;
	ofxAudioUnitOutput output;
	ofxAudioUnitMixer outMixer;
	ofxAudioUnitMixer inMixer;
	ofxAudioUnitTap tap;
	ofxAudioUnitFftNode fft;
	
	ofPolyline waveform;
	ofPolyline fftWave;
	
	bool playing;
	bool normalized;
	
	Sound() {
		inMixer.setInputBusCount(2);
		input.connectTo(inMixer, 0);
		filePlayer.connectTo(inMixer, 1);
		inMixer.connectTo(fft).connectTo(tap).connectTo(outMixer).connectTo(output);
		
		fft.setFftBufferSize(2048);
		tap.setBufferLength(44100 / 4.);
	}
	
	void setup() {
		playing = false;
		normalized = false;
		
		int i;
		float x;
		float xStep = ofGetWidth() / 255.;
		fftWave.clear();
		
		for(i = 0, x = -10; x < ofGetWidth(); i++, x += xStep) {
			fftWave.addVertex(x, ofGetHeight());
		}
		
		filePlayer.setFile(ofToDataPath("boc.mp3"));
		
		fft.setNormalizeInput(false);
		fft.setNormalizeOutput(false);
		
		outMixer.setOutputVolume(0);
		input.start();
		output.start();
	}
	
	void exit() {
		output.stop();
		input.stop();
	}
	
	void update() {
		tap.getLeftWaveform(waveform, ofGetWidth(), ofGetHeight());
		
		vector<float> amp;
		fft.getAmplitude(amp);
		
		for(int i = 0; i < fftWave.getVertices().size() && i < amp.size() / 4.; i++) {
			if(!isnan(amp[i])) {
				ofVec3f& v = fftWave.getVertices()[i];
				if(normalized) {
					v.y = ofLerp(v.y, ofGetHeight() - ofGetHeight() * amp[i], 0.3);
				} else {
					v.y = ofLerp(v.y, ofGetHeight() - ofGetHeight() * amp[i] * 0.2, 0.3);
				}
				
			}
		}
	}
	
	void draw() {
		ofClear(ofColor::black);
		
		ofPushStyle();
		{
			ofSetLineWidth(3);
			ofSetColor(palette(1));
			waveform.draw();
			
			ofSetColor(palette(4));
			fftWave.draw();
		}
		ofPopStyle();
	}
	
	void keyPressed(int key) {
		if(key == 'p') {
			if(playing) {
				filePlayer.stop();
				outMixer.setOutputVolume(0);
				inMixer.setInputVolume(0, 1);
			} else {
				filePlayer.play();
				outMixer.setOutputVolume(1);
				inMixer.setInputVolume(0, 0);
			}
			
			playing = !playing;
		} else if (key == 'n') {
			normalized = !normalized;
			fft.setNormalizeOutput(normalized);
			fft.setNormalizeInput(normalized);
		}
	}
};

} // end namespace fourier
