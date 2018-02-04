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
		yPos += midiChoice->box.size.y + margin;
	}

    {
        Label *label = new Label();
        label->box.pos = Vec(margin, yPos);
        label->text = "Bend Range";
        addChild(label);
        // xPos += label->box.size.x - 20 ; //+ margin;
        addParam(createParam<RoundSmallBlackSnapKnob>(Vec(15 * 6, yPos), module, MPEToCVInterface::BEND_RANGE_PARAM, 1.0, 8.0, 8.0));
        yPos += labelHeight + 4*margin;
        // SemitoneChoice *semitoneChoice = new SemitoneChoice();
        // semitoneChoice->box.pos = Vec(xPos,yPos);
        // addChild(semitoneChoice);
     
    }


    //addParam(createParam<NKK>(Vec(89, 140), module, MPEToCVInterface::BEND_RANGE, 1, 2, 3,4,8));

	// {
	// 	Label *label = new Label();
	// 	label->box.pos = Vec(margin, yPos);
	// 	label->text = "Channel";
	// 	addChild(label);
	// 	yPos += labelHeight + margin;

	// 	ChannelChoice *channelChoice = new ChannelChoice();
	// 	channelChoice->midiModule = dynamic_cast<MidiIO *>(module);
	// 	channelChoice->box.pos = Vec(margin, yPos);
	// 	channelChoice->box.size.x = box.size.x - 10;
	// 	addChild(channelChoice);
	// 	yPos += channelChoice->box.size.y + margin + 15;
	// }

	std::string labels[MPEToCVInterface::NUM_OUTPUTS] = {"1V/oct", "Gate", "Velocity", "Pressure", "Pitch Wheel",
														  "Aftertouch"};

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