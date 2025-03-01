include_directories(BEFORE
	${CMAKE_CURRENT_SOURCE_DIR}/mock
)
include_directories(
    ../../Audio/
    ../../
)

find_package(Qt5 REQUIRED COMPONENTS Core)

add_executable(Spc700Test
    Spc700Test.cpp
    ../../Audio/Spc700.cpp
    ../../Logger.cpp
    ../../Utils.cpp
    mock/Memory.cpp
)

target_link_libraries(Spc700Test
    gtest
    gtest_main
    Qt5::Core
)

add_test(NAME Spc700Test COMMAND Spc700Test)
set_property(TEST Spc700Test PROPERTY WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

add_compile_definitions("TESTING")