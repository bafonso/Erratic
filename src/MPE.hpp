#include "rack.hpp"


using namespace rack;


extern Plugin *plugin;

////////////////////
// module widgets
////////////////////

struct MPEToCVWidget : ModuleWidget {
	MPEToCVWidget();
	void step() override;
};
