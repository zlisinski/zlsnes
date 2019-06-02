find_package(Threads REQUIRED)

set(CORE_SRC
    Cpu.cpp
    Emulator.cpp
    Logger.cpp
    Memory.cpp
)

add_library(zlsnes_core
    ${CORE_SRC}
)
target_link_libraries(zlsnes_core
    Threads::Threads
)

add_subdirectory(tests)