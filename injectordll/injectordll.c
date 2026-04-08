#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>


#include <tchar.h>

#include <psapi.h>



TCHAR Namesofmodules[MAX_PATH * 1024] = { 0 };

int PrintModules(DWORD processID)
{

    FILE* fptr;
    HMODULE hMods[1024];
    HANDLE hProcess;
    DWORD cbNeeded;
    unsigned int i;


    // Print the process identifier.

    printf("\nProcess ID: %u\n", processID);

    // Get a handle to the process.

    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
        PROCESS_VM_READ,
        FALSE, processID);
    if (NULL == hProcess) {
        MessageBoxA(NULL, "failed", "whatisthis", MB_OK);

        return 1;

    }
    // Get a list of all the modules in this process.

    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
    {
        

        for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
        {
            TCHAR szModName[MAX_PATH];
            //TCHAR tempszModName[MAX_PATH] = { 0 };



            // Get the full path to the module's file.

            if (GetModuleFileNameA( hMods[i], szModName,
                sizeof(szModName) / sizeof(TCHAR)))
            {

                

                //fptr = fopen("modules.txt", "a");
                //if (fptr == NULL) {
                //   
                //    exit(EXIT_FAILURE);
                //}

                // Append data to file
                //fprintf(fptr, "%s\n", szModName);
               

                //fclose(fptr);
               



                //_tprintf(TEXT("\t%ls (0x%08X)\n"), szModName, hMods[i]);
                printf("%s\n", szModName);
            }


        }
    }

    // Release the handle to the process.


    CloseHandle(hProcess);

    return 0;
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DWORD pid = GetCurrentProcessId(); 
        char msg[256];                     


        FreeConsole();

        //AllocConsole();

        // attaching parent process pid
        if (!(AttachConsole(ATTACH_PARENT_PROCESS))) {
            MessageBoxA(NULL, "failed", "whatisthis", MB_OK);

        }


        // if any problem occurs then it can be use to attach the console to it's pid
       /* if (!(AttachConsole(pid))) {
            MessageBoxA(NULL, "failed", "whatisthis", MB_OK);

        }*/

        printf("is it a parent console ?");
        
        sprintf(msg, "DLL Loaded!\nPID ----> %lu", (unsigned long)pid);



        

            DWORD aProcesses[1024];
            DWORD cbNeeded;
            DWORD cProcesses;
            unsigned int i;

        

            PrintModules(pid);

           /* Sleep(10000);*/
            return 0;
        

       





       
    }
    return TRUE;
}


