#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <regex>
#include <Eigen/Dense>

#include "image_processing.hxx"
#include "imagefuncs.hxx"
#include "microscopy_structs.hxx"
#include "microscopy_funcs.hxx"

void convolve(Eigen::MatrixXd* mat, const Eigen::MatrixXd& kern);

std::vector<Eigen::MatrixXd> process_files(const std::vector<std::string>& filenames, int start, int end, HWND hProgressBar, HWND hDialog, const ifx::Experiment_Params& params);

void start_threads(const std::vector<std::string>& filenames, HWND hProgressBar, HWND hDialog, const ifx::Experiment_Params& params);

std::vector<std::string> get_sorted_filenames(const ifx::Experiment_Params& params);