include(GoogleTest)
add_executable(test_advent
		test_advent_01.cpp
		main.cpp)

target_link_libraries(test_advent
    PRIVATE
		Advent::Advent
    	Catch2::Catch2
		OpenCL::HeadersCpp
		range-v3::range-v3
)
