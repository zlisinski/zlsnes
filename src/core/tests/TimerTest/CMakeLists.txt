include_directories(BEFORE
	${CMAKE_CURRENT_SOURCE_DIR}/mock
)
include_directories(
    ../../
)

find_package(Qt5 REQUIRED COMPONENTS Core)

add_executable(TimerTest
    TimerTest.cpp
    ../../Logger.cpp
    ../../Timer.cpp
    ../../Utils.cpp
    ../CommonMocks/Apu.cpp
    ../CommonMocks/Interrupt.cpp
    ../CommonMocks/Memory.cpp
)

target_link_libraries(TimerTest
    gtest
    gtest_main
    Qt5::Core
)

add_test(NAME TimerTest COMMAND TimerTest)
set_property(TEST TimerTest PROPERTY WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

add_compile_definitions("TESTING")