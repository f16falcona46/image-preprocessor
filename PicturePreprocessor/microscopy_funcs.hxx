#ifndef MICROSCOPY_EXPERIMENT_PARAM_STRUCT_FUNCTIONS
#define MICROSCOPY_EXPERIMENT_PARAM_STRUCT_FUNCTIONS

#include "microscopy_structs.hxx"
#include <vector>
#include <string>
#include <iostream>
namespace ifx{
void read_experiment_params(std::string path, ifx::Experiment_Params* params);
std::vector<std::string> get_rawfile_filenames(std::string path);
}

std::ostream& operator<<(std::ostream& os, const ifx::Experiment_Params& params);

#endif