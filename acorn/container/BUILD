load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "slot_map",
    srcs = ["slot_map.h"],
    visibility = ["//visibility:public"],
    deps = [],
)

cc_library(
    name = "tuple",
    srcs = ["tuple.h"] + glob(["tuple/*.h"]),
    visibility = ["//visibility:public"],
    deps = [
        "//acorn/traits",
        "@com_google_absl//absl/utility",
    ],
)
