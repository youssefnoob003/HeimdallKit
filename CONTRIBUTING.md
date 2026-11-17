# Contributing

Contributions are welcome. This document explains how to build, test, and
submit changes.

## Build

```sh
cmake -S . -B build
cmake --build build
```

For development, enable warnings-as-errors and sanitizers (GCC/Clang use
AddressSanitizer and UBSan; MSVC uses AddressSanitizer):

```sh
cmake -S . -B build-dev \
  -DCMAKE_BUILD_TYPE=Debug \
  -DHEIM_WARNINGS_AS_ERRORS=ON \
  -DHEIM_ENABLE_SANITIZERS=ON
cmake --build build-dev
```

## Test

```sh
ctest --test-dir build --output-on-failure
```

Tests are labeled by domain (`signal`, `lti`, `io`, `status`), so a focused
run is available with:

```sh
ctest --test-dir build -L signal
```

## Code style

The project uses `clang-format` with the settings in `.clang-format`. Format
your changes before committing:

```sh
clang-format -i src/**/*.c include/**/*.h internal/**/*.h
```

## Submitting changes

1. Fork the repository and create a feature branch.
2. Make your changes. Add or update tests for any new or changed behavior.
3. Ensure `cmake --build build` succeeds with no warnings under
   `-DHEIM_WARNINGS_AS_ERRORS=ON`.
4. Ensure all tests pass, including under sanitizers.
5. Open a pull request with a clear description of what changed and why.

## Project layout

- `include/heim/` — public API headers (opaque types, function declarations)
- `internal/heim/` — private headers (struct definitions, internal helpers)
- `src/` — implementation, organized by domain
- `tests/` — one test executable per domain
- `examples/` — small standalone programs demonstrating the API
