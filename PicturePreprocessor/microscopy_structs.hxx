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
	std::string path;
	std::string outpath;
	bool swap_layers;
	Experiment_Params() {
		width = 0;
		height = 0;
		z_stepsize = 0;
		x_frames = 0;
		y_frames = 0;
		z_frames = 0;
		calibrationdate = "";
		path = "";
		outpath = "";
		swap_layers = false;
	}
};
}

#endif