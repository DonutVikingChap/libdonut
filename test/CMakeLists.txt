cmake_minimum_required(VERSION 3.21 FATAL_ERROR)
project("libdonut-test")

FetchContent_Declare(Catch2
	GIT_REPOSITORY https://github.com/catchorg/Catch2
	GIT_TAG fee81626d2a4811095c3a39d20fb355eeb954101 # v3.9.0
)
FetchContent_MakeAvailable(Catch2)

add_library(donut-test-base INTERFACE)
target_compile_features(donut-test-base INTERFACE cxx_std_20)
target_compile_options(donut-test-base INTERFACE
	$<$<CXX_COMPILER_ID:GNU>:   -std=c++20  -Wall -Wextra   -Wconversion    -Wpedantic      -Werror                 $<$<CONFIG:Debug>:-g3>  $<$<CONFIG:Release>:-O3>    $<$<CONFIG:MinSizeRel>:-Os> $<$<CONFIG:RelWithDebInfo>:-O3 -g3>>
	$<$<CXX_COMPILER_ID:Clang>: -std=c++20  -Wall -Wextra   -Wconversion    -Wpedantic      -Werror                 $<$<CONFIG:Debug>:-g3>  $<$<CONFIG:Release>:-O3>    $<$<CONFIG:MinSizeRel>:-Os> $<$<CONFIG:RelWithDebInfo>:-O3 -g3>>
	$<$<CXX_COMPILER_ID:MSVC>:  /std:c++20  /W4                             /permissive-    /WX     /wd4996 /utf-8  $<$<CONFIG:Debug>:/Od>  $<$<CONFIG:Release>:/Ot>    $<$<CONFIG:MinSizeRel>:/Os> $<$<CONFIG:RelWithDebInfo>:/Ot /Od>>)
target_link_libraries(donut-test-base INTERFACE donut::donut Catch2::Catch2WithMain)

add_executable(donut-test-json "test_json.cpp")
target_link_libraries(donut-test-json PRIVATE donut-test-base)
add_test(NAME donut-test-json COMMAND donut-test-json)

if(BUILD_SHARED_LIBS)
	target_link_libraries(donut-test-json PRIVATE ${CMAKE_DL_LIBS})
	if(CMAKE_IMPORT_LIBRARY_SUFFIX)
		add_custom_command(TARGET donut-test-json POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:donut-test-json> $<TARGET_FILE_DIR:donut-test-json> COMMAND_EXPAND_LISTS)
	endif()
endif()
