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
#include "image_processing.hxx"

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
			handle = std::async(std::launch::async, ifx::start_threads, filenames, g_hProgressBar, hwnd, params);
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