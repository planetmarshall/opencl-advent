#include <euler/euler.hpp>

#include <boost/program_options.hpp>

#include <iostream>

namespace po = boost::program_options;

int main(int argc, char **argv) {
    po::options_description description("Project Euler Problem 1 <https://projecteuler.net/>");
    constexpr auto default_maximum = 100;
    int maximum; //NOLINT(cppcoreguidelines-init-variables)
    description.add_options()
        ("help", "show help message")
        ("maximum", po::value<int>(&maximum)->default_value(default_maximum), "maximum number up to which to generate multiples of 3 and 5")
    ;
    po::variables_map options;
    po::store(po::parse_command_line(argc, argv, description), options);
    po::notify(options);

    std::cout << "Sum of multiples of 3 and 5 up to 42 is "
      << euler::sum_multiples_of_three_or_five(maximum)
      << std::endl;

    return 0;
}
