
load("@rules_cc//cc:defs.bzl", "cc_test")

cc_test(
    name = "pool",
    srcs = ["pool.cc"],
    deps = ["//threads:pool", "@com_google_googletest//:gtest_main"],
    size = "small",
)

cc_test(
    name = "simple",
    srcs = ["simple.cc"],
    deps = ["//threads:pool"],
    linkopts = ["-pthread"],
    size = "small",
)
