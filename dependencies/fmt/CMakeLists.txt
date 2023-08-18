message(STATUS "Fetching {fmt}...")

FetchContent_Declare(fmt
	GIT_REPOSITORY https://github.com/fmtlib/fmt
	GIT_TAG e57ca2e3685b160617d3d95fcd9e789c4e06ca88 # 10.1.0
)

FetchContent_MakeAvailable(fmt)

get_target_property(DONUT_FMT_INCLUDE_DIRS fmt INTERFACE_INCLUDE_DIRECTORIES)
set_target_properties(fmt PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${DONUT_FMT_INCLUDE_DIRS}")
set(DONUT_PUBLIC_DEPENDENCIES ${DONUT_PUBLIC_DEPENDENCIES} fmt PARENT_SCOPE)
