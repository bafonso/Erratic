#include "QuadMPEToCV.hpp"


QuadMPEToCVWidget::QuadMPEToCVWidget(QuadMPEToCV *module):ModuleWidget(module) {
	// QuadMPEToCV *module = new QuadMPEToCV();
	// setModule(module);
	box.size = Vec(16 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	
	Vec pos = Vec();
	QuadMPEMidiWidget *quadmpemidiWidget ;

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/QuadMPE.svg")));
		addChild(panel);
	}

	float margin = 5;
	float labelHeight = 15;
	float yPos = margin;
	
	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

	quadmpemidiWidget = Widget::create<QuadMPEMidiWidget>(mm2px(Vec(5.41891, 11)));
	quadmpemidiWidget->initialize(module);
	quadmpemidiWidget->box.size = mm2px(Vec(60, 28));
	// box.size = mm2px(Vec(44, 28));
	quadmpemidiWidget->midiIO = &module->midiInput;
	addChild(quadmpemidiWidget);
	pos = quadmpemidiWidget->box.getBottomLeft();



		// MidiChoice *midiChoice = new MidiChoice();
		// midiChoice->midiModule = dynamic_cast<MidiIO *>(module);
		// midiChoice->box.pos = Vec(margin, 40);
		// midiChoice->box.size.x = box.size.x - 10;
		// addChild(midiChoice);
		// yPos += midiChoice->box.size.y + 4*margin;


	yPos = quadmpemidiWidget->box.pos.y + quadmpemidiWidget->box.size.y + 6*margin ;

	std::string labels[QuadMPEToCV::NUM_OUTPUTS] = {"1V/oct", "Gate", "Velocity", "Pressure", "Y axis"} ;
	for (int i = 0; i < 5 ; i++) {
		addOutput(createOutput<PJ3410Port>(Vec(2 * (40), yPos - 5), module, i * 4));
		addOutput(createOutput<PJ3410Port>(Vec(3 * (40), yPos - 5), module, i * 4 + 1));
		addOutput(createOutput<PJ3410Port>(Vec(4 * (40), yPos - 5), module, i * 4 + 2));
		addOutput(createOutput<PJ3410Port>(Vec(5 * (40), yPos - 5), module, i * 4 + 3));
		yPos += 40;
		// yPos += yGap + margin;
	}
	addOutput(createOutput<PJ3410Port>(Vec(2 * (40), yPos - 5), module, 20));

};

Model *modelQuadMPEToCV = Model::create<QuadMPEToCV, QuadMPEToCVWidget>("Erratic", "QuadMPEToCV", "Quad MPE To CV", MIDI_TAG, EXTERNAL_TAG);
