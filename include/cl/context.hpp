#include <CL/cl.h>

#include <filesystem>
#include <string>
#include <string_view>

namespace cl {
enum class device_type {
    Gpu = CL_DEVICE_TYPE_GPU,
    Cpu = CL_DEVICE_TYPE_CPU
};

constexpr std::string_view to_string(device_type type) {
    switch (type) {
    case device_type::Gpu:
        return "GPU";
    case device_type::Cpu:
        return "CPU";
    default:
        throw std::runtime_error("No such device type");
    }
}


class context {
  public:
    context();

    [[nodiscard]]
    device_type get_device_type() const { return m_device_type; }

    [[nodiscard]]
    std::string_view get_platform_name() const { return m_platform_name; }
    [[nodiscard]]
    std::string_view get_platform_profile() const { return m_platform_profile; }
    [[nodiscard]]
    std::string_view get_platform_vendor() const { return m_platform_vendor; }
    [[nodiscard]]
    std::string_view get_platform_version() const { return m_platform_version; }

    cl_program create_program(const std::filesystem::path & source_file);

  private:
    cl_platform_id m_platform_id;
    cl_context m_context;
    cl_command_queue m_command_queue;
    cl_device_id m_device;
    device_type m_device_type;
    std::string m_platform_name;
    std::string m_platform_vendor;
    std::string m_platform_profile;
    std::string m_platform_version;
};
}
