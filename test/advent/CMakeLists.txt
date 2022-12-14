include(GoogleTest)
add_executable(test_advent
		test_advent_01.cpp)

target_link_libraries(test_advent
    PRIVATE
    Catch2::Catch2WithMain
    Advent::Advent
)
