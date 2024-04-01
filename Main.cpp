#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <commdlg.h>
#include <dlgs.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "ExtProtect.h"
#include "resource.h"

#pragma warning( once : 4244 4312 4276 )

const char *TITLE_STRING = "Extrarius's W3M Protector";
const char *PROTECTOR_WIN_CLASS = "W3M Protector";
const int MAX_CONTROLS = 20;
const char *FILE_FILTER = "Any Map Files (*.w3m;*.w3x)\0*.w3m;*.w3x\0Warcraft 3 Map Files (*.w3m)\0*.w3m\0Frozen Throne Map Files (*.w3x)\0*.w3x\0All Files (*.*)\0*.*\0\0";
const int THE_FONT_TO_USE = DEFAULT_GUI_FONT;
const int BUFFER_LEN = 1000;
const char *ABOUT_TEXT = "Extrarius's Warcraft 3 Map Protector Version 0.1.2.0ß\nCopyright 2004 By Extrarius (ExtProtect@PsychoSanity.com)\nRedistribution and use without modification are permitted.\n\nUse This Progarm At Your Own Risk (See readme.txt for details).";
const char *ABOUT_TITLE = "About Extrarius's W3M Protector";

#define CONTROL_ID(name) const int name = 100 + __COUNTER__

CONTROL_ID(CID_OPEN_FILENAME);
CONTROL_ID(CID_OPEN_BROWSE);
CONTROL_ID(CID_PASSWORD);
CONTROL_ID(CID_CONFIRM);
CONTROL_ID(CID_SAVE_FILENAME);
CONTROL_ID(CID_SAVE_BROWSE);
CONTROL_ID(CID_CLOSE);
CONTROL_ID(CID_UNPROTECT);
CONTROL_ID(CID_ABOUT);
CONTROL_ID(CID_STATUS);

//INI Variables
char iniFileName[BUFFER_LEN];

bool iniReadOnlyFilenames;
bool iniWarnNoBackup;
//End INI Variables

HINSTANCE hInst;
HWND hWnd;
HBRUSH hBG;
HWND hControls[MAX_CONTROLS] = {0};
int iNumControls = 0;
bool ProtectMap = false; //false = Unprotect, true = Protect


void LoadINIData(void)
{
	char buffer[BUFFER_LEN];
	char *p;
	HWND c;
	int len;

	iniReadOnlyFilenames = true;
	iniWarnNoBackup = true;

	GetModuleFileName(NULL, iniFileName, BUFFER_LEN-1);
	p = strrchr(iniFileName, '.');
	if(p == NULL)
		return;
	if(strcmpi(p, ".exe") == 0)
		strcpy(p, ".ini");
	
	GetPrivateProfileString("Last Filenames", "Open", "",  buffer,  BUFFER_LEN,  iniFileName);
	c = GetDlgItem(hWnd, CID_OPEN_FILENAME);
	SendMessage(c, WM_SETTEXT, 0, (LPARAM)buffer);
	len = strlen(buffer);
	SendMessage(c, EM_SETSEL, (WPARAM)len, (LPARAM)len);

	GetPrivateProfileString("Last Filenames", "Save", "",  buffer,  BUFFER_LEN,  iniFileName);
	c = GetDlgItem(hWnd, CID_SAVE_FILENAME);
	SendMessage(c, WM_SETTEXT, 0, (LPARAM)buffer);
	len = strlen(buffer);
	SendMessage(c, EM_SETSEL, (WPARAM)len, (LPARAM)len);

	GetPrivateProfileString("Options", "ReadOnlyFileNames", "True",  buffer,  BUFFER_LEN,  iniFileName);
	if(strcmpi(buffer, "false") == 0)
		iniReadOnlyFilenames = false;

	c = GetDlgItem(hWnd, CID_OPEN_FILENAME);
	SendMessage(c, EM_SETREADONLY, (WPARAM) iniReadOnlyFilenames, 0);
	c = GetDlgItem(hWnd, CID_SAVE_FILENAME);
	SendMessage(c, EM_SETREADONLY, (WPARAM) iniReadOnlyFilenames, 0);

	GetPrivateProfileString("Options", "WarnNoBackup", "True",  buffer,  BUFFER_LEN,  iniFileName);
	if(strcmpi(buffer, "false") == 0)
		iniWarnNoBackup = false;
}

void SaveINIData(void)
{
	char buffer[BUFFER_LEN];

	if(iniReadOnlyFilenames)
		WritePrivateProfileString("Options", "ReadOnlyFileNames", "True", iniFileName);
	else
		WritePrivateProfileString("Options", "ReadOnlyFileNames", "False", iniFileName);

	if(iniWarnNoBackup)
		WritePrivateProfileString("Options", "WarnNoBackup", "True", iniFileName);
	else
		WritePrivateProfileString("Options", "WarnNoBackup", "False", iniFileName);

	SendMessage(GetDlgItem(hWnd, CID_OPEN_FILENAME), WM_GETTEXT, (WPARAM)BUFFER_LEN-1, (LPARAM)buffer);
	WritePrivateProfileString("Last Filenames", "Open", buffer, iniFileName);
	SendMessage(GetDlgItem(hWnd, CID_SAVE_FILENAME), WM_GETTEXT, (WPARAM)BUFFER_LEN-1, (LPARAM)buffer);
	WritePrivateProfileString("Last Filenames", "Save", buffer, iniFileName);
}

void UpdateStatus(const char *status)
{
	HWND c;

	c = GetDlgItem(hWnd, CID_STATUS);
	SendMessage(c, WM_SETTEXT, 0, (LPARAM)status);
	UpdateWindow(c);
}

bool IsValidFileName(const char *filename)
{
	bool valid = true;

	if(filename == NULL)
		valid = false;
	if(strlen(filename) == 0)
		valid = false;
	if(strcspn(filename, "<>?*\"|") != strlen(filename))
		valid = false;
	return valid;
}

LRESULT CALLBACK ProtectorWinProc(HWND whWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	OPENFILENAME oFN;
	HWND c;
	bool pSuccess;
	LPDRAWITEMSTRUCT itemInfo;
	char strBuffer1[BUFFER_LEN] = "";
	char strBuffer2[BUFFER_LEN] = "";
	char strBuffer3[BUFFER_LEN] = "";
	char strBuffer4[BUFFER_LEN] = "";

	switch(uMsg)
    {
	case WM_DRAWITEM:
		if((int)wParam == CID_STATUS)
		{
			HDC hDC;
			RECT rect;

			itemInfo = (LPDRAWITEMSTRUCT)lParam;
			c = itemInfo->hwndItem;
			GetClientRect(c, &rect);
			hDC = itemInfo->hDC;
			SelectObject(hDC, (HFONT)GetStockObject(THE_FONT_TO_USE));
			FillRect(hDC, &rect, hBG);
			DrawEdge(hDC, &rect, EDGE_ETCHED, BF_RECT);
			SetBkMode(hDC, TRANSPARENT);
			SendMessage(c, WM_GETTEXT, (WPARAM)BUFFER_LEN-1, (LPARAM)strBuffer1);
			DrawText(hDC, strBuffer1, (int)strlen(strBuffer1), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		}
		break; //End Case WM_DRAWITEM
    case WM_DESTROY:
		SaveINIData();
        PostQuitMessage(0);
        break; //End Case WM_DESTROY
	
	case WM_COMMAND:
		if(HIWORD(wParam) == EN_CHANGE)
		{
			//Update Protect Button Based On
			if(LOWORD(wParam) == CID_OPEN_FILENAME)
			{
				SendMessage((HWND)lParam, WM_GETTEXT, (WPARAM)BUFFER_LEN-1, (LPARAM)strBuffer1);
				c = GetDlgItem(whWnd, CID_UNPROTECT);
				if(IsW3M(strBuffer1))
				{
					if(IsProtected(strBuffer1))
					{
						if(HasExtBackup(strBuffer1))
						{
							ProtectMap = false;
							SetWindowLong(c, GWL_STYLE, GetWindowLong(c, GWL_STYLE) & ~WS_DISABLED);
							SendMessage(c, WM_SETTEXT, 0, (LPARAM)"UnProtect");
						}
						else
						{
							SetWindowLong(c, GWL_STYLE, GetWindowLong(c, GWL_STYLE) | WS_DISABLED);
							SendMessage(c, WM_SETTEXT, 0, (LPARAM)"No Backup");
						}
					}
					else
					{
						ProtectMap = true;
						SetWindowLong(c, GWL_STYLE, GetWindowLong(c, GWL_STYLE) & ~WS_DISABLED);
						SendMessage(c, WM_SETTEXT, 0, (LPARAM)"Protect");
					}
				}
				else
				{
					SetWindowLong(c, GWL_STYLE, GetWindowLong(c, GWL_STYLE) | WS_DISABLED);
					SendMessage(c, WM_SETTEXT, 0, (LPARAM)"Not A Map");
				}
			}
		}

		if(HIWORD(wParam) == BN_CLICKED)
		{
			switch(LOWORD(wParam))
			{
			case CID_OPEN_BROWSE:
				ZeroMemory(&oFN, sizeof(oFN));
				oFN.lStructSize = sizeof(oFN); 
				oFN.hwndOwner = whWnd;
				oFN.hInstance = hInst;
				oFN.lpstrFilter = FILE_FILTER;
				oFN.lpstrFile = strBuffer1; 
				oFN.nMaxFile = BUFFER_LEN;
				oFN.lpstrTitle = "Select Warcraft 3 Map To Protect/Unprotect"; 
				oFN.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER; 
				oFN.lpstrDefExt = "w3x";

				if(GetOpenFileName(&oFN))
				{
					int len = (int)strlen(strBuffer1);
					c = GetDlgItem(whWnd, CID_OPEN_FILENAME);
					SendMessage(c, WM_SETTEXT, 0, (LPARAM)strBuffer1);
					SendMessage(c, EM_SETSEL, (WPARAM)len, (LPARAM)len);
				}
				break; //End Case CID_OPEN_BROWSE

			case CID_SAVE_BROWSE:
				ZeroMemory(&oFN, sizeof(oFN));
				oFN.lStructSize = sizeof(oFN); 
				oFN.hwndOwner = whWnd;
				oFN.hInstance = hInst;
				oFN.lpstrFilter = FILE_FILTER;
				oFN.lpstrFile = strBuffer1; 
				oFN.nMaxFile = BUFFER_LEN;
				oFN.lpstrTitle = "Save Modified Map To"; 
				oFN.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER | OFN_OVERWRITEPROMPT; 
				oFN.lpstrDefExt = "w3x";

				if(GetSaveFileName(&oFN))
				{
					int len = (int)strlen(strBuffer1);
					c = GetDlgItem(whWnd, CID_SAVE_FILENAME);
					SendMessage(c, WM_SETTEXT, 0, (LPARAM)strBuffer1);
					SendMessage(c, EM_SETSEL, (WPARAM)len, (LPARAM)len);
				}
				break; //End Case CID_SAVE_BROWSE

			case CID_ABOUT:
				MSGBOXPARAMS mbInfo;
				ZeroMemory(&mbInfo, sizeof(mbInfo));
				mbInfo.cbSize = sizeof(mbInfo);
				mbInfo.hwndOwner = whWnd;
				mbInfo.hInstance = hInst;
				mbInfo.lpszText = ABOUT_TEXT;
				mbInfo.lpszCaption = ABOUT_TITLE;
				mbInfo.dwStyle = MB_OK | MB_USERICON | MB_APPLMODAL;
				mbInfo.lpszIcon = MAKEINTRESOURCE(IDI_ABOUT);
				mbInfo.dwContextHelpId = 0;
				mbInfo.dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
				MessageBoxIndirect(&mbInfo);
				break; //End Case CID_ABOUT

			case CID_UNPROTECT:
				SendMessage(GetDlgItem(whWnd, CID_SAVE_FILENAME), WM_GETTEXT, (WPARAM)BUFFER_LEN-1, (LPARAM)strBuffer1);
				if(!IsValidFileName(strBuffer1))
				{
					MessageBox(whWnd, "Error: Filename selected to save as is not a valid filename", "Error", MB_ICONERROR);
					UpdateStatus("Error");
					break;
				}
				SendMessage(GetDlgItem(whWnd, CID_PASSWORD), WM_GETTEXT, (WPARAM)BUFFER_LEN-1, (LPARAM)strBuffer3);
				SendMessage(GetDlgItem(whWnd, CID_CONFIRM), WM_GETTEXT, (WPARAM)BUFFER_LEN-1, (LPARAM)strBuffer4);
				if(strcmp(strBuffer3, strBuffer4) != 0)
				{
					MessageBox(whWnd, "Error: Password and Confirmation do not match!", "Error", MB_ICONERROR);
					UpdateStatus("Error");
					break;
				}
				if(ProtectMap && (strcmp(strBuffer3, "")==0) && iniWarnNoBackup)
				{
					if(MessageBox(whWnd, "Warning: A map protected without a password can not be unprotected!\nAre you sure you want to protect the map without a password?", "Warning!", MB_YESNO | MB_ICONWARNING) == IDNO)
						break;
				}
				SendMessage(GetDlgItem(whWnd, CID_OPEN_FILENAME), WM_GETTEXT, (WPARAM)BUFFER_LEN-1, (LPARAM)strBuffer1);
				strcpy(strBuffer2, _tempnam(".", "Ext"));
				UpdateStatus((ProtectMap?"Protecting":"UnProtecting"));
				if(CopyFile(strBuffer1, strBuffer2, FALSE))
				{
					if(ProtectMap)
					{
						CompressMPQ(strBuffer2);
						pSuccess = ProtectW3M(strBuffer2, strBuffer3);
					}
					else
					{
						pSuccess = UnprotectW3M(strBuffer2, strBuffer3);
						CompressMPQ(strBuffer2);
					}
					if(pSuccess)
					{
						SendMessage(GetDlgItem(whWnd, CID_SAVE_FILENAME), WM_GETTEXT, (WPARAM)BUFFER_LEN-1, (LPARAM)strBuffer1);
						if(!CopyFile(strBuffer2, strBuffer1, FALSE))
						{
							MessageBox(whWnd, "Error: Unable to save protected map!", "Error", MB_ICONERROR);
							UpdateStatus("Error");
							DeleteFile(strBuffer2);
						}
						else
						{
							UpdateStatus("Done");
						}
					}
					else
					{
						char ErrorString[BUFFER_LEN];
						sprintf(ErrorString, "Error %s the map: %s!", (ProtectMap?"Protecting":"Unprotecting"),
							GetProtectError());
						MessageBox(whWnd, ErrorString, "Error", MB_ICONERROR);
						UpdateStatus("Error");
					}
				}
				else
				{
					MessageBox(whWnd, "Error: Unable to create temporary file!", "Error", MB_ICONERROR);
					UpdateStatus("Error");
				}
				break; //End Case CID_UNPROTECT

			case CID_CLOSE:
				ShowWindow(whWnd, SW_HIDE);
				SendMessage(whWnd, WM_DESTROY, 0, 0);
				break; //End Case CID_CLOSE
			}
		}
		break; //End Case WM_COMMAND
    }
    return DefWindowProc(whWnd, uMsg, wParam, lParam);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR cmdLine, int cmdShow)
{
	WNDCLASSEX wClass;
	MSG mMsg;
	const DWORD winStyle = WS_POPUPWINDOW | WS_CAPTION | WS_VISIBLE | WS_MINIMIZEBOX;
	TEXTMETRIC fontInfo;
	RECT cCoords;
	int tHeight, tWidth, wWidth, wHeight, cyPos;
	HDC hDC;

	srand((unsigned long)time(NULL));

	hInst = hInstance;
	hBG = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));

	wClass.cbSize = sizeof(wClass);
	wClass.style = 0;
	wClass.lpfnWndProc = ProtectorWinProc;
	wClass.cbClsExtra = 0;
	wClass.cbWndExtra = 0;
	wClass.hInstance = hInstance;
	wClass.hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
	wClass.hCursor = (HCURSOR)LoadCursor(NULL, IDC_ARROW);
	wClass.hbrBackground = hBG;
	wClass.lpszMenuName = 0;
	wClass.lpszClassName = PROTECTOR_WIN_CLASS;
	wClass.hIconSm = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, 16, 16, 0);
	RegisterClassEx(&wClass);

	hWnd = CreateWindowEx(WS_EX_ACCEPTFILES | WS_EX_APPWINDOW | WS_EX_WINDOWEDGE | WS_EX_CONTROLPARENT,
		PROTECTOR_WIN_CLASS, TITLE_STRING, winStyle,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		0, 0, hInstance, NULL);

	//Get Text Dimensions
	SendMessage(hWnd, WM_SETFONT, (WPARAM)GetStockObject(THE_FONT_TO_USE), TRUE);
	hDC = GetDC(hWnd);
	memset(&fontInfo, 0, sizeof(fontInfo));
	SetMapMode(hDC, MM_TEXT);
	GetTextMetrics(hDC, &fontInfo);
	ReleaseDC(hWnd, hDC);
	tHeight = (LONG)(fontInfo.tmHeight * 1.1); //x1.1 For Padding
	tWidth = (LONG)(fontInfo.tmAveCharWidth * 1.1); //x1.1 For Padding

	//Resize Window
	wWidth = (int)(tWidth * 56.5 + 10);
	wHeight = (int)(tHeight * 8.0 + 10);
	cCoords.left = 0;
	cCoords.top = 0;
	cCoords.right = wWidth;
	cCoords.bottom = wHeight;
	AdjustWindowRect(&cCoords, winStyle, FALSE);
	SetWindowPos(hWnd, 0, (GetSystemMetrics(SM_CXSCREEN)-(cCoords.right - cCoords.left))/2, (GetSystemMetrics(SM_CYSCREEN)-(cCoords.bottom - cCoords. top))/2, cCoords.right - cCoords.left, cCoords.bottom - cCoords. top, 0);

	//FileName Selection Controls
	cyPos = tHeight*0.5;
	hControls[iNumControls++] = CreateWindowEx(0, "static", "Map Name:",
		WS_CHILD | WS_VISIBLE | SS_RIGHT,
		tWidth*0.5+0, cyPos+3, tWidth*9, tHeight, hWnd, (HMENU)0, hInstance, NULL);

	hControls[iNumControls++] = CreateWindowEx(WS_EX_CLIENTEDGE, "edit", "", 
		WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_OEMCONVERT | WS_TABSTOP,
		tWidth*10.5+0, cyPos, tWidth*44, tHeight+5, hWnd, (HMENU)CID_OPEN_FILENAME, hInstance, NULL);

	hControls[iNumControls++] = CreateWindowEx(0, "button", "...", 
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_TEXT | WS_TABSTOP,
		tWidth*54.5+0, cyPos, tWidth*3, tHeight*1+5, hWnd, (HMENU)CID_OPEN_BROWSE, hInstance, NULL);

	//Password Box
	cyPos = tHeight*2.0;
	hControls[iNumControls++] = CreateWindowEx(0, "static", "Password:",
		WS_CHILD | WS_VISIBLE | SS_RIGHT,
		tWidth*0.5+0, cyPos+3, tWidth*9, tHeight, hWnd, (HMENU)0, hInstance, NULL);

	hControls[iNumControls++] = CreateWindowEx(WS_EX_CLIENTEDGE, "edit", "", 
		WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_PASSWORD | WS_TABSTOP | ES_UPPERCASE,
		tWidth*10.5+0, cyPos, tWidth*47, tHeight+5, hWnd, (HMENU)CID_PASSWORD, hInstance, NULL);

	//Password Confirmation Box
	cyPos = tHeight*3.5;
	hControls[iNumControls++] = CreateWindowEx(0, "static", "Confirm:",
		WS_CHILD | WS_VISIBLE | SS_RIGHT,
		tWidth*0.5+0, cyPos+3, tWidth*9, tHeight, hWnd, (HMENU)0, hInstance, NULL);

	hControls[iNumControls++] = CreateWindowEx(WS_EX_CLIENTEDGE, "edit", "", 
		WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_PASSWORD | WS_TABSTOP | ES_UPPERCASE,
		tWidth*10.5+0, cyPos+0, tWidth*47, tHeight+5, hWnd, (HMENU)CID_CONFIRM, hInstance, NULL);

	//Save FileName Dialog
	cyPos = tHeight*5.0;
	hControls[iNumControls++] = CreateWindowEx(0, "static", "Save As:",
		WS_CHILD | WS_VISIBLE | SS_RIGHT,
		tWidth*0.5+0, cyPos+3, tWidth*9, tHeight, hWnd, (HMENU)0, hInstance, NULL);

	hControls[iNumControls++] = CreateWindowEx(WS_EX_CLIENTEDGE, "edit", "", 
		WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_OEMCONVERT | WS_TABSTOP,
		tWidth*10.5+0, cyPos, tWidth*44, tHeight+5, hWnd, (HMENU)CID_SAVE_FILENAME, hInstance, NULL);

	hControls[iNumControls++] = CreateWindowEx(0, "button", "...", 
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_TEXT | WS_TABSTOP,
		tWidth*54.5+0, cyPos, tWidth*3, tHeight*1+5, hWnd, (HMENU)CID_SAVE_BROWSE, hInstance, NULL);

	//Buttons: Do It, About, Close
	cyPos = wHeight-(tHeight*1+10);
	hControls[iNumControls++] = CreateWindowEx(0, "static", "",
		WS_CHILD | WS_VISIBLE | WS_DLGFRAME,
		tWidth*0.5, cyPos-tHeight*0.5, wWidth-tWidth*0.5, 5, hWnd, (HMENU)0, hInstance, NULL);

	hControls[iNumControls++] = CreateWindowEx(0, "button", "About", 
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_TEXT | WS_TABSTOP,
		tWidth*0.5, cyPos, tWidth*11, tHeight*1+5, hWnd, (HMENU)CID_ABOUT, hInstance, NULL);
	hControls[iNumControls++] = CreateWindowEx(0, "button", "Not A Map", 
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_TEXT | WS_TABSTOP | WS_DISABLED,
		tWidth*35.5+0, cyPos, tWidth*11, tHeight*1+5, hWnd, (HMENU)CID_UNPROTECT, hInstance, NULL);
	hControls[iNumControls++] = CreateWindowEx(0, "button", "Close", 
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_TEXT | WS_TABSTOP,
		tWidth*47.5+0, cyPos, tWidth*10, tHeight*1+5, hWnd, (HMENU)CID_CLOSE, hInstance, NULL);

	//Custom Drawn Status Box
	hControls[iNumControls++] = CreateWindowEx(0, "static", "Done",
		WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
		tWidth*12.5+0, cyPos, tWidth*22, tHeight*1+5, hWnd, (HMENU)CID_STATUS, hInstance, NULL);

	for(int i = 0; i < iNumControls; ++i)
		SendMessage(hControls[i], WM_SETFONT, (WPARAM)GetStockObject(THE_FONT_TO_USE), TRUE);

	LoadINIData();

	SetFocus(GetDlgItem(hWnd, CID_OPEN_FILENAME));
	
	while(GetMessage(&mMsg,0,0,0))
    {
		if(!IsDialogMessage(hWnd, &mMsg))
		{
			TranslateMessage(&mMsg);
			DispatchMessage(&mMsg);
		}
    }
	DeleteObject((HGDIOBJ)hBG);
	return 0;
}