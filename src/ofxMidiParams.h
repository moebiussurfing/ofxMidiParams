//
//  ofxMidiParams.h
//  MidiParameter
//
//  Created by Nick Hardeman on 12/28/18.
//

#pragma once
#include "ofMain.h"

#include "ofxMidi.h"
#include "ofxGui.h"
#include "ofxSurfingHelpers.h"
#include "ofxSurfing_ofxGui.h"

class ofxMidiParams : public ofxMidiListener {

public:
	void setup();
	void startup();

private:
	std::string path_Global;
	std::string path_Ports;
	std::string path_ParamsList;
	//std::string path_GLOBAL = "ofxMidiParams";

private:
	enum ParamType {
		PTYPE_FLOAT = 0,
		PTYPE_INT,
		PTYPE_BOOL,
		PTYPE_UNKNOWN
	};

private:
	class MidiParamAssoc {
	public:
		int midiId = -1;
		int paramIndex = 0;
		ParamType ptype = PTYPE_UNKNOWN;
		ofRectangle drawRect;
		string displayMidiName = "";
		bool bListening = false;
		bool bNeedsTextPrompt = false;
		string xmlParentName = "";
		string xmlName = "";
	};

public:
	ofxMidiParams();
	~ofxMidiParams();

private:
	ofxPanel gui;

	void Changed_Controls_MidiPorts(ofAbstractParameter &e);
	
	ofParameterGroup params_MidiPorts{ "Midi" };

	ofParameter<int> midiIn_Port{ "Midi In", 0, 0, 10 };
	ofParameter<int> midiOut_Port{ "Midi Out", 2, 0, 10 };
	ofParameter<std::string> midiIn_Port_name{ "In" , ""};
	ofParameter<std::string> midiOut_Port_name{ "Out", "" };
	ofParameter<glm::vec2> posGui{ "Position GUI", glm::vec2(10,10), glm::vec2(0,0), glm::vec2(1920,1080) };
	ofParameter<bool> ShowGui{ "Show Internal", true };
	ofParameter<bool> bVisible{ "Show Gui", true };
	ofParameter<bool>  bAutoName{ "Auto Name Learn", true };
	ofParameter<bool> bMinimize{ "Minimize", false};

public:
	bool connect();
	bool connect(int aport, bool abRetryConnection);
	bool connect(string aDeviceName, bool abRetryConnection);
	void disconnect();
	bool isConnected();

private:
	bool save(string aXmlFilepath = "");
	bool load(string aXmlFilepath);

private:
	void update(ofEventArgs& args);

public:
	void draw();

private:
	void newMidiMessage(ofxMidiMessage& amsg);
	void setParamValue(shared_ptr<MidiParamAssoc>& am, float avaluePct);
	float getParamValue(shared_ptr<MidiParamAssoc>& am);
	float getParamValue(const shared_ptr<MidiParamAssoc>& am) const;

	float getParamValuePct(shared_ptr<MidiParamAssoc>& am);
	float getParamValuePct(const shared_ptr<MidiParamAssoc>& am) const;

	int getId(ofxMidiMessage& amess);

public:
	void add(ofParameterGroup aparams);
	void add(ofParameter<float>& aparam);
	void add(ofParameter<bool>& aparam);
	void add(ofParameter<int>& aparam);
	void addParam(ofAbstractParameter& aparam);

public:
	ofxMidiIn& getMidi() { return midiIn; }

	bool isVisible();
	void setVisible(bool ab);
	void toggleVisible();

	//added api
	void setPosition(float ax, float ay) { pos = glm::vec2(ax, ay); }
	glm::vec2 getPosition() {
		return pos;
	};
	float getWidth()
	{
		return width;
	};

protected:
	void enableMouseEvents();
	void disableMouseEvents();

	void onMouseMoved(ofMouseEventArgs& args);
	void onMouseDragged(ofMouseEventArgs& args);
	void onMousePressed(ofMouseEventArgs& args);
	void onMouseReleased(ofMouseEventArgs& args);

	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);

protected:
	int _getDesiredPortToOpen();
	void _updatePositions();

	ofxMidiIn midiIn;
	std::vector<ofxMidiMessage> midiMessages;
	bool bConnected = false;
	bool bHasUpdateEvent = false;

	vector< shared_ptr<MidiParamAssoc> > mAssocParams;

	//TODO:
	void Changed_Controls_Out(ofAbstractParameter &e);
	ofxMidiOut midiOut;

	ofParameterGroup mParamsGroup;
	string mMidiMessageHistoryStr;
	bool bHasMouseEvents = false;

	float width = 300.0f;
	glm::vec2 pos, mMouseOffset;
	bool bDragging = false;

	ofRectangle mHeaderRect, mMessageRect, mSaveBtnRect;
	ofRectangle mShowGuiInternalRect, mMinimizeRect;

	shared_ptr<MidiParamAssoc> mSelectedParam;

	uint64_t mNextCheckMillis = 0;
	string mDesiredDeviceNameToOpen = "";
	bool bTryReconnect = false;

	string mXmlFilePath = "";

	//-

	ofColor colorFill;
	ofColor colorBack;


	void setPathGlobal(string s)
	{
		path_Global = s;
	}
	ofTrueTypeFont myFont;
	int fontSize;
};
