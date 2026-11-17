# HeimdallKit Examples

This directory contains standalone, runnable examples demonstrating core digital signal processing workflows using HeimdallKit.

## Examples Overview

| Executable | Source File | Concepts Demonstrated |
| :--- | :--- | :--- |
| `signal_basics` | [signal_basics.c](signal_basics.c) | Analytical mappers, canonical signals (impulse, step, exponential), support metrics, energy & power, parity checks, transforms (shift, scale, reverse), and custom signal combinations with `HeimComplex`. |
| `lti_filtering` | [lti_filtering.c](lti_filtering.c) | Linear Time-Invariant (LTI) system properties (causality, memorylessness), discrete-time convolution, step response, and difference equation (LCCDE) IIR filtering. |
| `matched_filter` | [matched_filter.c](matched_filter.c) | Practical RF / radar pulse compression and matched filtering ($h[n] = s^*[-n]$), correlation peak detection for time-of-arrival estimation, and `.heim` binary file I/O. |

## Building and Running

To build all examples with CMake:

```sh
cmake -S . -B build -DHEIM_BUILD_EXAMPLES=ON
cmake --build build
```

Run any example directly from the build directory:

```sh
./build/signal_basics
./build/lti_filtering
./build/matched_filter
```

Or run via the convenience Makefile:

```sh
make run
```
