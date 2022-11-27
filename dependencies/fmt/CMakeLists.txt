message(STATUS "Fetching fmt...")

FetchContent_Declare(fmt
	GIT_REPOSITORY https://github.com/fmtlib/fmt
	GIT_TAG a33701196adfad74917046096bf5a2aa0ab0bb50 # 9.1.0
)

FetchContent_MakeAvailable(fmt)

get_target_property(DONUT_FMT_INCLUDE_DIRS fmt INTERFACE_INCLUDE_DIRECTORIES)
set_target_properties(fmt PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${DONUT_FMT_INCLUDE_DIRS}")

set(DONUT_PRIVATE_DEPENDENCIES ${DONUT_PRIVATE_DEPENDENCIES} fmt PARENT_SCOPE)
