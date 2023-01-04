message(STATUS "Fetching schrift...")

FetchContent_Declare(schrift
	GIT_REPOSITORY https://github.com/tomolt/libschrift
	GIT_TAG 8e533fd07acc2f8ae4cffe7f95d2c3392773e2b5 # 0.10.2
)

FetchContent_MakeAvailable(schrift)

add_library(schrift STATIC "${schrift_SOURCE_DIR}/schrift.c")
target_include_directories(schrift PUBLIC "${schrift_SOURCE_DIR}")
target_compile_features(schrift PRIVATE c_std_99)
target_compile_options(schrift PRIVATE
	$<$<C_COMPILER_ID:GNU>:     -std=c99            $<$<CONFIG:Debug>:-g>   $<$<CONFIG:Release>:-O3>    $<$<CONFIG:MinSizeRel>:-Os> $<$<CONFIG:RelWithDebInfo>:-O3 -g>>
	$<$<C_COMPILER_ID:Clang>:   -std=c99            $<$<CONFIG:Debug>:-g>   $<$<CONFIG:Release>:-O3>    $<$<CONFIG:MinSizeRel>:-Os> $<$<CONFIG:RelWithDebInfo>:-O3 -g>>
	$<$<C_COMPILER_ID:MSVC>:    /std:c99    /utf-8  $<$<CONFIG:Debug>:/Od>  $<$<CONFIG:Release>:/Ot>    $<$<CONFIG:MinSizeRel>:/Os> $<$<CONFIG:RelWithDebInfo>:/Ot /Od>>)

set(DONUT_PRIVATE_DEPENDENCIES ${DONUT_PRIVATE_DEPENDENCIES} schrift PARENT_SCOPE)
