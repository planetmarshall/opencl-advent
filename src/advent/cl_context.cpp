#include <CL/cl.h>
#include <cl/context.hpp>

#include <array>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

using cl::context;

namespace {
void check_ocl_function(cl_int result) {
    if (result != CL_SUCCESS) {
        throw std::runtime_error("Error in OpenCL");
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
    m_device = devices[0];
    clGetContextInfo(m_context, CL_CONTEXT_DEVICES, device_buffer_size, devices.data(), nullptr);

    m_command_queue = clCreateCommandQueueWithProperties(m_context, devices[0], nullptr, &result);

    m_platform_name = get_platform_property<CL_PLATFORM_NAME>(m_platform_id);
    m_platform_vendor = get_platform_property<CL_PLATFORM_VENDOR>(m_platform_id);
    m_platform_profile = get_platform_property<CL_PLATFORM_PROFILE>(m_platform_id);
    m_platform_version = get_platform_property<CL_PLATFORM_VERSION>(m_platform_id);

}