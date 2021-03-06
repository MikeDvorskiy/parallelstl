/*
    Copyright (c) 2017-2018 Intel Corporation

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.




*/

// Tests for partial_sort

#include <cmath>

#include "pstl/execution"
#include "pstl/algorithm"
#include "utils.h"

using namespace TestUtils;

template <typename T>
struct Num {
    T val;
    explicit Num(const T& v) : val(v) {}

    //for "includes" checks
    template <typename T1>
    bool operator<(const Num<T1>& v1) const{
        return val < v1.val;
    }

    //The types Type1 and Type2 must be such that an object of type InputIt can be dereferenced and then implicitly converted to both of them
    template <typename T1>
    operator Num<T1>() const { return Num<T1>((T1)val); }
};

struct test_one_policy {
    template <typename Policy, typename InputIterator1, typename InputIterator2, typename Compare>
    typename std::enable_if<!TestUtils::isReverse<InputIterator1>::value, void>::type
    operator()(Policy&& exec, InputIterator1 first1, InputIterator1 last1, InputIterator2 first2, InputIterator2 last2, Compare comp) {

            auto expect_res = std::includes(first1, last1, first2, last2, comp);
            auto res = std::includes(exec, first1, last1, first2, last2, comp);

            EXPECT_TRUE(expect_res == res, "wrong result for includes");
    }

    template <typename Policy, typename InputIterator1, typename InputIterator2, typename Compare>
    typename std::enable_if<TestUtils::isReverse<InputIterator1>::value, void>::type
        operator()(Policy&& exec, InputIterator1 first1, InputIterator1 last1, InputIterator2 first2, InputIterator2 last2, Compare comp) {}
};

template<typename T1, typename T2, typename Compare>
void test_includes(Compare compare) {

    const std::size_t n_max = 1000000;

    // The rand()%(2*n+1) encourages generation of some duplicates.
    std::srand(42);

    for (std::size_t n = 0; n < n_max; n = n <= 16 ? n + 1 : size_t(3.1415 * n)) {
        for (std::size_t m = 0; m < n_max; m = m <= 16 ? m + 1 : size_t(2.71828 * m)) {
            //prepare the input ranges
            Sequence<T1> in1(n, [n](std::size_t k) { return rand() % (2 * k + 1); });
            Sequence<T2> in2(m, [m](std::size_t k) { return rand() % (k+1); });

            std::sort(in1.begin(), in1.end(), compare);
            std::sort(in2.begin(), in2.end(), compare);

            invoke_on_all_policies(test_one_policy(), in1.begin(), in1.end(), in2.cbegin(), in2.cend(), compare);

            //test w/ non constant predicate
            if(n < 5 && m < 5)
                invoke_on_all_policies(test_one_policy(), in1.begin(), in1.end(), in2.cbegin(), in2.cend(), non_const(compare));
        }
    }
}

int32_t main() {

    test_includes<float64_t, float64_t>(std::less<float64_t>());
    test_includes<Num<int64_t>, Num<int32_t>>([](const Num<int64_t>& x, const Num<int32_t>& y) { return x < y; });
    std::cout << done() << std::endl;

    return 0;
}
