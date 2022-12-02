cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project("libdonut-test")

find_package(doctest 2.4.9 REQUIRED)

add_library(donut-test-base INTERFACE)

target_compile_features(donut-test-base INTERFACE cxx_std_20)

target_compile_options(donut-test-base INTERFACE
	$<$<CXX_COMPILER_ID:GNU>:   -std=c++20  -Wall -Wextra   -Wpedantic      -Werror                 $<$<CONFIG:Debug>:-g>   $<$<CONFIG:Release>:-O3 -flto>  $<$<CONFIG:MinSizeRel>:-Os -flto>   $<$<CONFIG:RelWithDebInfo>:-O3 -g -flto>>
	$<$<CXX_COMPILER_ID:Clang>: -std=c++20  -Wall -Wextra   -Wpedantic      -Werror                 $<$<CONFIG:Debug>:-g>   $<$<CONFIG:Release>:-O3 -flto>  $<$<CONFIG:MinSizeRel>:-Os -flto>   $<$<CONFIG:RelWithDebInfo>:-O3 -g -flto>>
	$<$<CXX_COMPILER_ID:MSVC>:  /std:c++20  /W3             /permissive-    /WX     /wd4996 /utf-8  $<$<CONFIG:Debug>:/Od>  $<$<CONFIG:Release>:/Ot>        $<$<CONFIG:MinSizeRel>:/Os>         $<$<CONFIG:RelWithDebInfo>:/Ot /Od>>)

target_link_libraries(donut-test-base INTERFACE
	donut::donut
	doctest::doctest
	fmt)
	
add_executable(donut-test-json "test_json.cpp")
target_link_libraries(donut-test-json PRIVATE donut-test-base)
add_test(NAME donut-test-json COMMAND donut-test-json)
