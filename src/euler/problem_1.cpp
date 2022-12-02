#include <euler/euler.hpp>

#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/iota.hpp>

namespace views = ranges::views;

std::vector<int> euler::multiples_of_three_or_five(int max) {
    // NOLINTBEGIN(*-magic-numbers)
    return views::ints(3, max)
           | views::filter([](int num) { return (num % 3 == 0) || (num % 5 == 0); })
           | ranges::to<std::vector>();
    // NOLINTEND(*-magic-numbers)
}

int euler::sum_multiples_of_three_or_five(int max) {
    return ranges::accumulate(euler::multiples_of_three_or_five(max), 0);
}
