find_package(Threads REQUIRED)

set(CORE_SRC
    Apu.cpp
    Cartridge.cpp
    Cpu.cpp
    Dma.cpp
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

add_subdirectory(Audio)
