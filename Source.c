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
	//char ProcessName[256] = "C:\\Users\\ACER\\Desktop\\devm\\APC_DLL_injectoni\\x64\\Release\\APC_DLL_injectoni.exe";
	char ProcessName[256] = "C:\\Users\\ACER\\Desktop\\devm\\calc_shellcode_via_virtual_allocation\\x64\\Debug\\calc_shellcode_via_virtual_allocation.exe";

	char* pProcessName = &ProcessName;

	char DllPath[200] = "C:\\Users\\ACER\\Desktop\\simpledll.dll";


	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	


// need to look at the creaprocess's dwcreationflags, which might be suitable
	if(!CreateProcessA((LPCSTR)pProcessName,NULL,NULL,NULL,FALSE,CREATE_NEW_PROCESS_GROUP|DETACHED_PROCESS|CREATE_SUSPENDED,NULL,NULL,&si,&pi)){
	
		printf("Failed to create the process with error 0x%x\n", GetLastError());
		ProgramExit();
	

	}

	


// starting to load the dll into it
	LPVOID lpAllocatedAddress = VirtualAllocEx(pi.hProcess, NULL, 256, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);


if (lpAllocatedAddress == NULL) {

	printf("Virtual Allocation failed with error 0x%x\n", GetLastError());


// need to look here if the terminate process is enough or i should also close handle 
	ChildProcTerminator(pi.hProcess, pi.hThread);



	ProgramExit();

}

printf("allocated at ----->   0x%p\n", lpAllocatedAddress);


size_t noofbyteswritten = 0; 
if (!WriteProcessMemory(pi.hProcess, lpAllocatedAddress, DllPath, sizeof(DllPath), &noofbyteswritten)) {


	printf("write process memroy failed with error no 0x%x\n", GetLastError());
	
	ChildProcTerminator(pi.hProcess, pi.hThread);
	ProgramExit();


 }


printf("the numofbytes written -----> %ld\n", noofbyteswritten);


SECURITY_ATTRIBUTES sb; 

ZeroMemory(&sb, sizeof(sb));
sb.nLength = sizeof(sb);


HANDLE hmodulehandle = GetModuleHandle(L"kernel32.dll");
if (hmodulehandle == 0) {
	printf("gethandle failed with error 0x%x\n", GetLastError());
	ChildProcTerminator(pi.hProcess, pi.hThread);
	ProgramExit();
}



LPVOID lpProcAddress = GetProcAddress(hmodulehandle, "LoadLibraryA");

if (lpProcAddress == NULL) {


	printf("procaddress failed with error 0x%x\n", GetLastError());
	ChildProcTerminator(pi.hProcess, pi.hThread);

	ProgramExit();
}
 


////
HANDLE hthreadremote = CreateRemoteThread(pi.hProcess, &sb, 0,(LPTHREAD_START_ROUTINE) lpProcAddress, lpAllocatedAddress,CREATE_SUSPENDED, 0);

if (hthreadremote == NULL) {
	printf("Failed to create remote thread with error 0x%x\n", GetLastError());
	ChildProcTerminator(pi.hProcess, pi.hThread);
	ProgramExit(0);
	

}

ResumeThread(hthreadremote);


WaitForSingleObject(hthreadremote, INFINITE);





// resuming the suspended process main thread
	ResumeThread(pi.hThread);




// waiting for the child process to exit then closing it's process handle and thread handle
	WaitForSingleObject(pi.hProcess, INFINITE);

	


	ChildProcTerminator(pi.hProcess, pi.hThread);


	
	ProgramExit();






	
	



}






void* ProgramExit() {


	printf("press anything to exityyyyyyyyyyyyy: ");
	char c = 'a';
	scanf("%c", &c);
	exit(0);

}

