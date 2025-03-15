cmake_minimum_required(VERSION 3.4...3.27)

project(ProcTrackerRoot)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

include(FetchContent)
include(ExternalProject)
include(ProcTrackerFunctions)

SetupLanguageVersion()

if(WIN32)
	set(LIB_PREFIX "")
	set(LIB_EXTENSION "lib")
else()
	set(LIB_PREFIX "lib")
	set(LIB_EXTENSION "a")
endif()

get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(isMultiConfig)
	set(LIB_SUB_FOLDER "Release")
else()
	set(LIB_SUB_FOLDER ".")
endif()

set(SDL_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/_deps/SDL-src/include)
set(SDL_LIBRARIES ${CMAKE_BINARY_DIR}/_deps/SDL-build/${LIB_SUB_FOLDER}/${LIB_PREFIX}SDL3-static.${LIB_EXTENSION} Setupapi.lib Version.lib Winmm.lib) # Move Win32 deps

ExternalProject_Add(SDL
	URL https://github.com/libsdl-org/SDL/archive/refs/tags/release-3.2.8.zip
	DOWNLOAD_DIR ${DOWNLOAD_CACHE_DIR}
	SOURCE_DIR ${CMAKE_BINARY_DIR}/_deps/SDL-src
	BINARY_DIR ${CMAKE_BINARY_DIR}/_deps/SDL-build
	CMAKE_ARGS
		-DSDL_SHARED=OFF
		-DSDL_STATIC=ON
		-DCMAKE_POSITION_INDEPENDENT_CODE=ON
		-DCMAKE_OSX_DEPLOYMENT_TARGET=14 #TODO only on MacOs platform
		-DCMAKE_BUILD_TYPE=Release
		-G ${CMAKE_GENERATOR}
	BUILD_COMMAND 
		${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/_deps/SDL-build --config Release --parallel
	INSTALL_COMMAND ""
	BUILD_BYPRODUCTS ${SDL_LIBRARIES}
)

message(STATUS "SDL is available in " ${SDL_INCLUDE_DIRS})
message(STATUS "SDL is available in " ${SDL_LIBRARIES})

set(freetype_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/_deps/freetype-src/include)
set(freetype_LIBRARIES ${CMAKE_BINARY_DIR}/_deps/freetype-build/${LIB_SUB_FOLDER}/${LIB_PREFIX}freetype.${LIB_EXTENSION})

ExternalProject_Add(freetype
	URL https://github.com/freetype/freetype/archive/refs/tags/VER-2-13-2.zip
	DOWNLOAD_DIR ${DOWNLOAD_CACHE_DIR}
	SOURCE_DIR ${CMAKE_BINARY_DIR}/_deps/freetype-src
	BINARY_DIR ${CMAKE_BINARY_DIR}/_deps/freetype-build
	CMAKE_ARGS
		-DFT_DISABLE_BROTLI=ON
		-DFT_DISABLE_HARFBUZZ=ON
		-DFT_DISABLE_PNG=ON
		-DFT_DISABLE_BZIP2=ON
		-DFT_DISABLE_ZLIB=ON
		-DCMAKE_POSITION_INDEPENDENT_CODE=ON
		-DCMAKE_OSX_DEPLOYMENT_TARGET=14 #TODO only on MacOs platform
		-DCMAKE_BUILD_TYPE=Release
		-G ${CMAKE_GENERATOR}
	BUILD_COMMAND 
		${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/_deps/freetype-build --config Release --parallel
	INSTALL_COMMAND ""
	BUILD_BYPRODUCTS ${freetype_LIBRARIES}
)

message(STATUS "freetype is available in " ${freetype_INCLUDE_DIRS})
message(STATUS "freetype is available in " ${freetype_LIBRARIES})

if (WIN32)
	FetchContent_Declare(phnt URL https://github.com/winsiderss/phnt/archive/refs/heads/master.zip)
	FetchContent_MakeAvailable(phnt)
	message(STATUS "phnt is available in " ${phnt_SOURCE_DIR})
endif()

SetupPlatform()
SetupCompilation()

set(CMAKE_SUPPRESS_REGENERATION true)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory("Sources")
