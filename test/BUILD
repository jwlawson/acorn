load("@rules_cc//cc:defs.bzl", "cc_test")

cc_test(
    name = "pool",
    size = "small",
    srcs = ["pool.cc"],
    deps = [
        "//acorn/threads:shared_thread_pool",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "simple",
    size = "small",
    srcs = ["simple.cc"],
    linkopts = ["-pthread"],
    deps = ["//acorn/threads:shared_thread_pool"],
)

cc_test(
    name = "logger",
    size = "small",
    srcs = ["logger.cc"],
    deps = [
        "//acorn/threads:logger",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "taskgraph",
    size = "small",
    srcs = ["taskgraph.cc"],
    deps = [
        "//acorn/threads:taskgraph",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "slot_map",
    size = "small",
    srcs = ["slot_map.cc"],
    deps = [
        "//acorn/container:slot_map",
        "@com_google_googletest//:gtest_main",
    ],
)
