//
//  ofxMidiParams.cpp
//  MidiParameter
//
//  Created by Nick Hardeman on 12/28/18.
//

#include "ofxMidiParams.h"

//--------------------------------------------------------------
ofxMidiParams::ofxMidiParams() {
	setPosition(ofGetWidth() - 320, 20); // default pos

	filenameSettings = "Midi-Params.xml";
	path_Global = "ofxMidiParams/";

	enableMouseEvents();
	_updatePositions();

#ifdef USE_OFX_GUI__MIDI_PARAMS
	ofxSurfingHelpers::setThemeDark_ofxGui();
#endif

	//custom colors
	colorFill = ofColor(0, 225);
	colorFill2 = ofColor(60, 225);
	colorFill3 = ofColor(80, 225);
	colorBack = ofColor(16, 200);

	//TODO:
	setup();
}

//TODO: splitted-out funtion to avoid crash...

//--------------------------------------------------------------
void ofxMidiParams::setup()
{
	// TODO: allow change paths on setup..

	path_Ports = path_Global + "Midi-Ports.xml";
	path_AppState = path_Global + "AppState.xml";
	path_ParamsList = path_Global + filenameSettings;

	ofxSurfingHelpers::CheckFolder(path_Global);

	//-

	midiIn.listInPorts();
	midiOut.listOutPorts();

	midiIn_Port.setMax(midiIn.getNumInPorts() - 1);
	midiOut_Port.setMax(midiOut.getNumOutPorts() - 1);

	//-

	//custom font
	std::string path;
	path = "assets/fonts/";

	fontSize = 8;
	path += "overpass-mono-bold.otf";

	//fontSize = 8;
	//path += "telegrama_render.otf";

	//path = ofToString(path);
	//path = ofToString(path_GLOBAL + "/" + "fonts/" + "overpass-mono-bold.otf");

	myFont.loadFont(path, fontSize, true, true, true);
	if (myFont.isLoaded())
	{
		ofLogNotice(__FUNCTION__) << "Loaded font '" << path << "'";
	}
	else
	{
		ofLogError(__FUNCTION__) << "Font '" << path << "' NOT FOUND!";
	}

	//--

	// exclude
	//bGui.setSerializable(false);
	//midiOut_Port_name.setSerializable(false); // include to display purposes on xml file only...
	bSave.setSerializable(false);
	bPopulate.setSerializable(false);

	//-

	params_MidiPorts.add(midiIn_Port);
	params_MidiPorts.add(midiIn_Port_name);
	params_MidiPorts.add(midiOut_Port);
	params_MidiPorts.add(midiOut_Port_name);

	params_AppState.add(bAutoName);
	params_AppState.add(posGui);
	params_AppState.add(bShowGuiInternal);
	params_AppState.add(bShowMapping);
	params_AppState.add(bGui);
	params_AppState.add(bAutoSave);
	params_AppState.add(bAutoReconnect);
	params_AppState.add(bSave);
	params_AppState.add(bLoad);
	params_AppState.add(bPopulate);
	params_AppState.add(bMinimize);

	ofAddListener(params_MidiPorts.parameterChangedE(), this, &ofxMidiParams::Changed_Controls);
	ofAddListener(params_AppState.parameterChangedE(), this, &ofxMidiParams::Changed_Controls);

	//--

#ifdef USE_OFX_GUI__MIDI_PARAMS
	gui.setup("ofxMidiParams");
	gui.add(params_MidiPorts);
#endif

#ifdef USE_OFX_IM_GUI__MIDI_PARAMS
	guiManager.setImGuiAutodraw(true);
	guiManager.setup(); // ofxImGui is instantiated inside the class, the we can forgot of declare ofxImGui here (ofApp scope).
#endif

	//--

	//TODO: 
	//midi out

	mParamsGroup.setName("ofxMidiParams");
	ofAddListener(mParamsGroup.parameterChangedE(), this, &ofxMidiParams::Changed_Controls_Out);

	// connect
	//midiOut.openPort(midiOut_Port); // by number
	//midiOut.openPort("IAC Driver Pure Data In"); // by name
	//midiOut.openVirtualPort("ofxMidiOut"); // open a virtual port

	//ofLogNotice(__FUNCTION__) << "MidiOut";
	//ofLogNotice(__FUNCTION__) << "Connected to Port " << midiOut.getPort();
	//ofLogNotice(__FUNCTION__) << midiOut.getName();
	//ofLogNotice(__FUNCTION__) << (midiOut.isVirtual() ? "is Virtual" : "Not Virtual");

	//-

	//startup();
}

//--------------------------------------------------------------
void ofxMidiParams::clear()
{
	ofLogNotice(__FUNCTION__);

	mParamsGroup.clear();
	mAssocParams.clear();

	//mParamsGroup.setName("ofxMidiParams");
	//ofAddListener(mParamsGroup.parameterChangedE(), this, &ofxMidiParams::Changed_Controls_Out);
}

//--------------------------------------------------------------
void ofxMidiParams::doPopulate()
{
	ofLogNotice(__FUNCTION__);

	//TODO:
	//fast assign midi controls...
}

//--------------------------------------------------------------
void ofxMidiParams::startup()
{
	ofLogNotice(__FUNCTION__);

	ofxSurfingHelpers::loadGroup(params_MidiPorts, path_Ports);
	ofxSurfingHelpers::loadGroup(params_AppState, path_AppState);

	//setPosition(ofGetWidth() - 320, 20);

	connect();

	load(path_ParamsList);
}

//--------------------------------------------------------------
void ofxMidiParams::Changed_Controls(ofAbstractParameter &e)
{
	std::string name = e.getName();

	ofLogVerbose(__FUNCTION__) << name << " : " << e;

	//----

	bool bLog = false;

	if (0) {}

	else if (name == bSave.getName() && bSave)
	{
		bSave = false;
		save(path_ParamsList);
	}

	else if (name == bLoad.getName() && bLoad)
	{
		bLoad = false;
		load(path_ParamsList);
	}
	else if (name == bPopulate.getName() && bPopulate)
	{
		bPopulate = false;
		doPopulate();
	}

	else if (name == midiIn_Port.getName())
	{
		vector<string> tportNames = midiIn.getInPortList();
		mDesiredDeviceNameToOpen = tportNames[midiIn_Port];

		//midiIn.closePort();
		disconnect();
		midiIn.openPort(midiIn_Port);
		midiIn_Port_name = midiIn.getName();
		bLog = true;

		refresh();
	}

	else if (name == midiOut_Port.getName())
	{
		midiOut.closePort();
		midiOut.openPort(midiOut_Port);
		midiOut_Port_name = midiOut.getName();
		bLog = true;
	}

	//#ifdef USE_OFX_GUI__MIDI_PARAMS
	else if (name == posGui.getName())
	{
		//pos = posGui.get();
		setPosition(posGui.get().x, posGui.get().y);
	}
	//#endif

	if (bLog && (name == midiIn_Port.getName() || name == midiOut_Port.getName()))
	{
		ofLogNotice(__FUNCTION__) << "-----------------------------------------------";
		ofLogNotice(__FUNCTION__) << "MidiOut";
		ofLogNotice(__FUNCTION__) << "Connected to Port " << midiOut.getPort();
		ofLogNotice(__FUNCTION__) << midiOut.getName();
		ofLogNotice(__FUNCTION__) << (midiOut.isVirtual() ? "is Virtual" : "Not Virtual") << endl;

		ofLogNotice(__FUNCTION__) << "-----------------------------------------------";
		ofLogNotice(__FUNCTION__) << "MidiIn";
		ofLogNotice(__FUNCTION__) << "Connected to Port " << midiIn.getPort();
		ofLogNotice(__FUNCTION__) << midiIn.getName();
		ofLogNotice(__FUNCTION__) << (midiIn.isVirtual() ? "is Virtual" : "Not Virtual") << endl;
	}
}

//--------------------------------------------------------------
void ofxMidiParams::Changed_Controls_Out(ofAbstractParameter &e)
{
	std::string name = e.getName();

	ofLogVerbose(__FUNCTION__) << name << " : " << e;

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
	if (bAutoSave) save(path_ParamsList);

	disableMouseEvents();
	disconnect();
	if (bHasUpdateEvent) {
		ofRemoveListener(ofEvents().update, this, &ofxMidiParams::update);
	}
	bHasUpdateEvent = false;

	ofRemoveListener(mParamsGroup.parameterChangedE(), this, &ofxMidiParams::Changed_Controls_Out);
	ofRemoveListener(params_MidiPorts.parameterChangedE(), this, &ofxMidiParams::Changed_Controls);

	// clean up
	midiOut.closePort();

	posGui = getPosition();
	ofxSurfingHelpers::saveGroup(params_MidiPorts, path_Ports);
	ofxSurfingHelpers::saveGroup(params_AppState, path_AppState);
}


//--------------------------------------------------------------
bool ofxMidiParams::connect()
{
	bool b = connect(-1, true);

	return b;
}

//--------------------------------------------------------------
bool ofxMidiParams::connect(int aport, bool abRetryConnection) {

	if (aport == -1)  aport = midiIn_Port;

	//-

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

	if (bConnected)
	{
		midiIn_Port_name = midiIn.getName();
	}

	// add ofApp as a listener
	if (bConnected) midiIn.addListener(this);
	return bConnected;
}

//--------------------------------------------------------------
bool ofxMidiParams::connect(string aDeviceName, bool abRetryConnection) {
	ofLogWarning() << __FUNCTION__;

	mDesiredDeviceNameToOpen = aDeviceName;
	int tport = _getDesiredPortToOpen();
	return connect(tport, abRetryConnection);
}

//--------------------------------------------------------------
void ofxMidiParams::disconnect() {
	ofLogWarning() << __FUNCTION__;

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
		ofLogNotice(__FUNCTION__) << aXmlFilepath;

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

	if (bAutoReconnect)
	{
		refresh();
	}

	//--

	if (mSelectedParam && mSelectedParam->bNeedsTextPrompt) {

		if (!bAutoName)
			mSelectedParam->displayMidiName = ofSystemTextBoxDialog("MIDI Display name.", mSelectedParam->displayMidiName);
		else mSelectedParam->displayMidiName = mSelectedParam->displayMidiName;

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

		////TODO:
		//if (bAutoSave) {
		//	save(path_ParamsList);
		//}
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
	//ofRemoveListener(mParamsGroup.parameterChangedE(), this, &ofxMidiParams::Changed_Controls_Out);
	//ofAddListener(mParamsGroup.parameterChangedE(), this, &ofxMidiParams::Changed_Controls_Out);

	//-

	//startup();
}

//--------------------------------------------------------------
void ofxMidiParams::_updatePositions() {
	//ofLogWarning() << __FUNCTION__ << " ";

	mHeaderRect.set(0, 0, width, 24.f);
	float wb = 40;
	float hb = mHeaderRect.height - 8;
	float xx = mHeaderRect.getRight() - 70;

	mSaveBtnRect.set(xx, 4, wb, hb);
	mShowGuiInternalRect.set(xx - 1 * (5 + wb), 4, wb, hb);
	mMinimizeRect.set(xx - 2 * (5 + wb) + wb / 2, 4, wb / 2, hb);

	mMessageRect.set(0, mHeaderRect.getBottom() + 2, width, 18 * 6);
	//mMessageRect.set(0, mHeaderRect.getBottom() + 2, width, 16 * 6);

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
	return bGui;
}

//--------------------------------------------------------------
void ofxMidiParams::setVisible(bool ab) {
	bGui = ab;
}

//--------------------------------------------------------------
void ofxMidiParams::toggleVisible() {
	bGui = !bGui;
}

//--------------------------------------------------------------
void ofxMidiParams::draw() {
	if (ofGetFrameNum() == 0) startup(); // -> load settings

	//--

	if (!bGui) return;

	if (bShowMapping)
	{
		ofPushMatrix();
		ofPushStyle();
		{
			//ofTranslate(posGui.get().x, posGui.get().y);
			ofTranslate(pos.x, pos.y);

			// device name
			string hstring = midiIn_Port_name;
			//string hstring = mDesiredDeviceNameToOpen;

			if (hstring == "") {
				hstring = "No MIDI Device.";
			}
			hstring = "MIDI IN: " + hstring;

			// header
			ofSetColor(30);
			ofDrawRectangle(mHeaderRect);

			// label
			ofSetColor(225);
			if (myFont.isLoaded())
			{
				myFont.drawString(hstring, mHeaderRect.x + 6, mHeaderRect.y + mHeaderRect.height / 2 + 6);
			}
			else
			{
				ofDrawBitmapString(hstring, mHeaderRect.x + 6, mHeaderRect.y + mHeaderRect.height / 2 + 6);
			}

			// green circle
			ofSetColor(200, 20, 70);
			if (isConnected())
			{
				ofSetColor(20, 210, 60);
			}
			ofDrawCircle(mHeaderRect.width - (mHeaderRect.height / 2), mHeaderRect.height / 2, mHeaderRect.height / 4);

#ifdef USE_OFX_GUI__MIDI_PARAMS
			// save button
			ofSetColor(colorFill3);
			ofDrawRectangle(mSaveBtnRect);

			// show internal gui
			if (bShowGuiInternal) ofSetColor(colorFill2); else ofSetColor(colorFill3);
			ofDrawRectangle(mShowGuiInternalRect);

			// show minimize
			if (bMinimize) ofSetColor(colorFill2); else ofSetColor(colorFill3);
			ofDrawRectangle(mMinimizeRect);

			// labels
			ofSetColor(200);
			std::string s = bMinimize ? "+" : "-";

			if (myFont.isLoaded())
			{
				float y;
				float w;
				float h;
				ofRectangle r;

				r = myFont.getStringBoundingBox("SAVE", 0, 0);
				w = r.getWidth();
				h = r.getHeight();
				y = mSaveBtnRect.getCenter().y + h / 2;
				myFont.drawString("SAVE", mSaveBtnRect.getCenter().x - w / 2, y);

				r = myFont.getStringBoundingBox("GUI", 0, 0);
				w = r.getWidth();
				h = r.getHeight();
				myFont.drawString("GUI", mShowGuiInternalRect.getCenter().x - w / 2, y);

				r = myFont.getStringBoundingBox("+", 0, 0);
				w = r.getWidth();
				h = r.getHeight();
				myFont.drawString(s, mMinimizeRect.getCenter().x - w / 2, y);
			}
			else
			{
				ofDrawBitmapString("SAVE", mSaveBtnRect.x + 4, mSaveBtnRect.getCenter().y + 4);
				ofDrawBitmapString("GUI", mShowGuiInternalRect.x + 6, mShowGuiInternalRect.getCenter().y + 4);
				ofDrawBitmapString(s, mMinimizeRect.x + 6, mMinimizeRect.getCenter().y + 4);
			}
#endif

			if (!bMinimize)
			{
				// bg log
				ofSetColor(10);
				ofDrawRectangle(mMessageRect);
				string messStr = mMidiMessageHistoryStr;
				if (!isConnected()) {
					messStr = "No MIDI messages received.";

				}
				// text log
				ofSetColor(200);
				if (myFont.isLoaded())
				{
					myFont.drawString(messStr, mMessageRect.x + 8, mMessageRect.y + 18);
				}
				else
				{
					ofDrawBitmapString(messStr, mMessageRect.x + 8, mMessageRect.y + 18);
				}

				//-

				// assigned params
				for (int i = 0; i < mAssocParams.size(); i++)
				{
					// value fill
					ofSetColor(colorBack);
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

					// left param label
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

					// right midi item value
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
					// midi learn
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
			}
		}
		ofPopStyle();
		ofPopMatrix();
	}

	if (bShowGuiInternal)
	{

#ifdef USE_OFX_IM_GUI__MIDI_PARAMS
		drawImGui();
#endif

#ifdef USE_OFX_GUI__MIDI_PARAMS
		auto p = getPosition();
		auto w = getWidth() + 5;
		gui.setPosition(p.x + w, p.y);
		gui.draw();
#endif
	}
}

//--------------------------------------------------------------
void ofxMidiParams::refresh() {
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
				connect(midiIn_Port, true);
				//connect(0, true);
			}
		}
		mNextCheckMillis = emillis + 3000;
	}
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

#ifdef USE_OFX_GUI__MIDI_PARAMS
	if (mSaveBtnRect.inside(mp)) {
		save();
		bate = true;
	}

	if (mShowGuiInternalRect.inside(mp)) {
		bShowGuiInternal = !bShowGuiInternal;
	}

	if (mMinimizeRect.inside(mp)) {
		bMinimize = !bMinimize;
	}
#endif

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


#ifdef USE_OFX_IM_GUI__MIDI_PARAMS
//--------------------------------------------------------------
void ofxMidiParams::drawImGui()
{
	guiManager.begin(); // global begin
	{
		{
			string n = "MIDI PARAMS";
			static bool bOpen0 = true;
			ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
			if (guiManager.bAutoResize) window_flags |= ImGuiWindowFlags_AlwaysAutoResize;

			guiManager.beginWindow(n.c_str(), &bOpen0, window_flags);
			{
				float _h = WIDGETS_HEIGHT;
				float _w100 = getWidgetsWidth(1);
				float _w50 = getWidgetsWidth(2);

				ImGui::Text("MIDI INPUT");

				ofxImGuiSurfing::AddBigToggle(bShowMapping, _w100, _h / 2, false);
				ofxImGuiSurfing::AddBigButton(bLoad, _w50, _h / 2);
				ImGui::SameLine();
				ofxImGuiSurfing::AddBigButton(bSave, _w50, _h / 2);

				//-

				bool bOpen = false;
				ImGuiTreeNodeFlags _flagt = (bOpen ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None);
				_flagt |= ImGuiTreeNodeFlags_Framed;
				
				if (ImGui::TreeNodeEx("MIDI", _flagt))
				{
					{
						bool bOpen = true;
						ImGuiTreeNodeFlags _flagt = (bOpen ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None);
						_flagt |= ImGuiTreeNodeFlags_Framed;

						if (ImGui::TreeNodeEx("CONNECT", _flagt))
						{
							_w100 = getWidgetsWidth(1);
							_w50 = getWidgetsWidth(2);

							string ni = "INPUT:  " + midiIn_Port_name.get();
							ImGui::Text(ni.c_str());
							if (ImGui::Button("ON", ImVec2(_w50, _h / 2)))
							{
								midiIn_Port = midiIn_Port;

								//midiIn.openPort(midiIn_Port);
								//midiIn_Port_name = midiIn.getName();
							}
							ImGui::SameLine();
							if (ImGui::Button("OFF", ImVec2(_w50, _h / 2)))
							{
								disconnect();
								midiIn_Port_name = "";
							}

							//-

							string no = "OUTPUT: " + midiOut_Port_name.get();
							ImGui::Text(no.c_str());
							if (ImGui::Button("ON ", ImVec2(_w50, _h / 2)))
							{
								midiOut_Port = midiOut_Port;

								//midiOut.openPort(midiOut_Port);
								//midiOut_Port_name = midiOut.getName();
							}
							ImGui::SameLine();
							if (ImGui::Button("OFF ", ImVec2(_w50, _h / 2)))
							{
								midiOut.closePort();
								midiOut_Port_name = "";
							}
							ImGui::TreePop();
						}
					}

					//-

					{
						bool bOpen = false;
						ImGuiTreeNodeFlags _flagt = (bOpen ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None);
						_flagt |= ImGuiTreeNodeFlags_Framed;

						ofxImGuiSurfing::AddGroup(params_MidiPorts, _flagt);
					}

					ImGui::TreePop();
				}

				//-

				ImGui::Dummy(ImVec2(0, 5)); // spacing

				ofxImGuiSurfing::AddToggleRoundedButton(guiManager.bExtra);
				if (guiManager.bExtra)
				{
					ImGui::Indent();

					_w100 = getWidgetsWidth(1);
					_w50 = getWidgetsWidth(2);

					ofxImGuiSurfing::AddToggleRoundedButton(bAutoReconnect);
					//ofxImGuiSurfing::AddBigButton(bPopulate, _w100, _h / 2);
					//static bool bClear = false;
					//ofxImGuiSurfing::ToggleRoundedButton("Clear", &bClear);
					//{
					//	if (bClear) bClear = false;
					//	clear();
					//}
					ofxImGuiSurfing::AddToggleRoundedButton(bAutoSave);

					bool bOpen = false;
					ImGuiTreeNodeFlags _flagt = (bOpen ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None);
					_flagt |= ImGuiTreeNodeFlags_Framed;
					if (ImGui::TreeNodeEx("PATH SETTINGS", _flagt))
					{
						ImGui::Indent();
						ImGui::Text(path_Global.c_str());
						ImGui::Text(filenameSettings.c_str());
						ImGui::Unindent();
						ImGui::TreePop();
					}

					ImGui::Dummy(ImVec2(0, 5)); // spacing

					//--

					ofxImGuiSurfing::AddToggleRoundedButton(guiManager.bAdvanced);
					if (guiManager.bExtra) guiManager.drawAdvancedSubPanel();

					ImGui::Unindent();
				}
			}
			guiManager.endWindow();
		}

	}
	guiManager.end(); // global end
}
#endif


