#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 300

#include <CL/opencl.hpp>
#include <fmt/format.h>
#include <range/v3/all.hpp>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>


namespace fs = std::filesystem;

namespace {
    std::pair<std::vector<char>, std::vector<char>> load_data(const fs::path & data_file) {
        if (!fs::exists(data_file)) {
            throw std::runtime_error(fmt::format("data file {} does not exist", data_file.string()));
        }
        auto fs = std::ifstream(data_file);
        std::vector<char> player_A;
        std::vector<char> player_B;
        char A;
        char B;
        while( fs >> A >> B) {
            player_A.emplace_back(A);
            player_B.emplace_back(B);
        }

        return {player_A, player_B};
    }
}

int main(int argc, char **argv)
{
    const auto& [player_A, player_B] = load_data(argv[1]);
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
        int score(char x) {
            switch (x) {
                case 'X':
                    return 1;
                case 'Y':
                    return 2;
                case 'Z':
                    return 3;
                default:
                    return 0;
            }
        }

        int round_score(char A, char B) {
            int loss = 0;
            int draw = 3;
            int win = 6;
            switch (A) {
                case 'A': // Rock
                    switch (B) {
                        case 'X':
                            return score('X') + draw;
                        case 'Y':
                            return score('Y') + win;
                        case 'Z':
                            return score('Z') + loss;
                        default:
                            return 0;
                    }
                case 'B': // Paper
                    switch (B) {
                        case 'X':
                            return score('X') + loss;
                        case 'Y':
                            return score('Y') + draw;
                        case 'Z':
                            return score('Z') + win;
                        default:
                            return 0;
                    }
                case 'C': // Scissors
                    switch (B) {
                        case 'X':
                            return score('X') + win;
                        case 'Y':
                            return score('Y') + loss;
                        case 'Z':
                            return score('Z') + draw;
                        default:
                            return 0;
                    }
                default:
                    return 0;
            }
        }

        kernel void score_rock_paper_scissors(global const char* player_A, global const char* player_B, global int *score)
        {
            int index = get_global_id(0);
            score[index] = round_score(player_A[index], player_B[index]);
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

    auto rock_paper_scissors_kernel = cl::KernelFunctor<cl::Buffer, cl::Buffer, cl::Buffer>(program, "score_rock_paper_scissors");

    std::vector<int> score(player_A.size());
    auto player_A_buffer = cl::Buffer(player_A.begin(), player_A.end(), true);
    auto player_B_buffer = cl::Buffer(player_B.begin(), player_B.end(), true);
    auto score_buffer = cl::Buffer(score.begin(), score.end(), false);
    cl_int error;
    rock_paper_scissors_kernel(cl::EnqueueArgs(cl::NDRange(score.size())), player_A_buffer, player_B_buffer, score_buffer, error);

    if (error != CL_SUCCESS) {
        spdlog::error("Error running kernel");
    }

    cl::copy(score_buffer, score.begin(), score.end());
    auto total_score = ranges::accumulate(score, 0);
    spdlog::info("The total score is {}", total_score);

    return 0;
}