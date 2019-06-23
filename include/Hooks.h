#pragma once

#include "RE/Skyrim.h"


namespace
{
	class GlobalPathsEx : public RE::BSResource::GlobalPaths
	{
	public:
		Result Hook_LocateFile(const char* a_relPath, RE::BSResource::Stream*& a_stream, RE::BSResource::Location*& a_location, char a_delim);	// 03
		Result Hook_TraverseFiles(const char* a_relPath, RE::BSResource::LocationTraverser* a_traverser);										// 05
		Result Hook_LocateFileData(const char* a_relPath, FileData* a_fileData, RE::BSResource::Location*& a_location);							// 06
		Result Hook_GetFileData(const char* a_relPath, FileData* a_fileData);																	// 07

		static void InstallHooks();

		using LocateFile_t = function_type_t<decltype(&RE::BSResource::GlobalPaths::LocateFile)>;
		inline static LocateFile_t* _LocateFile = 0;

		using TraverseFiles_t = function_type_t<decltype(&RE::BSResource::GlobalPaths::TraverseFiles)>;
		inline static TraverseFiles_t* _TraverseFiles = 0;

		using LocateFileData_t = function_type_t<decltype(&RE::BSResource::GlobalPaths::LocateFileData)>;
		inline static LocateFileData_t* _LocateFileData = 0;

		using GetFileData_t = function_type_t<decltype(&RE::BSResource::GlobalPaths::GetFileData)>;
		inline static GetFileData_t* _GetFileData = 0;
	};


	class GlobalLocationsEx : public RE::BSResource::GlobalLocations
	{
	public:
		Result Hook_LocateFile(const char* a_relPath, RE::BSResource::Stream*& a_stream, RE::BSResource::Location*& a_location, char a_delim);	// 03
		Result Hook_TraverseFiles(const char* a_relPath, RE::BSResource::LocationTraverser* a_traverser);										// 05
		Result Hook_LocateFileData(const char* a_relPath, FileData* a_fileData, RE::BSResource::Location*& a_location);							// 06
		Result Hook_GetFileData(const char* a_relPath, FileData* a_fileData);																	// 07

		static void InstallHooks();

		using LocateFile_t = function_type_t<decltype(&RE::BSResource::GlobalLocations::LocateFile)>;
		inline static LocateFile_t* _LocateFile = 0;

		using TraverseFiles_t = function_type_t<decltype(&RE::BSResource::GlobalLocations::TraverseFiles)>;
		inline static TraverseFiles_t* _TraverseFiles = 0;

		using LocateFileData_t = function_type_t<decltype(&RE::BSResource::GlobalLocations::LocateFileData)>;
		inline static LocateFileData_t* _LocateFileData = 0;

		using GetFileData_t = function_type_t<decltype(&RE::BSResource::GlobalLocations::GetFileData)>;
		inline static GetFileData_t* _GetFileData = 0;
	};


	class TESDataHandlerEx : public RE::TESDataHandler
	{
	public:
		void ParseDirectory(const char* a_dirPath);

		static void InstallHooks();

		using func_t = function_type_t<decltype(&ParseDirectory)>;
		inline static func_t* func = 0;
	};
}


void InstallHooks();
