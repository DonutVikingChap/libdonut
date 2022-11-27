message(STATUS "Fetching stb...")

FetchContent_Declare(
	stb
	GIT_REPOSITORY https://github.com/nothings/stb
	GIT_TAG 8b5f1f37b5b75829fc72d38e7b5d4bcbf8a26d55 # stb_image: 2.27x, stb_image_write: 1.16
)

FetchContent_MakeAvailable(stb)

add_library(stb STATIC "${CMAKE_CURRENT_SOURCE_DIR}/stb_image_write.c" "${CMAKE_CURRENT_SOURCE_DIR}/stb_image.c")
target_include_directories(stb PUBLIC "${stb_SOURCE_DIR}")
target_compile_features(stb PRIVATE c_std_99)
target_compile_options(stb PRIVATE
	$<$<C_COMPILER_ID:GNU>:     -std=c99            $<$<CONFIG:Debug>:-g>   $<$<CONFIG:Release>:-O3 -flto>  $<$<CONFIG:MinSizeRel>:-Os -flto>   $<$<CONFIG:RelWithDebInfo>:-O3 -g -flto>>
	$<$<C_COMPILER_ID:Clang>:   -std=c99            $<$<CONFIG:Debug>:-g>   $<$<CONFIG:Release>:-O3 -flto>  $<$<CONFIG:MinSizeRel>:-Os -flto>   $<$<CONFIG:RelWithDebInfo>:-O3 -g -flto>>
	$<$<C_COMPILER_ID:MSVC>:    /std:c99    /utf-8  $<$<CONFIG:Debug>:/Od>  $<$<CONFIG:Release>:/Ot>        $<$<CONFIG:MinSizeRel>:/Os>         $<$<CONFIG:RelWithDebInfo>:/Ot /Od>>)

set(DONUT_PRIVATE_DEPENDENCIES ${DONUT_PRIVATE_DEPENDENCIES} stb PARENT_SCOPE)
