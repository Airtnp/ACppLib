/******************************************************************************
A utility for running arbitrary command with administrator access on Windows
Use the following command to compile:
  gcc -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -pedantic-errors \
    sudo.c -o sudo.exe -O3 -nostdlib -lshlwapi -lshell32 -lkernel32 \
    -Wl,--entry,entrypoint,--subsystem,windows,--strip-all
Copyright (c) 2017, LH_Mouse
All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.
* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/

#include <windows.h>
#include <shlwapi.h>

DWORD entrypoint(void *pUnknown) __asm__("entrypoint");

DWORD entrypoint(void *pUnknown){
	static WCHAR awszBuffer[32768 + 256];
	STARTUPINFOW vStartupInfo;
	LPWSTR pwszCmdLine, pwszFile, pwszArgs;
	DWORD dwWorkingDirLen;
	SHELLEXECUTEINFOW vShellExecInfo;
	DWORD dwProcessExitCode;
	(void)pUnknown;

	/* Get the `wShowWindow` parameter. */
	GetStartupInfoW(&vStartupInfo);

	pwszCmdLine = GetCommandLineW();
	pwszFile = PathGetArgsW(pwszCmdLine);
	if(*pwszFile == 0){
		/* Run CMD if no command is given. An elevated `cmd.exe` has
		   its working directory set to `%SystemRoot%\System32\`, which is
		   a bit confusing. We tell CMD to switch to our working directory
		   upon its launch. */
		pwszFile = L"cmd.exe";
		pwszArgs = awszBuffer;
		CopyMemory(pwszArgs, L"/s /k pushd \"", 13 * sizeof(WCHAR));
		dwWorkingDirLen = GetCurrentDirectoryW(32768, pwszArgs + 13);
		CopyMemory(pwszArgs + 13 + dwWorkingDirLen, L"\"", 2 * sizeof(WCHAR));
	} else {
		/* Disassemble the command given. */
		pwszArgs = PathGetArgsW(pwszFile);
		StrCpyNW(awszBuffer, pwszArgs, 32768);
		pwszArgs = awszBuffer;
		PathRemoveArgsW(pwszFile);
	}
	/* Execute it with elevated access now. */
	vShellExecInfo.cbSize       = sizeof(vShellExecInfo);
	vShellExecInfo.fMask        = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_UNICODE;
	vShellExecInfo.hwnd         = NULL;
	vShellExecInfo.lpVerb       = L"runas";
	vShellExecInfo.lpFile       = pwszFile;
	vShellExecInfo.lpParameters = pwszArgs;
	vShellExecInfo.lpDirectory  = NULL;
	vShellExecInfo.nShow        = vStartupInfo.wShowWindow;
	if(!ShellExecuteExW(&vShellExecInfo)){
		/* Return its error code in case of failure. */
		dwProcessExitCode = GetLastError();
	} else if(!vShellExecInfo.hProcess){
		/* Assume success, should no process handle be returned. */
		dwProcessExitCode = 0;
	} else {
		/* Retrieve the exit code of the process created. */
		WaitForSingleObject(vShellExecInfo.hProcess, INFINITE);
		GetExitCodeProcess(vShellExecInfo.hProcess, &dwProcessExitCode);
		CloseHandle(vShellExecInfo.hProcess);
	}
	/* Forward the exit code. */
	ExitProcess(dwProcessExitCode);
}

#include <windows.h>
#include <shlwapi.h>

DWORD SudoEntryPoint(void *pUnknown) __asm__("@SudoEntryPoint");

static STARTUPINFOW vStartupInfo;
static LPCWSTR pwszCmdLine;
static WCHAR awchSomeBuffer[32768 + 256];

DWORD SudoEntryPoint(void *pUnknown){
	LPWSTR pwszFile, pwszArgs;
	SHELLEXECUTEINFOW vShellExecInfo;
	DWORD dwExitCode;
	(void)pUnknown;

	/* Perform global initialization. */
	GetStartupInfoW(&vStartupInfo);
	pwszCmdLine = GetCommandLineW();

	/* Obtain the command and arguments to run. */
	pwszFile = PathGetArgsW(pwszCmdLine);
	if(pwszFile[0] == 0){
		/* Run CMD if no filename is given, passing the working directory to
		   it. By default an elevated CMD has its working directory
		   set to `%SystemRoot%\System32\`, which is a bit confusing.
		   We tell CMD to switch to our working directory upon launch. */
		pwszArgs = awchSomeBuffer;
		CopyMemory(pwszArgs, L"/s /k pushd \"", 13 * sizeof(WCHAR));
		dwExitCode = GetCurrentDirectoryW(32768, pwszArgs + 13);
		CopyMemory(pwszArgs + 13 + dwExitCode, L"\"", 2 * sizeof(WCHAR));
		pwszFile = L"CMD.EXE";
	} else {
		/* Use the filename provided if any. */
		pwszArgs = PathGetArgsW(pwszFile);
		/* Trim the filename, leaving the arguments alone. */
		if(pwszArgs[0] != 0){
			pwszArgs[-1] = 0;  // Direcly truncate the filename
		}
		StrTrimW(pwszFile, L" \t");
	}
	/* Launch it with elevated access now. */
	vShellExecInfo.cbSize       = sizeof(vShellExecInfo);
	vShellExecInfo.fMask        = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_UNICODE;
	vShellExecInfo.hwnd         = NULL;
	vShellExecInfo.lpVerb       = L"runas";
	vShellExecInfo.lpFile       = pwszFile;
	vShellExecInfo.lpParameters = pwszArgs;
	vShellExecInfo.lpDirectory  = NULL;
	vShellExecInfo.nShow        = vStartupInfo.wShowWindow;
	if(!ShellExecuteExW(&vShellExecInfo)){
		/* Return its error code in case of failure. */
		dwExitCode = GetLastError();
	} else if(!vShellExecInfo.hProcess){
		/* Assume success, should no process handle be returned. */
		dwExitCode = 0;
	} else {
		/* Retrieve the exit code of the process created. */
		WaitForSingleObject(vShellExecInfo.hProcess, INFINITE);
		GetExitCodeProcess(vShellExecInfo.hProcess, &dwExitCode);
		CloseHandle(vShellExecInfo.hProcess);
	}
	/* Forward the exit code. */
	ExitProcess(dwExitCode);
}