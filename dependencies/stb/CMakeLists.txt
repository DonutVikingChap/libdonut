message(STATUS "Fetching stb...")

FetchContent_Declare(
	stb
	GIT_REPOSITORY https://github.com/nothings/stb
	GIT_TAG f58f558c120e9b32c217290b80bad1a0729fbb2c # stb_image: 2.30, stb_image_write: 1.16
)

FetchContent_MakeAvailable(stb)

add_library(stb STATIC "${CMAKE_CURRENT_SOURCE_DIR}/stb_image_write.c" "${CMAKE_CURRENT_SOURCE_DIR}/stb_image.c")
target_include_directories(stb PUBLIC "${stb_SOURCE_DIR}")
target_compile_features(stb PRIVATE c_std_99)
target_compile_options(stb PRIVATE
	$<$<C_COMPILER_ID:GNU>:     -std=c99            $<$<CONFIG:Debug>:-g3>  $<$<CONFIG:Release>:-O3>    $<$<CONFIG:MinSizeRel>:-Os> $<$<CONFIG:RelWithDebInfo>:-O3 -g3>>
	$<$<C_COMPILER_ID:Clang>:   -std=c99            $<$<CONFIG:Debug>:-g3>  $<$<CONFIG:Release>:-O3>    $<$<CONFIG:MinSizeRel>:-Os> $<$<CONFIG:RelWithDebInfo>:-O3 -g3>>
	$<$<C_COMPILER_ID:MSVC>:    /std:c99    /utf-8  $<$<CONFIG:Debug>:/Od>  $<$<CONFIG:Release>:/Ot>    $<$<CONFIG:MinSizeRel>:/Os> $<$<CONFIG:RelWithDebInfo>:/Ot /Od>>)

set(DONUT_PRIVATE_DEPENDENCIES ${DONUT_PRIVATE_DEPENDENCIES} stb PARENT_SCOPE)
