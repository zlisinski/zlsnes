include_directories(BEFORE
	${CMAKE_CURRENT_SOURCE_DIR}/mock
)
include_directories(
    ../../
)

find_package(Qt5 REQUIRED COMPONENTS Core)

add_executable(AddressModeTest
    AddressModeTest.cpp
    ../../Cpu.cpp
    ../../Dma.cpp
    ../../Interrupt.cpp
    ../../Logger.cpp
    ../../Utils.cpp
    ../CommonMocks/Memory.cpp
    ../CommonMocks/Timer.cpp
)

target_link_libraries(AddressModeTest
    gtest
    gtest_main
    Qt5::Core
)

add_test(NAME AddressModeTest COMMAND AddressModeTest)
set_property(TEST AddressModeTest PROPERTY WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")