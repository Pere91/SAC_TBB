# SAC_TBB
Acumulative histogram using Intel TBB library

Parallel architectures assignment for Sistemes Actuals de Computació

---

## Use

This simulation is designed to be lauched in a Docker container, although the following image is enough with no need of further configuration:

1. Launch the Docker container.

- Unix-like systems
```bash
docker run --rm -it -v $(pwd):/project mfisherman/onetbb
```

- Windows
```bash
docker run --rm -it -v .:/project mfisherman/onetbb # Be sure to be in the file's directory
```

> ⚠️ A running Docker Engine is required.

2. Once inside the container, compile the file.

```bash
g++ -g -std=c++17 main.cpp -pthread -ltbb
```

3. Run the generated binary file.
```bash
./a.out
```

---

## Introduction

This repository contains the code for the generation of a **cumulative histogram** to classify the values of an array of integers. It consists in a predefined number of bins that represent a range of numbers, each one containing the numbers that fall within their range plus the sum of all previous bins.

The goal of this project is to implement the solution using [Intel's oneAPI Threading Building Blocks (TBB)](https://www.intel.com/content/www/us/en/developer/tools/oneapi/onetbb.html), by which the data is parallelized across several resources. From its primitives, the ones used in this project are:

- `parallel_for`: applies the same logic to all elements of a chunk of the array, parallelizing the iterations of a for loop to perform them simultaneously; similar to a mapping.
- `parallel_reduce`: applies a function that reduces all the elements to a singular output value.
- `parallel_scan`: a reduction function is applied, but the intermediary results are preserved in a "history-like" array.

Furthermore, a **sequential solution** is also implemented in order to compare the performance between the two modalities. To make the comparison as fair as possible, the exact same steps are followed in both solutions.

---

## Design decisions

For the development of this project, the following decisions have been made:

- **C++** is the programming language used, because it provides the `oneapi` and `tbb` libraries for using TBB.
- The array of values contains only **non-negative numbers**, for simplicity.
- All bins are **equal in size**, calculated from the largest integer value allowed, with one exception: the first bin features one element more than the others because it also includes the value 0.
- The random generated numbers are only integers bounded **between 0 and 120** because this last number can be easily divided by 2, 3, 4 and 6 bins to see different histograms. However, the number may be changed in the code.
- The random numbers are generated following an **exponential distribution** to simulate unequal bins.

---

## Solution

### Mapping values to convenient data structure

The first step of the solution converts each number of the array to a data structure that allows the future steps to be performed in parallel. This data structure consists in an array of as many elements as bins; all the elements shall be 0 except the one in the position of the bin where the element falls, which shall be 1. Some examples:

- 3 bins: `0 - 40`,  `41 - 80`,  `81 - 120`
- Value 3 falls in the first bin -> `[1, 0, 0]`
- Value 38 also falls in the first bin -> `[1, 0, 0]`
- Value 56 falls in the second bin -> `[0, 1, 0]`
- Value 101 falls in the third bin -> `[0, 0, 1]`

This is achieved with the `parallel_for` primitive, which applies a function to all the elements of a chunk of the original array. The different chunks are processed in parallel. At the end of the process, the output is an array with the mapped values of the original array, where now each one indicates which bin it falls into.

---

### Reducing to a regular histogram

The second step sums all the arrays resulting of the previous transformation element by element, as in a vector sum: `[a, b, c] + [d, e, f] = [a+d, b+e, c+f]`.

The `parallel_reduce` primitive performs this action, applying two functions:

- A function is applied to the chunk selected, which consists in a cumulative sum: `total_accumulated += next_element`.
- A function is applied to each pair of values on each step, in this case, a simple sum.

The result is a single array with as many elements as number of bins, where each element is the number of elements from the original array that fall into that bin, i.e. a **regular histogram**.

---

### Scanning the regular histogram to form the cumulative histogram

The last step takes the output of the previous step and sums to each element all previous elements, starting with the first that remains unchanged. This is acomplished by the `parallel_scan` primitive that, similarly to `parallel_reduce`, applies two functions to the data:

- A function is applied to the chunk selected, which consists too in a cumulative sum but with a final condition for each chunk, so the intermediate value is stored.
- A function is applied to each pair of values on each step, exactly the same as in the reduce, a simple sum again.

The result is once again a single array representing the bins, but now each bin sums all previous bins to its elements, building the **cumulative histogram**.

---

## Final considerations

The **sequential solutions**, as has already been mentioned, applies the same exact logical steps as the parallel one, but without the `TBB` primitives, so the operations (map, reduce and scan) are performed in sequence. 

The **complexity of the code** is slightly bigger in the parallel case, with the verbosity of the `TBB` primitives and data structures and the lambda functions required for each one, although in terms of code length is not a big deal. From the programmer's point of view, the only noticeable difference is, perhaps, figuring out the lambda functions and the data structures to make it work.

The **efficiency and performance** depend on the number of elements: for a small number of elements, the overhead of the parallel solution makes it even more slow than the sequential one; on the other hand, as the dimension increases, the **parallel solution escalates much better** than the sequential one, widening the difference between the two the larger the number of elements is.
