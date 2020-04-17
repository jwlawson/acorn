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

cc_test(
    name = "member_tuple",
    size = "small",
    srcs = ["tuple.cc"],
    defines = ["ACORN_STANDARD_LAYOUT_TUPLE"],
    deps = [
        "//acorn/container:tuple",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "nested_inherited_tuple",
    size = "small",
    srcs = ["tuple.cc"],
    defines = ["ACORN_NESTED_LAYOUT_TUPLE"],
    deps = [
        "//acorn/container:tuple",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "flat_inherited_tuple",
    size = "small",
    srcs = ["tuple.cc"],
    deps = [
        "//acorn/container:tuple",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "with_ref_matching",
    size = "small",
    srcs = ["with_ref_matching.cc"],
    deps = [
        "//acorn/traits",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "remove_const_ref",
    size = "small",
    srcs = ["remove_const_ref.cc"],
    deps = [
        "//acorn/traits",
        "@com_google_googletest//:gtest_main",
    ],
)
