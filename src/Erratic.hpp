#include "rack.hpp"


using namespace rack;

// Forward-declare the Plugin, defined in Template.cpp
extern Plugin *plugin;

// Forward-declare each Model, defined in each module source file
extern Model *modelMPEToCV;
extern Model *modelQuadMPEToCV;
// extern Model *Notes;
// extern Model *QuadMPEToCV;

// struct MPEToCVWidget : ModuleWidget {
// 	MPEToCVWidget();
// 	Menu *createContextMenu() override;
// 	void step() override;
// };

// struct QuadMPEToCVWidget : ModuleWidget {
// 	QuadMPEToCVWidget();
// 	Menu *createContextMenu() override;
// 	void step() override;
// };
