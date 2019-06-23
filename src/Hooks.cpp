#include "Hooks.h"

#include "skse64_common/BranchTrampoline.h"
#include "skse64_common/SafeWrite.h"

#include "WinAPICacher.h"

#include "REL/Relocation.h"


namespace
{
	auto GlobalPathsEx::Hook_LocateFile(const char* a_relPath, RE::BSResource::Stream*& a_stream, RE::BSResource::Location*& a_location, char a_delim)
		-> Result
	{
		if (WinAPICacher::ExistsInCurDir(a_relPath)) {
			return globalLocations->LocateFile(a_relPath, a_stream, a_location, a_delim);
		} else {
			char buf[MAX_PATH];
			std::snprintf(buf, sizeof(buf), "data\\%s", a_relPath);
			return globalLocations->LocateFile(buf, a_stream, a_location, a_delim);
		}
	}


	auto GlobalPathsEx::Hook_TraverseFiles(const char* a_relPath, RE::BSResource::LocationTraverser* a_traverser)
		-> Result
	{
		if (WinAPICacher::ExistsInCurDir(a_relPath)) {
			return globalLocations->TraverseFiles(a_relPath, a_traverser);
		} else {
			char buf[MAX_PATH];
			std::snprintf(buf, sizeof(buf), "data\\%s", a_relPath);
			return globalLocations->TraverseFiles(buf, a_traverser);
		}
	}


	auto GlobalPathsEx::Hook_LocateFileData(const char* a_relPath, FileData* a_fileData, RE::BSResource::Location*& a_location)
		-> Result
	{
		if (WinAPICacher::ExistsInCurDir(a_relPath)) {
			return globalLocations->LocateFileData(a_relPath, a_fileData, a_location);
		} else {
			char buf[MAX_PATH];
			std::snprintf(buf, sizeof(buf), "data\\%s", a_relPath);
			return globalLocations->LocateFileData(buf, a_fileData, a_location);
		}
	}


	auto GlobalPathsEx::Hook_GetFileData(const char* a_relPath, FileData* a_fileData)
		-> Result
	{
		if (WinAPICacher::ExistsInCurDir(a_relPath)) {
			return globalLocations->GetFileData(a_relPath, a_fileData);
		} else {
			char buf[MAX_PATH];
			std::snprintf(buf, sizeof(buf), "data\\%s", a_relPath);
			return globalLocations->GetFileData(buf, a_fileData);
		}
	}


	void GlobalPathsEx::InstallHooks()
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


	auto GlobalLocationsEx::Hook_LocateFile(const char* a_relPath, RE::BSResource::Stream*& a_stream, RE::BSResource::Location*& a_location, char a_delim)
		-> Result
	{
		auto result = Result::kUnhandled;
		lock.lock();
		if (locations) {
			result = locations->current->LocateFile(a_relPath, a_stream, a_location, a_delim);
		}
		lock.unlock();
		return result;
	}


	auto GlobalLocationsEx::Hook_TraverseFiles(const char* a_relPath, RE::BSResource::LocationTraverser* a_traverser)
		-> Result
	{
		auto result = Result::kUnhandled;
		lock.lock();
		if (locations) {
			result = locations->current->TraverseFiles(a_relPath, a_traverser);
		}
		lock.unlock();
		return result;
	}


	auto GlobalLocationsEx::Hook_LocateFileData(const char* a_relPath, FileData* a_fileData, RE::BSResource::Location*& a_location)
		-> Result
	{
		auto result = Result::kUnhandled;
		lock.lock();
		if (locations) {
			result = locations->current->LocateFileData(a_relPath, a_fileData, a_location);
		}
		lock.unlock();
		return result;
	}


	auto GlobalLocationsEx::Hook_GetFileData(const char* a_relPath, FileData* a_fileData)
		-> Result
	{
		auto result = Result::kUnhandled;
		lock.lock();
		if (locations) {
			result = locations->current->GetFileData(a_relPath, a_fileData);
		}
		lock.unlock();
		return result;
	}


	void GlobalLocationsEx::InstallHooks()
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


	void TESDataHandlerEx::ParseDirectory(const char* a_dirPath)
	{
		func(this, "data\\");
	}


	void TESDataHandlerEx::InstallHooks()
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
}


void InstallHooks()
{
	WinAPICacher::InstallHooks();
	GlobalPathsEx::InstallHooks();
	GlobalLocationsEx::InstallHooks();
	TESDataHandlerEx::InstallHooks();
}
