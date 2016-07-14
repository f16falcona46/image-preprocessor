#ifndef MICROSCOPY_EXPERIMENT_PARAM_STRUCT
#define MICROSCOPY_EXPERIMENT_PARAM_STRUCT

#include <string>
#include <iostream>

namespace ifx {
struct Experiment_Params {
	int width;
	int height;
	double z_stepsize;
	int x_frames;
	int y_frames;
	int z_frames;
	std::string calibrationdate;
};
}

#endif