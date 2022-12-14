add_executable(adventapp main.cpp)

target_link_libraries(adventapp
        PRIVATE
            Advent::Advent
            Boost::program_options
            OpenCL::OpenCL
        )

install(TARGETS adventapp RUNTIME DESTINATION bin)
