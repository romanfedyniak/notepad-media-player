#pragma once
#ifndef _NOTEPAD_DRAW_H_
#define _NOTEPAD_DRAW_H_

#include <windows.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <windowsx.h>
#include <richedit.h>
#include <iostream>
#include <string>
#include <cmath>

#define WIN32_LEAN_AND_MEAN

typedef unsigned __int64 QWORD;

class NotepadDraw
{
private:
    std::string pathToNotepadExe;
    int windowWidth, windowHeight, charsPerLine, charsPerCol, charBufferSize, fontSize;
    DWORD pid;
    HANDLE process;
    HWND topWindow, editWindow;
    char *frontBuffer, *backBuffer;
    HDC hdc;
    bool isStatusBar;

private:
    HANDLE launchNotepad();
    HWND getTopWindowForProcess(DWORD pid);
    HWND getWindowForProcessAndClassName(DWORD pid, const char* className);
    char* findPattern(char* src, size_t srcLen, const char* pattern, size_t len);
    int findBytePatternInProcessMemory(HANDLE handle, const char* bytePattern,
        size_t patternLen, char** outPointers, size_t outPointerCount);
    char* createAndAcquireRemoteCharBufferPtr();
    std::string getErrorDescription(DWORD err);
    void setFontSize(int size);
    SIZE calcWindowSize();
public:
    NotepadDraw(std::string pathToNotepadExe, int charsPerLine, int charsPerCol, int fontSize, bool isStatusBar);

    void init();
    void shutdownNotepad();
    void drawChar(int x, int y, char c);
    void clearScreen();
    void swapBuffersAndRedraw();
    virtual ~NotepadDraw() = default;
};

#endif