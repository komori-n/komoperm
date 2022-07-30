# komoperm

[![ct-ubuntu](https://github.com/komori-n/komoperm/actions/workflows/ubuntu.yaml/badge.svg)](https://github.com/komori-n/komoperm/actions/workflows/ubuntu.yaml)

[![ct-macos](https://github.com/komori-n/komoperm/actions/workflows/macos.yaml/badge.svg)](https://github.com/komori-n/komoperm/actions/workflows/macos.yaml)

[![CodeQL](https://github.com/komori-n/komoperm/actions/workflows/codeql.yaml/badge.svg)](https://github.com/komori-n/komoperm/actions/workflows/codeql.yaml)

A modern C++ library for permutation with duplicates.

## Usage

```cpp
#include "komoperm/komoperm.hpp"

enum Hoge {
    A, B, C,
};

int main() {
    constexpr komoperm::Permutations<Hoge, A, A, A, B, B, C> p;

    static_assert(p.Size() == 60, "The number os possible permutations is 60");

    static_assert(p.Index({B, A, B, C, A, A}) < 60, "{B, A, B, C, A, A} is a permutation of {A, A, A, B, B, C}");

    static_assert(p.Index(p.Get(10)) == 10, "Get() generates the 10th permutation");
}
```

`komoperm::Permutations<T, Vals...>` represents a permutation set of `Vals...`.
You can use integer or enum type for the permutation type `T`.

`komoperm::Permutations` has the following features.

- `Size()`: Returns the number of possible permutations
- `Index(perm)`: Calculate the index for `perm` ([`0`, `Size()`))
- `Get(index)`: Get the `index`th permutation. `index` must be less than `Size()`
  - `Index(Get(index))` is always equals to `index`.

Note that all operations stated above are constexpr, so you can use the results at compile time.

### C++17 features

If you use c++17 or later, you can also use `PermutationAuto` instead of `Permutation`.

```cpp
auto Permutation = komoperm::PermutationAuto<
    A, A, A, B, B, C
>;

// The above code is the same as the following:
// auto Permutation = komoperm::Permutation<
//     decltype(A), A, A, A, B, B, C
// >;
```

This feature is only for c++17 or later because the use of `auto` in template parameters is permitted at that version.

## Install

### bazel

Simply add this repository to your `WORKSPACE` (or `WORKSPACE.bazel`).

```bazel
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
  name = "komoperm",
  # specify the commit hash which you want to use
  commit = "c8229ff06868eee3909b26132549e90962faa9c8",
  remote = "https://github.com/komori-n/komoperm.git",
)
```

After that, you can use this library by `@komoperm//:komoperm_lib`.

```bazel
cc_library(
  name = "example_lib",
  srcs = ["example.cpp"],
  deps = [
    "@komoperm//:komoperm_lib",
  ],
)
```

### Otherwise

Use `git submodule`, or just place `src/komoperm.hpp` at any location.

## To Do

- [ ] iterator for `Permutations`

## License

MIT License
