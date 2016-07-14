#include <windows.h>
#include <commctrl.h>
#include <future>
#include <vector>
#include <regex>
#include <Eigen/Dense>
#include "resource.h"
#include "imagefuncs.hxx"
#include "microscopy_funcs.hxx"
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

bool g_abortthreads;

void process_files(const std::vector<std::string>& filenames, int start, int end, HWND hProgressBar, HWND hDialog) {
	for (std::vector<std::string>::size_type i = start; i < end; ++i) {
		std::vector<Eigen::MatrixXd> layers = ifx::readbin(filenames[i], 1024, 1024);
		for (Eigen::MatrixXd& layer : layers) {
			layer = layer / 16383;
		}
		ifx::write_image(layers, "test" + std::to_string(i + 1) + ".png");
		if (g_abortthreads) return;
		SendMessage(hProgressBar, PBM_STEPIT, 0, 0);
	}
}

void start_threads(const std::vector<std::string>& filenames, HWND hProgressBar, HWND hDialog) {
	auto num_threads = std::thread::hardware_concurrency();
	if (num_threads == 0) num_threads = 1;
	const std::vector<std::string>::size_type files_per_thread = filenames.size()/num_threads;
	std::vector<std::future<void>> threads;
	for (int i = 0; i < num_threads - 1; ++i) {
		threads.push_back(std::async(std::launch::async, process_files, filenames, files_per_thread*i, files_per_thread*(i+1), hProgressBar, hDialog));
	}
	threads.push_back(std::async(std::launch::async, process_files, filenames, files_per_thread*(num_threads - 1), filenames.size(), hProgressBar, hDialog));

	for (std::future<void>& thread : threads) {
		thread.get();
	}
	EndDialog(hDialog, IDOK);
}

static HWND g_hProgressBar = NULL;
static std::future<void> handle;
//callback for the progress bar dialog
INT_PTR CALLBACK ProgressDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_INITDIALOG:
		{
			std::vector<std::string> filenames = ifx::get_rawfile_filenames(std::string("."));
			std::sort(filenames.begin(), filenames.end(), //natural sort technology (2 comes before 30)
				[](std::string n1, std::string n2) {
					std::regex rawbin_index (".*Raw_([0-9]+)\\.bin");
					std::smatch n1match;
					std::regex_match(n1, n1match, rawbin_index);
					std::smatch n2match;
					std::regex_match(n2, n2match, rawbin_index);
					return std::stoi(n1match[1].str()) < std::stoi(n2match[1].str());
				});
			
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
			handle = std::async(std::launch::async, start_threads, filenames, g_hProgressBar, hwnd);
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