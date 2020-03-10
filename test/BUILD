load("@rules_cc//cc:defs.bzl", "cc_test")

cc_test(
    name = "pool",
    size = "small",
    srcs = ["pool.cc"],
    deps = [
        "//threads:pool",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "simple",
    size = "small",
    srcs = ["simple.cc"],
    linkopts = ["-pthread"],
    deps = ["//threads:pool"],
)

cc_test(
    name = "logger",
    size = "small",
    srcs = ["logger.cc"],
    deps = [
        "//threads:logger",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "taskgraph",
    size = "small",
    srcs = ["taskgraph.cc"],
    deps = [
        "//threads:taskgraph",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "slot_map",
    size = "small",
    srcs = ["slot_map.cc"],
    deps = [
        "//container:slot_map",
        "@com_google_googletest//:gtest_main",
    ],
)
