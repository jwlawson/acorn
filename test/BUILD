
cc_test(
    name = "pool",
    srcs = ["pool.cc"],
    deps = ["//threads:pool", "@gtest//:gtest_main"],
    size = "small",
)

cc_test(
    name = "simple",
    srcs = ["simple.cc"],
    deps = ["//threads:pool"],
    linkopts = ["-pthread"],
    size = "small",
)
