workspace(name = "jwlawson_threads")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "gtest",
    urls = ["https://github.com/google/googletest/archive/release-1.8.1.zip"],
    sha256 = "927827c183d01734cc5cfef85e0ff3f5a92ffe6188e0d18e909c5efebf28a0c7",
    strip_prefix = "googletest-release-1.8.1",
)

http_archive(
    name = "com_github_absl",
    urls = ["https://github.com/abseil/abseil-cpp/archive/abea769b551f7a100f540967cb95debdb0080df8.zip"],
    sha256 = "6f367c8643e69f5b53444c49400ed21d11f09f76c5af99e36181248ad333410a",
    strip_prefix = "abseil-cpp-abea769b551f7a100f540967cb95debdb0080df8",
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
