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

using namespace oneapi;

int main() {
    std::vector<int> values = {0, 7, 8, 10, 24, 48, 73, 120};
    const int N = values.size();
    const int NUM_BINS = 3;

    // Sort vector just in case
    std::sort(values.begin(), values.end());

    // Get the biggest element
    const int MAX_VALUE = values[N - 1];
    const int BIN_SPAN = MAX_VALUE / NUM_BINS;

    // Distribute the bins evenly
    auto bin_range_values = std::vector<int>(NUM_BINS);
    for (int i = 0; i < NUM_BINS; i++) {
        bin_range_values[i] = MAX_VALUE - (NUM_BINS - 1 - i) * BIN_SPAN;
    }
 
    // Map each value to its corresponding bin
    std::vector<std::array<int, 3>> mapped_values(N);
    oneapi::tbb::parallel_for(
        oneapi::tbb::blocked_range<int>(0, N),
        [&](tbb::blocked_range<int> r){
            for (int i = r.begin(); i < r.end(); i++) {
                int val = values[i] > 0 ? values[i] - 1 : values[i];
                int idx = val / BIN_SPAN;
                std::array<int, 3> arr = {0, 0, 0};
                arr[idx]++;
                mapped_values[i] = arr;
            }
        }
    );

    for (int i = 0; i < mapped_values.size(); i++) {
        std::cout << "{ ";
        for (int x : mapped_values[i]) {
            std:: cout << x << " "; 
        }

        std::cout << "} ";
    }


    // Sum up all values for each bin
    // auto bins = std::vector<int>(NUM_BINS);
    // oneapi::tbb::parallel_reduce(
    //     oneapi::tbb::blocked_range<int>(1, N),
    //             {0, 0 ,0},
    //             [&](tbb::blocked_range<int> r, int total){
    //                 for (int i = r.begin(); i < r.end(); i++) {
    //                     bins[mapped_values[i]]++;
    //                 }
    //             },
    //             [&](int left, int right){

    //             }
    // );

}