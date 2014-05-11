#pragma once

#include "SlideUtils.h"
#include "ofxPresentSlide.h"
#include "ofxCv.h"
#include "ofxAudioUnit.h"
#include <algorithm>

BEGIN_NAMESPACE(convolution)

struct AutoConvolutionSlide : public ofxPresentSlide {
	
	virtual ~AutoConvolutionSlide() { };
	virtual bool wantsVideoGrabber() { return true; }
	
	virtual void createKernel(cv::Mat1f& kernel) { kernel = cv::Mat1f::zeros(3, 3); }
	virtual void updateKernel(cv::Mat1f& kernel) { };

	virtual ofVec2f getKernelRange() { return ofVec2f(-1, 1); }
	virtual float getKernelBias() { return 0; }
	
	void setup() {
		createKernel(kernel);
		displayTex = ofPtr<ofTexture>(new ofTexture);
	}
	
	void update() {
		if(sharedVidGrabber) {
			sharedVidGrabber->update();
			if(sharedVidGrabber->isFrameNew()) {
				updateKernel(kernel);
				ofxCv::pyrDown(ofxCv::toCv(sharedVidGrabber->getPixelsRef()), downsampled);
				ofxCv::medianBlur(downsampled, 3);
				ofxCv::filter2D(downsampled, output, -1, kernel);
				
				float bias = getKernelBias();
				if(bias != 0) {
					output += bias;
				}
				
				ofxCv::toOf(output, outputPixels);
				
				displayTex->loadData(outputPixels);
			}
		}
	}
	
	void draw() {
		if(displayTex->isAllocated()) {
			ofSetColor(ofColor::white);
			displayTex->draw(0, 0, ofGetWidth(), ofGetHeight());
			
			const float imageSize = ofGetWidth() / 4.;
			const float pad = imageSize / 4.;
			ofVec2f range = getKernelRange();
			
			// deciding whether to draw kernel as grid-o-rectangles or raw image
			if(kernel.cols * kernel.rows < 300) {
				ofVec2f rectSize(imageSize / kernel.cols, imageSize / kernel.rows);
				ofVec2f origin(ofGetWidth() - imageSize - pad, ofGetHeight() - imageSize - pad);
				
				for(int kernY = 0; kernY < kernel.rows; kernY++) {
					for(int kernX = 0; kernX < kernel.cols; kernX++) {
						ofVec2f pos = ofVec2f(kernX, kernY) * rectSize + origin;
						ofSetColor(ofMap(kernel(kernY, kernX), range.x, range.y, 0, 255, true));
						ofRect(pos.x + 1, pos.y + 1, rectSize.x - 2, rectSize.y - 2);
					}
				}
				
			} else {
				kernelToImage(kernel, kernelImage, range);
				kernelImage.draw(ofGetWidth() - imageSize - pad,
								 ofGetHeight() - imageSize - pad,
								 imageSize, imageSize);
			}
			
		} else {
			ofClear(ofColor::black);
		}
	}
	
	void exit() {
		displayTex = ofPtr<ofTexture>();
	}
	
private:
	cv::Mat1f kernel;
	cv::Mat downsampled, output;
	ofPtr<ofTexture> displayTex;
	ofPixels outputPixels;
	ofImage kernelImage;
	
	void kernelToImage(const cv::Mat1f& kern, ofImage& img, const ofVec2f& range) {
		
		ofPixels p;
		p.allocate(kern.cols, kern.rows, 1);
		
		for(int y = 0; y < kern.rows; y++) {
			for(int x = 0; x < kern.cols; x++) {
				unsigned char c = ofMap(kernel(y, x), range.x, range.y, 0, 255, true);
				p.setColor(x, y, c);
			}
		}
		
		img.setFromPixels(p);
		img.update();
	}
};

struct Blank : public AutoConvolutionSlide {
	
	void createKernel(cv::Mat1f &kernel) {
		
		float k[] = {
			0, 0, 0,
			0, 1, 0,
			0, 0, 0
		};
		
		kernel = cv::Mat1f(3, 3, k).clone();
	}
};

struct MexicanHat : public AutoConvolutionSlide {
	
	void createKernel(cv::Mat1f& kernel) {
		
		float k[] = {
			1,  1, 1,
			1, -8, 1,
			1,  1, 1
		};
		
		kernel = cv::Mat1f(3, 3, k).clone();
	}
	
	ofVec2f getKernelRange() {
		return ofVec2f(-4, 1);
	}
};

struct Perlin : public AutoConvolutionSlide {
	
	void createKernel(cv::Mat1f& kernel) {
		kernel = cv::Mat1f(9, 9).clone();
	}
	
	void updateKernel(cv::Mat1f& kernel) {
		float t = ofGetElapsedTimef() * 0.5;
		for(int y = 0; y < kernel.rows; y++) {
			for(int x = 0; x < kernel.cols; x++) {
				kernel(y, x) = ofSignedNoise(x * 0.25 + t, y * 0.25 + t, t * 0.1);
			}
		}
	}
};

struct Gabor : public AutoConvolutionSlide {
	
	void updateKernel(cv::Mat1f& kernel) {
		
		float size = 25;
		float sigma = mapX(0, CV_PI * 2.);
		float gamma = mapY(0, CV_PI * 0.5);
		float theta = ofGetElapsedTimef();
		float lambda = CV_PI;
		
		kernel = cv::getGaborKernel(cv::Size(size, size), sigma, theta, lambda, gamma);
	}
};

struct Emboss : public AutoConvolutionSlide {
	
	void createKernel(cv::Mat1f& kernel) {
		
		float k[] = {
			4,  0,  0,
			0,  1,  0,
			0,  0, -4
		};
		
		kernel = cv::Mat1f(3, 3, k).clone();
	}
	
	ofVec2f getKernelRange() {
		return ofVec2f(-2, 2);
	}
};

struct BoxBlur : public AutoConvolutionSlide {
	
	void updateKernel(cv::Mat1f& kernel) {
		
		int size = mapX(3, 21);
		
		if(!(size % 2)) {
			size++; // make sure size is always an odd number
		}
		
		float v = 1 / (float)(size * size);
		vector<float> k(size * size, v);
		
		kernel = cv::Mat1f(size, size, &k.front()).clone();
	}
};

struct MotionBlur : public AutoConvolutionSlide {
	
	void updateKernel(cv::Mat1f& kernel) {
		
		int size = mapX(3, 29);
		
		if(!(size % 2)) {
			size++; // make sure size is always an odd number
		}
		
		float v = 1 / (float)size;
		vector<float> k(size * size);
		for(int i = 0; i < size; i++) {
			k[i * size + i] = v;
		}
		
		kernel = cv::Mat1f(size, size, &k.front()).clone();
	}
	
	ofVec2f getKernelRange() {
		return ofVec2f(-0.5, 0.5);
	}
};

struct GaussianBlur : public AutoConvolutionSlide {
	
	void updateKernel(cv::Mat1f& kernel) {
		cv::Mat1f kernX = cv::getGaussianKernel(21, -1);
		cv::Mat1f kernY = cv::getGaussianKernel(21, -1);
		kernel = kernX * kernY.t();
	}
	
	ofVec2f getKernelRange() {
		return ofVec2f(-0.005, 0.01);
	}
};

struct Sharpen : public AutoConvolutionSlide {
	
	void createKernel(cv::Mat1f& kernel) {
		
		float k[] = {
			0, -1, 0,
			-1, 5, -1,
			0, -1, 0
		};
		
		kernel = cv::Mat1f(3, 3, k).clone();
	}
};

struct HorizontalLines : public AutoConvolutionSlide {
	
	void createKernel(cv::Mat1f& kernel) {
		
		float k[] = {
			-1, -1, -1,
			2, 2, 2,
			-1, -1, -1
		};
		
		kernel = cv::Mat1f(3, 3, k).clone();
	}
};


struct VerticalLines : public AutoConvolutionSlide {
	
	void createKernel(cv::Mat1f& kernel) {
		
		float k[] = {
			-1, 2, -1,
			-1, 2, -1,
			-1, 2, -1
		};
		
		kernel = cv::Mat1f(3, 3, k).clone();
	}
};

struct SobelHorizontal : public AutoConvolutionSlide {
	
	void createKernel(cv::Mat1f& kernel) {
		
		float k[] = {
			-1, -2, -1,
			0, 0, 0,
			1, 2, 1
		};
		
		kernel = cv::Mat1f(3, 3, k).clone();
	}
};

struct SobelVertical : public AutoConvolutionSlide {
	
	void createKernel(cv::Mat1f& kernel) {
		
		float k[] = {
			-1, 0, 1,
			-2, 0, 2,
			-1, 0, 1
		};
		
		kernel = cv::Mat1f(3, 3, k).clone();
	}
};

struct Canny : public ofxPresentSlide {
	
	bool wantsVideoGrabber() { return true; }
	cv::Mat matA, matB;
	ofImage img;
	
	void draw() {
		sharedVidGrabber->update();
		
		if(sharedVidGrabber->isFrameNew()) {
			ofxCv::cvtColor(ofxCv::toCv(sharedVidGrabber->getPixelsRef()), matA, CV_BGR2GRAY);
			ofxCv::pyrDown(matA, matB);
			ofxCv::medianBlur(matB, 5);
			cv::equalizeHist(matB, matA);
			ofxCv::Canny(matA, matB, mapX(20, 200), mapX(50, 255));
			ofxCv::toOf(matB, img);
			img.update();
		}
		
		ofClear(ofColor::black);
		
		if(img.isAllocated()) {
			img.draw(0, 0, ofGetWidth(), ofGetHeight());
		}
	}
};

struct Reverb : public ofxPresentSlide {
	
	ofxAudioUnitFilePlayer sample;
	ofxAudioUnitFilePlayer impulse;
	ofxAudioUnitFilePlayer convolved;
	ofxAudioUnitTap sampleTap;
	ofxAudioUnitTap impulseTap;
	ofxAudioUnitTap convolvedTap;
	ofxAudioUnitOutput output;
	ofxAudioUnitMixer mixer;
	bool drawConvolved;
	
	void setup() {
		sample.setFile(ofToDataPath("room.aif"));
		impulse.setFile(ofToDataPath("impulse.aif"));
		convolved.setFile(ofToDataPath("room-convolved.aif"));
		
		sampleTap.setBufferLength(8192);
		impulseTap.setBufferLength(8192);
		convolvedTap.setBufferLength(8192);
		
		mixer.setInputBusCount(3);
		
		sample.connectTo(sampleTap).connectTo(mixer, 0);
		impulse.connectTo(impulseTap).connectTo(mixer, 1);
		convolved.connectTo(convolvedTap).connectTo(mixer, 2);
		
		mixer.connectTo(output);
		
		output.start();
		
		drawConvolved = false;
	}
	
	void exit() {
		output.stop();
	}
	
	void draw() {
		ofClear(ofColor::black);
		ofSetLineWidth(3);
		
		if(drawConvolved) {
			ofPolyline convolvedWaveform;
			convolvedTap.getLeftWaveform(convolvedWaveform, ofGetWidth(), ofGetHeight(), 16);
			ofSetColor(palette(3));
			convolvedWaveform.draw();
		} else {
			ofPolyline sampleWaveform, impulseWaveform;
			sampleTap.getLeftWaveform(sampleWaveform, ofGetWidth(), ofGetHeight() / 2., 16);
			impulseTap.getLeftWaveform(impulseWaveform, ofGetWidth(), ofGetHeight() / 2., 16);
			
			ofPushMatrix();
			{
				ofSetColor(palette(1));
				sampleWaveform.draw();
				
				ofTranslate(0, ofGetHeight() / 2.);
				
				ofSetColor(palette(4));
				impulseWaveform.draw();
			}
			ofPopMatrix();
		}
		
	}
	
	void keyPressed(int key) {
		if(key == '1') {
			sample.play();
			drawConvolved = false;
		} else if(key == '2') {
			impulse.play();
			drawConvolved = false;
		} else if(key == '3') {
			convolved.play();
			drawConvolved = true;
		}
	}
};

struct Reactive : public ofxPresentSlide {
	
	struct ReactivePrimitive {
		ofPtr<of3dPrimitive> primitive;
		vector<ofVec3f> origVerts;
		vector<ofVec3f> convVerts;
		vector<ofVec3f> lerpVerts;
		ofColor color;
	};
	
	vector<ReactivePrimitive> prims;
	
	ofxAudioUnitInput input;
	ofxAudioUnitOutput output;
	ofxAudioUnitFilePlayer filePlayer;
	ofxAudioUnitMixer inMixer, outMixer;
	ofxAudioUnitTap tap;
	
	void setup() {
		prims.resize(3);
		
		ofPtr<ofIcoSpherePrimitive> ico(new ofIcoSpherePrimitive);
		ico->set(ofGetHeight() / 7., 2);
		prims[0].primitive = ico;
		prims[0].color = ofFloatColor::fromHex(0x21A68D);
		
		ofPtr<ofSpherePrimitive> sphere(new ofSpherePrimitive);
		sphere->set(ofGetHeight() / 7., 35);
		prims[1].primitive = sphere;
		prims[1].color = ofFloatColor::fromHex(0x4E3C51);
		
		ofPtr<ofBoxPrimitive> box(new ofBoxPrimitive);
		float s = ofGetHeight() / 6.;
		float r = 20;
		box->set(s, s, s, r, r, r);
		prims[2].primitive = box;
		prims[2].color = ofFloatColor::fromHex(0xF25749);
		
		for(auto& p : prims) {
			p.origVerts = p.primitive->getMesh().getVertices();
			p.origVerts.insert(p.origVerts.end(), 1024, ofVec3f());
			p.lerpVerts = p.convVerts = p.origVerts;
		}
		
		filePlayer.setFile(ofToDataPath("talabot.m4a"));
		
		tap.setBufferLength(1024);
		
		filePlayer.connectTo(inMixer, 0);
		input.connectTo(inMixer, 1);
		inMixer.setInputVolume(1, 1);
		outMixer.setOutputVolume(0);
		output.setDevice("Built-in Output");
		
		inMixer.connectTo(tap).connectTo(outMixer).connectTo(output);
		
		input.start();
		output.start();
		
		ofEnableDepthTest();
	}
	
	void exit() {
		output.stop();
		ofDisableDepthTest();
		ofDisableBlendMode();
	}
	
	void update() {
		vector<float> samples;
		tap.getLeftSamples(samples);
		
		for(float& samp : samples) {
			samp *= 0.08;
		}
		
		if(!samples.empty()) {
			for(auto& p : prims) {
				vDSP_conv((float *)&p.origVerts.front(), 1,
						  &samples.back(), -1,
						  (float *)&p.convVerts.front(), 1,
						  p.convVerts.size() * 3 - samples.size(),
						  samples.size());
				
				vDSP_conv((float *)&p.origVerts.front() + 1, 3,
						  &samples.front(), 1,
						  (float *)&p.convVerts.front() + 1, 3,
						  p.convVerts.size() - samples.size(),
						  samples.size());
				
				vDSP_conv(((float *)&p.origVerts.front()) + 2, 3,
						  &samples.front() + samples.size() / 2, 1,
						  (float *)&p.convVerts.front() + 2, 3,
						  p.convVerts.size() - samples.size(),
						  samples.size() / 2);
				
				vector<ofVec3f>& outVerts = p.primitive->getMesh().getVertices();
				for(size_t i = 0; i < outVerts.size(); i++) {
					p.lerpVerts[i] = p.lerpVerts[i] * 0.6 + p.convVerts[i] * 0.4;
					outVerts[i] += p.lerpVerts[i] * 0.25;
					outVerts[i] = outVerts[i] * 0.98 + p.origVerts[i] * 0.02;
				}
			}
		}
	}
	
	void draw() {
		ofClear(ofColor::black);
		ofPushMatrix();
		{
			ofTranslate(ofGetWidth() / 2., ofGetHeight() / 2.);
			ofRotateX(ofGetElapsedTimef() * 10.);
			ofRotateY(ofGetElapsedTimef() * 13.);
			ofRotateZ(ofGetElapsedTimef() * 16.);
			
			ofEnableBlendMode(OF_BLENDMODE_SCREEN);
			
			for(auto& p : prims) {
				ofSetColor(p.color);
				p.primitive->drawFaces();
			}
		}
		ofPopMatrix();
	}
	
	void keyPressed(int key) {
		if(key == 'p') {
			filePlayer.play();
			inMixer.setInputVolume(0, 1);
			outMixer.setOutputVolume(1);
		}
	}
};

struct Title : public ofxPresentSlide {
	bool wantsHTMLPage() { return true; }
	string HTMLPageName() { return "slides/convolution-title"; }
};

struct Diagram : public ofxPresentSlide {
	bool wantsHTMLPage() { return true; }
	string HTMLPageName() { return "slides/convolution-diagram"; }
};

struct Code : public ofxPresentSlide {
	bool wantsHTMLPage() { return true; }
	string HTMLPageName() { return "slides/convolution-code"; }
};


struct KinectBandingOne : public ofxPresentSlide {
	bool wantsHTMLPage() { return true; }
	string HTMLPageName() { return "slides/convolution-kinect-one"; }
};

struct KinectBandingTwo : public ofxPresentSlide {
	bool wantsHTMLPage() { return true; }
	string HTMLPageName() { return "slides/convolution-kinect-two"; }
};

struct KinectBandingForum : public ofxPresentSlide {
	bool wantsHTMLPage() { return true; }
	string HTMLPageName() { return "slides/convolution-kinect-forum"; }
};

struct KinectBandingThree : public ofxPresentSlide {
	bool wantsHTMLPage() { return true; }
	string HTMLPageName() { return "slides/convolution-kinect-three"; }
};

struct NeuralNetworkDiagram : public ofxPresentSlide {
	bool wantsHTMLPage() { return true; }
	string HTMLPageName() { return "slides/convolution-neural-network-diagram"; }
};

struct NeuralNetworkDiagramHighlighted : public ofxPresentSlide {
	bool wantsHTMLPage() { return true; }
	string HTMLPageName() { return "slides/convolution-neural-network-diagram-highlighted"; }
};

struct NeuralNetworkGoogle : public ofxPresentSlide {
	bool wantsHTMLPage() { return true; }
	string HTMLPageName() { return "slides/convolution-neural-network-google"; }
};

struct NeuralNetworkDeepFace : public ofxPresentSlide {
	bool wantsHTMLPage() { return true; }
	string HTMLPageName() { return "slides/convolution-neural-network-deepface"; }
};

struct NeuralNetworkPaper : public ofxPresentSlide {
	bool wantsHTMLPage() { return true; }
	string HTMLPageName() { return "slides/convolution-neural-network-paper"; }
};

struct NeuralNetworkKernels : public ofxPresentSlide {
	bool wantsHTMLPage() { return true; }
	string HTMLPageName() { return "slides/convolution-neural-network-kernels"; }
};

struct NeuralNetworkVideo : public ofxPresentSlide {
	bool wantsHTMLPage() { return true; }
	string HTMLPageName() { return "slides/convolution-neural-network-video"; }
};

} // namespace conv
