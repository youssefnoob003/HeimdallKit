# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.0-alpha] - 2026-11-17

Initial alpha release of **HeimdallKit**, a zero-dependency, memory-safe C11 DSP engine for complex I/Q signal processing and discrete-time RF analysis.

### Added
- **Core Signal Model (`heim_sig_*`):**
  - Opaque `HeimSig` handle encapsulating discrete finite support `[n_start, n_start + len - 1]` with checked bounds.
  - Safe memory lifecycle: `heim_sig_alloc`, `heim_sig_from_arrays`, `heim_sig_copy`, and `heim_sig_free`.
  - Non-throwing accessors and setters: `heim_sig_len`, `heim_sig_start`, `heim_sig_end`, `heim_sig_step`, `heim_sig_contains`, `heim_sig_get_sample`, `heim_sig_set_sample`, and `heim_sig_get_sample_complex`.
- **Complex Arithmetic (`HeimComplex`):**
  - Dedicated `HeimComplex` value struct with inline arithmetic helpers: `heim_complex`, `heim_complex_add`, `heim_complex_sub`, `heim_complex_mul`, `heim_complex_scale`, `heim_complex_conj`, `heim_complex_abs`, and `heim_complex_abs_sq`.
- **Canonical Generation (`heim_sig_gen_*`):**
  - Analytical signal mapper `heim_sig_gen_create` and zero-centered mapper `heim_sig_gen_centered`.
  - Canonical discrete signals: unit impulse (`heim_sig_gen_impulse`), unit step (`heim_sig_gen_step`), and complex exponential (`heim_sig_gen_exp`).
- **Signal Transformations (`heim_sig_xform_*`):**
  - Discrete coordinate shift (`heim_sig_xform_shift`), real scaling (`heim_sig_xform_scale`), complex rotation/scaling (`heim_sig_xform_scale_complex`), and time reversal (`heim_sig_xform_reverse`).
- **Signal Analysis (`heim_sig_an_*`):**
  - Support-wide equality checking (`heim_sig_an_equal`) and overlap-only equality (`heim_sig_an_equal_overlap`).
  - Symmetry verification: odd parity (`heim_sig_an_is_odd`) and even parity (`heim_sig_an_is_even`).
  - Numerical energy integrals (`heim_sig_an_energy`, `heim_sig_an_energy_range`) and average power calculation (`heim_sig_an_power`, `heim_sig_an_power_range`).
- **Signal Operations (`heim_sig_op_*`):**
  - Extensible union-support binary combinator `heim_sig_op_combine` taking `HeimSigOpCombiner` callbacks.
  - Point-by-point signal addition (`heim_sig_op_add`) and multiplication (`heim_sig_op_mul`).
- **LTI Systems & Filtering (`heim_lti_*`):**
  - Discrete-time linear convolution (`heim_lti_convolve`).
  - System property verification: causality (`heim_lti_is_causal`) and memorylessness (`heim_lti_is_memoryless`).
  - Cumulative step response derivation (`heim_lti_step_response`).
  - Direct Form recursive Linear Constant-Coefficient Difference Equation (LCCDE) IIR/FIR filtering (`heim_lti_filter_lccde`).
- **Binary I/O Engine (`heim_sig_io_*`):**
  - Unframed raw native IEEE double `{I, Q}` streaming (`heim_sig_io_write_raw`, `heim_sig_io_read_raw`).
  - Self-describing `.heim` binary format with little-endian metadata header, magic bytes, and IEEE-754 sample encoding (`heim_sig_io_write_heim`, `heim_sig_io_read_heim`).
  - Atomic file write transactions via POSIX `mkstemp`/rename to prevent corrupt partially written files.
- **Diagnostic Status Subsystem (`HeimStatus`):**
  - Rich error reporting struct carrying `HeimErrorCode`, originating file, line number, and context string.
  - Variadic `HEIM_CHECK_NOT_NULL(...)` macro reporting the exact argument name that failed.
  - Integer overflow protection macros: `HEIM_CHECK_ADD_OVERFLOW_I64`, `HEIM_CHECK_ADD_OVERFLOW_SIZE`, and `HEIM_CHECK_OVERFLOW`.
  - Error propagation helper `HEIM_PROPAGATE_ERR(...)`.
- **Examples & Tooling:**
  - Standalone runnable demonstrations: `signal_basics`, `lti_filtering`, and `matched_filter` (radar pulse compression).
  - Domain-labeled CTest suite (`signal`, `lti`, `io`, `status`).
  - CMake packaging with `HeimdallKit::heim` target export, standard installation rules, and package configuration generation.
