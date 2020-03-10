load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "pool",
    srcs = ["pool.h"],
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
        ":pool",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/synchronization",
    ],
)
