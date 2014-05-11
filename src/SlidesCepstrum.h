#pragma once

#include "SlideUtils.h"
#include "ofxAudioUnit.h"
#include "xtract/libxtract.h"

BEGIN_NAMESPACE(cepstrum)

struct Title : public ofxPresentSlide {
	bool wantsHTMLPage() { return true; }
	string HTMLPageName() { return "slides/cepstrum-title"; }
};

struct Wiki : public ofxPresentSlide {
	bool wantsHTMLPage() { return true; }
	string HTMLPageName() { return "slides/cepstrum-wiki"; }
};

struct CepstrumSlide : public ofxPresentSlide {
		
	CepstrumSlide() {
		mel_filters.n_filters = mel_band_count;
		mel_filters.filters = (double **)malloc(mel_band_count * sizeof(double *));
		
		for(size_t n = 0; n < mel_band_count; ++n) {
			mel_filters.filters[n] = (double *)malloc(block_size * sizeof(double));
		}
		
		_lerpedMfccs.resize(mel_band_count);
		_spectrum.resize(block_size);
		window = xtract_init_window(block_size, XTRACT_HANN);
		xtract_init_fft(block_size, XTRACT_SPECTRUM);
		xtract_init_mfcc(block_size >> 1, 44100 >> 1, XTRACT_EQUAL_GAIN, freq_min, freq_max, mel_filters.n_filters, mel_filters.filters);
	}
	
	~CepstrumSlide() {
		for(size_t n = 0; n < mel_band_count; ++n) {
			free(mel_filters.filters[n]);
		}
		free(mel_filters.filters);
		
		xtract_free_fft();
		xtract_free_window(window);
	}
	
	void setup() {
		_setup();
		
		mixer.setInputVolume(0);
		mixer.setOutputVolume(0);
		
		tap.setBufferLength(2048);
		input.connectTo(tap).connectTo(mixer).connectTo(output);
		
		input.start();
		output.start();
	}
	
	void exit() {
		output.stop();
		input.stop();
		_exit();
	}
	
	void calcCepstrum(const vector<float>& signal) {
		
		if(signal.size() < block_size) {
			return;
		}
		
		_signal.resize(signal.size());
		vDSP_vspdp(&signal[0], 1, &_signal[0], 1, _signal.size());
		
		// windowing
		double windowed[block_size];
		xtract_windowed(&_signal[0],
						min(_signal.size(), block_size),
						window,
						windowed);
		
		// fft
		double fftArgs[] = {44100 / (double)block_size, XTRACT_MAGNITUDE_SPECTRUM, 0, 1};
		xtract[XTRACT_SPECTRUM](windowed, block_size, fftArgs, &_spectrum[0]);
		
		// cepstrum
		_mfccs.resize(mel_band_count);
		xtract_mfcc(&_spectrum[0], block_size >> 1, &mel_filters, &_mfccs[0]);
		
		for(int i = 0; i < _mfccs.size(); i++) {
			_lerpedMfccs[i] = ofLerp(_lerpedMfccs[i], _mfccs[i], 0.7);
		}
	}
	
	const vector<double>& getLerpedMFCCs() { return _lerpedMfccs; }
	const vector<double>& getSpectrum() { return _spectrum; }
	
protected:
	
	virtual void _setup() { };
	virtual void _exit() { };
	
	ofxAudioUnitInput input;
	ofxAudioUnitOutput output;
	ofxAudioUnitMixer mixer;
	ofxAudioUnitTap tap;
	
	xtract_mel_filter mel_filters;
	double * window;
	
	const size_t block_size = 1024;
	const size_t mel_band_count = 10;
	const int freq_min = 20;
	const int freq_max = 15000;
	vector<double> _signal;
	vector<double> _mfccs;
	vector<double> _lerpedMfccs;
	vector<double> _spectrum;
};

struct ComparisonWithFourier : public CepstrumSlide {
	
	void update() {
		vector<float> signal;
		tap.getSamples(signal);
		calcCepstrum(signal);
	}
	
	void draw() {
		
		ofPolyline cepstrumLine;
		const vector<double>& mfccs = getLerpedMFCCs();
		for (int i = 1; i < mfccs.size(); i++) {
			cepstrumLine.addVertex(ofMap(i, 1, mfccs.size() - 1, 0, ofGetWidth()),
								   ofMap(mfccs[i], -8, 8, ofGetHeight() / 2., 0));
		}
		
		ofPolyline spectrumLine;
		const vector<double>& spectrum = getSpectrum();
		for (int i = 0; i < spectrum.size() / 2; i++) {
			spectrumLine.addVertex(ofMap(i, 0, spectrum.size() / 2. - 1, 0, ofGetWidth()),
								   ofMap(spectrum[i], 0, 1, ofGetHeight() / 2., 0));
		}
		
		ofClear(ofColor::black);
		ofSetLineWidth(5);
		
		ofSetColor(palette(4));
		cepstrumLine.draw();
		
		ofPushMatrix();
		{
			ofTranslate(0, ofGetHeight() / 2.);
			ofSetColor(palette(3));
			spectrumLine.draw();
		}
		ofPopMatrix();
	}
};

struct Mesh : public CepstrumSlide {
	
	ofPlanePrimitive plane;
	const float depth = 800;
	
	void _setup() {
		plane.setUseVbo(true);
		plane.enableColors();
		plane.set(depth * 3., depth * 2., mel_band_count, 50);
		plane.getMesh().setColorForIndices(0, plane.getMesh().getVertices().size(), ofColor::white);
		ofEnableDepthTest();
	}
	
	void _exit() {
		ofDisableDepthTest();
	}
	
	void update() {
		vector<float> signal;
		tap.getSamples(signal);
		calcCepstrum(signal);
		
		vector<double> cepstrum = getLerpedMFCCs();
		
		vector<ofVec3f>& verts = plane.getMesh().getVertices();
		vector<ofFloatColor>& colors = plane.getMesh().getColors();

		for(int i = verts.size() - 1; i >= plane.getNumColumns(); i--) {
			verts[i].z = verts[i - plane.getNumColumns()].z;
			colors[i] = colors[i - plane.getNumColumns()];
		}
		
		for(int i = 0; i < MIN(plane.getNumColumns(), cepstrum.size()); i++) {
			verts[i].z = cepstrum[i] * ((i + 1) * 0.5) * (depth * 0.1) ;
			colors[i] = ofFloatColor::fromHsb(ofMap(cepstrum[i], -3, 3, 0, 0.5, true),
											  ofMap(abs(cepstrum[i]), 0, 5, 0.4, 1, true),
											  ofMap(abs(cepstrum[i]), 0, 2, 0.7, 1, true));
		}
		
	}
	
	void draw() {
		ofClear(ofColor::black);
		
		ofPushMatrix();
		{
			ofTranslate(ofGetWidth() / 2., ofGetHeight() / 2., -depth * 1.5);
			ofRotateX(sin(ofGetElapsedTimef() * 0.4) * 10. + 60.);
			ofRotateZ(cos(ofGetElapsedTimef() * 0.3) * 10. + 180.);
			
			ofSetColor(ofColor::white);
			plane.draw();
		}
		ofPopMatrix();
	}
	
};

} // end namespace cepstrum
