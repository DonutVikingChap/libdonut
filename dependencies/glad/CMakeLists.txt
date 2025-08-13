message(STATUS "Fetching glad...")

add_library(glad_330_core STATIC "${CMAKE_CURRENT_SOURCE_DIR}/src/gl.c")
target_include_directories(glad_330_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(glad_330_core PRIVATE c_std_99)
target_compile_options(glad_330_core PRIVATE
	$<$<C_COMPILER_ID:GNU>:     -std=c99            $<$<CONFIG:Debug>:-g3>  $<$<CONFIG:Release>:-O3>    $<$<CONFIG:MinSizeRel>:-Os> $<$<CONFIG:RelWithDebInfo>:-O3 -g3>>
	$<$<C_COMPILER_ID:Clang>:   -std=c99            $<$<CONFIG:Debug>:-g3>  $<$<CONFIG:Release>:-O3>    $<$<CONFIG:MinSizeRel>:-Os> $<$<CONFIG:RelWithDebInfo>:-O3 -g3>>
	$<$<C_COMPILER_ID:MSVC>:    /std:c99    /utf-8  $<$<CONFIG:Debug>:/Od>  $<$<CONFIG:Release>:/Ot>    $<$<CONFIG:MinSizeRel>:/Os> $<$<CONFIG:RelWithDebInfo>:/Ot /Od>>)

set(DONUT_PUBLIC_DEPENDENCIES ${DONUT_PUBLIC_DEPENDENCIES} glad_330_core PARENT_SCOPE)
