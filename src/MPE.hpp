#include "rack.hpp"


using namespace rack;

extern Plugin *plugin;

////////////////////
// module widgets
////////////////////

struct MPEToCVWidget : ModuleWidget {
	MPEToCVWidget();
	Menu *createContextMenu() override;
	void step() override;
};

struct QuadMPEToCVWidget : ModuleWidget {
	QuadMPEToCVWidget();
	Menu *createContextMenu() override;
	void step() override;
};
