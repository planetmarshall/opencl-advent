#include <catch2/catch_session.hpp>
#include <CL/opencl.hpp>

#include <range/v3/algorithm.hpp>

#include <iostream>
#include <vector>

namespace v3 = ranges;

int main( int argc, char* argv[] ) {
    auto platforms = std::vector<cl::Platform>();
    cl::Platform::get(&platforms);

    auto ocl_3_platform = v3::find_if(platforms, [] (const auto & platform) {
      auto platver = platform. template getInfo<CL_PLATFORM_VERSION>();
      return platver.find("OpenCL 3.") != std::string::npos;
    });

    if (ocl_3_platform == platforms.end()) {
        std::cout << "No OpenCL 3.0 or newer platform found.\n";
        return -1;
    }

    std::cout << "Open CL Platform Name: " << ocl_3_platform->getInfo<CL_PLATFORM_NAME>() << '\n';
    std::cout << "Open CL Platform Version: " << ocl_3_platform->getInfo<CL_PLATFORM_VERSION>() << '\n';

    return Catch::Session().run( argc, argv );
}