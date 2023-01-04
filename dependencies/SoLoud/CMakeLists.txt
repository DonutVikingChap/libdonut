message(STATUS "Fetching SoLoud...")

FetchContent_Declare(
    SoLoud
    GIT_REPOSITORY https://github.com/jarikomppa/soloud
    GIT_TAG 1157475881da0d7f76102578255b937c7d4e8f57 # 2021-11-24
)

FetchContent_MakeAvailable(SoLoud)

add_library(SoLoud STATIC
    "${soloud_SOURCE_DIR}/src/audiosource/wav/dr_impl.cpp"
    "${soloud_SOURCE_DIR}/src/audiosource/wav/soloud_wav.cpp"
    "${soloud_SOURCE_DIR}/src/audiosource/wav/soloud_wavstream.cpp"
    "${soloud_SOURCE_DIR}/src/audiosource/wav/stb_vorbis.c"
    "${soloud_SOURCE_DIR}/src/backend/sdl2_static/soloud_sdl2_static.cpp"
    "${soloud_SOURCE_DIR}/src/core/soloud.cpp"
    "${soloud_SOURCE_DIR}/src/core/soloud_audiosource.cpp"
    "${soloud_SOURCE_DIR}/src/core/soloud_bus.cpp"
    "${soloud_SOURCE_DIR}/src/core/soloud_core_3d.cpp"
    "${soloud_SOURCE_DIR}/src/core/soloud_core_basicops.cpp"
    "${soloud_SOURCE_DIR}/src/core/soloud_core_faderops.cpp"
    "${soloud_SOURCE_DIR}/src/core/soloud_core_filterops.cpp"
    "${soloud_SOURCE_DIR}/src/core/soloud_core_getters.cpp"
    "${soloud_SOURCE_DIR}/src/core/soloud_core_setters.cpp"
    "${soloud_SOURCE_DIR}/src/core/soloud_core_voicegroup.cpp"
    "${soloud_SOURCE_DIR}/src/core/soloud_core_voiceops.cpp"
    "${soloud_SOURCE_DIR}/src/core/soloud_fader.cpp"
    "${soloud_SOURCE_DIR}/src/core/soloud_fft.cpp"
    "${soloud_SOURCE_DIR}/src/core/soloud_fft_lut.cpp"
    "${soloud_SOURCE_DIR}/src/core/soloud_file.cpp"
    "${soloud_SOURCE_DIR}/src/core/soloud_filter.cpp"
    "${soloud_SOURCE_DIR}/src/core/soloud_misc.cpp"
    "${soloud_SOURCE_DIR}/src/core/soloud_queue.cpp"
    "${soloud_SOURCE_DIR}/src/core/soloud_thread.cpp")
target_include_directories(SoLoud PUBLIC "${soloud_SOURCE_DIR}/include")
target_compile_definitions(SoLoud PRIVATE WITH_SDL2_STATIC)
target_compile_features(SoLoud PRIVATE c_std_99 cxx_std_20)
target_compile_options(SoLoud PRIVATE
	$<$<OR:$<C_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:GNU>>:        $<$<COMPILE_LANGUAGE:C>:-std=c99>   $<$<COMPILE_LANGUAGE:CXX>:-std=c++20>           $<$<CONFIG:Debug>:-g>   $<$<CONFIG:Release>:-O3>    $<$<CONFIG:MinSizeRel>:-Os> $<$<CONFIG:RelWithDebInfo>:-O3 -g>>
	$<$<OR:$<C_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:Clang>>:    $<$<COMPILE_LANGUAGE:C>:-std=c99>   $<$<COMPILE_LANGUAGE:CXX>:-std=c++20>           $<$<CONFIG:Debug>:-g>   $<$<CONFIG:Release>:-O3>    $<$<CONFIG:MinSizeRel>:-Os> $<$<CONFIG:RelWithDebInfo>:-O3 -g>>
	$<$<OR:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>:      $<$<COMPILE_LANGUAGE:C>:/std:c99>   $<$<COMPILE_LANGUAGE:CXX>:/std:c++20>   /utf-8  $<$<CONFIG:Debug>:/Od>  $<$<CONFIG:Release>:/Ot>    $<$<CONFIG:MinSizeRel>:/Os> $<$<CONFIG:RelWithDebInfo>:/Ot /Od>>)
target_link_libraries(SoLoud PRIVATE SDL2-static)

set(DONUT_PRIVATE_DEPENDENCIES ${DONUT_PRIVATE_DEPENDENCIES} SoLoud PARENT_SCOPE)
