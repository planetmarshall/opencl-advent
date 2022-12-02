set(EULER_API_HEADERS ${CMAKE_SOURCE_DIR}/include/euler/euler.hpp)
add_library(euler
        problem_1.cpp
        ${EULER_API_HEADERS}
        )

target_compile_features(euler PUBLIC cxx_std_20)
set_target_properties(euler PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
add_library(Euler::Euler ALIAS euler)
target_link_libraries(euler PRIVATE range-v3::range-v3)

target_include_directories(
    euler
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

install(
        TARGETS euler
        EXPORT SampleTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
        )

install(
        DIRECTORY ${CMAKE_SOURCE_DIR}/include/euler
        DESTINATION include
        FILES_MATCHING PATTERN *.h
)
