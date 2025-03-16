include_directories(BEFORE
	${CMAKE_CURRENT_SOURCE_DIR}/mock
)
include_directories(
    ../../
)

find_package(Qt5 REQUIRED COMPONENTS Core)

add_executable(PpuTest
    PpuTest.cpp
    ../../Ppu.cpp
    ../../Logger.cpp
    ../../Utils.cpp
    ../CommonMocks/Memory.cpp
    ../CommonMocks/Timer.cpp
)

target_link_libraries(PpuTest
    gtest
    gtest_main
    Qt5::Core
)

add_test(NAME PpuTest COMMAND PpuTest)
set_property(TEST PpuTest PROPERTY WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")