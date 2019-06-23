#pragma once

#include <Windows.h>
#include <WinSock2.h>

#include <unordered_map>
#include <set>
#include <string>
#include <unordered_map>


namespace
{
	struct CIStringComp
	{
		using is_transparent = char*;

		bool operator()(const std::string& a_lhs, const std::string& a_rhs) const
		{
			return _stricmp(a_lhs.c_str(), a_rhs.c_str()) < 0;
		}

		bool operator()(const std::string& a_lhs, const is_transparent& a_rhs) const
		{
			return _stricmp(a_lhs.c_str(), a_rhs) < 0;
		}

		bool operator()(const is_transparent& a_lhs, const std::string& a_rhs) const
		{
			return _stricmp(a_lhs, a_rhs.c_str()) < 0;
		}
	};
}


class WinAPICacher
{
public:
	static void InstallHooks();
	static bool ExistsInCurDir(const char* a_file);

	static HANDLE Hook_FindFirstFileA(LPCSTR a_fileName, LPWIN32_FIND_DATAA a_findFileData);
	static BOOL Hook_FindNextFileA(HANDLE a_findFile, LPWIN32_FIND_DATAA a_findFileData);
	static BOOL Hook_FindClose(HANDLE a_findFile);
	static DWORD Hook_GetCurrentDirectoryA(DWORD a_bufferLength, LPTSTR a_buffer);
	static DWORD Hook_GetFileAttributesA(LPCSTR a_fileName);
	static BOOL Hook_GetFileAttributesExA(LPCSTR a_fileName, GET_FILEEX_INFO_LEVELS a_infoLevelID, LPVOID a_fileInformation);
	static HANDLE Hook_CreateFileA(LPCSTR a_fileName, DWORD a_desiredAccess, DWORD a_shareMode, LPSECURITY_ATTRIBUTES a_securityAttributes, DWORD a_creationDisposition, DWORD a_flagsAndAttributes, HANDLE a_templateFile);

private:
	WinAPICacher() = delete;
	WinAPICacher(const WinAPICacher&) = delete;
	WinAPICacher(WinAPICacher&&) = delete;
	~WinAPICacher() = delete;

	WinAPICacher& operator=(const WinAPICacher&) = delete;
	WinAPICacher& operator=(WinAPICacher&&) = delete;

	static void ParseCurDir();


	static std::string _curDir;
	static std::set<std::string, CIStringComp> _curDirMap;
	static std::unordered_map<HANDLE, std::string> _handleMap;
	static std::unordered_map<std::string, WIN32_FILE_ATTRIBUTE_DATA> _attributeMap;
};
