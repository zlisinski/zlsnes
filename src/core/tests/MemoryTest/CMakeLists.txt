include_directories(BEFORE
	${CMAKE_CURRENT_SOURCE_DIR}/mock
)
include_directories(
    ../../
)

find_package(Qt5 REQUIRED COMPONENTS Core)

add_executable(MemoryTest
    MemoryTest.cpp
    ../../Cartridge.cpp
    ../../Logger.cpp
    ../../Memory.cpp
    ../../Ppu.cpp
    ../../Utils.cpp
    ../CommonMocks/Timer.cpp
)

target_link_libraries(MemoryTest
    gtest
    gtest_main
    Qt5::Core
)

add_test(NAME MemoryTest COMMAND MemoryTest)
set_property(TEST MemoryTest PROPERTY WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")