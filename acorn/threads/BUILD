load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "shared_thread_pool",
    srcs = ["shared_thread_pool.h"],
    visibility = ["//visibility:public"],
    deps = ["@com_google_absl//absl/synchronization"],
)

cc_library(
    name = "logger",
    srcs = ["logger.h"],
    visibility = ["//visibility:public"],
    deps = ["@com_google_absl//absl/synchronization"],
)

cc_library(
    name = "taskgraph",
    srcs = ["taskgraph.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":shared_thread_pool",
        "//acorn/container:slot_map",
        "@com_google_absl//absl/synchronization",
    ],
)
