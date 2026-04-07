#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>


#define WIN32_LEAN_AND_MEAN
#include <Windows.h>


void* ProgramExit();

void* ChildProcTerminator(HANDLE hProcess, HANDLE hThread);




void* ChildProcTerminator(HANDLE hProcess, HANDLE hThread) {

	TerminateProcess(hProcess, 0);
	CloseHandle(hProcess);
	CloseHandle(hThread);


	return NULL;
}



int main(int argc, char* argv[]) {

	// need to do malloc in the first place
	char ProcessName[256] = "C:\\Users\\ACER\\Desktop\\devm\\APC_DLL_injectoni\\x64\\Release\\APC_DLL_injectoni.exe";

	char* pProcessName = &ProcessName;

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	

	if(!CreateProcessA((LPCSTR)pProcessName,NULL,NULL,NULL,FALSE,CREATE_SUSPENDED|DETACHED_PROCESS,NULL,NULL,&si,&pi)){
	
		printf("Failed to create the process with error 0x%x\n", GetLastError());
		ProgramExit();
	

	}




// starting to load the dll into it
	LPVOID lpAllocatedAddress = VirtualAllocEx(pi.hProcess, NULL, 256, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);


if (lpAllocatedAddress == NULL) {

	printf("Virtual Allocation failed with error 0x%x\n", GetLastError());


// need to look here if the terminate process is enough or i should also close handle 
	ChildProcTerminator(pi.hProcess, pi.hThread);



	ProgramExit();

}

size_t noofbyteswritten = 0; 
if (!WriteProcessMemory(pi.hProcess, lpAllocatedAddress, "pathdll\x0", 12, &noofbyteswritten)) {


	printf("write process memroy failed with error no 0x%x\n", GetLastError());
	
	ChildProcTerminator(pi.hProcess, pi.hThread);
	ProgramExit();


 }







// resuming the suspended process main thread
	ResumeThread(pi.hThread);




// waiting for the child process to exit then closing it's process handle and thread handle
	WaitForSingleObject(pi.hProcess, INFINITE);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);


	
	ProgramExit();






	
	



}






void* ProgramExit() {


	printf("press anything to exit: ");
	char c = 'a';
	scanf("%c", &c);
	exit(0);

}

