cmake_minimum_required(VERSION 3.21 FATAL_ERROR)
project("libdonut-examples")

add_library(donut-example-base INTERFACE)

target_compile_features(donut-example-base INTERFACE cxx_std_20)

target_compile_options(donut-example-base INTERFACE
	$<$<CXX_COMPILER_ID:GNU>:   -std=c++20  -Wall -Wextra   -Wconversion    -Wpedantic      -Werror                 $<$<CONFIG:Debug>:-g3>  $<$<CONFIG:Release>:-O3>    $<$<CONFIG:MinSizeRel>:-Os> $<$<CONFIG:RelWithDebInfo>:-O3 -g3>>
	$<$<CXX_COMPILER_ID:Clang>: -std=c++20  -Wall -Wextra   -Wconversion    -Wpedantic      -Werror                 $<$<CONFIG:Debug>:-g3>  $<$<CONFIG:Release>:-O3>    $<$<CONFIG:MinSizeRel>:-Os> $<$<CONFIG:RelWithDebInfo>:-O3 -g3>>
	$<$<CXX_COMPILER_ID:MSVC>:  /std:c++20  /W4                             /permissive-    /WX     /wd4996 /utf-8  $<$<CONFIG:Debug>:/Od>  $<$<CONFIG:Release>:/Ot>    $<$<CONFIG:MinSizeRel>:/Os> $<$<CONFIG:RelWithDebInfo>:/Ot /Od>>)

target_link_libraries(donut-example-base INTERFACE donut::donut)

add_executable(donut-example-game "example_game.cpp")
target_link_libraries(donut-example-game PRIVATE donut-example-base)
add_test(NAME donut-example-game COMMAND donut-example-game)

add_executable(donut-example-rectangle "example_rectangle.cpp")
target_link_libraries(donut-example-rectangle PRIVATE donut-example-base)
add_test(NAME donut-example-rectangle COMMAND donut-example-rectangle)

if(BUILD_SHARED_LIBS)
	target_link_libraries(donut-example-game PRIVATE ${CMAKE_DL_LIBS})
	target_link_libraries(donut-example-rectangle PRIVATE ${CMAKE_DL_LIBS})
	if(CMAKE_IMPORT_LIBRARY_SUFFIX)
		add_custom_command(TARGET donut-example-game POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:donut-example-game> $<TARGET_FILE_DIR:donut-example-game> COMMAND_EXPAND_LISTS)
		add_custom_command(TARGET donut-example-rectangle POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:donut-example-rectangle> $<TARGET_FILE_DIR:donut-example-rectangle> COMMAND_EXPAND_LISTS)
	endif()
endif()
