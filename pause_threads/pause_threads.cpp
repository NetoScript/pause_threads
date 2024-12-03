#include <windows.h>
#include <iostream>
#include <WinBase.h>
#include <TlHelp32.h>
#include <tchar.h>
#include <cstring>
#include <string>

bool Suspended = false;
bool Found = false;
BOOL SuspendProcess(DWORD ProcessId, bool Suspend);
DWORD FindProcessId(const std::wstring& processName);

void ResumeAllThreads(DWORD ProcessId);
MODULEENTRY32 GetModule(const char* ModuleName, unsigned long long ProcessID);
DWORD ProcPID;


int valorDelay = 100;

using namespace std;
bool inithh = true;
bool inithh2 = true;
int main() {
	string procName;
	int valorDelay;
	DWORD ProcPID = 0;
	bool inithh = true;

	
	cout << "[+] Nome do processo (exemplo: Gta5.exe): ";
	cin >> ws; 
	getline(cin, procName);

	cout << "[+] Delay para Sleep (em milissegundos): ";
	cin >> valorDelay;

	
	wstring wProcName(procName.begin(), procName.end());

	for (;;) {
		ProcPID = FindProcessId(wProcName);

		Sleep(valorDelay);

		if (ProcPID == 0) {
			cout << "[!] Aguardando o processo: " << procName << endl;
		}
		else {
			cout << "[+] Processo encontrado! PID: " << ProcPID << endl;
			break;
		}
	}

	cout << "[+] Suspendendo o processo..." << endl;
	SuspendProcess(ProcPID, true);

	cout << "[+] O processo está agora suspenso!" << endl;
	cout << "[+] Prossiga com o Debugging." << endl;
	Sleep(1000);

	while (inithh) {
		cout << "[!] Pressione F1 para resumir o processo..." << endl;
		Sleep(500);
		system("cls");
		if (GetAsyncKeyState(VK_F1) & 1) {
			ResumeAllThreads(ProcPID);
			inithh = false;
		}
	}

	return 0;
}

BOOL SuspendProcess(DWORD ProcessId, bool Suspend)
{
	HANDLE snHandle = NULL;
	BOOL rvBool = FALSE;
	THREADENTRY32 te32 = { 0 };

	snHandle = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	
	if (snHandle == INVALID_HANDLE_VALUE) return (FALSE);

	te32.dwSize = sizeof(THREADENTRY32);
	if (Thread32First(snHandle, &te32))
	{
		do
		{
			if (te32.th32OwnerProcessID == ProcessId)
			{
				HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
				cout << " at " << te32.th32OwnerProcessID << " (PID: " << te32.th32ThreadID << ")" << endl;
				if (Suspend == false)
				{
					if (Suspended)ResumeThread(hThread);
				}
				else
				{
					SuspendThread(hThread);
					Suspended = true;
				}

				CloseHandle(hThread);
			}
		} while (Thread32Next(snHandle, &te32));
		rvBool = TRUE;
	}
	else
		rvBool = FALSE;
	CloseHandle(snHandle);
	return (rvBool);
}


void ResumeAllThreads(DWORD ProcessId)
{
	
	HANDLE hCurProc = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

	if (hCurProc != INVALID_HANDLE_VALUE)
	{
		THREADENTRY32 te = { 0 };
		te.dwSize = sizeof(te);

		if (Thread32First(hCurProc, &te))
		{
			do
			{
				if (te.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(te.th32OwnerProcessID))
				{
					if (te.th32OwnerProcessID == ProcessId)
					{
						HANDLE thread = ::OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);

						if (thread != NULL)
						{
							ResumeThread(thread);
							CloseHandle(thread);
						}
					}
				}

				te.dwSize = sizeof(te);
			} while (Thread32Next(hCurProc, &te));
		}

		CloseHandle(hCurProc);
	}
}

DWORD FindProcessId(const std::wstring& processName)
{
	PROCESSENTRY32 processInfo;
	processInfo.dwSize = sizeof(processInfo);

	HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (processesSnapshot == INVALID_HANDLE_VALUE)
		return 0;

	Process32First(processesSnapshot, &processInfo);
	if (!processName.compare(processInfo.szExeFile))
	{
		CloseHandle(processesSnapshot);
		return processInfo.th32ProcessID;
	}

	while (Process32Next(processesSnapshot, &processInfo))
	{
		if (!processName.compare(processInfo.szExeFile))
		{
			CloseHandle(processesSnapshot);
			return processInfo.th32ProcessID;
		}
	}

	CloseHandle(processesSnapshot);
	return 0;
}

MODULEENTRY32 GetModule(const char* ModuleName, unsigned long long ProcessID) {
	HANDLE ModuleSnapshot = INVALID_HANDLE_VALUE;
	MODULEENTRY32 ModuleEntry;

	ModuleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, ProcessID);
	if (ModuleSnapshot == INVALID_HANDLE_VALUE) { cout << "Failed while snapshotting modules. 4" << endl; return ModuleEntry; system("pause"); }
	ModuleEntry.dwSize = sizeof(MODULEENTRY32);
	cout << "Sanning for modules on PID " << ProcessID << "..." << endl;

	Module32First(ModuleSnapshot, &ModuleEntry);
	while (true) {
		cout << "Found module: \"" << ModuleEntry.szModule << "\" at " << ModuleEntry.th32ModuleID << " (PID: " << ModuleEntry.th32ProcessID << ")" << endl;
		if (!(ModuleEntry.szModule, ModuleName) && ModuleEntry.th32ModuleID == 1) { break; }
		Module32Next(ModuleSnapshot, &ModuleEntry);
	}

	CloseHandle(ModuleSnapshot);
	return ModuleEntry;
}
