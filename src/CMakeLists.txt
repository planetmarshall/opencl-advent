function(ADD_PROBLEM NAME)
    add_executable(${NAME} ${NAME}.cpp)
    target_compile_features(${NAME} PRIVATE cxx_std_20)
    target_link_libraries(${NAME}
            PRIVATE
            OpenCL::HeadersCpp
            range-v3::range-v3
            spdlog::spdlog
            )
endfunction()

add_problem(advent_01)
add_problem(advent_02)
