#include <windows.h>
#include <commctrl.h>
#include <iostream>
#include <future>
#include <vector>
#include <string>
#include <algorithm>
#include <regex>

#include <Eigen/Dense>
#include <png.h>

#include "resource.h"
#include "imagefuncs.hxx"
#include "microscopy_structs.hxx"
#include "microscopy_funcs.hxx"
#include "callbacks.hxx"

#define DEFAULT_WINDOW_WIDTH 640
#define DEFAULT_WINDOW_HEIGHT 480
#define MINIMUM_WINDOW_WIDTH 320
#define MINIMUM_WINDOW_HEIGHT 240

const char* notepad_szClassName = "picturepreprocessor_window_class";
/*
//loads a filename from an open file dialog into g_szOpenFilename
//can spawn a messagebox if the file dialog fails
//TRUE if g_szOpenFilename changed, FALSE if not
BOOL LoadOpenFileName(HWND hwnd, LPSTR pszFilename) {
	OPENFILENAME ofn;
	char szFilename[MAX_PATH] = "";
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szFilename;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = "txt";
	
	if (GetOpenFileName(&ofn)) {
		LPSTR ptr = lstrcpy(pszFilename, szFilename);
		if (ptr == NULL) {
			MessageBox(hwnd, "Couldn't load filename!", "Error!", MB_OK | MB_ICONERROR);
		}
		else {
			updateTitleText(hwnd);
			return TRUE;
		}
	}
	else {
		if (CommDlgExtendedError() != 0) {
			MessageBox(hwnd, "Couldn't load filename!", "Error!", MB_OK | MB_ICONERROR);
		}
		else return FALSE;
	}
}

//loads a filename from a save file dialog into g_szOpenFilename
//can spawn a messagebox if the file dialog fails
//TRUE if g_szOpenFilename changed, FALSE if not
BOOL LoadSaveFileName(HWND hwnd, LPSTR pszFilename) {
	OPENFILENAME ofn;
	char szFilename[MAX_PATH] = "";
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szFilename;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = "txt";
	
	if (GetSaveFileName(&ofn)) {
		LPSTR ptr = lstrcpy(pszFilename, szFilename);
		if (ptr == NULL) {
			MessageBox(hwnd, "Couldn't load filename!", "Error!", MB_OK | MB_ICONERROR);
		}
		else {
			updateTitleText(hwnd);
			return TRUE;
		}
	}
	else {
		if (CommDlgExtendedError()) {
			MessageBox(hwnd, "Couldn't load filename!", "Error!", MB_OK | MB_ICONERROR);
		}
		else return FALSE;
	}
}

//saves the edit control's text into g_szOpenFilename
//returns TRUE if it was successful, FALSE if not
BOOL SaveTextToFile(HWND hEdit, LPCSTR pszFilename) {
	BOOL bSuccess = FALSE;
	HANDLE hFile = CreateFile(pszFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		DWORD dwTextLength = GetWindowTextLength(hEdit);
		LPSTR pszText = static_cast<LPSTR>(GlobalAlloc(GMEM_FIXED, dwTextLength+1));
		if (pszText != NULL) {
			if (GetWindowText(hEdit, pszText, dwTextLength+1)) {
				DWORD dwWritten; //dummy param
				if (WriteFile(hFile, pszText, dwTextLength, &dwWritten, NULL))
					bSuccess = TRUE;
			}
			GlobalFree(pszText);
		}
		CloseHandle(hFile);
	}
	return bSuccess;
}

//loads the text in g_szOpenFilename into the edit control
//returns TRUE if it was successful, FALSE if not
BOOL LoadTextFileIntoEdit(HWND hEdit, LPCSTR pszFilename) {
	BOOL bSuccess = FALSE;
	HANDLE hFile = CreateFile(pszFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		DWORD dwFileSize = GetFileSize(hFile, NULL);
		if (dwFileSize != 0xffffffff) {
			LPSTR pszFileText = static_cast<LPSTR>(GlobalAlloc(GMEM_FIXED, dwFileSize+1));
			if (pszFileText != NULL) {
				DWORD dwRead; //dummy param
				if (ReadFile(hFile, pszFileText, dwFileSize, &dwRead, NULL)) {
					pszFileText[dwFileSize] = 0;
					if (SetWindowText(hEdit, pszFileText))
						bSuccess = TRUE;
				}
				GlobalFree(pszFileText);
			}
		}
		CloseHandle(hFile);
	}
	return bSuccess;
}
*/

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, char* lpCmdLine, int nCmdShow) {
	InitCommonControls();
	
	Eigen::MatrixXd layer1(200,200);
	for (int i = 0; i < 200; ++i) {
		layer1.col(i) = Eigen::ArrayXd::LinSpaced(200, 0, 1);
	}
	//std::cout << mat.cols() << ' ' << mat.rows() << std::endl;
	Eigen::MatrixXd layer2 = layer1.transpose();
	
	std::vector<Eigen::MatrixXd> layers1;
	layers1.push_back(layer1);
	layers1.push_back(layer2);
	ifx::write_image(layers1, "test.png");
	
	ifx::Experiment_Params params;
	params.calibrationdate = "20130125"; //default calibration date
	ifx::read_experiment_params("Experiment Parameters.txt", &params);
	std::cout << params << std::endl;
	
	INT_PTR ret = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SELECTDIRECTORY), NULL, MainDlgProc);
	if (ret != IDOK) {
		return -2;
	}
	
	ret = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_PROCESSPROGRESS), NULL, ProgressDlgProc);
	if (ret == IDOK) {
		MessageBox(NULL, "Operation completed successfully.", "Notice", MB_OK | MB_ICONINFORMATION);
		return 0;
	}
	else if (ret == IDCANCEL) {
		MessageBox(NULL, "Operation canceled.", "Notice", MB_OK | MB_ICONINFORMATION);
		return -3;
	}
	else {
		MessageBox(NULL, "Operation failed to complete.", "Notice", MB_OK | MB_ICONERROR);
		return -1;
	}
}
