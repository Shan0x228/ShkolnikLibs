#pragma once

#include <Windows.h>
#include <TlHelp32.h>
#include <shellapi.h>

#include <string>
#include <vector>
#include <utility>
#include <algorithm>

// Root namespace
namespace Shkolnik
{
	// Library type namespace
	namespace System
	{
		// Library class declaration
		class Processes
		{
		// Public section with methods
		public:
			// Default CTOR
			Processes();

			/*-------------------------GET-------------------------*/
			// Public method return vector pID's
			static std::vector<int> GetProcessIdByName(const std::wstring& ProcName);

			// Public method return vector with all processes
			static std::vector<std::wstring> GetProcessList();

			// Public method return sorted vector with all processes and their id (pID)
			static std::vector<std::pair<std::wstring, int>> GetProcessListWithPID();
			/*-----------------------------------------------------*/


			/*-------------------------KILL------------------------*/
			// Public method kill process by pID
			static bool KillProcess(const int& pID);

			// Public method kill process by image-name
			static bool KillProcess(const std::wstring& ProcName);

			// Public method kill all processes in vector
			static bool KillProcess(const std::vector<std::pair<std::wstring, int>>& process_list);

			// Public method kill all processes in vector with pID
			static bool KillProcess(const std::vector<int>& process_pid_list);
			/*-----------------------------------------------------*/


			/*-------------------------RUN-------------------------*/
			// Public method run processes
			static bool RunProcess(const std::wstring& ProcName);

			// Public method run vector of processes
			static bool RunProcess(const std::vector<std::wstring>& process_list);
			/*-----------------------------------------------------*/


			/*-----------------------STOP/START--------------------*/
			// Public method stops process
			static bool StopProcess(const int& pID);

			// Public method stops process by vector of pID's
			static bool StopProcess(const std::vector<int>& process_pid_list);

			// Public method resume process
			static bool ResumeProcess(const int& pID);

			// Public method resume processes by pID
			static bool ResumeProcess(const std::vector<int>& process_pid_list);
			/*-----------------------------------------------------*/

			// DTOR
			~Processes();
		};
	}
}

// Default CTOR
Shkolnik::System::Processes::Processes()
{
}

// Public method return vector pID's
inline std::vector<int> Shkolnik::System::Processes::GetProcessIdByName(const std::wstring& ProcName)
{
	std::vector<int> result{};								// vector with pID's
	HANDLE hSnap{};											// variable with snapshot handle
	PROCESSENTRY32W pe32{ sizeof(PROCESSENTRY32W) };		// structure with process information

	// trying to create snapshot...
	if (hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0); !hSnap)
		return result;

	// trying to get 1st process
	if (!Process32FirstW(hSnap, &pe32))
	{
		// closing handle
		CloseHandle(hSnap);
		return result;
	}
	do
	{
		if (pe32.szExeFile == ProcName)
			// pushing pe-file-name to vector
			result.push_back(pe32.th32ProcessID);
	} while (Process32NextW(hSnap, &pe32));

	// closing handle
	CloseHandle(hSnap);
	return result;
}

// Public method return vector with all processes
inline std::vector<std::wstring> Shkolnik::System::Processes::GetProcessList()
{
	std::vector<std::wstring> result{};						// vector with processes
	HANDLE hSnap{};											// variable with snapshot handle
	PROCESSENTRY32W pe32{ sizeof(PROCESSENTRY32W) };		// structure with process information

	// trying to create snapshot...
	if (hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0); !hSnap)
		return result;

	// trying to get 1st process
	if (!Process32FirstW(hSnap, &pe32))
	{
		// closing handle
		CloseHandle(hSnap);
		return result;
	}
	do
	{
		// pushing pe-file-name to vector
		result.push_back(pe32.szExeFile);
	} while (Process32NextW(hSnap, &pe32));

	// closing handle
	CloseHandle(hSnap);
	return result;
}

// Public method return sorted vector with all processes and their id (pID)
inline std::vector<std::pair<std::wstring, int>> Shkolnik::System::Processes::GetProcessListWithPID()
{
	std::vector<std::pair<std::wstring, int>> result{};		// vector contains processes with pID
	HANDLE hSnap{};											// variable with snapshot handle
	PROCESSENTRY32W pe32{ sizeof(PROCESSENTRY32W) };		// structure with process information

	// trying to create snapshot...
	if (hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0); !hSnap)
		return result;

	// trying to get 1st process
	if (!Process32FirstW(hSnap, &pe32))
	{
		// closing handle
		CloseHandle(hSnap);
		return result;
	}
	do
	{
		// pushing pe-file-name to vector
		result.push_back({pe32.szExeFile, pe32.th32ProcessID});
	} while (Process32NextW(hSnap, &pe32));

	// closing handle
	CloseHandle(hSnap);
	
	// sorting result
	std::sort(std::begin(result), std::end(result), [](auto& left, auto& right) {
		return left.second < right.second;
	});

	return result;
}

// Public method kill process by pID
inline bool Shkolnik::System::Processes::KillProcess(const int & pID)
{
	HANDLE hProcess{};					// variable with process handle
	// trying to open process...
	if (hProcess = OpenProcess(PROCESS_TERMINATE, false, static_cast<DWORD>(pID)); !hProcess)
		return false;

	// trying to terminate process
	if (!TerminateProcess(hProcess, 0))
	{
		// closing handle
		CloseHandle(hProcess);
		return false;
	}

	// closing handle
	CloseHandle(hProcess);

	return true;
}

// Public method kill process by image-name
inline bool Shkolnik::System::Processes::KillProcess(const std::wstring & ProcName)
{
	// getting process list with pID
	std::vector<std::pair<std::wstring, int>> process_list { Processes::GetProcessListWithPID() };

	// sorting process list(deleting)
	process_list.erase(
	std::remove_if(std::begin(process_list), std::end(process_list), [&ProcName](const auto& process) {
		return process.first != ProcName;
	}), std::end(process_list));
	
	// check founded processes
	if (process_list.size() == 0)
		return false;

	// killing them!
	for (const auto&[process_name, process_id] : process_list)
		if (!Processes::KillProcess(process_id))
			return false;

	return true;
}

// Public method kill all processes in vector
inline bool Shkolnik::System::Processes::KillProcess(const std::vector<std::pair<std::wstring, int>> & process_list)
{
	// killing them!
	for (const auto&[process_name, process_id] : process_list)
		if (!Processes::KillProcess(process_id))
			return false;

	return true;
}

// Public method kill all processes in vector with pID
inline bool Shkolnik::System::Processes::KillProcess(const std::vector<int> & process_pid_list)
{
	// foreach...
	for (const auto &pID : process_pid_list)
		if (!Processes::KillProcess(pID))
			return false;

	return true;
}

// Public method run processes
inline bool Shkolnik::System::Processes::RunProcess(const std::wstring & ProcName)
{
	// trying to execute process using shellapi
	if (HINSTANCE hInst = ShellExecuteW(nullptr, nullptr, ProcName.c_str(), nullptr, nullptr, SW_SHOWNORMAL); hInst == reinterpret_cast<HINSTANCE>(ERROR_FILE_NOT_FOUND))
		return false;

	return true;
}

// Public method run vector of processes
inline bool Shkolnik::System::Processes::RunProcess(const std::vector<std::wstring> & process_list)
{
	// trying to execute vector of processes using shellapi
	for (const auto& process_name : process_list)
		if (HINSTANCE hInst = ShellExecuteW(nullptr, nullptr, process_name.c_str(), nullptr, nullptr, SW_SHOWNORMAL); hInst == reinterpret_cast<HINSTANCE>(ERROR_FILE_NOT_FOUND))
			return false;

	return true;
}

// Public method stops process(main thread)
inline bool Shkolnik::System::Processes::StopProcess(const int & pID)
{
	HANDLE hSnap{};									// variable contains snapshot handle
	THREADENTRY32 th32{sizeof(THREADENTRY32)};		// native thread structure

	// trying to create threads snapshots and obtaining handle
	if (hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0); !hSnap)
		return false;

	// trying to obtain first thread and fill native structure
	if (!Thread32First(hSnap, &th32))
	{
		// closing handle
		CloseHandle(hSnap);
		return false;
	}
	do
	{
		// check pID from native structure with method argument
		if (th32.th32OwnerProcessID == pID)
		{
			// obtaining thread handle
			HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, false, th32.th32ThreadID);
			
			// suspending thread
			SuspendThread(hThread);

			// closing thread handle
			CloseHandle(hThread);
		}
	} while (Thread32Next(hSnap, &th32));

	// closing snapthot handle
	CloseHandle(hSnap);
	return true;
}

// Public method stops process by vector of pID's
inline bool Shkolnik::System::Processes::StopProcess(const std::vector<int>& process_pid_list)
{
	// foreach...
	for (const auto &pID : process_pid_list)
		if (!Processes::StopProcess(pID))
			return false;

	return true;
}

// Public method resume process
inline bool Shkolnik::System::Processes::ResumeProcess(const int & pID)
{
	HANDLE hSnap{};										// variable contains snapshot handle
	THREADENTRY32 th32{ sizeof(THREADENTRY32) };		// native thread structure

	// trying to create threads snapshots and obtaining handle
	if (hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0); !hSnap)
		return false;

	// trying to obtain first thread and fill native structure
	if (!Thread32First(hSnap, &th32))
	{
		// closing handle
		CloseHandle(hSnap);
		return false;
	}
	do
	{
		// check pID from native structure with method argument
		if (th32.th32OwnerProcessID == pID)
		{
			// obtaining thread handle
			HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, false, th32.th32ThreadID);

			// resuming thread
			ResumeThread(hThread);

			// closing thread handle
			CloseHandle(hThread);
		}
	} while (Thread32Next(hSnap, &th32));

	// closing snapshot handle
	CloseHandle(hSnap);
	return true;
}

// Public method resume processes by pID
inline bool Shkolnik::System::Processes::ResumeProcess(const std::vector<int>& process_pid_list)
{
	// foreach...
	for (const auto &pID : process_pid_list)
		if (!Processes::ResumeProcess(pID))
			return false;

	return true;
}

// DTOR
Shkolnik::System::Processes::~Processes()
{
}