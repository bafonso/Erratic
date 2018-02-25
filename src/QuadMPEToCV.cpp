#include "QuadMPEToCV.hpp"

QuadMPEToCVWidget::QuadMPEToCVWidget() {
	QuadMPEToCVInterface *module = new QuadMPEToCVInterface();
	setModule(module);
	box.size = Vec(16 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	// {
	// 	Panel *panel = new LightPanel();
	// 	panel->box.size = box.size;
	// 	addChild(panel);
	// }

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/QuadMPE.svg")));
		addChild(panel);
	}

	float margin = 5;
	float labelHeight = 15;
	float yPos = margin;

	addChild(createScrew<ScrewSilver>(Vec(15, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(createScrew<ScrewSilver>(Vec(15, 365)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 30, 365)));

	// {
	// 	Label *label = new Label();
	// 	label->box.pos = Vec(box.size.x - margin - 11 * 15, margin);
	// 	label->text = "Quad MPE to CV";
	// 	addChild(label);
	// 	yPos = labelHeight * 2;
	// }

	// addParam(createParam<LEDButton>(Vec(7 * 15, labelHeight), module, QuadMPEToCVInterface::RESET_PARAM, 0.0, 1.0, 0.0));
	// addChild(createLight<SmallLight<RedLight>>(Vec(7 * 15 + 5, labelHeight + 5), module,
	// 										   QuadMPEToCVInterface::RESET_LIGHT));
	{
		// Label *label = new Label();
		// label->box.pos = Vec(margin, yPos);
		// label->text = "MIDI Interface";
		// addChild(label);
		// yPos += labelHeight + margin;

		MidiChoice *midiChoice = new MidiChoice();
		midiChoice->midiModule = dynamic_cast<MidiIO *>(module);
		midiChoice->box.pos = Vec(margin, 40);
		midiChoice->box.size.x = box.size.x - 10;
		addChild(midiChoice);
		yPos += midiChoice->box.size.y + 4*margin;
	}

	// Set of Midi outputs
	// {
	// 	Label *label = new Label();
	// 	label->box.pos = Vec(84, yPos);
	// 	label->text = "1";
	// 	addChild(label);
	// }
	// {
	// 	Label *label = new Label();
	// 	label->box.pos = Vec(125, yPos);
	// 	label->text = "2";
	// 	addChild(label);
	// }
	// {
	// 	Label *label = new Label();
	// 	label->box.pos = Vec(164, yPos);
	// 	label->text = "3";
	// 	addChild(label);
	// }
	// {
	// 	Label *label = new Label();
	// 	label->box.pos = Vec(203, yPos);
	// 	label->text = "4";
	// 	addChild(label);
	// }

	yPos += labelHeight + margin * 2;
	yPos = 115 ;

	std::string labels[QuadMPEToCVInterface::NUM_OUTPUTS] = {"1V/oct", "Gate", "Velocity", "Pressure", "Y axis"} ;
	for (int i = 0; i < 5 ; i++) {
		// Label *label = new Label();
		// label->box.pos = Vec(margin, yPos);
		// label->text = labels[i];
		// addChild(label);
		addOutput(createOutput<PJ3410Port>(Vec(2 * (40), yPos - 5), module, i * 4));
		addOutput(createOutput<PJ3410Port>(Vec(3 * (40), yPos - 5), module, i * 4 + 1));
		addOutput(createOutput<PJ3410Port>(Vec(4 * (40), yPos - 5), module, i * 4 + 2));
		addOutput(createOutput<PJ3410Port>(Vec(5 * (40), yPos - 5), module, i * 4 + 3));
		yPos += 40;
		// yPos += yGap + margin;
	}
	// Pedal output
	// Label *label = new Label();
	// label->box.pos = Vec(margin, yPos);
	// label->text = "Pedal";
	// addChild(label);
	addOutput(createOutput<PJ3410Port>(Vec(2 * (40), yPos - 5), module, 20));
}

void QuadMPEToCVWidget::step() {
	ModuleWidget::step();
}

struct QuadMPEBendRangeItem : MenuItem {
	QuadMPEToCVInterface *mpetocv;
	int bendRange ;
	void onAction(EventAction &e) override {
		std::cout << "We run the menu and set bend range to " << bendRange << std::endl;
		mpetocv->bendRange = bendRange;
	}
	void step() override {
		rightText = (mpetocv->bendRange==bendRange) ? "✔" : "";
	}
};

struct QuadMPEBaseMidiChannelItem : MenuItem {
	QuadMPEToCVInterface *mpetocv;
	int channel;
	void onAction(EventAction &e) override {
		std::cout << "channel is " << channel << " and mpetocv->baseMIDIChannel is " << mpetocv->baseMIDIChannel << std::endl;
		mpetocv->baseMIDIChannel = channel ; 
		std::cout << "channel is " << channel << " and mpetocv->baseMIDIChannel is " << mpetocv->baseMIDIChannel << std::endl;
		mpetocv->setupMIDIChannels();
	}

	void step() override {
		// std::cout << "mpetocv->channel, channel : " << mpetocv->channel << "," << channel << std::endl; 
		rightText = (mpetocv->baseMIDIChannel==channel) ? "✔" : "";
	}
};

struct QuadMPEGlobalMidiChannelItem : MenuItem {
	QuadMPEToCVInterface *mpetocv;
	int globalChannel;
	void onAction(EventAction &e) override {
		mpetocv->globalMIDIChannel = globalChannel ; 
	}
	void step() override {
		rightText = (mpetocv->globalMIDIChannel==globalChannel) ? "✔" : "";
	}
};

struct QuadMPEModeItem : MenuItem {
	QuadMPEToCVInterface *mpetocv;
	bool MPEPlus;
	void onAction(EventAction &e) override {
		mpetocv->MPEPlus = MPEPlus ; 
	}
	void step() override {
		rightText = (mpetocv->MPEPlus==MPEPlus) ? "✔" : "";
	}
};

struct QuadMPENoteOffMenuItem : MenuItem {
	QuadMPEToCVInterface *mpetocv;
	//bool noteOffReset;
	void onAction(EventAction &e) override {
		mpetocv->noteOffReset ^= true ; 
	}
	void step() override {
		rightText = (mpetocv->noteOffReset) ? "✔" : "";
	}
};


Menu *QuadMPEToCVWidget::createContextMenu() {
	Menu *menu = ModuleWidget::createContextMenu();
	QuadMPEToCVInterface *mpetocv = dynamic_cast<QuadMPEToCVInterface*>(module);
	assert(mpetocv);

	MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

	QuadMPENoteOffMenuItem *quadMPENoteOffMenuItem = new QuadMPENoteOffMenuItem();
	quadMPENoteOffMenuItem->mpetocv = mpetocv;
	quadMPENoteOffMenuItem->text ="Note off resets voltages to 0" ;
	menu->addChild(quadMPENoteOffMenuItem);


	MenuLabel *QuadMPEStartMIDIChannelLabel = new MenuLabel();
	QuadMPEStartMIDIChannelLabel->text ="Base MIDI Channel (e.g. 2->16)" ;
	menu->addChild(QuadMPEStartMIDIChannelLabel);
	
	for (int c=1; c<= 2; c++) {
		QuadMPEBaseMidiChannelItem *quadMPEBaseMidiChannelItem = new QuadMPEBaseMidiChannelItem();
		quadMPEBaseMidiChannelItem->mpetocv = mpetocv;
		quadMPEBaseMidiChannelItem->text = std::to_string(c);
		quadMPEBaseMidiChannelItem->channel = c;
		menu->addChild(quadMPEBaseMidiChannelItem);
	}

	// menu->addChild(spacerLabel);

	MenuLabel *bendRangeLabel = new MenuLabel();
	bendRangeLabel->text ="Bend Range (semitones)" ;
	menu->addChild(bendRangeLabel);

	std::vector<int> bendRanges = {1,2,3,4,12,24,48,96}; // The bend range we use
	for (auto const& bendRangeValue: bendRanges) {
		QuadMPEBendRangeItem *item = new QuadMPEBendRangeItem();
		item->mpetocv = mpetocv;
		item->text = std::to_string(bendRangeValue);
		item->bendRange = bendRangeValue;
		menu->addChild(item);
	}

	MenuLabel *globalMidiChannelLabel = new MenuLabel();
	globalMidiChannelLabel->text ="Global MIDI Channel" ;
	menu->addChild(globalMidiChannelLabel);
	// Roli
	QuadMPEGlobalMidiChannelItem *roliGlobChan = new QuadMPEGlobalMidiChannelItem();
	roliGlobChan->mpetocv = mpetocv;
	roliGlobChan->text = "1 - ROLI";
	roliGlobChan->globalChannel = 1;
	menu->addChild(roliGlobChan);
	// Haken Continuum
	QuadMPEGlobalMidiChannelItem *continuumGlobChan = new QuadMPEGlobalMidiChannelItem();
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
	QuadMPEModeItem *MPE = new QuadMPEModeItem();
	MPE->mpetocv = mpetocv;
	MPE->text = "MPE - Standard (ROLI, etc)";
	MPE->MPEPlus = false;
	menu->addChild(MPE);
	// MPE Plus
	QuadMPEModeItem *MPEPlus = new QuadMPEModeItem();
	MPEPlus->mpetocv = mpetocv;
	MPEPlus->text = "MPE+ - High Res for Haken Continuum";
	MPEPlus->MPEPlus = true;
	menu->addChild(MPEPlus);

	return menu;
}
