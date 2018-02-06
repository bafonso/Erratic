#include "MPEToCV.hpp"

MPEToCVWidget::MPEToCVWidget() {
	MPEToCVInterface *module = new MPEToCVInterface();
	setModule(module);
	box.size = Vec(15 * 9, 380);

	{
		Panel *panel = new LightPanel();
		panel->box.size = box.size;
		addChild(panel);
	}

	float margin = 5;
	float labelHeight = 15;
	float yPos = margin;
	float yGap = 35;
    float xPos = 0; // Not sure how to initialize it.

	addChild(createScrew<ScrewSilver>(Vec(15, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(createScrew<ScrewSilver>(Vec(15, 365)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 30, 365)));

	{
		Label *label = new Label();
		label->box.pos = Vec(box.size.x - margin - 7 * 15, margin);
		label->text = "MPE to CV";
		addChild(label);
		yPos = labelHeight * 2;
	}

	addParam(createParam<LEDButton>(Vec(7 * 15, labelHeight), module, MPEToCVInterface::RESET_PARAM, 0.0, 1.0, 0.0));
	addChild(createLight<SmallLight<RedLight>>(Vec(7 * 15 + 5, labelHeight + 5), module,
											   MPEToCVInterface::RESET_LIGHT));
	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "MIDI Interface";
		addChild(label);
		yPos += labelHeight + margin;

		MidiChoice *midiChoice = new MidiChoice();
		midiChoice->midiModule = dynamic_cast<MidiIO *>(module);
		midiChoice->box.pos = Vec(margin, yPos);
		midiChoice->box.size.x = box.size.x - 10;
		addChild(midiChoice);
		yPos += midiChoice->box.size.y + 4*margin;
	}


	std::string labels[MPEToCVInterface::NUM_OUTPUTS] = {"1V/oct", "Gate", "Velocity", "Pressure", "Y axis","Pedal"
														  };

	for (int i = 0; i < MPEToCVInterface::NUM_OUTPUTS; i++) {
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = labels[i];
		addChild(label);

		addOutput(createOutput<PJ3410Port>(Vec(15 * 6, yPos - 5), module, i));

		yPos += yGap + margin;
	}
}

void MPEToCVWidget::step() {

	ModuleWidget::step();
}

struct BendRangeItem : MenuItem {
	MPEToCVInterface *mpetocv;
	int bendRange ;
	void onAction(EventAction &e) override {
		mpetocv->bendRange = bendRange;
	}
	void step() override {
		rightText = (mpetocv->bendRange==bendRange) ? "✔" : "";
	}
};

struct MidiChannelItem : MenuItem {
	MPEToCVInterface *mpetocv;
	int channel;
	void onAction(EventAction &e) override {
		mpetocv->channel = channel ; 
	}
	void step() override {
		// std::cout << "mpetocv->channel, channel : " << mpetocv->channel << "," << channel << std::endl; 
		rightText = (mpetocv->channel==channel) ? "✔" : "";
	}
};

struct GlobalMidiChannelItem : MenuItem {
	MPEToCVInterface *mpetocv;
	int globalChannel;
	void onAction(EventAction &e) override {
		mpetocv->globalChannel = globalChannel ; 
	}
	void step() override {
		rightText = (mpetocv->globalChannel==globalChannel) ? "✔" : "";
	}
};


struct MPEModeItem : MenuItem {
	MPEToCVInterface *mpetocv;
	bool MPEPlus;
	void onAction(EventAction &e) override {
		mpetocv->MPEPlus = MPEPlus ; 
	}
	void step() override {
		rightText = (mpetocv->MPEPlus==MPEPlus) ? "✔" : "";
	}
};


Menu *MPEToCVWidget::createContextMenu() {
	Menu *menu = ModuleWidget::createContextMenu();
	MPEToCVInterface *mpetocv = dynamic_cast<MPEToCVInterface*>(module);
	assert(mpetocv);

	MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

	MenuLabel *MIDIChannelLabel = new MenuLabel();
	MIDIChannelLabel->text ="MIDI Channel" ;
	menu->addChild(MIDIChannelLabel);
	
	for (int c=1; c<= 16; c++) {
		MidiChannelItem *channelItem = new MidiChannelItem();
		channelItem->mpetocv = mpetocv;
		channelItem->text = std::to_string(c);
		channelItem->channel = c;
		menu->addChild(channelItem);
	}

	// menu->addChild(spacerLabel);

	MenuLabel *bendRangeLabel = new MenuLabel();
	bendRangeLabel->text ="Bend Range (semitones)" ;
	menu->addChild(bendRangeLabel);

	std::vector<int> bendRanges = {1,2,3,4,12,24,48,96}; // The bend range we use
	for (auto const& bendRangeValue: bendRanges) {
		BendRangeItem *item = new BendRangeItem();
		item->mpetocv = mpetocv;
		item->text = std::to_string(bendRangeValue);
		item->bendRange = bendRangeValue;
		menu->addChild(item);
	}

	MenuLabel *globalMidiChannelLabel = new MenuLabel();
	globalMidiChannelLabel->text ="Global MIDI Channel" ;
	menu->addChild(globalMidiChannelLabel);
	// Roli
	GlobalMidiChannelItem *roliGlobChan = new GlobalMidiChannelItem();
	roliGlobChan->mpetocv = mpetocv;
	roliGlobChan->text = "1 - ROLI";
	roliGlobChan->globalChannel = 1;
	menu->addChild(roliGlobChan);
	// Haken Continuum
	GlobalMidiChannelItem *continuumGlobChan = new GlobalMidiChannelItem();
	continuumGlobChan->mpetocv = mpetocv;
	continuumGlobChan->text = "16 - Haken Continuum";
	continuumGlobChan->globalChannel = 16;
	menu->addChild(continuumGlobChan);

	// std::vector<int> globalMIDIChannels = {1,16}; // The bend range we use
	// std::string globalMIDIChannelsLabels[2] = {"1 - Use with ROLI", "2 - Use with Continuum"};
	// for (auto const& globChanValue: globalMIDIChannels) {
	// 	GlobalMidiChannelItem *item = new GlobalMidiChannelItem();
	// 	item->mpetocv = mpetocv;
	// 	item->text = std::to_string(bendRangeValue);
	// 	item->bendRange = bendRangeValue;
	// 	menu->addChild(item);
	// }

	MenuLabel *MPEModeLabel = new MenuLabel();
	MPEModeLabel->text ="MPE Mode" ;
	menu->addChild(MPEModeLabel);
	// MPE
	MPEModeItem *MPE = new MPEModeItem();
	MPE->mpetocv = mpetocv;
	MPE->text = "MPE - Standard (ROLI, etc)";
	MPE->MPEPlus = false;
	menu->addChild(MPE);
	// MPE Plus
	MPEModeItem *MPEPlus = new MPEModeItem();
	MPEPlus->mpetocv = mpetocv;
	MPEPlus->text = "MPE+ - High Res for Haken Continuum";
	MPEPlus->MPEPlus = true;
	menu->addChild(MPEPlus);

	return menu;
}
