#include <cl/context.hpp>
#include <advent/advent.hpp>

#include <boost/program_options.hpp>
#include <CL/cl.h>

#include <iostream>

namespace po = boost::program_options;



template<int DeviceProperty, typename PropertyT>
PropertyT get_cl_device_property(cl_device_id id) {
    PropertyT prop;
    size_t size;
    auto result = clGetDeviceInfo(id, DeviceProperty, sizeof(PropertyT), &prop, &size);
    return prop;
}

int main(int argc, char **argv) {
    po::options_description description("Advent of Code Problem 1 <https://projecteuler.net/>");
    std::string input_file_name;
    description.add_options()
        ("help", "show help message")
        ("input", po::value<std::string>(&input_file_name), "input file")
    ;
    po::variables_map options;
    po::store(po::parse_command_line(argc, argv, description), options);
    po::notify(options);


    auto context = cl::context();

    return 0;
}
