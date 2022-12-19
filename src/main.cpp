#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 200

#include <CL/opencl.hpp>
#include <fmt/format.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

namespace fs = std::filesystem;

namespace {
    std::tuple<cl::coarse_svm_vector<int>, cl::coarse_svm_vector<int>, cl::coarse_svm_vector<int>>
    load_data(const fs::path & data_file) {
        if (!fs::exists(data_file)) {
            throw std::runtime_error(fmt::format("data file {} does not exist", data_file.string()));
        }
        auto fs = std::ifstream(data_file);
        cl::SVMAllocator<int, cl::SVMTraitCoarse<>> svm_alloc;
        cl::coarse_svm_vector<int> values(svm_alloc);
        cl::coarse_svm_vector<int> offsets(svm_alloc);
        cl::coarse_svm_vector<int> counts(svm_alloc);
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
    cl::coarse_svm_vector<int> calories;
    cl::coarse_svm_vector<int> offsets;
    cl::coarse_svm_vector<int> counts;
    std::tie(calories, offsets, counts) = load_data(argv[1]);

    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    cl::Platform plat;
    for (auto &p : platforms) {
        std::string platver = p.getInfo<CL_PLATFORM_VERSION>();
        if (platver.find("OpenCL 2.") != std::string::npos ||
            platver.find("OpenCL 3.") != std::string::npos) {
            // Note: an OpenCL 3.x platform may not support all required features!
            plat = p;
            std::cout << p.getInfo<CL_PLATFORM_NAME>() << '\n';
            break;
        }
    }
    if (plat() == 0) {
        std::cout << "No OpenCL 2.0 or newer platform found.\n";
        return -1;
    }

    cl::Platform newP = cl::Platform::setDefault(plat);
    if (newP != plat) {
        std::cout << "Error setting default platform.\n";
        return -1;
    }

    std::string kernel{R"CLC(
        kernel void count_calories(global const int *calories, global const int *offsets, global const int *counts, global int *partials)
        {
          int index = get_global_id(0);
          int offset = offsets[index];
          int count = counts[index];
          partials[index] = 1;
          printf("offset[%d]: %d\n", index, offsets[index]);
          printf("count[i]: %d\n", counts[index]);
          for (int i = offset; i < offset + count; ++i) {
            partials[index] += calories[i];
          }
        }
    )CLC"};

    std::vector<std::string> program_sources;
    program_sources.push_back(kernel);

    cl::Program count_calories_program(program_sources);
    try {
        count_calories_program.build("-cl-std=CL3.0");
    }
    catch (...) {
        // Print build info for all devices
        cl_int buildErr = CL_SUCCESS;
        auto buildInfo = count_calories_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(&buildErr);
        for (auto &pair : buildInfo) {
            std::cerr << pair.second << std::endl << std::endl;
        }

        return 1;
    }

    //cl::SVMAllocator<int, cl::SVMTraitCoarse<>> svmAlloc;
    //std::vector<int, cl::SVMAllocator<int, cl::SVMTraitCoarse<>>> inputA(numElements, 1, svmAlloc);
    cl::coarse_svm_vector<int> partials(offsets.size());

    for (auto v : offsets) {
        std::cout << v << "\n";
    }
    auto count_calories_kernel =
        cl::KernelFunctor<
            cl::coarse_svm_vector<int>&,
            cl::coarse_svm_vector<int>&,
            cl::coarse_svm_vector<int>&,
            cl::coarse_svm_vector<int>&
            >(count_calories_program, "count_calories");

    cl_int error;
    count_calories_kernel(
        cl::EnqueueArgs(cl::NDRange(offsets.size())),
        calories,
        offsets,
        counts,
        partials,
        error
    );

    //cl::copy(outputBuffer, begin(output), end(output));

    std::cout << "Output:\n";
    for (const auto partial : partials) {
        std::cout << "\t" << partial << '\n';
    }
    std::cout << "\n\n";

    return 0;
}