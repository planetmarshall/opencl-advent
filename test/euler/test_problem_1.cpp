#include <euler/euler.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

using Catch::Matchers::Equals;

TEST_CASE("List Multiples of 3 and 5 Below 10", "[Project Euler Problem 1]") {
  auto multiples = euler::multiples_of_three_or_five(10);
  REQUIRE_THAT(multiples, Equals(std::vector<int>{3, 5, 6, 9}));
}

TEST_CASE("Sum Multiples of 3 and 5 Below 10", "[Project Euler Problem 1]") {
  auto sum = euler::sum_multiples_of_three_or_five(10);
  REQUIRE(sum == 23);
}
