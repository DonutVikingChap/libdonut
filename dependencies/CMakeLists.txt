include(FetchContent)

add_subdirectory(fmt)
add_subdirectory(glad)
add_subdirectory(glm)
add_subdirectory(physfs)
add_subdirectory(schrift)
add_subdirectory(SDL)
add_subdirectory(SoLoud) # Depends on SDL.
add_subdirectory(stb)

set(DONUT_INTERFACE_DEPENDENCIES ${DONUT_INTERFACE_DEPENDENCIES} PARENT_SCOPE)
set(DONUT_PUBLIC_DEPENDENCIES ${DONUT_PUBLIC_DEPENDENCIES} PARENT_SCOPE)
set(DONUT_PRIVATE_DEPENDENCIES ${DONUT_PRIVATE_DEPENDENCIES} PARENT_SCOPE)
