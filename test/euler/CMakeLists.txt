include(GoogleTest)
add_executable(test_euler
    test_problem_1.cpp)

target_link_libraries(test_euler
    PRIVATE
    Catch2::Catch2WithMain
    Euler::Euler
)

include(CTest)
include(Catch)

if (NOT CMAKE_CROSSCOMPILING)
	catch_discover_tests(test_euler)
endif()
