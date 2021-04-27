//MIT License
//
//Copyright(c) 2020 Kyle Halladay
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this softwareand associated documentation files(the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions :
//
//The above copyright noticeand this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.


#include "notepadDraw.h"


NotepadDraw::NotepadDraw(
	std::string pathToNotepadExe, int charsPerLine, int charsPerCol, int fontSize, bool isStatusBar)
	:
	pathToNotepadExe(pathToNotepadExe),
	charsPerLine(charsPerLine),
	charsPerCol(charsPerCol),
	charBufferSize(charsPerLine * charsPerCol),
	fontSize(fontSize),
	isStatusBar(isStatusBar) {}

void NotepadDraw::init()
{
	srand((unsigned)time(NULL));

	process = launchNotepad();
	Sleep(500); //wait for process to start up

	pid = GetProcessId(process);
	topWindow = getTopWindowForProcess(pid);
	editWindow = getWindowForProcessAndClassName(pid, "Edit");
	backBuffer = new char[charBufferSize * 2]; // *2 for utf16 chars
	hdc = GetDC(editWindow);

	setFontSize(fontSize);
	SIZE windowSize = calcWindowSize();
	windowWidth = windowSize.cx;
	windowHeight = windowSize.cy;
	frontBuffer = createAndAcquireRemoteCharBufferPtr();

	// magic constants are added to window size because when resize a notepad, it is always a little smaller
	MoveWindow(topWindow, 0, 0, windowWidth + 16, windowHeight + 9, true);

	swapBuffersAndRedraw();
}

HANDLE NotepadDraw::launchNotepad()
{
	PROCESS_INFORMATION pi = { 0 };
	STARTUPINFO si = { 0 };
	if (CreateProcess(pathToNotepadExe.c_str(), NULL, NULL, NULL, false, 0, NULL, NULL, &si, &pi))
	{
		return pi.hProcess;
	}

	std::cout << "LaunchNotepad Error: " << getErrorDescription(GetLastError()) << std::endl;

	return 0;
}

HWND NotepadDraw::getTopWindowForProcess(DWORD pid)
{
	HWND hCurWnd = GetTopWindow(0);
	while (hCurWnd != NULL)
	{
		DWORD cur_pid;
		DWORD dwThreadId = GetWindowThreadProcessId(hCurWnd, &cur_pid);

		if (cur_pid == pid)
		{
			HWND lastParent = hCurWnd;
			HWND parent = GetParent(hCurWnd);
			while (parent != NULL)
			{
				lastParent = parent;
				parent = GetParent(lastParent);
			}
			return lastParent;
		}
		hCurWnd = GetNextWindow(hCurWnd, GW_HWNDNEXT);
	}

	return NULL;
}

HWND NotepadDraw::getWindowForProcessAndClassName(DWORD pid, const char* className)
{
	HWND curWnd = GetTopWindow(0);
	char classNameBuf[256];

	while (curWnd != NULL)
	{
		DWORD curPid;
		DWORD dwThreadId = GetWindowThreadProcessId(curWnd, &curPid);

		if (curPid == pid)
		{
			GetClassName(curWnd, classNameBuf, 256);
			if (strcmp(className, classNameBuf) == 0)
			{
				return curWnd;
			}

			HWND childWindow = FindWindowEx(curWnd, NULL, className, NULL);

			if (childWindow != NULL)
			{
				return childWindow;
			}
		}
		curWnd = GetNextWindow(curWnd, GW_HWNDNEXT);
	}
	return NULL;
}

std::string NotepadDraw::getErrorDescription(DWORD err)
{
	switch (err)
	{
	case 0: return "Success";
	case 1: return "Invalid Function";
	case 2: return "File Not Found";
	case 3: return "Path Not Found";
	case 4: return "Too Many Open Files";
	case 5: return "Access Denied";
	case 6: return "Invalid Handle";
	case 87: return "Invalid Parameter";
	case 1114: return "DLL Init Failed";
	case 127: return "Proc Not Found";
	}

	return "";
}

void NotepadDraw::setFontSize(int size)
{
	int lfHeight = -MulDiv(size, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	HFONT newFont = CreateFontA(lfHeight, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Consolas");

	SendMessage(editWindow, WM_SETFONT, WPARAM(newFont), true); // set font for notepad
	SelectObject(hdc, newFont); // set font for device context
}

SIZE NotepadDraw::calcWindowSize()
{
	SIZE windowSize;

	RECT textRect;
	textRect.left = textRect.right = textRect.top = textRect.bottom = 0;
	std::string str;

	for (int i = 0; i < charsPerCol; ++i)
	{
		str += std::string(charsPerLine, 'A');
		if (i != charsPerCol - 1) str += "\n";
	}

	DrawText(hdc, str.c_str(), charBufferSize, &textRect, DT_CALCRECT);

	int indent[] = {
		1, 3, 3, 3, 4, 5, 5, 6, 7, 7, 8, 8, 9, 10, 10, 11, 12, 12, 13, 14
	};

	windowSize.cx =
		textRect.right
		+ 16 // scroll bar size
		+ indent[fontSize - 1]; // indent text from the edges
		// I did not find how to determine the indentation correctly, so I picked up a constant
	windowSize.cy =
		textRect.bottom
		+ 50; // toolbar size 

	if (isStatusBar)
		windowSize.cy += 23;

	return windowSize;
}

char* NotepadDraw::findPattern(char* src, size_t srcLen, const char* pattern, size_t len)
{
	char* cur = src;
	size_t curPos = 0;

	while (curPos < srcLen)
	{
		if (memcmp(cur, pattern, len) == 0)
		{
			return cur;
		}

		curPos++;
		cur = &src[curPos];
	}
	return nullptr;
}

int NotepadDraw::findBytePatternInProcessMemory(
	HANDLE handle, const char* bytePattern, size_t patternLen, char** outPointers, size_t outPointerCount
)
{
	MEMORY_BASIC_INFORMATION memInfo;
	char* basePtr = (char*)0x0;
	VirtualQueryEx(handle, (void*)basePtr, &memInfo, sizeof(MEMORY_BASIC_INFORMATION));
	int foundPointers = 0;

	do {
		const DWORD mem_commit = 0x1000;
		const DWORD page_readwrite = 0x04;
		if (memInfo.State == mem_commit && memInfo.Protect == page_readwrite)
		{
			char* buffContents = new char[memInfo.RegionSize];
			char* baseAddr = (char*)memInfo.BaseAddress;
			SIZE_T bytesRead = 0;

			if (ReadProcessMemory(handle, memInfo.BaseAddress, buffContents, memInfo.RegionSize, &bytesRead))
			{
				char* match = findPattern(buffContents, memInfo.RegionSize, bytePattern, patternLen);
				if (match)
				{
					QWORD diff = (QWORD)match - (QWORD)buffContents;
					char* patternPtr = (char*)((QWORD)memInfo.BaseAddress + diff);
					outPointers[foundPointers++] = patternPtr;

				}
			}
			delete[] buffContents;
		}


		basePtr = (char*)memInfo.BaseAddress + memInfo.RegionSize;

	} while (VirtualQueryEx(handle, (void*)basePtr, &memInfo, sizeof(MEMORY_BASIC_INFORMATION)));

	return foundPointers;
}


char* NotepadDraw::createAndAcquireRemoteCharBufferPtr()
{
	size_t notepadCharBufferSize = (size_t)charBufferSize * 2;

	char* frameBuffer = new char[notepadCharBufferSize];
	for (int i = 0; i < charBufferSize; i++)
	{	
		char v = 0x41 + (rand() % 26);
		SendMessage(editWindow, WM_CHAR, v, 0);
		frameBuffer[i * 2] = v;
		frameBuffer[i * 2 + 1] = 0x00;
	}

	char* pointers[256];
	int foundPatterns = findBytePatternInProcessMemory(process, frameBuffer, notepadCharBufferSize, pointers, 256);

	if (foundPatterns == 0)
	{
		std::cout << "Error: Pattern Not Found" << std::endl;
		return NULL;
	}
	if (foundPatterns > 1)
	{
		std::cout << "Error: Found multiple instances of char pattern in memory" << std::endl;
		return NULL;
	}

	return pointers[0];

}

void NotepadDraw::shutdownNotepad()
{
	if (!TerminateProcess(process, 0))
	{
		std::cout << "Error terminating process: " << getErrorDescription(GetLastError()) << std::endl;
	}
	CloseHandle(process);
}

void NotepadDraw::swapBuffersAndRedraw()
{
	SIZE_T written = 0;
	WriteProcessMemory(process, frontBuffer, backBuffer, charBufferSize * 2, &written);

	RECT r;
	GetClientRect(editWindow, &r);
	InvalidateRect(editWindow, &r, false);
}

void NotepadDraw::drawChar(int x, int y, char c)
{
	backBuffer[(charsPerLine * 2) * y + x * 2] = c;
}

void NotepadDraw::clearScreen()
{
	memset(backBuffer, 0, charBufferSize * 2);
}