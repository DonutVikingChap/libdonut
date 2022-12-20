cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project("libdonut-examples")

add_library(donut-example-base INTERFACE)

target_compile_features(donut-example-base INTERFACE cxx_std_20)

target_compile_options(donut-example-base INTERFACE
	$<$<CXX_COMPILER_ID:GNU>:   -std=c++20  -Wall -Wextra   -Wconversion    -Wpedantic      -Werror                 $<$<CONFIG:Debug>:-g>   $<$<CONFIG:Release>:-O3 -flto>  $<$<CONFIG:MinSizeRel>:-Os -flto>   $<$<CONFIG:RelWithDebInfo>:-O3 -g -flto>>
	$<$<CXX_COMPILER_ID:Clang>: -std=c++20  -Wall -Wextra   -Wconversion    -Wpedantic      -Werror                 $<$<CONFIG:Debug>:-g>   $<$<CONFIG:Release>:-O3 -flto>  $<$<CONFIG:MinSizeRel>:-Os -flto>   $<$<CONFIG:RelWithDebInfo>:-O3 -g -flto>>
	$<$<CXX_COMPILER_ID:MSVC>:  /std:c++20  /W4                             /permissive-    /WX     /wd4996 /utf-8  $<$<CONFIG:Debug>:/Od>  $<$<CONFIG:Release>:/Ot>        $<$<CONFIG:MinSizeRel>:/Os>         $<$<CONFIG:RelWithDebInfo>:/Ot /Od>>)

target_link_libraries(donut-example-base INTERFACE
	donut::donut
	fmt)

add_executable(donut-example-game "example_game.cpp")
target_link_libraries(donut-example-game PRIVATE donut-example-base)
add_test(NAME donut-example-game COMMAND donut-example-game)

add_executable(donut-example-rectangle "example_rectangle.cpp")
target_link_libraries(donut-example-rectangle PRIVATE donut-example-base)
add_test(NAME donut-example-rectangle COMMAND donut-example-rectangle)
