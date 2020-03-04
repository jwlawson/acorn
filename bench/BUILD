load("@rules_cc//cc:defs.bzl", "cc_test")

cc_test(
    name = "pool",
    size = "small",
    srcs = ["pool.cc"],
    tags = ["benchmark"],
    deps = [
        "//threads:pool",
        "@com_google_benchmark//:benchmark_main",
    ],
)
