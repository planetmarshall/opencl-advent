add_custom_command(
        OUTPUT advent_01.cl.bin
        COMMAND clang -cl-std=clc++2021 -c advent_01.cl
        DEPENDS advent_01.cl
        COMMENT "Build OpenCL for C++ Kernel Code"
)


