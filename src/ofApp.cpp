#include "ofApp.h"

#include "Slides.h"

void ofApp::setup(){
	
	// This is the slide list. If you'd like to see the source
	// for a particular slide, hold CMD and click on it below
	
	// Some slides contain music, which isn't included in this
	// repo. Just add the file you'd like to use to this app's
	// "data" folder, and replace the filename in the slide
	
	addSlide<intro::Hi>();
	addSlide<intro::NoInspireOne>();
	addSlide<intro::NoInspireTwo>();
	addSlide<intro::TalkName>();
	
	addSlide<intro::RightQuestions>();
	addSlide<intro::SignalQuestion>();
	addSlide<intro::TimeDomainVis>();
	
	addSlide<intro::Intuition>();
	addSlide<intro::Sin>();
	addSlide<intro::SinDefinition>();
	addSlide<intro::SinDefinitionPic>();
	addSlide<intro::SinVis>();
	
	addSlide<convolution::Title>();
	addSlide<convolution::Diagram>();
	addSlide<convolution::Code>();
	
	addSlide<convolution::Blank>();
	addSlide<convolution::MotionBlur>();
	addSlide<convolution::GaussianBlur>();
	addSlide<convolution::BoxBlur>();
	addSlide<convolution::Sharpen>();
	addSlide<convolution::Emboss>();
	addSlide<convolution::Perlin>();
	
	addSlide<convolution::Gabor>();
	addSlide<convolution::HorizontalLines>();
	addSlide<convolution::VerticalLines>();
	addSlide<convolution::MexicanHat>();
	addSlide<convolution::SobelHorizontal>();
	addSlide<convolution::SobelVertical>();
	addSlide<convolution::Canny>();
	
	addSlide<convolution::KinectBandingOne>();
	addSlide<convolution::KinectBandingTwo>();
	addSlide<convolution::KinectBandingForum>();
	addSlide<convolution::KinectBandingThree>();
	
	addSlide<convolution::Reverb>();
	
	addSlide<convolution::NeuralNetworkDiagram>();
	addSlide<convolution::NeuralNetworkDiagramHighlighted>();
	addSlide<convolution::NeuralNetworkGoogle>();
	addSlide<convolution::NeuralNetworkDeepFace>();
	addSlide<convolution::NeuralNetworkPaper>();
	addSlide<convolution::NeuralNetworkKernels>();
	addSlide<convolution::NeuralNetworkVideo>();
	
	addSlide<convolution::Reactive>();
	
	addSlide<convolution::Diagram>();
	addSlide<convolution::Code>();
	
	addSlide<fourier::Title>();
	addSlide<fourier::Sound>();
	addSlide<fourier::Video>();
	
	addSlide<cepstrum::Title>();
	addSlide<cepstrum::Wiki>();
	addSlide<cepstrum::ComparisonWithFourier>();
	addSlide<cepstrum::Mesh>();
	
	addSlide<intro::Bye>();
}

void ofApp::update(){

}

void ofApp::draw(){
	ofSetColor(ofColor::white);
	ofDrawBitmapStringHighlight(ofToString((int)ofGetFrameRate()), 20, 20);
}

void ofApp::keyPressed(int key){
	if(key == OF_KEY_RIGHT) {
		nextSlide();
	} else if(key == OF_KEY_LEFT) {
		previousSlide();
	} else if(key == 'r') {
		refreshHTML();
	} else {
		getCurrentSlide()->keyPressed(key);
	}
}

void ofApp::keyReleased(int key){

}

void ofApp::mouseMoved(int x, int y ){

}

void ofApp::mouseDragged(int x, int y, int button){

}

void ofApp::mousePressed(int x, int y, int button){

}

void ofApp::mouseReleased(int x, int y, int button){

}

void ofApp::windowResized(int w, int h){

}

void ofApp::gotMessage(ofMessage msg){

}

void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
