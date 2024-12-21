include_directories(BEFORE
	${CMAKE_CURRENT_SOURCE_DIR}/mock
)
include_directories(
    ../../
)

find_package(Qt5 REQUIRED COMPONENTS Core)

add_executable(CpuTest
    CpuTest.cpp
    ../../Cpu.cpp
    ../../Interrupt.cpp
    ../../Logger.cpp
    ../../Utils.cpp
    mock/Memory.cpp
    mock/Timer.cpp
)

target_link_libraries(CpuTest
    gtest
    gtest_main
    Qt5::Core
)

add_test(NAME CpuTest COMMAND CpuTest)
set_property(TEST CpuTest PROPERTY WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

add_compile_definitions("TESTING")