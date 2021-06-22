#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {

	padParams.setName("Pad Params");
	padParams.add(pad1.set("Pad1", false));
	padParams.add(pad2.set("Pad2", false));
	padParams.add(pad3.set("Pad3", false));
	padParams.add(pad4.set("Pad4", false));
	padParams.add(pad5.set("Pad5", false));
	padParams.add(pad6.set("Pad6", false));
	padParams.add(pad7.set("Pad7", false));
	padParams.add(pad8.set("Pad8", false));

	kParams.add(k1.set("K1", 0.5, 0.0, 1.0));
	kParams.add(k2.set("K2", 0.5, 0.0, 1.0));
	kParams.add(k3.set("K3", 0.5, 0.0, 1.0));
	kParams.add(k4.set("K4", 0.5, 0.0, 1.0));
	kParams.add(k5.set("K5", 0.5, 0.0, 1.0));
	kParams.add(k6.set("K6", 0.5, 0.0, 1.0));
	kParams.add(k7.set("K7", 0.5, 0.0, 1.0));
	kParams.add(k8.set("K8", 0.5, 0.0, 1.0));

	//-

	mMidiParams.connect();
	mMidiParams.add(padParams); // -> add groups
	mMidiParams.add(kParams);

	//mMidiParams.setFilenameSettings("settings2.xml"); // -> you can customize name if required

	//// you could also add individually
	////mMidiParams.add( k1 );
	////mMidiParams.add( k2 );
	////mMidiParams.add( k3 );

	//TODO: nested groups seems to fail..

	// to connect incoming MIDI to ofParameters passed into mMidiParams,
	// click on the parameter listed in the mMidiParams gui and
	// the next MIDI message will be used for mapping to that parameter.

	//--

	ofxSurfingHelpers::setThemeDark_ofxGui();
	gui.setup("ofApp");
	gui.add(padParams); // test midi out
	gui.add(kParams);
	gui.setPosition(10, ofGetWindowHeight() - gui.getHeight() - 10);
}

//--------------------------------------------------------------
void ofApp::update() {
	// buttons off
	if (ofGetElapsedTimeMillis() - lastKey > 30 && bWait) {
		bWait = false;

		if (pad5) pad5 = false;
		if (pad6) pad6 = false;
		if (pad7) pad7 = false;
		if (pad8) pad8 = false;
	}
}

//--------------------------------------------------------------
void ofApp::draw() {
	float tx = 0.f;
	float ty = 0.f;
	ofPushMatrix(); {
		ofScale(0.4);

		ofTranslate(30, 30);
		for (int i = 0; i < padParams.size(); i++) {
			if (i % 4 == 0 && i > 0) {
				tx = 0;
				ty += 150 + 10.f;
			}

			ofRectangle trect(tx, ty, 150, 150);

			ofSetColor(170);

			ofDrawRectangle(trect);

			ofSetColor(60);
			ofDrawRectangle(trect.x + 3, trect.y + 3, trect.width - 6, trect.height - 6);

			if (padParams.getBool(i)) {
				ofSetColor(220, 50, 80);
				ofDrawRectangle(trect.x + 3, trect.y + 3, trect.width - 6, trect.height - 6);
			}
			ofSetColor(100);
			ofNoFill();
			ofDrawRectangle(trect);
			ofFill();

			tx += trect.width + 10.f;
		}

		ofTranslate(75, 150);
		ty += 150;
		tx = 0;
		for (int i = 0; i < kParams.size(); i++) {
			if (i % 4 == 0 && i > 0) {
				tx = 0;
				ty += 150 + 10.f;
			}

			ofPushMatrix(); {
				ofTranslate(tx, ty);

				ofSetColor(170);
				ofDrawCircle(0, 0, 56);

				ofSetColor(60);
				ofDrawCircle(0, 0, 50);

				ofSetColor(220);
				ofPushMatrix(); {
					ofRotateDeg(ofMap(kParams.getFloat(i), 0.0, 1.0, -140, 140));
					ofDrawRectangle(-2, -50, 4, 50);
				} ofPopMatrix();

				ofSetColor(100);
				ofNoFill();
				ofDrawCircle(0, 0, 56);
				ofFill();

			} ofPopMatrix();

			tx += 150 + 10.f;
		}
	} ofPopMatrix();

	mMidiParams.draw();

	gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

	// to test midi out

	// toggles
	if (key == '1') pad1 = !pad1;
	if (key == '2') pad2 = !pad2;
	if (key == '3') pad3 = !pad3;
	if (key == '4') pad4 = !pad4;
	//buttons
	if (key == '5') { pad5 = true; lastKey = ofGetElapsedTimeMillis(); bWait = true; }
	if (key == '6') { pad6 = true; lastKey = ofGetElapsedTimeMillis(); bWait = true; }
	if (key == '7') { pad7 = true; lastKey = ofGetElapsedTimeMillis(); bWait = true; }
	if (key == '8') { pad8 = true; lastKey = ofGetElapsedTimeMillis(); bWait = true; }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}
