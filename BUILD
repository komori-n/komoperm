cc_library(
  name = "komoperm_lib",
  srcs = [],
  hdrs = [
    "src/komoperm.hpp",
  ],
  include_prefix = "komoperm",
  strip_include_prefix = "src",
)

cc_test(
  name = "komoperm_test",
  srcs = [
    "tests/komoperm_test.cpp",
  ],
  deps = [
    ":komoperm_lib",
    "@com_google_googletest//:gtest_main"
  ],
)