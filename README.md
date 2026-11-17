# HeimdallKit

> [!NOTE]
> **Alpha Release (v0.1.0-alpha)** — *HeimdallKit is in early, active development. Core abstractions and APIs are establishing stability and subject to evolutionary refinement as additional RF capabilities are added.*

**HeimdallKit** is a zero-dependency, memory-safe C11 DSP engine engineered for complex In-Phase/Quadrature (I/Q) discrete-time signal processing, Linear Time-Invariant (LTI) systems, and RF analysis.

---

## Highlights

- **Zero External Dependencies:** Implemented in standard ISO C11 with `libm`.
- **Memory-Safe Architecture:** Opaque data types (`HeimSig`), strict ownership semantics, bounds-checked support ranges, integer overflow guards, and atomic file transactions.
- **Complex I/Q Foundations:** Native `HeimComplex` value arithmetic and planar sample arrays optimized for numerical stability.
- **LTI Systems & Filtering:** Discrete-time linear convolution, step response calculation, system property verification (causality, memorylessness), and Direct Form recursive LCCDE IIR/FIR difference equations.
- **Rich Diagnostic Status:** Explicit error handling via `HeimStatus`, recording originating files, line numbers, and argument names on validation failures without overhead on success.
- **Binary I/O Engine:** Raw double-precision `{I, Q}` binary streaming and self-describing `.heim` metadata container formats.

---

## Quick Start

### Build and Test

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

To enable compiler warning enforcement and sanitizers:

```sh
cmake -S . -B build -DHEIM_WARNINGS_AS_ERRORS=ON -DHEIM_ENABLE_SANITIZERS=ON
cmake --build build
```

Run demonstration binaries:

```sh
./build/signal_basics
./build/lti_filtering
./build/matched_filter
```

---

## Quick Example

```c
#include "heim/heim.h"
#include <stdio.h>

int main(void) {
  HeimSig *sig = NULL;

  /* Generate a complex exponential: amplitude * exp(j * (omega_0 * n + phase)) */
  HeimStatus status = heim_sig_gen_exp(1.0, HEIM_PI / 4.0, 0.0, 0, 16, 1.0, &sig);
  if (HEIM_IS_ERR(status)) {
    heim_status_print(status);
    return 1;
  }

  double energy = 0.0;
  heim_sig_an_energy(sig, &energy);
  printf("Generated %zu samples, total energy: %.2f\n", heim_sig_len(sig), energy);

  heim_sig_free(sig);
  return 0;
}
```

---

## Architecture & Signal Model

### Finite-Support Signal Model
`HeimSig` models discrete-time signals with finite support over inclusive indices:
$$\{ n \in \mathbb{Z} \mid n_{\text{start}} \le n \le n_{\text{start}} + \text{len} - 1 \}$$
Samples evaluated outside this finite support window are implicitly zero.

### API Namespaces
The public API provides uniform domain prefixes:
- **`Heim*` Types:** `HeimSig`, `HeimComplex`, `HeimStatus`, `HeimErrorCode`
- **`heim_sig_*`:** Core signal lifecycle (`alloc`, `free`, `from_arrays`, `copy`) and accessors (`len`, `start`, `end`, `step`, `get_sample`, `set_sample`)
- **`heim_sig_gen_*`:** Signal generators (analytical mapper, centered pulse, unit impulse, step, complex exponential)
- **`heim_sig_xform_*`:** Coordinate transformations (time shift, scalar scale, complex rotation, time reversal)
- **`heim_sig_an_*`:** Analytical metrics (support equality, overlap equality, odd/even parity, energy, average power)
- **`heim_sig_op_*`:** Signal algebra (callback combinator `heim_sig_op_combine`, addition, multiplication)
- **`heim_sig_io_*`:** File I/O (`write_raw`, `read_raw`, `write_heim`, `read_heim`)
- **`heim_lti_*`:** LTI systems (`convolve`, `step_response`, `is_causal`, `is_memoryless`, `filter_lccde`)

---

## Repository Structure

```
├── include/heim/          # Public headers (modular headers under signal/*.h)
├── internal/heim/         # Private representations and internal helpers
├── src/                   # Source implementations (signal, lti, io, status, common)
├── tests/                 # CTest test suites categorized by domain
└── examples/              # Standalone runnable demonstration programs
```

---

## License

This project is licensed under the [MIT License](LICENSE).
