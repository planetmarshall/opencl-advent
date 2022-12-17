set(ADVENT_API_HEADERS
        ${CMAKE_SOURCE_DIR}/include/advent/advent.hpp)

add_library(advent
        advent_01.cpp
        ${ADVENT_API_HEADERS}
        )

target_compile_features(advent PUBLIC cxx_std_20)
set_target_properties(advent PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
add_library(Advent::Advent ALIAS advent)
target_link_libraries(advent
        PRIVATE
                OpenCL::OpenCL
                fmt::fmt
        PUBLIC
                OpenCL::Headers)

target_include_directories(
    advent
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

install(
        TARGETS advent
        EXPORT AdventTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
        )

install(
        DIRECTORY ${CMAKE_SOURCE_DIR}/include/advent
        DESTINATION include
        FILES_MATCHING PATTERN *.hpp
)
