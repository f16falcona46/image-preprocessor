#include <windows.h>
#include <commctrl.h>
#include <future>
#include <vector>
#include <regex>
#include <stdexcept>
#include <cerrno>
#include <cstdlib>
#include <string.h>
#include <Eigen/Dense>
#include "resource.h"
#include "imagefuncs.hxx"
#include "microscopy_funcs.hxx"
#include "microscopy_structs.hxx"
#include "callbacks.hxx"

//callback for the main dialog
INT_PTR CALLBACK MainDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_INITDIALOG:
		{
			HICON hIcon = static_cast<HICON>(LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MYICON), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE));
			SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
		}
		return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					EndDialog(hwnd, IDOK);
				break;
				case IDCANCEL:
					EndDialog(hwnd, IDCANCEL);
				break;
			}
		break;
		default:
		return FALSE;
	}
	return TRUE;
}

void convolve(Eigen::MatrixXd* mat, const Eigen::MatrixXd& kern) {
	return; //do nothing, not implemented yet
}

bool g_abortthreads;

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

static HWND g_hProgressBar = NULL;
static std::future<void> handle;
//callback for the progress bar dialog
INT_PTR CALLBACK ProgressDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_INITDIALOG:
		{
			ifx::Experiment_Params params;
			params.path = "./in/";
			params.outpath = "./out/";
			read_experiment_params(&params);
			std::vector<std::string> filenames = get_sorted_filenames(params);

			HICON hIcon = static_cast<HICON>(LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MYICON), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE));
			SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
			
			g_hProgressBar = CreateWindowEx(0, PROGRESS_CLASS, "", PBS_SMOOTH | WS_CHILD, 20, 20, 280, 40, hwnd, NULL, GetModuleHandle(NULL), NULL);
			RECT rProgressBar;
			rProgressBar.left = 10;
			rProgressBar.right = 330;
			rProgressBar.top = 10;
			rProgressBar.bottom = 24;
			MapDialogRect(hwnd, &rProgressBar);
			MoveWindow(g_hProgressBar, rProgressBar.left, rProgressBar.top, rProgressBar.right - rProgressBar.left, rProgressBar.bottom - rProgressBar.top, TRUE);
			ShowWindow(g_hProgressBar, SW_SHOW);
			SendMessage(g_hProgressBar, PBM_SETRANGE, 0, (LPARAM)(MAKELONG(0,filenames.size())));
			SendMessage(g_hProgressBar, PBM_SETSTEP, (WPARAM)1, 0);

			g_abortthreads = false;
			handle = std::async(std::launch::async, start_threads, filenames, g_hProgressBar, hwnd, params);
		}
		break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					EndDialog(hwnd, IDOK);
				break;
				case IDCANCEL:
					EndDialog(hwnd, IDCANCEL);
				break;
			}
		break;
		case WM_DESTROY:
			g_abortthreads = true;
		break;
		default:
		return FALSE;
	}
	return TRUE;
}