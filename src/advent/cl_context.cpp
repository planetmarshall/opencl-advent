#include <CL/cl.h>

#include <cl/context.hpp>
#include <fmt/format.h>

#include <array>
#include <bit>
#include <fstream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using cl::context;

namespace {

constexpr auto CL_DEVICE_CXX_FOR_OPENCL_NUMERIC_VERSION_EXT = 0x4230;

void check_ocl_function(cl_int result) {
    if (result != CL_SUCCESS) {
        throw std::runtime_error(fmt::format("Error in OpenCL: {}", result));
    }
}

std::vector<cl_platform_id> get_platform_ids() {
    cl_uint num_platforms;
    check_ocl_function(clGetPlatformIDs(0, nullptr, &num_platforms));
    auto platform_ids = std::vector<cl_platform_id>(num_platforms);
    check_ocl_function(clGetPlatformIDs(num_platforms, platform_ids.data(), nullptr));
    return platform_ids;
}

template<int Property>
std::string get_platform_property(cl_platform_id id) {
    size_t size;
    check_ocl_function(clGetPlatformInfo(id, Property, 0, nullptr, &size));
    std::vector<char> data(size);
    check_ocl_function(clGetPlatformInfo(id, Property, size, data.data(), nullptr));
    return std::string(data.begin(), data.end() - 1);
}

template<int DeviceType>
std::optional<cl_device_id> get_device(cl_platform_id id) {
    cl_uint num_devices;
    check_ocl_function(clGetDeviceIDs(id, DeviceType, 0, nullptr, &num_devices));
    if (num_devices == 0) {
        return std::nullopt;
    }
    std::array<cl_device_id, 1> device_ids{};
    check_ocl_function(clGetDeviceIDs(id, DeviceType, 1, device_ids.data(), nullptr));
    return device_ids[0];
}

template<int DeviceProperty, typename PropertyType>
PropertyType get_device_property(cl_device_id device_id) {
    size_t size;
    check_ocl_function(clGetDeviceInfo(device_id, DeviceProperty, 0, nullptr, &size));
    auto data = std::array<std::byte, sizeof(PropertyType)>{};
    check_ocl_function(clGetDeviceInfo(device_id, DeviceProperty, size, data.data(), nullptr));
    return std::bit_cast<PropertyType>(data);
}

template<int DeviceProperty>
std::string get_device_property(cl_device_id device_id) {
    size_t size;
    check_ocl_function(clGetDeviceInfo(device_id, DeviceProperty, 0, nullptr, &size));
    auto data = std::string(size, '\0');
    check_ocl_function(clGetDeviceInfo(device_id, DeviceProperty, size, data.data(), nullptr));
    return data;
}

}

context::context() {
    auto platform_ids = get_platform_ids();
    if (platform_ids.empty()) {
        throw std::runtime_error("No OpenCL platforms available");
    }
    m_platform_id = platform_ids[0];
    auto context_properties = std::array<cl_context_properties, 3>{
        CL_CONTEXT_PLATFORM,
        reinterpret_cast<cl_context_properties>(m_platform_id),
        0
    };

    cl_int result;
    m_context = clCreateContextFromType(context_properties.data(), CL_DEVICE_TYPE_GPU, nullptr, nullptr, &result);
    if (result != CL_SUCCESS) {
        m_context = clCreateContextFromType(context_properties.data(), CL_DEVICE_TYPE_CPU, nullptr, nullptr, &result);
        m_device_type = device_type::Cpu;
        if (result != CL_SUCCESS) {
            throw std::runtime_error("Could not create OpenCL GPU or CPU context");
        }
    } else {
        m_device_type = device_type::Gpu;
    }


    size_t device_buffer_size = 0;
    result = clGetContextInfo(m_context, CL_CONTEXT_DEVICES, 0, nullptr, &device_buffer_size);
    if (result != CL_SUCCESS || device_buffer_size == 0) {
        throw std::runtime_error("No devices available");
    }
    auto devices = std::vector<cl_device_id>(device_buffer_size / sizeof(cl_device_id));
    check_ocl_function(clGetContextInfo(m_context, CL_CONTEXT_DEVICES, device_buffer_size, devices.data(), nullptr));
    m_device = devices[0];

    m_command_queue = clCreateCommandQueueWithProperties(m_context, devices[0], nullptr, &result);

    m_platform_name = get_platform_property<CL_PLATFORM_NAME>(m_platform_id);
    m_platform_vendor = get_platform_property<CL_PLATFORM_VENDOR>(m_platform_id);
    m_platform_profile = get_platform_property<CL_PLATFORM_PROFILE>(m_platform_id);
    m_platform_version = get_platform_property<CL_PLATFORM_VERSION>(m_platform_id);

}

cl_program cl::context::create_program(const fs::path & source_file) {
    if (!fs::exists(source_file)) {
        throw std::runtime_error(fmt::format("source file doe not exist: {}", source_file.string()));
    }
    auto is = std::ifstream(source_file, std::ios::binary);
    //auto oss = std::ostringstream();
    //oss << is.rdbuf();
    //auto source_str = oss.str();

    cl_int ret;
    //auto sources = std::array<const char *, 1>{source_str.data()};
    //auto lengths = std::array<size_t, 1>{source_str.size()};
    //auto program = clCreateProgramWithSource(m_context, 1, sources.data(), lengths.data(), &ret);

    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(is), {});

    //auto program = clCreateProgramWithIL(m_context, buffer.data(), buffer.size(), &ret);
    std::array<cl_device_id , 1> device_list{m_device};
    auto binaries = std::array<const unsigned char *, 1>{buffer.data()};
    auto binary_size = std::array<size_t, 1>{buffer.size()};
    cl_int status;
    auto program = clCreateProgramWithBinary(m_context, 1, device_list.data(), binary_size.data(), binaries.data(), &status, &ret);
    check_ocl_function(ret);

    //ret = clBuildProgram(program, 0, nullptr, "-cl-std=clc++2021", nullptr, nullptr);
    //ret = clBuildProgram(program, 0, nullptr, nullptr, nullptr, nullptr);
    auto programs = std::array<cl_program, 1>{program};
    auto linked_program = clLinkProgram(m_context, 0, nullptr, "-lclc", 1, programs.data(), nullptr, nullptr, &ret);
    if (ret != CL_SUCCESS) {
        size_t build_log_size = 0;
        clGetProgramBuildInfo(program, m_device, CL_PROGRAM_BUILD_LOG_LOG, 0, nullptr, &build_log_size);
        std::vector<char> build_log(build_log_size);
        clGetProgramBuildInfo(program, m_device, CL_PROGRAM_BUILD_LOG, build_log.size(), build_log.data(), nullptr);
        clReleaseProgram(program);
        throw std::runtime_error(std::string(build_log.begin(), build_log.end()));
    }
    //std::array<size_t, 1> program_sizes{1};
    //check_ocl_function(clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, sizeof(size_t), program_sizes.data(), nullptr));
    //std::vector<unsigned char> program_data(program_sizes[0]);
    //std::array<unsigned char*, 1> programs{program_data.data()};
    //clGetProgramInfo(program, CL_PROGRAM_BINARIES, program_sizes[0], programs.data(), nullptr);
    //auto fp = fopen("advent_01_out.cl.ptx", "wb");
    //fwrite(programs[0], 1, program_sizes[0], fp);
    //fclose(fp);

    return program;
}
