#include <oneapi/tbb/info.h>
#include "oneapi/tbb/parallel_invoke.h"
#include <oneapi/tbb/parallel_reduce.h>
#include <oneapi/tbb/parallel_for.h>
#include <oneapi/tbb/parallel_scan.h>
#include <cassert>
#include <iostream>
#include <cmath>
#include <vector>

// https://docs.oneapi.io/versions/latest/onetbb/tbb_userguide/Migration_Guide/Task_Scheduler_Init.html

const int NUM_BINS = 3;

void parallel_solution(std::vector<int> &values) {
    const int N = values.size();

    std::cout << std::endl << "=== PARALLEL SOLUTION =======================================" << std::endl << std::endl;

    // Sort vector just in case
    std::sort(values.begin(), values.end());

    // Get the biggest element and compute the bin size from it
    const int MAX_VALUE = values[N - 1];
    const int BIN_SPAN = std::ceil(MAX_VALUE / NUM_BINS);

    // Distribute the bins evenly
    auto bin_range_values = std::vector<int>(NUM_BINS);
    for (int i = 0; i < NUM_BINS; i++)
    {
        bin_range_values[i] = MAX_VALUE - (NUM_BINS - 1 - i) * BIN_SPAN;
    }

    // Map each value to its corresponding bin
    std::vector<std::array<int, NUM_BINS>> mapped_values(N);
    oneapi::tbb::parallel_for(
        oneapi::tbb::blocked_range<int>(0, N),
        [&](tbb::blocked_range<int> r)
        {
            for (int i = r.begin(); i < r.end(); i++)
            {
                int val = values[i] > 0 ? values[i] - 1 : values[i]; // 0 belongs in the first bin
                int idx = val / BIN_SPAN;
                std::array<int, NUM_BINS> arr{};
                arr[idx]++;
                mapped_values[i] = arr;
            }
        }
    );


    std::cout << "STEP 1: MAP" << std::endl;
    for (int i = 0; i < mapped_values.size(); i++)
    {
        std::cout << "{ ";
        for (int x : mapped_values[i])
        {
            std::cout << x << " ";
        }

        if (i == mapped_values.size() - 1) {
            std::cout << "}" << std::endl;
        } else {
            std::cout << "}, ";
        }
    }

    // Sum up all values for each bin
    std::array<int, NUM_BINS> bins{};
    bins = oneapi::tbb::parallel_reduce(
        oneapi::tbb::blocked_range<int>(0, N),
        std::array<int, NUM_BINS>{},
        [&](oneapi::tbb::blocked_range<int> r, std::array<int, NUM_BINS> total)
        {
            for (int i = r.begin(); i < r.end(); i++)
            {
                for (int j = 0; j < NUM_BINS; j++)
                {
                    total[j] += mapped_values[i][j];
                }
            }
            return total;
        },
        [&](std::array<int, NUM_BINS> left, std::array<int, NUM_BINS> right)
        {
            std::array<int, NUM_BINS> res{};
            for (int i = 0; i < NUM_BINS; i++)
            {
                res[i] = left[i] + right[i];
            }
            return res;
        }
    );


    std::cout << std::endl << "STEP 2: REDUCE" << std::endl;
    for (int i : bins)
    {
        std::cout << i << " ";
    }
    std::cout << std::endl;
    

    // Scan through the bins to build the cumulative histogram
    std::array<int, NUM_BINS> cumulative_histogram{};
    oneapi::tbb::parallel_scan(
        oneapi::tbb::blocked_range<int>(0, NUM_BINS),
        0,
        [&](oneapi::tbb::blocked_range<int> r, int total, bool is_final_scan){
            for (int i = r.begin(); i < r.end(); i++) {
                total += bins[i];
                if (is_final_scan) {
                    cumulative_histogram[i] = total;
                }
            }
            return total;
        },
        [&](int x, int y){
            return x + y;
        }
    );

    std::cout << std::endl << "STEP 3: SCAN" << std::endl;
    for (int i = 0; i < NUM_BINS; i++) {
        std::cout << cumulative_histogram[i] << " ";
    }
    std::cout << std::endl << std::endl;
    std::cout << "=============================================================" << std::endl << std::endl;
}

void sequential_solution(std::vector<int> values) {
        const int N = values.size();

    std::cout << std::endl << "=== SEQUENTIAL SOLUTION =====================================" << std::endl << std::endl;

    // Sort vector just in case
    std::sort(values.begin(), values.end());

    // Get the biggest element and compute the bin size from it
    const int MAX_VALUE = values[N - 1];
    const int BIN_SPAN = std::ceil(MAX_VALUE / NUM_BINS);

    // Distribute the bins evenly
    auto bin_range_values = std::vector<int>(NUM_BINS);
    for (int i = 0; i < NUM_BINS; i++)
    {
        bin_range_values[i] = MAX_VALUE - (NUM_BINS - 1 - i) * BIN_SPAN;
    }

    // Map each value to its corresponding bin
    std::vector<std::array<int, NUM_BINS>> mapped_values(N);
    for (int i = 0; i < N; i++) {
                int val = values[i] > 0 ? values[i] - 1 : values[i];
                int idx = val / BIN_SPAN;
                std::array<int, NUM_BINS> arr{};
                arr[idx]++;
                mapped_values[i] = arr;
    }

    std::cout << "STEP 1: MAP" << std::endl;
    for (int i = 0; i < mapped_values.size(); i++)
    {
        std::cout << "{ ";
        for (int x : mapped_values[i])
        {
            std::cout << x << " ";
        }

        if (i == mapped_values.size() - 1) {
            std::cout << "}" << std::endl;
        } else {
            std::cout << "}, ";
        }
    }

    // Sum up all values for each bin
    std::array<int, NUM_BINS> bins{};
    for (int i = 0; i < mapped_values.size(); i++) {
        for (int j = 0; j < NUM_BINS; j++) {
            bins[j] += mapped_values[i][j];
        }
    }

    std::cout << std::endl << "STEP 2: REDUCE" << std::endl;
    for (int i : bins)
    {
        std::cout << i << " ";
    }
    std::cout << std::endl;
    

    // Scan through the bins to build the cumulative histogram
    std::array<int, NUM_BINS> cumulative_histogram{};
    int total = 0;
    for (int i = 0; i < NUM_BINS; i++) {
        total += bins[i];
        cumulative_histogram[i] = total;
    }

    std::cout << std::endl << "STEP 3: SCAN" << std::endl;
    for (int i = 0; i < NUM_BINS; i++) {
        std::cout << cumulative_histogram[i] << " ";
    }
    std::cout << std::endl << std::endl;
    std::cout << "=============================================================" << std::endl << std::endl;
}

int main()
{
    std::vector<int> values = {0, 7, 8, 10, 24, 48, 73, 120}; // Assuming non-negative values
    parallel_solution(values);
    sequential_solution(values);
}