#include <windows.h>
#include <CommCtrl.h>
#include <vector>
#include <string.h>
#include <iostream>
#include <string>
#include <future>
#include <regex>
#include <Eigen/Dense>

#include "image_processing.hxx"
#include "imagefuncs.hxx"
#include "microscopy_structs.hxx"
#include "microscopy_funcs.hxx"

void convolve(Eigen::MatrixXd* mat, const Eigen::MatrixXd& kern) {
	return; //do nothing, not implemented yet
}

extern bool g_abortthreads;

std::vector<Eigen::MatrixXd> process_files(const std::vector<std::string>& filenames, int start, int end, HWND hProgressBar, HWND hDialog, const ifx::Experiment_Params& params) {
	try {
		const int num_layers = ifx::readbin(filenames[0], params.width, params.height).size();
		std::vector<Eigen::MatrixXd> sums;
		for (int i = 0; i < num_layers; ++i) {
			sums.emplace_back(params.width, params.height);
		}
		for (std::vector<std::string>::size_type i = start; i < end; ++i) {
			std::vector<Eigen::MatrixXd> layers = ifx::readbin(filenames[i], params.width, params.height);
			for (int j = 0; j < num_layers; ++j) {
				layers[j] = layers[j] / 16383;
				sums[j] += layers[j];
			}
			ifx::write_image(layers, params.outpath + "test" + std::to_string(i + 1) + ".png");
			if (g_abortthreads) return sums;
			SendMessage(hProgressBar, PBM_STEPIT, 0, 0);
		}
		return sums;
	}
	catch (const std::exception& e) {
		int err = errno;
		char errs[1000];
		strerror_s(errs, 1000, err);
		std::cout << e.what() << std::endl;
	}
	catch (...) {
		int err = errno;
		char errs[1000];
		strerror_s(errs, 1000, err);
		std::cout << "rip gg" << std::endl;
	}
}

void start_threads(const std::vector<std::string>& filenames, HWND hProgressBar, HWND hDialog, const ifx::Experiment_Params& params) {
	try {
		auto num_threads = std::thread::hardware_concurrency();
		if (num_threads == 0) num_threads = 1;
		num_threads = 10;
		const std::vector<std::string>::size_type files_per_thread = filenames.size() / num_threads;
		std::vector<std::future<std::vector<Eigen::MatrixXd>>> threads;
		for (int i = 0; i < num_threads - 1; ++i) {
			threads.push_back(std::async(std::launch::async, process_files, filenames, files_per_thread*i, files_per_thread*(i + 1), hProgressBar, hDialog, params));
		}
		threads.push_back(std::async(std::launch::async, process_files, filenames, files_per_thread*(num_threads - 1), filenames.size(), hProgressBar, hDialog, params));

		const int num_layers = ifx::readbin(filenames[0], params.width, params.height).size();
		std::vector<Eigen::MatrixXd> final_sums;
		for (int i = 0; i < num_layers; ++i) {
			final_sums.emplace_back(params.width, params.height);
		}
		for (std::future<std::vector<Eigen::MatrixXd>>& thread : threads) {
			std::vector<Eigen::MatrixXd> layers = thread.get();
			for (int i = 0; i < num_layers; ++i) {
				final_sums[i] += layers[i];
			}
		}
		for (int i = 0; i < num_layers; ++i) {
			final_sums[i] /= final_sums[i].maxCoeff();
		}
		ifx::write_image(final_sums, params.outpath + "/sum.png");
		EndDialog(hDialog, IDOK);
	}
	catch (const std::exception& e) {
		int err = errno;
		char errs[1000];
		strerror_s(errs, 1000, err);
		std::cout << e.what() << std::endl;
	}
	catch (...) {
		int err = errno;
		char errs[1000];
		strerror_s(errs, 1000, err);
		std::cout << "rip gg" << std::endl;
	}
}

std::vector<std::string> get_sorted_filenames(const ifx::Experiment_Params& params) {
	std::vector<std::string> filenames = ifx::get_rawfile_filenames(params);
	std::sort(filenames.begin(), filenames.end(), //natural sort technology (2 comes before 30)
		[](std::string n1, std::string n2) {
		std::regex rawbin_index(".*Raw_([0-9]+)\\.bin");
		std::smatch n1match;
		std::regex_match(n1, n1match, rawbin_index);
		std::smatch n2match;
		std::regex_match(n2, n2match, rawbin_index);
		return std::stoi(n1match[1].str()) < std::stoi(n2match[1].str());
	});
	return filenames;
}