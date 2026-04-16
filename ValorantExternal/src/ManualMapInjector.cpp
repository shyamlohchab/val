#include "ManualMapInjector.h"
#include <cstdint>
#include <fstream>
#include <vector>

// ============================================================
//  Manual Map Injector
//  Loads a DLL into a target process without using LoadLibrary,
//  bypassing basic IAT-based detection.
// ============================================================

struct ManualMapData
{
    HINSTANCE  hMod;
    FARPROC    fnLoadLibraryA;
    FARPROC    fnGetProcAddress;
};

// Shellcode executed in the target process to fix relocations,
// resolve imports, and call DllMain.
static void __stdcall ShellCode(ManualMapData* pData)
{
    if (!pData) return;

    BYTE* base = reinterpret_cast<BYTE*>(pData->hMod);
    auto  pNT  = reinterpret_cast<IMAGE_NT_HEADERS*>(
                     base + reinterpret_cast<IMAGE_DOS_HEADER*>(base)->e_lfanew);

    // Fix base relocations
    uintptr_t delta = reinterpret_cast<uintptr_t>(base)
                    - pNT->OptionalHeader.ImageBase;
    if (delta)
    {
        auto* reloc = reinterpret_cast<IMAGE_BASE_RELOCATION*>(
            base + pNT->OptionalHeader.DataDirectory[
                IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);

        while (reloc->VirtualAddress)
        {
            UINT count = (reloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / 2;
            WORD* list = reinterpret_cast<WORD*>(reloc + 1);
            for (UINT i = 0; i < count; ++i)
            {
                if ((list[i] >> 12) == IMAGE_REL_BASED_DIR64)
                {
                    uintptr_t* patch = reinterpret_cast<uintptr_t*>(
                        base + reloc->VirtualAddress + (list[i] & 0xFFF));
                    *patch += delta;
                }
            }
            reloc = reinterpret_cast<IMAGE_BASE_RELOCATION*>(
                reinterpret_cast<BYTE*>(reloc) + reloc->SizeOfBlock);
        }
    }

    // Resolve imports
    auto* importDir = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(
        base + pNT->OptionalHeader.DataDirectory[
            IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    auto fnLoadLib  = reinterpret_cast<decltype(&LoadLibraryA)>(pData->fnLoadLibraryA);
    auto fnGetProc  = reinterpret_cast<decltype(&GetProcAddress)>(pData->fnGetProcAddress);

    for (; importDir->Name; ++importDir)
    {
        HMODULE hLib = fnLoadLib(reinterpret_cast<char*>(base + importDir->Name));
        auto* thunk      = reinterpret_cast<IMAGE_THUNK_DATA*>(
                               base + importDir->FirstThunk);
        auto* origThunk  = reinterpret_cast<IMAGE_THUNK_DATA*>(
                               base + importDir->OriginalFirstThunk);

        for (; origThunk->u1.AddressOfData; ++thunk, ++origThunk)
        {
            if (IMAGE_SNAP_BY_ORDINAL(origThunk->u1.Ordinal))
            {
                thunk->u1.Function = reinterpret_cast<uintptr_t>(
                    fnGetProc(hLib, reinterpret_cast<char*>(
                        IMAGE_ORDINAL(origThunk->u1.Ordinal))));
            }
            else
            {
                auto* import = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(
                    base + origThunk->u1.AddressOfData);
                thunk->u1.Function = reinterpret_cast<uintptr_t>(
                    fnGetProc(hLib, import->Name));
            }
        }
    }

    // Call DllMain
    if (pNT->OptionalHeader.AddressOfEntryPoint)
    {
        auto dllMain = reinterpret_cast<BOOL(WINAPI*)(HINSTANCE, DWORD, LPVOID)>(
            base + pNT->OptionalHeader.AddressOfEntryPoint);
        dllMain(pData->hMod, DLL_PROCESS_ATTACH, nullptr);
    }
}

bool ManualMap(HANDLE hProc, const char* dllPath)
{
    // Read DLL from disk
    std::ifstream file(dllPath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return false;

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<BYTE> buf(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char*>(buf.data()), size)) return false;

    auto* dosHdr = reinterpret_cast<IMAGE_DOS_HEADER*>(buf.data());
    auto* ntHdr  = reinterpret_cast<IMAGE_NT_HEADERS*>(
                       buf.data() + dosHdr->e_lfanew);

    // Allocate memory in target process
    BYTE* targetBase = reinterpret_cast<BYTE*>(
        VirtualAllocEx(hProc, nullptr,
            ntHdr->OptionalHeader.SizeOfImage,
            MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
    if (!targetBase) return false;

    // Write PE headers
    WriteProcessMemory(hProc, targetBase, buf.data(),
        ntHdr->OptionalHeader.SizeOfHeaders, nullptr);

    // Write sections
    auto* section = IMAGE_FIRST_SECTION(ntHdr);
    for (WORD i = 0; i < ntHdr->FileHeader.NumberOfSections; ++i, ++section)
    {
        if (section->SizeOfRawData)
        {
            WriteProcessMemory(hProc,
                targetBase + section->VirtualAddress,
                buf.data() + section->PointerToRawData,
                section->SizeOfRawData, nullptr);
        }
    }

    // Allocate and write shellcode data
    ManualMapData mapData{};
    mapData.hMod           = reinterpret_cast<HINSTANCE>(targetBase);
    mapData.fnLoadLibraryA = reinterpret_cast<FARPROC>(GetProcAddress(
                                 GetModuleHandleA("kernel32.dll"), "LoadLibraryA"));
    mapData.fnGetProcAddress = reinterpret_cast<FARPROC>(GetProcAddress(
                                   GetModuleHandleA("kernel32.dll"), "GetProcAddress"));

    void* pData = VirtualAllocEx(hProc, nullptr, sizeof(ManualMapData),
                                 MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    WriteProcessMemory(hProc, pData, &mapData, sizeof(ManualMapData), nullptr);

    void* pShell = VirtualAllocEx(hProc, nullptr, 0x1000,
                                  MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    WriteProcessMemory(hProc, pShell, ShellCode, 0x1000, nullptr);

    // Execute shellcode in target
    HANDLE hThread = CreateRemoteThread(hProc, nullptr, 0,
        reinterpret_cast<LPTHREAD_START_ROUTINE>(pShell), pData, 0, nullptr);
    if (hThread)
    {
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
    }

    VirtualFreeEx(hProc, pData,  0, MEM_RELEASE);
    VirtualFreeEx(hProc, pShell, 0, MEM_RELEASE);
    return true;
}
