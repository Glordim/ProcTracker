cmake_minimum_required(VERSION 3.4...3.27)

include(EmbeedInSource)

if(APPLE)
	project(ProcTracker LANGUAGES CXX OBJCXX)
else()
	project(ProcTracker LANGUAGES CXX)
endif()

CollectSourceFiles(${PROJECT_SOURCE_DIR} Srcs Includes)

EmbeedBinaryInSource("Font/Roboto-Regular.ttf"
	${PROJECT_SOURCE_DIR}/EmbeedInSourceTemplate/BinarySourceFileTemplate.hpp.in
	${PROJECT_SOURCE_DIR}/EmbeedInSourceTemplate/BinarySourceFileTemplate.cpp.in
	${PROJECT_SOURCE_DIR}/Font/Generated
	GeneratedFileH
	GeneratedFileCPP
)
list(APPEND Srcs ${GeneratedFileH})
list(APPEND Includes ${GeneratedFileH})
list(APPEND Srcs ${GeneratedFileCPP})

EmbeedBinaryInSource("Font/MaterialDesignIcons.ttf"
	${PROJECT_SOURCE_DIR}/EmbeedInSourceTemplate/BinarySourceFileTemplate.hpp.in
	${PROJECT_SOURCE_DIR}/EmbeedInSourceTemplate/BinarySourceFileTemplate.cpp.in
	${PROJECT_SOURCE_DIR}/Font/Generated
	GeneratedFileH
	GeneratedFileCPP
)
list(APPEND Srcs ${GeneratedFileH})
list(APPEND Includes ${GeneratedFileH})
list(APPEND Srcs ${GeneratedFileCPP})

assign_source_group(${Srcs}) # for VS

add_executable(ProcTracker ${LIB_TYPE} ${Srcs} ${Includes})
target_precompile_headers(ProcTracker PRIVATE "Pch.hpp")
set_property(TARGET ProcTracker PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")

add_dependencies(ProcTracker freetype SDL)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${freetype_INCLUDE_DIRS} ${SDL_INCLUDE_DIRS})

include_directories(SYSTEM
	"DearImGui" # for imguiFreetype include
)

target_link_libraries(ProcTracker ${freetype_LIBRARIES} ${SDL_LIBRARIES})

target_compile_definitions(ProcTracker PUBLIC ${PLATFORM_DEFINE})
