message(STATUS "Fetching {fmt}...")

FetchContent_Declare(fmt
	GIT_REPOSITORY https://github.com/fmtlib/fmt
	GIT_TAG 40626af88bd7df9a5fb80be7b25ac85b122d6c21 # 11.2.0
)

FetchContent_MakeAvailable(fmt)

get_target_property(DONUT_FMT_INCLUDE_DIRS fmt INTERFACE_INCLUDE_DIRECTORIES)
set_target_properties(fmt PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${DONUT_FMT_INCLUDE_DIRS}")
set(DONUT_PUBLIC_DEPENDENCIES ${DONUT_PUBLIC_DEPENDENCIES} fmt PARENT_SCOPE)
