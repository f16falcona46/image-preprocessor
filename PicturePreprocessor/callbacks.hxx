#ifndef CALLBACKS_FOR_PICPREPROCESSOR
#define CALLBACKS_FOR_PICPREPROCESSOR

#include <windows.h>
#include <commctrl.h>

INT_PTR CALLBACK MainDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ProgressDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif