add_executable(advent_01 main.cpp)

target_compile_features(advent_01 PRIVATE cxx_std_20)
target_compile_definitions(advent_01 PRIVATE CL_HPP_ENABLE_EXTENSIONS)

target_link_libraries(advent_01
        PRIVATE
        OpenCL::HeadersCpp
        range-v3::range-v3
        spdlog::spdlog
        )

set(CL_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/cl/advent_01.cl)
set(CL_DEVICE_LIB_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/cl/device.cl)

set(CL_TMP ${CMAKE_CURRENT_BINARY_DIR}/cl/advent_01.cl.bc)
set(CL_DEVICE_LIB_DEST ${CMAKE_CURRENT_BINARY_DIR}/cl/device.cl.bc)

set(CL_LINK_DEST ${CMAKE_CURRENT_BINARY_DIR}/cl/linked.bc)

set(CL_DEST $<TARGET_FILE_DIR:advent_01>/advent_01.cl.ptx)
add_custom_command(
        TARGET advent_01 PRE_BUILD
        DEPENDS ${CL_SOURCE}
        COMMAND clang -target nvptx64-unknown-unknown -cl-std=CL3.0 -emit-llvm ${CL_SOURCE} -S -o ${CL_TMP}
        COMMAND clang -target nvptx64-unknown-unknown -cl-std=CL3.0 -emit-llvm ${CL_DEVICE_LIB_SOURCE} -S -o ${CL_DEVICE_LIB_DEST}
        COMMAND llvm-link ${CL_DEVICE_LIB_DEST} ${CL_TMP} -o ${CL_LINK_DEST}
        COMMAND clang -target nvptx64-unknown-unknown ${CL_LINK_DEST} -S -o ${CL_DEST}
        VERBATIM)
