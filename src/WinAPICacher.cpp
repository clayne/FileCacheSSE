#include "WinAPICacher.h"

#include "skse64_common/BranchTrampoline.h"
#include "skse64_common/SafeWrite.h"
#include "xbyak/xbyak.h"

#include <Windows.h>

#include <cassert>
#include <cstring>
#include <string_view>

#include "RE/Skyrim.h"
#include "REL/Relocation.h"


namespace
{
	class GlobalPaths : public RE::BSResource::GlobalPaths
	{
	public:
		Result Hook_LocateFile(const char* a_relPath, RE::BSResource::Stream*& a_stream, Location*& a_location, char a_delim)	// 03
		{
			if (WinAPICacher::ExistsInCurDir(a_relPath)) {
				return globalLocations->LocateFile(a_relPath, a_stream, a_location, a_delim);
			} else {
				char buf[MAX_PATH];
				std::snprintf(buf, sizeof(buf), "data\\%s", a_relPath);
				return globalLocations->LocateFile(buf, a_stream, a_location, a_delim);
			}
		}


		Result Hook_TraverseFiles(const char* a_relPath, RE::BSResource::LocationTraverser* a_traverser)	// 05
		{
			if (WinAPICacher::ExistsInCurDir(a_relPath)) {
				return globalLocations->TraverseFiles(a_relPath, a_traverser);
			} else {
				char buf[MAX_PATH];
				std::snprintf(buf, sizeof(buf), "data\\%s", a_relPath);
				return globalLocations->TraverseFiles(buf, a_traverser);
			}
		}


		Result Hook_LocateFileData(const char* a_relPath, FileData* a_fileData, Location*& a_location)	// 06
		{
			if (WinAPICacher::ExistsInCurDir(a_relPath)) {
				return globalLocations->LocateFileData(a_relPath, a_fileData, a_location);
			} else {
				char buf[MAX_PATH];
				std::snprintf(buf, sizeof(buf), "data\\%s", a_relPath);
				return globalLocations->LocateFileData(buf, a_fileData, a_location);
			}
		}


		Result Hook_GetFileData(const char* a_relPath, FileData* a_fileData)	// 07
		{
			if (WinAPICacher::ExistsInCurDir(a_relPath)) {
				return globalLocations->GetFileData(a_relPath, a_fileData);
			} else {
				char buf[MAX_PATH];
				std::snprintf(buf, sizeof(buf), "data\\%s", a_relPath);
				return globalLocations->GetFileData(buf, a_fileData);
			}
		}


		static void InstallHooks()
		{
			//
			constexpr std::uintptr_t VTBL = 0x01763600;	// 1_5_80

			{
				REL::Offset<LocateFile_t**> vFunc(VTBL + (0x8 * 0x3));
				_LocateFile = *vFunc;
				SafeWrite64(vFunc.GetAddress(), unrestricted_cast<std::uintptr_t>(&Hook_LocateFile));
			}

			{
				REL::Offset<TraverseFiles_t**> vFunc(VTBL + (0x8 * 0x5));
				_TraverseFiles = *vFunc;
				SafeWrite64(vFunc.GetAddress(), unrestricted_cast<std::uintptr_t>(&Hook_TraverseFiles));
			}

			{
				REL::Offset<LocateFileData_t**> vFunc(VTBL + (0x8 * 0x6));
				_LocateFileData = *vFunc;
				SafeWrite64(vFunc.GetAddress(), unrestricted_cast<std::uintptr_t>(&Hook_LocateFileData));
			}

			{
				REL::Offset<GetFileData_t**> vFunc(VTBL + (0x8 * 0x7));
				_GetFileData = *vFunc;
				SafeWrite64(vFunc.GetAddress(), unrestricted_cast<std::uintptr_t>(&Hook_GetFileData));
			}
		}

		using LocateFile_t = function_type_t<decltype(&RE::BSResource::GlobalPaths::LocateFile)>;
		inline static LocateFile_t* _LocateFile = 0;

		using TraverseFiles_t = function_type_t<decltype(&RE::BSResource::GlobalPaths::TraverseFiles)>;
		inline static TraverseFiles_t* _TraverseFiles = 0;

		using LocateFileData_t = function_type_t<decltype(&RE::BSResource::GlobalPaths::LocateFileData)>;
		inline static LocateFileData_t* _LocateFileData = 0;

		using GetFileData_t = function_type_t<decltype(&RE::BSResource::GlobalPaths::GetFileData)>;
		inline static GetFileData_t* _GetFileData = 0;
	};


	class GlobalLocations : public RE::BSResource::GlobalLocations
	{
	public:
		Result Hook_LocateFile(const char* a_relPath, RE::BSResource::Stream*& a_stream, Location*& a_location, char a_delim)	// 03
		{
			auto result = Result::kUnhandled;
			lock.lock();
			if (locations) {
				result = locations->current->LocateFile(a_relPath, a_stream, a_location, a_delim);
			}
			lock.unlock();
			return result;
		}


		Result Hook_TraverseFiles(const char* a_relPath, RE::BSResource::LocationTraverser* a_traverser)	// 05
		{
			auto result = Result::kUnhandled;
			lock.lock();
			if (locations) {
				result = locations->current->TraverseFiles(a_relPath, a_traverser);
			}
			lock.unlock();
			return result;
		}


		Result Hook_LocateFileData(const char* a_relPath, FileData* a_fileData, Location*& a_location)	// 06
		{
			auto result = Result::kUnhandled;
			lock.lock();
			if (locations) {
				result = locations->current->LocateFileData(a_relPath, a_fileData, a_location);
			}
			lock.unlock();
			return result;
		}


		Result Hook_GetFileData(const char* a_relPath, FileData* a_fileData)	// 07
		{
			auto result = Result::kUnhandled;
			lock.lock();
			if (locations) {
				result = locations->current->GetFileData(a_relPath, a_fileData);
			}
			lock.unlock();
			return result;
		}


		static void InstallHooks()
		{
			//
			constexpr std::uintptr_t VTBL = 0x01763530;	// 1_5_80

			{
				REL::Offset<LocateFile_t**> vFunc(VTBL + (0x8 * 0x3));
				_LocateFile = *vFunc;
				SafeWrite64(vFunc.GetAddress(), unrestricted_cast<std::uintptr_t>(&Hook_LocateFile));
			}

			{
				REL::Offset<TraverseFiles_t**> vFunc(VTBL + (0x8 * 0x5));
				_TraverseFiles = *vFunc;
				SafeWrite64(vFunc.GetAddress(), unrestricted_cast<std::uintptr_t>(&Hook_TraverseFiles));
			}

			{
				REL::Offset<LocateFileData_t**> vFunc(VTBL + (0x8 * 0x6));
				_LocateFileData = *vFunc;
				SafeWrite64(vFunc.GetAddress(), unrestricted_cast<std::uintptr_t>(&Hook_LocateFileData));
			}

			{
				REL::Offset<GetFileData_t**> vFunc(VTBL + (0x8 * 0x7));
				_GetFileData = *vFunc;
				SafeWrite64(vFunc.GetAddress(), unrestricted_cast<std::uintptr_t>(&Hook_GetFileData));
			}
		}

		using LocateFile_t = function_type_t<decltype(&RE::BSResource::GlobalLocations::LocateFile)>;
		inline static LocateFile_t* _LocateFile = 0;

		using TraverseFiles_t = function_type_t<decltype(&RE::BSResource::GlobalLocations::TraverseFiles)>;
		inline static TraverseFiles_t* _TraverseFiles = 0;

		using LocateFileData_t = function_type_t<decltype(&RE::BSResource::GlobalLocations::LocateFileData)>;
		inline static LocateFileData_t* _LocateFileData = 0;

		using GetFileData_t = function_type_t<decltype(&RE::BSResource::GlobalLocations::GetFileData)>;
		inline static GetFileData_t* _GetFileData = 0;
	};


	class TESDataHandler : public RE::TESDataHandler
	{
	public:
		void ParseDirectory(const char* a_dirPath)
		{
			func(this, "data\\");
		}


		static void InstallHooks()
		{
			// E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? 8B 05 ? ? ? ? 83 F8 01
			constexpr std::uintptr_t ADDR = 0x0016D720;	// 1_5_80
			constexpr UInt8 NOP = 0x90;

			REL::Offset<std::uintptr_t> funcBase(ADDR);

			{
				auto funcAddress = funcBase.GetAddress() + 0x11F;
				auto offset = reinterpret_cast<std::int32_t*>(funcAddress + 1);
				auto nextOp = funcAddress + 5;
				func = unrestricted_cast<func_t*>(nextOp + *offset);

				g_branchTrampoline.Write5Call(funcAddress, unrestricted_cast<std::uintptr_t>(&ParseDirectory));
			}

			// duplicate call to parse data directory
			for (std::size_t i = 0x16D; i < 0x172; ++i) {
				SafeWrite8(funcBase.GetAddress() + i, NOP);
			}
		}


		using func_t = function_type_t<decltype(&ParseDirectory)>;
		inline static func_t* func = 0;
	};


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

	WriteHook(0x01509208, Hook_FindFirstFileA);	// 1_5_80
	WriteHook(0x01509160, Hook_FindNextFileA);	// 1_5_80
	WriteHook(0x01509210, Hook_FindClose);	// 1_5_80
	WriteHook(0x01509260, Hook_GetCurrentDirectoryA);	// 1_5_80
	WriteHook(0x01509228, Hook_GetFileAttributesA);	// 1_5_80
	WriteHook(0x015093C8, Hook_GetFileAttributesExA);	// 1_5_80
	WriteHook(0x01509340, Hook_CreateFileA);	// 1_5_80

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

	GlobalPaths::InstallHooks();
	GlobalLocations::InstallHooks();
	TESDataHandler::InstallHooks();
}


bool WinAPICacher::ExistsInCurDir(const char* a_file)
{
	return _curDirMap.find(a_file) != _curDirMap.end();
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


decltype(WinAPICacher::_curDir) WinAPICacher::_curDir;
decltype(WinAPICacher::_curDirMap) WinAPICacher::_curDirMap;
decltype(WinAPICacher::_handleMap) WinAPICacher::_handleMap;
decltype(WinAPICacher::_attributeMap) WinAPICacher::_attributeMap;
