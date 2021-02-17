//
//  ofxMidiParams.cpp
//  MidiParameter
//
//  Created by Nick Hardeman on 12/28/18.
//

#include "ofxMidiParams.h"

//--------------------------------------------------------------
ofxMidiParams::ofxMidiParams() {
	enableMouseEvents();
	_updatePositions();

	//TODO:
	setup();
}

//TODO: splitted-out funtion to avoid crash...

//--------------------------------------------------------------
void ofxMidiParams::setup()
{
	//-

	//custom colors
	//colorFill = ofColor::yellowGreen;
	colorFill = 0;

	//-

	//custom font
	std::string path;
	path = "assets/fonts/";
	path += "overpass-mono-bold.otf";
	path = ofToString(path);
	//path = ofToString(path_GLOBAL + "/" + "fonts/" + "overpass-mono-bold.otf");

	fontSize = 9;
	myFont.loadFont(path, fontSize);
	if (myFont.isLoaded())
	{
		ofLogNotice(__FUNCTION__) << "Loaded font '" << path << "'";
	}
	else
	{
		ofLogError(__FUNCTION__) << "Font '" << path << "' NOT FOUND!";
	}

	//--

	//TODO: 
	//midi out

	mParamsGroup.setName("ofxMidiParams");
	ofAddListener(mParamsGroup.parameterChangedE(), this, &ofxMidiParams::Changed_Controls);

	// connect
	midiOut.listOutPorts();
	midiOut.openPort(2); // by number
	//midiOut.openPort("IAC Driver Pure Data In"); // by name
	//midiOut.openVirtualPort("ofxMidiOut"); // open a virtual port

	ofLogNotice(__FUNCTION__) << "MidiOut";
	ofLogNotice(__FUNCTION__) << "Connected to Port " << midiOut.getPort();
	ofLogNotice(__FUNCTION__) << midiOut.getName();
	ofLogNotice(__FUNCTION__) << (midiOut.isVirtual() ? "is Virtual" : "Not Virtual");
}

//--------------------------------------------------------------
void ofxMidiParams::Changed_Controls(ofAbstractParameter &e)
{
	std::string name = e.getName();

	ofLogNotice(__FUNCTION__) << name << " : " << e;

	//----

	for (int i = 0; i < mAssocParams.size(); i++) {//iterate all added parameters...

		string n1 = mAssocParams[i]->xmlName;
		string n2 = mParamsGroup.getName(i);
		//ofLogNotice(__FUNCTION__) << "n1:" << n1;
		//ofLogNotice(__FUNCTION__) << "n2:" << n2;

		//auto& ma = mAssocParams[i];
		//float val = getParamValuePct(ma);

		if (n1 == name)
		{
			auto &aparam = mParamsGroup.get(n2);

			//TODO:
			//only bools/notes implemented. 
			//bc I have only an Akai mpd218 with illuminated toggles. 
			//not motorized/illuminated knobs. so i don't need midi cc's.

			if (aparam.type() == typeid(ofParameter<bool>).name()) {
				ofParameter<bool> b = aparam.cast<bool>();

				int pitch = mAssocParams[i]->midiId - 512;
				//auto type2 = mAssocParams[i]->ptype;//bool
				//auto name2 = mAssocParams[i]->displayMidiName;

				int ch = 1;
				if (b.get()) {
					midiOut.sendNoteOn(ch, pitch, 127);
					ofLogNotice() << "noteOn: " << pitch;
				}
				else {
					midiOut.sendNoteOff(ch, pitch, 0);
					ofLogNotice() << "noteOff: " << pitch;
				}

				return;
			}
		}
	}
}

//--------------------------------------------------------------
ofxMidiParams::~ofxMidiParams() {
	disableMouseEvents();
	disconnect();
	if (bHasUpdateEvent) {
		ofRemoveListener(ofEvents().update, this, &ofxMidiParams::update);
	}
	bHasUpdateEvent = false;

	ofRemoveListener(mParamsGroup.parameterChangedE(), this, &ofxMidiParams::Changed_Controls);
	// clean up
	midiOut.closePort();
}

//--------------------------------------------------------------
bool ofxMidiParams::connect(int aport, bool abRetryConnection) {

	bTryReconnect = abRetryConnection;
	// print input ports to console
	midiIn.listInPorts();

	if (!bHasUpdateEvent) {
		ofAddListener(ofEvents().update, this, &ofxMidiParams::update);
	}
	bHasUpdateEvent = true;

	vector<string> tportNames = midiIn.getInPortList();
	if (aport < 0 || aport >= tportNames.size()) {
		ofLogNotice("ofxMidiParams :: connect : port ") << aport << " is out of range.";
		return false;
	}

	// now try to match to a device //
	mDesiredDeviceNameToOpen = tportNames[aport];
	ofLogNotice("ofxMidiParams :: connect : attempting to connect to port: ") << aport << " for device: " << mDesiredDeviceNameToOpen;

	// open port by number (you may need to change this)
	bConnected = midiIn.openPort(aport);
	//midiIn.openPort("IAC Pure Data In");    // by name
	//midiIn.openVirtualPort("ofxMidiIn Input"); // open a virtual port

	// don't ignore sysex, timing, & active sense messages,
	// these are ignored by default
	midiIn.ignoreTypes(false, false, false);

	// add ofApp as a listener
	if (bConnected) midiIn.addListener(this);
	return bConnected;
}

//--------------------------------------------------------------
bool ofxMidiParams::connect(string aDeviceName, bool abRetryConnection) {
	//ofLogWarning() << __FUNCTION__ << " ";

	mDesiredDeviceNameToOpen = aDeviceName;
	int tport = _getDesiredPortToOpen();
	return connect(tport, abRetryConnection);
}

//--------------------------------------------------------------
void ofxMidiParams::disconnect() {
	//ofLogWarning() << __FUNCTION__ << " ";

	if (bConnected) {
		midiIn.closePort();
		midiIn.removeListener(this);
	}
	bConnected = false;
}

//--------------------------------------------------------------
bool ofxMidiParams::isConnected() {
	//ofLogWarning() << __FUNCTION__ << " ";

	return bConnected && midiIn.isOpen();
}

//--------------------------------------------------------------
bool ofxMidiParams::save(string aXmlFilepath) {
	//ofLogWarning() << __FUNCTION__ << " ";

	if (aXmlFilepath != "") {
		mXmlFilePath = aXmlFilepath;
	}

	if (mXmlFilePath == "") {
		ofFileDialogResult fd = ofSystemSaveDialog("midiparams.xml", "Choose XML file name to save. ");
		if (fd.fileName != "" && fd.bSuccess) {
			mXmlFilePath = fd.filePath;
		}
	}

	ofXml txml;
	auto root = txml.getChild("midiparams");
	if (!root) {
		root = txml.appendChild("midiparams");
	}

	for (int i = 0; i < mAssocParams.size(); i++) {
		auto max = root.appendChild("param");
		max.setAttribute("paramName", mAssocParams[i]->xmlName);
		max.setAttribute("group", mAssocParams[i]->xmlParentName);
		max.setAttribute("midiid", mAssocParams[i]->midiId);
		max.setAttribute("displayName", mAssocParams[i]->displayMidiName);
	}

	return txml.save(mXmlFilePath);
}

//--------------------------------------------------------------
bool ofxMidiParams::load(string aXmlFilepath) {
	//ofLogWarning() << __FUNCTION__ << " ";

	mXmlFilePath = aXmlFilepath;
	ofXml txml;
	if (txml.load(mXmlFilePath)) {
		auto paramsXML = txml.find("//midiparams/param");
		for (auto& pxml : paramsXML) {
			string paramName = pxml.getAttribute("paramName").getValue();
			string groupName = pxml.getAttribute("group").getValue();
			int midiid = pxml.getAttribute("midiid").getIntValue();
			string dname = pxml.getAttribute("displayName").getValue();
			// now find the associated param //
			for (auto& ma : mAssocParams) {
				if (ma->xmlName == paramName && ma->xmlParentName == groupName) {
					ma->midiId = midiid;
					ma->displayMidiName = dname;
				}
			}
		}
		return true;
	}
	ofLogError() << "Couldn't load xml file from " << mXmlFilePath;
	return false;
}

#pragma mark - Update
//--------------------------------------------------------------
void ofxMidiParams::update(ofEventArgs& args) {
	uint64_t emillis = ofGetElapsedTimeMillis();
	if (emillis >= mNextCheckMillis) {
		//ofLogWarning() << __FUNCTION__ << " ";

		vector<string> tportNames = midiIn.getInPortList();
		bool bopen = midiIn.isOpen();
		if (tportNames.size() == 0) {
			if (bopen) {
				disconnect();
			}
		}

		bopen = midiIn.isOpen();
		if (bopen && mDesiredDeviceNameToOpen != "") {
			int tport = _getDesiredPortToOpen();
			if (tport < 0) disconnect();
		}


		//cout << "bopen: " << bopen << " port: " << midiIn.getPort() << " | " << ofGetFrameNum() << endl;

		if (!bConnected && bTryReconnect) {
			if (mDesiredDeviceNameToOpen != "") {
				connect(_getDesiredPortToOpen(), true);
			}
			else {
				connect(0, true);
			}
		}
		mNextCheckMillis = emillis + 3000;
	}

	if (mSelectedParam && mSelectedParam->bNeedsTextPrompt) {
		mSelectedParam->displayMidiName = ofSystemTextBoxDialog("MIDI Display name.", mSelectedParam->displayMidiName);
		ofStringReplace(mSelectedParam->displayMidiName, "<", "");
		ofStringReplace(mSelectedParam->displayMidiName, ">", "");
		ofStringReplace(mSelectedParam->displayMidiName, "\r", "");
		ofStringReplace(mSelectedParam->displayMidiName, "\n", "");
		ofStringReplace(mSelectedParam->displayMidiName, "\t", "");
		ofStringReplace(mSelectedParam->displayMidiName, "%", "");
		mSelectedParam->bNeedsTextPrompt = false;
		mSelectedParam.reset();
	}
}

#pragma mark - MIDI Messages
//--------------------------------------------------------------
void ofxMidiParams::newMidiMessage(ofxMidiMessage& amsg) {
	//ofLogWarning() << __FUNCTION__ << " ";

	midiMessages.push_back(amsg);

	while (midiMessages.size() > 6) {
		midiMessages.erase(midiMessages.begin());
	}

	mMidiMessageHistoryStr = "";
	stringstream ss;
	for (int i = (int)midiMessages.size() - 1; i >= 0; i--) {
		ofxMidiMessage& message = midiMessages[i];
		string sstatus = ofxMidiMessage::getStatusString(message.status);
		if (message.status == MIDI_CONTROL_CHANGE) {
			sstatus = "CC";
		}
		ss << sstatus;
		int treq = 10 - sstatus.length();
		for (int k = 0; k < treq; k++) {
			ss << " ";
		}
		if (message.status < MIDI_SYSEX) {
			ss << "ch:" << message.channel;
			if (message.status == MIDI_CONTROL_CHANGE) {
				ss << " ctl:" << message.control << " val:" << message.value;// << " pitch: " << message.pitch;
			}
			else if (message.status == MIDI_PITCH_BEND) {
				ss << "\tval:" << message.value;
			}
			else {
				ss << "\tpitch:" << message.pitch;// << " value: "<<message.value;// << " control: " << message.control
				ss << "\tvel:" << message.velocity;
			}
		}
		ss << endl;
	}
	if (midiMessages.size() > 0) {
		mMidiMessageHistoryStr = ss.str();
	}


	if (mSelectedParam) {
		//ofLogWarning() << __FUNCTION__ << " ";

		mSelectedParam->midiId = getId(amsg);
		string tname = "default";
		if (amsg.status < MIDI_SYSEX) {
			//ss << "chan: " << message.channel;
			if (amsg.status == MIDI_CONTROL_CHANGE) {
				tname = "Control" + ofToString(amsg.control, 0);
			}
			else if (amsg.status == MIDI_PITCH_BEND) {
				tname = "PitchBend";
			}
			else {
				tname = "Note " + ofToString(amsg.pitch, 0);
			}
		}
		mSelectedParam->displayMidiName = tname;//ofSystemTextBoxDialog("MIDI Display name.", tname );
		mSelectedParam->bNeedsTextPrompt = true;
		mSelectedParam->bListening = false;
		//mSelectedParam.reset();
	}

	int tid = getId(amsg);
	// see if we have an assoc //
	for (int i = 0; i < mAssocParams.size(); i++) {
		//ofLogWarning() << __FUNCTION__ << " ";

		auto& ma = mAssocParams[i];
		if (ma->midiId == tid) {
			if (amsg.status == MIDI_CONTROL_CHANGE) {
				float pct = ofMap(amsg.value, 0, 127, 0.0, 1.0f, true);
				setParamValue(ma, pct);
			}
			else if (amsg.status == MIDI_NOTE_OFF) {
				setParamValue(ma, 0.0);
			}
			else if (amsg.status == MIDI_NOTE_ON) {
				setParamValue(ma, 1.0);
			}
		}
	}
}

//--------------------------------------------------------------
void ofxMidiParams::setParamValue(shared_ptr<MidiParamAssoc>& am, float avaluePct) {
	//ofLogWarning() << __FUNCTION__ << " ";

	if (am->ptype == PTYPE_FLOAT) {
		ofParameter<float> f = mParamsGroup.getFloat(am->paramIndex);
		f = f.getMin() + ofClamp(avaluePct, 0, 1) * (f.getMax() - f.getMin());

		////TODO: this gets when incomming midi 
		//cout << "setParamValue f: " << f << endl;
	}
	else if (am->ptype == PTYPE_BOOL) {
		ofParameter<bool> f = mParamsGroup.getBool(am->paramIndex);
		if (avaluePct > 0.0) f = true;
		else f = false;
	}
	else if (am->ptype == PTYPE_INT) {
		ofParameter<int> f = mParamsGroup.getInt(am->paramIndex);
		f = (float)f.getMin() + ofClamp(avaluePct, 0, 1) * (float)(f.getMax() - f.getMin());
	}

}

//--------------------------------------------------------------
float ofxMidiParams::getParamValue(shared_ptr<MidiParamAssoc>& am) {
	//ofLogWarning() << __FUNCTION__ << " ";

	if (am->ptype == PTYPE_FLOAT) {
		return mParamsGroup.getFloat(am->paramIndex);
	}
	else if (am->ptype == PTYPE_BOOL) {
		ofParameter<bool> f = mParamsGroup.getBool(am->paramIndex);
		//TODO: this gets when incomming midi 
		//cout << "getParamValue f: " << f << endl;
		return (f == true ? 1.f : 0.0);
	}
	else if (am->ptype == PTYPE_INT) {
		return (float)mParamsGroup.getInt(am->paramIndex);
	}
	return 0.0;
}

//--------------------------------------------------------------
float ofxMidiParams::getParamValue(const shared_ptr<MidiParamAssoc>& am) const {
	//ofLogWarning() << __FUNCTION__ << " ";

	if (am->ptype == PTYPE_FLOAT) {
		ofParameter<float> f = mParamsGroup.getFloat(am->paramIndex);
		//TODO: this gets when incomming midi 
		//cout << "getParamValue f: " << f << endl;
		return f;
	}
	else if (am->ptype == PTYPE_BOOL) {
		ofParameter<bool> f = mParamsGroup.getBool(am->paramIndex);
		//TODO: this gets when incomming midi 
		//cout << "getParamValue f: " << f << endl;
		return (f == true ? 1.f : 0.0);
	}
	else if (am->ptype == PTYPE_INT) {
		ofParameter<int> f = mParamsGroup.getInt(am->paramIndex);
		//TODO: this gets when incomming midi 
		//cout << "getParamValue f: " << f << endl;
		return f;
	}
	return 0.0;
}

//--------------------------------------------------------------
float ofxMidiParams::getParamValuePct(shared_ptr<MidiParamAssoc>& am) {
	//ofLogWarning() << __FUNCTION__ << " ";

	if (am->ptype == PTYPE_FLOAT) {
		ofParameter<float> f = mParamsGroup.getFloat(am->paramIndex);
		return (f - f.getMin()) / (f.getMax() - f.getMin());
	}
	else if (am->ptype == PTYPE_BOOL) {
		ofParameter<bool> f = mParamsGroup.getBool(am->paramIndex);
		return (f == true ? 1.f : 0.0);
	}
	else if (am->ptype == PTYPE_INT) {
		ofParameter<int> f = mParamsGroup.getInt(am->paramIndex);
		return ((float)f - (float)f.getMin()) / ((float)f.getMax() - (float)f.getMin());
	}
	return 0.0;
}

//--------------------------------------------------------------
float ofxMidiParams::getParamValuePct(const shared_ptr<MidiParamAssoc>& am) const {
	//ofLogWarning() << __FUNCTION__ << " ";

	if (am->ptype == PTYPE_FLOAT) {
		ofParameter<float> f = mParamsGroup.getFloat(am->paramIndex);
		return (f - f.getMin()) / (f.getMax() - f.getMin());
	}
	else if (am->ptype == PTYPE_BOOL) {
		ofParameter<bool> f = mParamsGroup.getBool(am->paramIndex);
		return (f == true ? 1.f : 0.0);
	}
	else if (am->ptype == PTYPE_INT) {
		ofParameter<int> f = mParamsGroup.getInt(am->paramIndex);
		return ((float)f - (float)f.getMin()) / ((float)f.getMax() - (float)f.getMin());
	}
	return 0.0;
}

//--------------------------------------------------------------
int ofxMidiParams::getId(ofxMidiMessage& amess) {
	//ofLogWarning() << __FUNCTION__ << " ";

	if (amess.control > 0) return amess.control;
	if (amess.pitch > 0) return 512 + amess.pitch;
	return amess.pitch + 512.f * (amess.control + 1);
}

//--------------------------------------------------------------
void ofxMidiParams::add(ofParameterGroup aparams) {
	for (int i = 0; i < aparams.size(); i++) {
		addParam(aparams.get(i));
	}
}

//--------------------------------------------------------------
void ofxMidiParams::add(ofParameter<float>& aparam) {
	addParam(aparam);
}

//--------------------------------------------------------------
void ofxMidiParams::add(ofParameter<bool>& aparam) {
	addParam(aparam);
}

//--------------------------------------------------------------
void ofxMidiParams::add(ofParameter<int>& aparam) {
	addParam(aparam);
}

//--------------------------------------------------------------
void ofxMidiParams::addParam(ofAbstractParameter& aparam) {
	auto mac = make_shared<ofxMidiParams::MidiParamAssoc>();
	mac->paramIndex = mParamsGroup.size();

	//ofLogWarning() << __FUNCTION__ << " ";

	if (aparam.type() == typeid(ofParameter<int>).name()) {
		mac->ptype = PTYPE_INT;
		ofParameter<int> ti = aparam.cast<int>();
		ofParameterGroup pgroup = ti.getFirstParent();
		if (pgroup) {
			mac->xmlParentName = pgroup.getEscapedName();
		}
	}
	else if (aparam.type() == typeid(ofParameter<float>).name()) {
		mac->ptype = PTYPE_FLOAT;
		ofParameter<float> fi = aparam.cast<float>();
		ofParameterGroup pgroup = fi.getFirstParent();
		if (pgroup) {
			mac->xmlParentName = pgroup.getEscapedName();
		}
	}
	else if (aparam.type() == typeid(ofParameter<bool>).name()) {
		mac->ptype = PTYPE_BOOL;
		ofParameter<bool> bi = aparam.cast<bool>();
		ofParameterGroup pgroup = bi.getFirstParent();
		if (pgroup) {
			mac->xmlParentName = pgroup.getEscapedName();
		}
	}
	if (mac->ptype == PTYPE_UNKNOWN) {
		//ofLogNotice("ofxMidiParams :: addParam : unsupported param type");
		return;
	}

	mac->xmlName = aparam.getEscapedName();

	mParamsGroup.add(aparam);
	mAssocParams.push_back(mac);
	_updatePositions();

	//-

	////TODO:
	//ofRemoveListener(mParamsGroup.parameterChangedE(), this, &ofxMidiParams::Changed_Controls);
	//ofAddListener(mParamsGroup.parameterChangedE(), this, &ofxMidiParams::Changed_Controls);
}

//--------------------------------------------------------------
void ofxMidiParams::_updatePositions() {
	//ofLogWarning() << __FUNCTION__ << " ";

	mHeaderRect.set(0, 0, width, 24.f);
	mSaveBtnRect.set(mHeaderRect.getRight() - 70, 4, 40, mHeaderRect.height - 8);
	mMessageRect.set(0, mHeaderRect.getBottom() + 2, width, 16 * 6);
	for (int i = 0; i < mAssocParams.size(); i++) {
		mAssocParams[i]->drawRect.height = 20.f;
		float ty = mMessageRect.getBottom() + 2.f + (float)i*(mAssocParams[i]->drawRect.height + 2.f);
		mAssocParams[i]->drawRect.x = 0;
		mAssocParams[i]->drawRect.y = ty;
		mAssocParams[i]->drawRect.width = width;
	}
}

//--------------------------------------------------------------
bool ofxMidiParams::isVisible() {
	return bVisible;
}

//--------------------------------------------------------------
void ofxMidiParams::setVisible(bool ab) {
	bVisible = ab;
}

//--------------------------------------------------------------
void ofxMidiParams::toggleVisible() {
	bVisible = !bVisible;
}

//--------------------------------------------------------------
void ofxMidiParams::draw() {
	if (!bVisible) return;

	ofPushMatrix(); {
		ofTranslate(pos.x, pos.y);

		//device name
		string hstring = mDesiredDeviceNameToOpen;
		if (hstring == "") {
			hstring = "No MIDI Device.";
		}
		hstring = "MIDI IN: " + hstring;

		//header
		ofSetColor(30);
		ofDrawRectangle(mHeaderRect);
		//label
		ofSetColor(225);
		if (myFont.isLoaded())
		{
			myFont.drawString(hstring, mHeaderRect.x + 4, mHeaderRect.y + mHeaderRect.height / 2 + 6);
		}
		else
		{
			ofDrawBitmapString(hstring, mHeaderRect.x + 4, mHeaderRect.y + mHeaderRect.height / 2 + 6);
		}

		//green circle
		ofSetColor(200, 20, 70);
		if (isConnected()) {
			ofSetColor(20, 210, 60);
		}
		ofDrawCircle(mHeaderRect.width - (mHeaderRect.height / 2), mHeaderRect.height / 2, mHeaderRect.height / 4);

		//save button
		ofSetColor(80);
		ofDrawRectangle(mSaveBtnRect);
		ofSetColor(200);
		if (myFont.isLoaded())
		{
			myFont.drawString("save", mSaveBtnRect.x + 4, mSaveBtnRect.getCenter().y + 4);
		}
		else
		{
			ofDrawBitmapString("save", mSaveBtnRect.x + 4, mSaveBtnRect.getCenter().y + 4);
		}

		//bg log
		ofSetColor(10);
		ofDrawRectangle(mMessageRect);
		string messStr = mMidiMessageHistoryStr;
		if (!isConnected()) {
			messStr = "No MIDI messages received.";

		}
		//text log
		ofSetColor(200);
		if (myFont.isLoaded())
		{
			myFont.drawString(messStr, mMessageRect.x + 8, mMessageRect.y + 18);
		}
		else
		{
			ofDrawBitmapString(messStr, mMessageRect.x + 8, mMessageRect.y + 18);
		}

		//assigned params
		for (int i = 0; i < mAssocParams.size(); i++) {
			ofSetColor(20);
			auto& ma = mAssocParams[i];
			float val = getParamValuePct(ma);
			ofDrawRectangle(ma->drawRect);
			if (val > 0.0) {
				ofSetColor(colorFill);
				ofDrawRectangle(ma->drawRect.x, ma->drawRect.y, val * ma->drawRect.width, ma->drawRect.height);
			}

			if (ma->bListening) {
				ofSetColor(cos(ofGetElapsedTimef() * 3.f) * 50 + 50);
				ofDrawRectangle(ma->drawRect);
			}

			//left param label
			ofSetColor(255);
			float texty = ma->drawRect.y + 14;
			if (myFont.isLoaded())
			{
				myFont.drawString(mParamsGroup.getName(i), 0.0 + 8, texty);
			}
			else
			{
				ofDrawBitmapString(mParamsGroup.getName(i), 0.0 + 8, texty);
			}

			//right midi item value
			if (ma->displayMidiName != "") {
				float sw = ma->displayMidiName.length() * 8.f;
				if (myFont.isLoaded())
				{
					myFont.drawString(ma->displayMidiName, ma->drawRect.getRight() - sw - 4, texty);
				}
				else
				{
					ofDrawBitmapString(ma->displayMidiName, ma->drawRect.getRight() - sw - 4, texty);
				}
			}
			//midi learn
			else if (ma->bListening) {
				string tstr = "Listening";
				float sw = tstr.length() * 8.f;
				if (myFont.isLoaded())
				{
					myFont.drawString(tstr, ma->drawRect.getRight() - sw - 4, texty);

				}
				else
				{
					ofDrawBitmapString(tstr, ma->drawRect.getRight() - sw - 4, texty);
				}
			}

		}
	} ofPopMatrix();
}

//--------------------------------------------------------------
int ofxMidiParams::_getDesiredPortToOpen() {
	//ofLogWarning() << __FUNCTION__ << " ";

	if (mDesiredDeviceNameToOpen == "") {
		ofLogNotice("ofxMidiParams :: desired device not found.");
		return -1;
	}
	vector<string> tportNames = midiIn.getInPortList();
	for (int i = 0; i < tportNames.size(); i++) {
		//cout << i << " port name: " << tportNames[i] << endl;
		// lets first try a hard match and then we can try a loose one //
		if (tportNames[i] == mDesiredDeviceNameToOpen) {
			return i;
		}
	}
	for (int i = 0; i < tportNames.size(); i++) {
		// lets first try a hard match and then we can try a loose one //
		if (ofIsStringInString(tportNames[i], mDesiredDeviceNameToOpen)) {
			return i;
		}
	}

	return -1;
}

#pragma mark - Mouse Events
//--------------------------------------------------------------
void ofxMidiParams::enableMouseEvents() {
	if (!bHasMouseEvents) {
		ofAddListener(ofEvents().mousePressed, this, &ofxMidiParams::onMousePressed);
		ofAddListener(ofEvents().mouseDragged, this, &ofxMidiParams::onMouseDragged);
		ofAddListener(ofEvents().mouseReleased, this, &ofxMidiParams::onMouseReleased);
		ofAddListener(ofEvents().mouseMoved, this, &ofxMidiParams::onMouseMoved);
	}
	bHasMouseEvents = true;
}

//--------------------------------------------------------------
void ofxMidiParams::disableMouseEvents() {
	if (bHasMouseEvents) {
		ofRemoveListener(ofEvents().mousePressed, this, &ofxMidiParams::onMousePressed);
		ofRemoveListener(ofEvents().mouseDragged, this, &ofxMidiParams::onMouseDragged);
		ofRemoveListener(ofEvents().mouseReleased, this, &ofxMidiParams::onMouseReleased);
		ofRemoveListener(ofEvents().mouseMoved, this, &ofxMidiParams::onMouseMoved);
	}
	bHasMouseEvents = false;
}

//--------------------------------------------------------------
void ofxMidiParams::onMousePressed(ofMouseEventArgs& args) {
	mousePressed(args.x, args.y, args.button);
}

//--------------------------------------------------------------
void ofxMidiParams::onMouseMoved(ofMouseEventArgs& args) {
	mouseMoved(args.x, args.y);
}

//--------------------------------------------------------------
void ofxMidiParams::onMouseDragged(ofMouseEventArgs& args) {
	mouseDragged(args.x, args.y, args.button);
}

//--------------------------------------------------------------
void ofxMidiParams::onMouseReleased(ofMouseEventArgs& args) {
	mouseReleased(args.x, args.y, args.button);
}

//--------------------------------------------------------------
void ofxMidiParams::mouseMoved(int x, int y) {
	if (!isVisible()) return;
	glm::vec2 mp = glm::vec2(x, y) - pos;
}

//--------------------------------------------------------------
void ofxMidiParams::mouseDragged(int x, int y, int button) {
	if (!isVisible()) return;
	if (bDragging) {
		pos = glm::vec2(x, y) - mMouseOffset;
	}
}

//--------------------------------------------------------------
void ofxMidiParams::mousePressed(int x, int y, int button) {
	if (!isVisible()) return;
	glm::vec2 mp = glm::vec2(x, y) - pos;

	bDragging = false;
	bool bate = false;

	if (mSaveBtnRect.inside(mp)) {
		save();
		bate = true;
	}

	if (!bate && mHeaderRect.inside(mp)) {
		mMouseOffset = mp;
		bDragging = true;
	}

	bool bHitParam = false;
	if (!bate) {
		for (int i = 0; i < mAssocParams.size(); i++) {
			if (mAssocParams[i]->drawRect.inside(mp)) {
				mSelectedParam = mAssocParams[i];
				mAssocParams[i]->bListening = true;
				bHitParam = true;
				break;
			}
		}
	}
	if (!bHitParam) {
		if (mSelectedParam) {
			mSelectedParam->bListening = false;
			mSelectedParam.reset();
		}
	}
}

//--------------------------------------------------------------
void ofxMidiParams::mouseReleased(int x, int y, int button) {
	if (!isVisible()) return;
	bDragging = false;
}





