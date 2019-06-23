#include "WinAPICacher.h"

#include "skse64_common/BranchTrampoline.h"
#include "skse64_common/SafeWrite.h"
#include "xbyak/xbyak.h"

#include <Windows.h>

#include <cassert>
#include <cstring>
#include <optional>
#include <string>

#include "REL/Relocation.h"


namespace
{
	template <class T>
	void WriteHook(std::uintptr_t a_offset, T* a_function)
	{
		REL::Offset<std::uintptr_t> target(a_offset);
		SafeWrite64(target.GetAddress(), unrestricted_cast<std::uintptr_t>(a_function));
	}


	void CopyAttributes(WIN32_FILE_ATTRIBUTE_DATA& a_lhs, const WIN32_FIND_DATAA& a_rhs)
	{
		std::memcpy(&a_lhs, &a_rhs, sizeof(a_lhs));
	}
}


void WinAPICacher::InstallHooks()
{
	_curDir.resize(GetCurrentDirectoryA(0, NULL) - 1);
	GetCurrentDirectoryA(_curDir.size() + 1, _curDir.data());

	ParseCurDir();

	WriteHook(0x01509208, Hook_FindFirstFileA);			// 1_5_80
	WriteHook(0x01509160, Hook_FindNextFileA);			// 1_5_80
	WriteHook(0x01509210, Hook_FindClose);				// 1_5_80
	WriteHook(0x01509260, Hook_GetCurrentDirectoryA);	// 1_5_80
	WriteHook(0x01509228, Hook_GetFileAttributesA);		// 1_5_80
	WriteHook(0x015093C8, Hook_GetFileAttributesExA);	// 1_5_80
	WriteHook(0x01509340, Hook_CreateFileA);			// 1_5_80

	// fix for parsing "data\DATA\myfile.esp" bug
	{
		// E8 ? ? ? ? EB 18 0F BA F1 1F
		constexpr std::uintptr_t BASE_ADDR = 0x0017A730;	// 1_5_80
		// E8 ? ? ? ? 85 C0 0F 85 ? ? ? ? 48 8B 4C 24 60
		constexpr std::uintptr_t CALL_ADDR = 0x00C44B90;	// 1_5_80

		REL::Offset<std::uintptr_t> baseFunc(BASE_ADDR + 0x1F0);
		REL::Offset<std::uintptr_t> callFunc(CALL_ADDR);

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(void* a_buf, std::size_t a_callAddr) : Xbyak::CodeGenerator(1024, a_buf)
			{
				Xbyak::Label callLbl;

				lea(rcx, ptr[rdi + 0x58]);
				jmp(ptr[rip + callLbl]);

				L(callLbl);
				dq(a_callAddr);
			}
		};

		auto patchBuf = g_localTrampoline.StartAlloc();
		Patch patch(patchBuf, callFunc.GetAddress());
		g_localTrampoline.EndAlloc(patch.getCurr());

		g_branchTrampoline.Write5Call(baseFunc.GetAddress(), reinterpret_cast<std::uintptr_t>(patch.getCode()));
	}

	// fix for pointless sanity check
	{
		// E8 ? ? ? ? E8 ? ? ? ? 48 8B 3D ? ? ? ?
		constexpr std::uintptr_t ADDR = 0x0016E660;	// 1_5_80
		constexpr UInt8 NOP = 0x90;

		REL::Offset<std::uintptr_t> target(ADDR);

		for (std::size_t i = 0x15E; i < 0x163; ++i) {
			SafeWrite8(target.GetAddress() + i, NOP);
		}
	}
}


bool WinAPICacher::ExistsInCurDir(const char* a_file)
{
	auto fileName = GetFirstFileName(a_file);
	if (fileName) {
		return _curDirMap.find(*fileName) != _curDirMap.end();
	} else {
		return false;
	}
}


HANDLE WinAPICacher::Hook_FindFirstFileA(LPCSTR a_fileName, LPWIN32_FIND_DATAA a_findFileData)
{
	_DMESSAGE("[FindFirstFileA] %s", a_fileName);
	auto result = FindFirstFileExA(a_fileName, FINDEX_INFO_LEVELS::FindExInfoBasic, a_findFileData, FINDEX_SEARCH_OPS::FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);

	std::string fileName(a_fileName);
	if (result != INVALID_HANDLE_VALUE) {
		while (!fileName.empty() && fileName.back() == '*') {
			fileName.pop_back();
		}
		while (!fileName.empty() && (fileName.back() == '\\' || fileName.back() == '/')) {
			fileName.pop_back();
		}
		_handleMap.insert({ result, fileName });
		WIN32_FILE_ATTRIBUTE_DATA attributeData;
		CopyAttributes(attributeData, *a_findFileData);
		_attributeMap.insert({ fileName, std::move(attributeData) });
	}

	return result;
}


BOOL WinAPICacher::Hook_FindNextFileA(HANDLE a_findFile, LPWIN32_FIND_DATAA a_findFileData)
{
	auto result = FindNextFileA(a_findFile, a_findFileData);

	if (result) {
		auto it = _handleMap.find(a_findFile);
		if (it != _handleMap.end()) {
			std::string fileName = it->second;
			fileName += '\\';
			fileName += a_findFileData->cFileName;
			WIN32_FILE_ATTRIBUTE_DATA attributeData;
			CopyAttributes(attributeData, *a_findFileData);
			_attributeMap.insert({ fileName, std::move(attributeData) });
			_DMESSAGE("[FindNextFileA] %s", fileName.c_str());
		}
	}

	return result;
}


BOOL WinAPICacher::Hook_FindClose(HANDLE a_findFile)
{
	_handleMap.erase(a_findFile);
	return FindClose(a_findFile);
}


DWORD WinAPICacher::Hook_GetCurrentDirectoryA(DWORD a_bufferLength, LPTSTR a_buffer)
{
	_DMESSAGE("[GetCurrentDirectoryA] %s", _curDir.c_str());
	auto result = GetCurrentDirectoryA(a_bufferLength, a_buffer);
	return result;
}


DWORD WinAPICacher::Hook_GetFileAttributesA(LPCSTR a_fileName)
{
	std::string fileName(a_fileName);
	auto it = _attributeMap.find(fileName);
	if (it != _attributeMap.end()) {
		return it->second.dwFileAttributes;
	} else {
		_DMESSAGE("[GetFileAttributesA] %s", a_fileName);
		auto result = GetFileAttributesA(a_fileName);
		return result;
	}
}


BOOL WinAPICacher::Hook_GetFileAttributesExA(LPCSTR a_fileName, GET_FILEEX_INFO_LEVELS a_infoLevelID, LPVOID a_fileInformation)
{
	std::string fileName(a_fileName);
	auto fileInformation = static_cast<WIN32_FILE_ATTRIBUTE_DATA*>(a_fileInformation);
	switch (a_infoLevelID) {
	case GET_FILEEX_INFO_LEVELS::GetFileExInfoStandard:
		{
			auto it = _attributeMap.find(fileName);
			if (it != _attributeMap.end()) {
				*fileInformation = it->second;
				return TRUE;
			}
		}
		[[fallthrough]] ;
	default:
		{
			_DMESSAGE("[GetFileAttributesExA] %s", a_fileName);
			auto result = GetFileAttributesExA(a_fileName, a_infoLevelID, a_fileInformation);
			_attributeMap.insert({ fileName, *fileInformation });
			return result;
		}
		break;
	}
}


HANDLE WinAPICacher::Hook_CreateFileA(LPCSTR a_fileName, DWORD a_desiredAccess, DWORD a_shareMode, LPSECURITY_ATTRIBUTES a_securityAttributes, DWORD a_creationDisposition, DWORD a_flagsAndAttributes, HANDLE a_templateFile)
{
	_DMESSAGE("[CreateFileA] %s", a_fileName);
	auto result = CreateFileA(a_fileName, a_desiredAccess, a_shareMode, a_securityAttributes, a_creationDisposition, a_flagsAndAttributes, a_templateFile);
	return result;
}


void WinAPICacher::ParseCurDir()
{
	WIN32_FIND_DATA findData;
	auto handle = FindFirstFileEx("*", FINDEX_INFO_LEVELS::FindExInfoBasic, &findData, FINDEX_SEARCH_OPS::FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);
	if (handle != INVALID_HANDLE_VALUE) {
		std::string fileName;
		do {
			fileName = findData.cFileName;
			_curDirMap.insert(fileName);
		} while (FindNextFile(handle, &findData));
		FindClose(handle);
	}
}


std::optional<std::string> WinAPICacher::GetFirstFileName(const char* a_filePath)
{
	constexpr std::size_t NPOS = static_cast<std::size_t>(-1);

	std::size_t pos = 0;
	std::size_t beg = NPOS;
	bool done = false;
	while (!done) {
		switch (a_filePath[pos]) {
		case '\\':
		case '/':
			break;
		case '\0':
			beg = NPOS;
			done = true;
			break;
		default:
			beg = pos;
			done = true;
			break;
		}
		++pos;
	}

	if (beg == NPOS) {
		return std::nullopt;
	}

	std::size_t end = NPOS;
	done = false;
	while (!done) {
		switch (a_filePath[pos]) {
		case '\\':
		case '/':
		case '\0':
			end = pos;
			done = true;
			break;
		default:
			break;
		}
		++pos;
	}

	return std::make_optional<std::string>(a_filePath, beg, end - beg);
}


decltype(WinAPICacher::_curDir) WinAPICacher::_curDir;
decltype(WinAPICacher::_curDirMap) WinAPICacher::_curDirMap;
decltype(WinAPICacher::_handleMap) WinAPICacher::_handleMap;
decltype(WinAPICacher::_attributeMap) WinAPICacher::_attributeMap;
