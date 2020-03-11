load("@rules_cc//cc:defs.bzl", "cc_test")

cc_test(
    name = "pool",
    size = "small",
    srcs = ["pool.cc"],
    tags = ["benchmark"],
    deps = [
        "//acorn:macros",
        "//acorn/threads:shared_thread_pool",
        "@com_google_benchmark//:benchmark_main",
    ],
)
