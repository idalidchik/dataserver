// priority_queue.cpp
#include "dataserver/numeric/priority_queue.h"
#include <algorithm>
#include <numeric>

#if SDL_DEBUG
namespace sdl { namespace {

    template<class T>
    inline bool is_sorted(T const & result) {
        return std::is_sorted(std::begin(result), std::end(result));
    }

    template<class T>
    inline bool is_sorted_desc(T const & result) {
        return std::is_sorted(std::begin(result), std::end(result),
            [](auto const & x, auto const & y) {
            return y < x;
        });
    }
    class unit_test {
        void test_queue(bool descending);
    public:
        unit_test() {
            test_queue(false);
            test_queue(true);
        }
    };
    void unit_test::test_queue(const bool descending)
    {
        struct cost_value {
            size_t m_weight = 0;
            int priority = 0;
            size_t weight() const {
                return m_weight;
            }
        };
        using data_array = std::vector<cost_value>;
        using queue_type = priority_queue<int, data_array>;
        enum { N = 10 };
        enum { trace = 0 };
        data_array data(N);
        for (size_t i = 0; i < data.size(); ++i) {
            data[i].m_weight = descending ? (data.size() - i) : i;
        }
        queue_type test(data);
        for (size_t i = 0; i < data.size(); ++i) {
            test.insert((queue_type::value_type) i);
        }
        SDL_ASSERT(test.size() == data.size());
        {
            std::vector<size_t> result;
            size_t i = 0;
            while (!test.empty()) {
                ++i;
                result.push_back(test.getmin());
                SDL_ASSERT(result.size() == i);
                SDL_ASSERT(test.size() == data.size() - i);
                if (trace) {
                    std::cout << result.back() << " ";
                }
            }
            SDL_ASSERT(i == data.size());
            SDL_ASSERT(result.size() == data.size());
            SDL_ASSERT(descending || is_sorted(result));
            SDL_ASSERT(!descending || is_sorted_desc(result));
        }
        if (trace) {
            std::cout << std::endl;
        }
    }
    static unit_test s_test;

}} // sdl
#endif //#if SDL_DEBUG

