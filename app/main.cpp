#include <advent/advent.hpp>
#include <CL/cl.h>

#include <boost/program_options.hpp>

#include <iostream>

namespace po = boost::program_options;

template<int Property>
std::string get_cl_platform_property(cl_platform_id id) {
    size_t size;
    auto result = clGetPlatformInfo(id, Property, 0, nullptr, &size);
    std::vector<char> data(size);
    result = clGetPlatformInfo(id, Property, size, data.data(), nullptr);
    return std::string(data.begin(), data.end() - 1);
}

template<int DeviceType>
std::optional<cl_device_id> get_cl_device(cl_platform_id id) {
    cl_uint num_devices;
    auto result = clGetDeviceIDs(id, DeviceType, 0, nullptr, &num_devices);
    if (num_devices == 0) {
        return std::nullopt;
    }
    std::array<cl_device_id, 1> device_ids;
    result = clGetDeviceIDs(id, DeviceType, 1, device_ids.data(), nullptr);
    return device_ids[0];
}

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

    cl_int CL_err = CL_SUCCESS;
    cl_uint num_platforms = 0;

    CL_err = clGetPlatformIDs(0, NULL, &num_platforms);

    if (CL_err == CL_SUCCESS) {
        std::cout << num_platforms << " found" << "\n";
    }

    auto platform_ids = std::vector<cl_platform_id>(num_platforms);
    clGetPlatformIDs(num_platforms, platform_ids.data(), nullptr);

    for (auto id : platform_ids) {
        std::cout << "Platform Name: " << get_cl_platform_property<CL_PLATFORM_NAME>(id) << "\n";
        std::cout << "Vendor Name: " << get_cl_platform_property<CL_PLATFORM_VENDOR>(id) << "\n";
        std::cout << "Profile: " << get_cl_platform_property<CL_PLATFORM_PROFILE>(id) << "\n";
        std::cout << "Version: " << get_cl_platform_property<CL_PLATFORM_VERSION>(id) << "\n";
    }

    auto gpu_device = get_cl_device<CL_DEVICE_TYPE_GPU>(platform_ids[0]);
    std::cout << "Number of compute units: " << get_cl_device_property<CL_DEVICE_MAX_COMPUTE_UNITS, cl_uint>(*gpu_device) << '\n';


    return 0;
}
