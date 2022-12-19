#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 300

#include <CL/opencl.hpp>
#include <fmt/format.h>
#include <range/v3/algorithm.hpp>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

namespace fs = std::filesystem;

namespace {
    std::tuple<std::vector<int>, std::vector<int>, std::vector<int>>
    load_data(const fs::path & data_file) {
        if (!fs::exists(data_file)) {
            throw std::runtime_error(fmt::format("data file {} does not exist", data_file.string()));
        }
        auto fs = std::ifstream(data_file);
        std::vector<int> values;
        std::vector<int> offsets;
        std::vector<int> counts;
        int offset = 0;
        for (std::string line; std::getline(fs, line); ) {
            if (!line.empty()) {
                values.emplace_back(std::atoi(line.c_str()));
            } else {
                offsets.emplace_back(offset);
                counts.emplace_back(values.size() - offset);
                offset = values.size();
            }
        }

        return {values, offsets, counts};
    }
}

int main(int argc, char **argv)
{
    const auto& [calories, offsets, counts] = load_data(argv[1]);
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    auto nvidia_platform = std::find_if(platforms.begin(), platforms.end(), [] (const auto & platform) {
      auto vendor = platform.template getInfo<CL_PLATFORM_NAME>();
      return vendor.find("NVIDIA") != std::string::npos;
    });

    if (nvidia_platform == platforms.end()) {
        spdlog::error("No OpenCL Platform available");
        return 1;
    }
    auto default_platform = cl::Platform::setDefault(*nvidia_platform);
    if (default_platform() != (*nvidia_platform)()) {
        spdlog::error("Error setting default platform");
        return 1;
    }
    spdlog::info("Using OpenCL Platform: {}", nvidia_platform->getInfo<CL_PLATFORM_NAME>());

    std::string kernel{R"CLC(
        kernel void count_calories(global const int* calories, global const int* offsets, global const int* counts, global int *partials)
        {
            int index = get_global_id(0);
            int offset = offsets[index];
            int count = counts[index];
            partials[index] = 0;
            for (int i = offset; i < offset + count; ++i) {
                partials[index] += calories[i];
            }
        }
    )CLC"};

    std::vector<std::string> program_sources;
    program_sources.push_back(kernel);

    cl::Program program(program_sources);
    try {
        program.build("-cl-std=CL3.0");
    }
    catch (...) {
        cl_int buildErr = CL_SUCCESS;
        auto buildInfo = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(&buildErr);
        for (auto &pair : buildInfo) {
            spdlog::error("Build log: {}", pair.second);
        }

        return 1;
    }

    auto count_calories_kernel = cl::KernelFunctor<cl::Buffer, cl::Buffer, cl::Buffer, cl::Buffer>(program, "count_calories");

    std::vector<int> output(counts.size());
    auto calories_buffer = cl::Buffer(calories.begin(), calories.end(), true);
    auto offsets_buffer = cl::Buffer(offsets.begin(), offsets.end(), true);
    auto counts_buffer = cl::Buffer(counts.begin(), counts.end(), true);
    auto output_buffer = cl::Buffer(output.begin(), output.end(), false);
    cl_int error;
    auto evt = count_calories_kernel(cl::EnqueueArgs(cl::NDRange(output.size())), calories_buffer, offsets_buffer, counts_buffer, output_buffer, error);
    if (error != CL_SUCCESS) {
        spdlog::error("Error running kernel");
    }

    cl::copy(output_buffer, output.begin(), output.end());
    auto max = ranges::max_element(output);
    spdlog::info("The elf carrying the maximum number of calories is elf {} with {} kcal", std::distance(output.begin(), max), *max);

    return 0;
}