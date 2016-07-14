#include "microscopy_structs.hxx"
#include "microscopy_funcs.hxx"
#include <vector>
#include <string>
#include <regex>
#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>

void ifx::read_experiment_params(ifx::Experiment_Params* params) {
	std::regex width ("Pixels: ([0-9]+)");
	std::regex height ("Y Pixels: ([0-9]+)");
	std::regex z_stepsize ("Z StepSize: ([.0-9]+)");
	std::regex x_frames ("Frames in one X Line: ([0-9]+)");
	std::regex y_frames ("Frames in one Y Line: ([0-9]+)");
	std::regex z_frames ("Frames in one Z Line: ([0-9]+)");
	std::regex calibrationdate ("calibration date: (.*)");
	std::smatch match;
	
	std::ifstream fin(params->path + "Experiment Parameters.txt");
	std::string line;
	while (std::getline(fin, line)) {
		if (std::regex_match(line, match, width)) {
			params->width = std::stoi(match[1].str());
			params->height = params->width;
		}
		else if (std::regex_match(line, match, height)) {
			params->height = std::stoi(match[1].str());
		}
		else if (std::regex_match(line, match, z_stepsize)) {
			params->z_stepsize = std::stod(match[1].str());
		}
		else if (std::regex_match(line, match, x_frames)) {
			params->x_frames = std::stoi(match[1].str());
		}
		else if (std::regex_match(line, match, y_frames)) {
			params->y_frames = std::stoi(match[1].str());
		}
		else if (std::regex_match(line, match, z_frames)) {
			params->z_frames = std::stoi(match[1].str());
		}
		else if (std::regex_match(line, match, calibrationdate)) {
			params->calibrationdate = match[1].str();
		}
	}
}

std::ostream& operator<<(std::ostream& os, const ifx::Experiment_Params& params) {
	os << "Experiment Params {" << std::endl
		<< "width: " << params.width << std::endl
		<< "height: " << params.height << std::endl
		<< "z_stepsize: " << params.z_stepsize << std::endl
		<< "x_frames: " << params.x_frames << std::endl
		<< "y_frames: " << params.y_frames << std::endl
		<< "z_frames: " << params.z_frames << std::endl
		<< "calibrationdate: " << params.calibrationdate << std::endl << '}';
	return os;
}

std::vector<std::string> ifx::get_rawfile_filenames(const ifx::Experiment_Params& params) {
	std::vector<std::string> files;
	std::regex rawbin (".*/Raw_[0-9]+\\.bin");
	std::smatch match;
	boost::filesystem::path p (params.path+"/RawFast");
	for (boost::filesystem::directory_entry& x : boost::filesystem::directory_iterator(p)) {
		std::string pathstring = x.path().generic_string();
		if (std::regex_match(pathstring, match, rawbin)) {
			files.push_back(pathstring);
		}
	}
	return files;
}