workspace(name = "jwlawson_threads")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "com_google_googletest",
    sha256 = "94c634d499558a76fa649edb13721dce6e98fb1e7018dfaeba3cd7a083945e91",
    strip_prefix = "googletest-release-1.10.0",
    urls = ["https://github.com/google/googletest/archive/release-1.10.0.zip"],
)

http_archive(
    name = "com_google_benchmark",
    sha256 = "2d22dd3758afee43842bb504af1a8385cccb3ee1f164824e4837c1c1b04d92a0",
    strip_prefix = "benchmark-1.5.0",
    urls = ["https://github.com/google/benchmark/archive/v1.5.0.zip"],
)

http_archive(
    name = "com_google_absl",
    sha256 = "6f367c8643e69f5b53444c49400ed21d11f09f76c5af99e36181248ad333410a",
    strip_prefix = "abseil-cpp-abea769b551f7a100f540967cb95debdb0080df8",
    urls = ["https://github.com/abseil/abseil-cpp/archive/abea769b551f7a100f540967cb95debdb0080df8.zip"],
)

http_archive(
    name = "rules_cc",
    sha256 = "67412176974bfce3f4cf8bdaff39784a72ed709fc58def599d1f68710b58d68b",
    strip_prefix = "rules_cc-b7fe9697c0c76ab2fd431a891dbb9a6a32ed7c3e",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/rules_cc/archive/b7fe9697c0c76ab2fd431a891dbb9a6a32ed7c3e.zip",
        "https://github.com/bazelbuild/rules_cc/archive/b7fe9697c0c76ab2fd431a891dbb9a6a32ed7c3e.zip",
    ],
)
