cmake_minimum_required(VERSION 3.21 FATAL_ERROR)
project("Your project name here")

option(BUILD_SHARED_LIBS "Build dependencies as shared libraries" OFF)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include(FetchContent)
FetchContent_Declare(donut
    GIT_REPOSITORY https://github.com/DonutVikingChap/libdonut
    GIT_TAG "origin/main")
FetchContent_MakeAvailable(donut)

set(APP_TARGET_NAME "app")

add_executable("${APP_TARGET_NAME}"
	"src/main.cpp")

target_compile_features("${APP_TARGET_NAME}" PRIVATE cxx_std_20)

target_compile_options("${APP_TARGET_NAME}" PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:   -std=c++20  -Wall -Wextra   -Wconversion    -Wpedantic      -Werror                 $<$<CONFIG:Debug>:-g3>  $<$<CONFIG:Release>:-O3>    $<$<CONFIG:MinSizeRel>:-Os> $<$<CONFIG:RelWithDebInfo>:-O3 -g3>>
    $<$<CXX_COMPILER_ID:Clang>: -std=c++20  -Wall -Wextra   -Wconversion    -Wpedantic      -Werror                 $<$<CONFIG:Debug>:-g3>  $<$<CONFIG:Release>:-O3>    $<$<CONFIG:MinSizeRel>:-Os> $<$<CONFIG:RelWithDebInfo>:-O3 -g3>>
    $<$<CXX_COMPILER_ID:MSVC>:  /std:c++20  /W4                             /permissive-    /WX     /wd4996 /utf-8  $<$<CONFIG:Debug>:/Od>  $<$<CONFIG:Release>:/Ot>    $<$<CONFIG:MinSizeRel>:/Os> $<$<CONFIG:RelWithDebInfo>:/Ot /Od>>)

target_link_libraries("${APP_TARGET_NAME}" PRIVATE
	donut::donut
	Threads::Threads)

if(CMAKE_IMPORT_LIBRARY_SUFFIX AND BUILD_SHARED_LIBS)
	add_custom_command(TARGET "${APP_TARGET_NAME}" POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_RUNTIME_DLLS:${APP_TARGET_NAME}>" "$<TARGET_FILE_DIR:${APP_TARGET_NAME}>" COMMAND_EXPAND_LISTS)
endif()

include(GNUInstallDirs)

set_target_properties("${APP_TARGET_NAME}" PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_INSTALL_LIBDIR}"
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_INSTALL_LIBDIR}"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_INSTALL_BINDIR}")
