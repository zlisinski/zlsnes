add_subdirectory(core)
add_subdirectory(ui)

set(MAIN_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    PARENT_SCOPE
)