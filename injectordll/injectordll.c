#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include <Shlwapi.h>
#include <tchar.h>

#include <psapi.h>

#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "Shlwapi.lib")

TCHAR Namesofmodules[MAX_PATH * 1024] = { 0 };

// hooking funciton
void* iamhook() {
    printf("hooked called psycosss\n");
    exit(0);
}

void* hooker(PBYTE FuncAddr, HANDLE hprocess, PBYTE LpPreviousBytes) {
    DWORD dwOldProtection = 0;
    SIZE_T bytesWritten = 0;

    
    BOOL vp = VirtualProtectEx(hprocess, FuncAddr, 16, PAGE_EXECUTE_READWRITE, &dwOldProtection);
    if (!vp) {
        printf("hooker VirtualProtectEx failed with error 0x%lx\n", GetLastError());
        return NULL;
    }

   
    BYTE OpCode[16] = { 0x49, 0xBA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x41, 0xFF, 0xE2, 0x90, 0x90, 0x90 };
    UINT64 upatch = (UINT64)iamhook;
    memcpy(&OpCode[2], &upatch, sizeof(upatch)); 

    if (!WriteProcessMemory(hprocess, FuncAddr, OpCode, 16, &bytesWritten) || bytesWritten != 16) {
        printf("hooker WriteProcessMemory failed with error 0x%lx (wrote %llu bytes)\n", GetLastError(), (unsigned long long)bytesWritten);
        
        DWORD dummy;
        VirtualProtectEx(hprocess, FuncAddr, 16, dwOldProtection, &dummy);
        return NULL;
    }

    // Ensure CPU sees the new code
    if (!FlushInstructionCache(hprocess, FuncAddr, 16)) {
        printf("FlushInstructionCache failed with error 0x%lx\n", GetLastError());
    }

    
    if (!VirtualProtectEx(hprocess, FuncAddr, 16, dwOldProtection, &dwOldProtection)) {
        printf("hooker VirtualProtectEx (restore) failed with error 0x%lx\n", GetLastError());
    }

    return NULL;
}

int PrintModules(DWORD processID)
{
    FILE* fptr;
    HMODULE hMods[1024];
    HANDLE hProcess;
    DWORD cbNeeded;
    unsigned int i;

    // Print the process identifier.
    printf("\nProcess ID: %u\n", processID);

  
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
        PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION,
        FALSE, processID);
    if (NULL == hProcess) {
        MessageBoxA(NULL, "OpenProcess failed", "whatisthis", MB_OK);
        return 1;
    }

    // Get a list of all the modules in this process.
    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
    {
        for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
        {
            TCHAR szModName[MAX_PATH];
            TCHAR SlicedszModname[MAX_PATH] = { 0 };

            // Get the full path to the module's file.
            if (GetModuleFileNameEx(hProcess, hMods[i], szModName,
                sizeof(szModName) / sizeof(TCHAR)))
            {
                // slicing only the dll name
                StrCpyW(&SlicedszModname, (PCWSTR)PathFindFileNameW(&szModName));
                _tprintf(_T("%s\t\t"), SlicedszModname);

                HMODULE hmodule = GetModuleHandleW(SlicedszModname);

                if (NULL == hmodule) {
                    printf("couldn't get the module handle with error 0x%lx\n", GetLastError());
                    // Continue, maybe the module is not loaded in the current module list
                    continue;
                }

                printf("handle ----> %p\n", hmodule);

                PBYTE pBase = (PBYTE)hmodule;
                // now getting the export directory
                PIMAGE_DOS_HEADER pImgDosHdr = (PIMAGE_DOS_HEADER)pBase;

                if (pImgDosHdr->e_magic != IMAGE_DOS_SIGNATURE) {
                    printf("Invalid DOS signature\n");
                    return -1;
                }

                PIMAGE_NT_HEADERS pImgNtHdrs = (PIMAGE_NT_HEADERS)(pBase + pImgDosHdr->e_lfanew);

                if (pImgNtHdrs->Signature != IMAGE_NT_SIGNATURE) {
                    printf("Invalid NT signature\n");
                    return -1;
                }

                IMAGE_OPTIONAL_HEADER ImgoptHdr = pImgNtHdrs->OptionalHeader;

                if (ImgoptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress == 0) {
                    // no exports
                } else {
                    PIMAGE_EXPORT_DIRECTORY pImgExpDir = (PIMAGE_EXPORT_DIRECTORY)(pBase + ImgoptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
                    PDWORD FunctionRVAArray = (PDWORD)(pBase + pImgExpDir->AddressOfFunctions);   // RVAs for functions 
                    PDWORD NameRVAArray = (PDWORD)(pBase + pImgExpDir->AddressOfNames);           // RVAs for names
                    PWORD  OrdinalArray   = (PWORD )(pBase + pImgExpDir->AddressOfNameOrdinals);  // ordinals

                    
                    for (DWORD k = 0; k < pImgExpDir->NumberOfNames; ++k) {
                        CHAR* pFunctionName = (CHAR*)(pBase + NameRVAArray[k]); [i]

                        DWORD ordinal = OrdinalArray[k];
                        DWORD funcRVA = FunctionRVAArray[ordinal];
                        PBYTE funcAddr = pBase + funcRVA;

                        printf("name: %s\n", pFunctionName);
                        printf("ordinal: %d\n", ordinal);
                        printf("RVA: %lx\n", funcRVA);
                        printf("function addr: 0x%p\n", funcAddr);

                        SIZE_T readbytes = 0;
                        PBYTE lpPreviousBytes = (PBYTE)calloc(16, sizeof(BYTE));
                        if (!lpPreviousBytes) {
                            printf("calloc failed\n");
                            continue;
                        }

                        if (!ReadProcessMemory(hProcess, funcAddr, (LPVOID)lpPreviousBytes, 16, &readbytes) || readbytes != 16) {
                            printf("Reading the process memory failed with error no 0x%lx (read %llu bytes)\n", GetLastError(), (unsigned long long)readbytes);
                            free(lpPreviousBytes);
                            continue;
                        } else {
                            printf("previous bytes ----> 0x");
                            for (BYTE j = 0; j < 16;  j++) {
                                printf("%02x", lpPreviousBytes[j]);
                            }
                            printf("\n\n\n");

                            // create the hooking function
                            hooker(funcAddr, hProcess, lpPreviousBytes);

                            free(lpPreviousBytes);
                        }
                    }
                }
            }
        }
    }

   
    CloseHandle(hProcess);

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DWORD pid = GetCurrentProcessId();
        char msg[256];

        FreeConsole();

        if (!(AttachConsole(ATTACH_PARENT_PROCESS))) {
            MessageBoxA(NULL, "failed", "whatisthis", MB_OK);
        }

        printf("is it a parent console ?");
        sprintf(msg, "DLL Loaded!\nPID ----> %lu", (unsigned long)pid);

        PrintModules(pid);

        return 0;
    }
    return TRUE;
}



