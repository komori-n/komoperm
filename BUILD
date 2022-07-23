cc_library(
  name = "kombi_lib",
  srcs = [],
  hdrs = [
    "src/kombi.hpp",
  ],
  include_prefix = "kombi",
  strip_include_prefix = "src",
)

cc_test(
  name = "kombi_test",
  srcs = [
    "tests/kombi_test.cpp",
  ],
  deps = [
    ":kombi_lib",
    "@com_google_googletest//:gtest_main"
  ],
)