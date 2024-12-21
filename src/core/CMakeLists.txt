find_package(Threads REQUIRED)

set(CORE_SRC
    Cartridge.cpp
    Cpu.cpp
    Emulator.cpp
    Input.cpp
    Interrupt.cpp
    Logger.cpp
    Memory.cpp
    Ppu.cpp
    Timer.cpp
    Utils.cpp
)

add_library(zlsnes_core
    ${CORE_SRC}
)
target_link_libraries(zlsnes_core
    Threads::Threads
)

enable_testing()

add_subdirectory(tests)
add_compile_definitions("TESTING")