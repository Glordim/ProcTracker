cmake_minimum_required(VERSION 3.4...3.27)

include(EmbedInSource)

if(APPLE)
	project(ProcTracker LANGUAGES CXX OBJCXX)
else()
	project(ProcTracker LANGUAGES CXX)
endif()

CollectSourceFiles(${PROJECT_SOURCE_DIR} Srcs Includes)

EmbedBinaryInSource("Font/Roboto-Regular.ttf"
	${PROJECT_SOURCE_DIR}/EmbedInSourceTemplate/BinarySourceFileTemplate.hpp.in
	${PROJECT_SOURCE_DIR}/EmbedInSourceTemplate/BinarySourceFileTemplate.cpp.in
	${PROJECT_BINARY_DIR}/Generated/Font
	GeneratedFileH
	GeneratedFileCPP
)
list(APPEND Srcs ${GeneratedFileH})
list(APPEND Includes ${GeneratedFileH})
list(APPEND Srcs ${GeneratedFileCPP})

EmbedBinaryInSource("Font/MaterialDesignIcons.ttf"
	${PROJECT_SOURCE_DIR}/EmbedInSourceTemplate/BinarySourceFileTemplate.hpp.in
	${PROJECT_SOURCE_DIR}/EmbedInSourceTemplate/BinarySourceFileTemplate.cpp.in
	${PROJECT_BINARY_DIR}/Generated/Font
	GeneratedFileH
	GeneratedFileCPP
)
list(APPEND Srcs ${GeneratedFileH})
list(APPEND Includes ${GeneratedFileH})
list(APPEND Srcs ${GeneratedFileCPP})

assign_source_group(${Srcs}) # for VS

add_executable(ProcTracker ${Srcs} ${Includes})
target_precompile_headers(ProcTracker PRIVATE "Pch.hpp")
set_property(TARGET ProcTracker PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")

add_dependencies(ProcTracker freetype SDL)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${freetype_INCLUDE_DIRS} ${SDL_INCLUDE_DIRS} ${PROJECT_BINARY_DIR}/Generated)

include_directories(SYSTEM
	"DearImGui" # for imguiFreetype include
	${phnt_SOURCE_DIR}
)

target_link_libraries(ProcTracker ${freetype_LIBRARIES} ${SDL_LIBRARIES})
if(WIN32)
	target_link_libraries(ProcTracker pdh.lib)
endif()

target_compile_definitions(ProcTracker PUBLIC ${PLATFORM_DEFINE})
