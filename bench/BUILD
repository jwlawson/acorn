
load("@rules_cc//cc:defs.bzl", "cc_test")

cc_test(
    name = "pool",
    srcs = ["pool.cc"],
    deps = ["//threads:pool", "@com_google_benchmark//:benchmark_main"],
    size = "small",
    tags = ["benchmark"],
)

