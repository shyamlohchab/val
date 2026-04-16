#pragma once
#include <Windows.h>

// ============================================================
//  Manual Map Injector
//  Loads a DLL into a target process without using LoadLibrary,
//  bypassing basic IAT-based detection.
// ============================================================

// Returns true on success.
bool ManualMap(HANDLE hProc, const char* dllPath);
