cmake_minimum_required(VERSION 3.1.0)
project(ZLSNES)

set(CMAKE_BUILD_TYPE RelWithDebInfo)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Winline")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Output executables in the current dir, instead of the build dir.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}")

add_subdirectory(src)

add_executable(zlsnes
    ${MAIN_SRC}
)
target_link_libraries(zlsnes
    zlsnes_core
    zlsnes_ui
)

enable_testing()