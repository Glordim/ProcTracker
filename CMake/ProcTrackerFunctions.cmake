cmake_minimum_required(VERSION 3.4...3.27)

function(SetupLanguageVersion)
	set(CMAKE_CXX_STANDARD 20 PARENT_SCOPE)
	set(CMAKE_CXX_STANDARD_REQUIRED TRUE PARENT_SCOPE)
	set(CMAKE_CXX_EXTENSIONS OFF PARENT_SCOPE)

	if(APPLE)
		set(CMAKE_OBJCXX_STANDARD 20 PARENT_SCOPE)
		set(CMAKE_OBJCXX_STANDARD_REQUIRED TRUE PARENT_SCOPE)
		set(CMAKE_OSX_DEPLOYMENT_TARGET 14 PARENT_SCOPE)
	endif()
endfunction()

function(SetupPlatform)
	if(WIN32)
		set(PLATFORM_DEFINE "PLATFORM_WINDOWS" PARENT_SCOPE)
		set(PLATFORM_WINDOWS True PARENT_SCOPE)
	elseif(APPLE)
		set(PLATFORM_DEFINE "PLATFORM_MACOS" PARENT_SCOPE)
		set(PLATFORM_MACOS True PARENT_SCOPE)
	else()
		set(PLATFORM_DEFINE "PLATFORM_LINUX" PARENT_SCOPE)
		set(PLATFORM_LINUX True PARENT_SCOPE)
		set(CMAKE_POSITION_INDEPENDENT_CODE ON PARENT_SCOPE) # TODO Move in Compilation Setup ?
	endif()
endfunction()

function(SetupCompilation)
	# No reason to set CMAKE_CONFIGURATION_TYPES if it's not a multiconfig generator
	# Also no reason mess with CMAKE_BUILD_TYPE if it's a multiconfig generator.
	get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
	if(isMultiConfig)
		set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

		foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
			string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG_UPPER)
			set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${CMAKE_BINARY_DIR}/${OUTPUTCONFIG} PARENT_SCOPE)
			set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${CMAKE_BINARY_DIR}/${OUTPUTCONFIG} PARENT_SCOPE)
			set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${CMAKE_BINARY_DIR}/${OUTPUTCONFIG} PARENT_SCOPE)
			set(CMAKE_PDB_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${CMAKE_BINARY_DIR}/${OUTPUTCONFIG} PARENT_SCOPE)
		endforeach()
	else()
		if(NOT CMAKE_BUILD_TYPE)
			message("Defaulting to Release build.")
			set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
		endif()
		set_property(CACHE CMAKE_BUILD_TYPE PROPERTY HELPSTRING "Choose the type of build")
		# set the valid options for cmake-gui drop-down list
		set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug;Release")

		string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_UPPER)
		set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CMAKE_BUILD_TYPE_UPPER} ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE} PARENT_SCOPE)
		set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CMAKE_BUILD_TYPE_UPPER} ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE} PARENT_SCOPE)
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CMAKE_BUILD_TYPE_UPPER} ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE} PARENT_SCOPE)
		set(CMAKE_PDB_OUTPUT_DIRECTORY_${CMAKE_BUILD_TYPE_UPPER} ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE} PARENT_SCOPE)
	endif()

	if (MSVC)
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi" PARENT_SCOPE)
		set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG" PARENT_SCOPE)
		set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG" PARENT_SCOPE)
	else()
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g" PARENT_SCOPE)
	endif()

	if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-offsetof -Wno-nullability-completeness -Werror" PARENT_SCOPE)
	elseif (MSVC)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX /wd4251" PARENT_SCOPE)
	elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wno-invalid-offsetof" PARENT_SCOPE)
	else()
		message("Unknown compiler: " ${CMAKE_CXX_COMPILER})
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-offsetof -Wno-nullability-completeness -Werror" PARENT_SCOPE)
	endif()

	if(APPLE)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fobjc-arc" PARENT_SCOPE)
	endif()

	if(MSVC)
		add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	endif()
endfunction()

function(CollectSourceFiles ProjectSourceDir SourcesVar IncludesVar)

	cmake_parse_arguments(PARSED_ARGS "" "" "EXCLUDES" ${ARGN})

	if (APPLE)
		file(GLOB_RECURSE Sources
			"${ProjectSourceDir}/*.cpp"
			"${ProjectSourceDir}/*.mm"
			"${ProjectSourceDir}/*.hpp"
			"${ProjectSourceDir}/*.inl"
			"${ProjectSourceDir}/*.h"
		)
	else()
		file(GLOB_RECURSE Sources
			"${ProjectSourceDir}/*.cpp"
			"${ProjectSourceDir}/*.hpp"
			"${ProjectSourceDir}/*.inl"
			"${ProjectSourceDir}/*.h"
		)
	endif()

	if(PARSED_ARGS_EXCLUDES)
		foreach(Exclude ${PARSED_ARGS_EXCLUDES})
			#message(Exclude:${Exclude})
			foreach(item ${Sources})
				if(item MATCHES ${Exclude})
					list(REMOVE_ITEM Sources ${item})
					#message(Remove:${item})
				endif()
			endforeach()
		endforeach()
	endif()

	foreach(item ${Sources})
		if(${item} MATCHES ".+\\..+\\.cpp")
			list(REMOVE_ITEM Sources ${item})
		endif()
	endforeach()

	set(PlatformDirs Windows MacOs Linux)
	if(PLATFORM_WINDOWS)
		list(REMOVE_ITEM PlatformDirs Windows)
	elseif(PLATFORM_MACOS)
		list(REMOVE_ITEM PlatformDirs MacOs)
	elseif (PLATFORM_LINUX)
		list(REMOVE_ITEM PlatformDirs Linux)
	endif()

	foreach(item ${Sources})
		set(shouldRemove FALSE)
		string(TOLOWER ${item} lower_item)
		
		foreach(platform ${PlatformDirs})
			string(TOLOWER ${platform} lower_platform)
			if(${lower_item} MATCHES "(/|^)(${lower_platform})(/|$)")
				set(shouldRemove TRUE)
				break()
			endif()
		endforeach()

		if(shouldRemove)
			#message(Exclude:${item})
			list(REMOVE_ITEM Sources ${item})
		endif()
	endforeach()

	if(WIN32)
		file(GLOB_RECURSE PlatformSources
			"${ProjectSourceDir}/*.windows.cpp"
		)
	elseif(APPLE)
		file(GLOB_RECURSE PlatformSources
			"${ProjectSourceDir}/*.macos.cpp"
		)
	else()
		file(GLOB_RECURSE PlatformSources
			"${ProjectSourceDir}/*.linux.cpp"
		)
	endif()

	set(${SourcesVar} ${Sources} ${PlatformSources} PARENT_SCOPE)

	file(GLOB_RECURSE Includes
		"${ProjectSourceDir}/*.hpp"
		"${ProjectSourceDir}/*.inl"
		"${ProjectSourceDir}/*.h"
	)
	set(${IncludesVar} ${Includes} PARENT_SCOPE)
endfunction()

function(assign_source_group)
	foreach(_source IN ITEMS ${ARGN})
		if (IS_ABSOLUTE "${_source}")
			file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
		else()
			set(_source_rel "${_source}")
		endif()
		get_filename_component(_source_path "${_source_rel}" PATH)
		string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
		source_group("${_source_path_msvc}" FILES "${_source_rel}")
	endforeach()
endfunction(assign_source_group)
