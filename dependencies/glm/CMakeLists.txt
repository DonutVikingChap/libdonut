message(STATUS "Fetching glm...")

set(GLM_TEST_ENABLE OFF CACHE BOOL "" FORCE)

FetchContent_Declare(glm
	GIT_REPOSITORY https://github.com/g-truc/glm
	GIT_TAG bf71a834948186f4097caa076cd2663c69a10e1e # 0.9.9.8
)

FetchContent_MakeAvailable(glm)

target_compile_definitions(glm INTERFACE GLM_FORCE_XYZW_ONLY)

get_target_property(DONUT_GLM_INCLUDE_DIRS glm INTERFACE_INCLUDE_DIRECTORIES)
set_target_properties(glm PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${DONUT_GLM_INCLUDE_DIRS}")

set(DONUT_PUBLIC_DEPENDENCIES ${DONUT_PUBLIC_DEPENDENCIES} glm PARENT_SCOPE)
