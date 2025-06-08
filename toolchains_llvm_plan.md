# Toolchains LLVM Implementation Plan

## Overview
Based on the example from [toolchains_llvm tests](https://github.com/bazel-contrib/toolchains_llvm/blob/1.0.0/tests/MODULE.bazel#L145), we will implement a simpler approach using toolchains_llvm's extension system instead of our custom toolchain configuration.

## Current Configuration
1. MODULE.bazel:
   - ✅ Added toolchains_llvm dependency
   - ✅ Configured http_archive for sysroot
   - ✅ Set up llvm extension with toolchain and sysroot

2. BUILD.bazel:
   - ✅ Simplified to just the hello world target
   - ✅ Removed custom toolchain configuration

## Implementation Steps

### 1. Verify Toolchains LLVM Setup
- [ ] Confirm toolchains_llvm version compatibility
- [ ] Verify extension system is working
- [ ] Check if any additional dependencies are needed

### 2. Sysroot Integration
- [ ] Verify sysroot path is correctly referenced
- [ ] Check if sysroot structure matches toolchains_llvm expectations
- [ ] Test sysroot visibility and accessibility

### 3. Toolchain Configuration
- [ ] Verify LLVM version (17.0.6) is available
- [ ] Check if additional toolchain options are needed
- [ ] Test toolchain registration

### 4. Build Testing
- [ ] Run initial build
- [ ] Check for any toolchain resolution issues
- [ ] Verify compiler and linker paths
- [ ] Test with different build configurations

## Expected Configuration

### MODULE.bazel
```python
module(
    name = "bazel_sysroot_c_hello_world",
    version = "1.0",
)

bazel_dep(name = "rules_cc", version = "0.0.9")
bazel_dep(name = "platforms", version = "0.0.8")
bazel_dep(name = "toolchains_llvm", version = "0.12.0")

http_archive(
    name = "bazel_sysroot_llvm_amd64",
    urls = ["file:///home/das/Downloads/bazel_sysroot_llvm_amd64.tar.gz"],
    sha256 = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
    build_file = "//:sysroot.BUILD.bazel",
)

llvm = use_extension("@toolchains_llvm//:extensions.bzl", "llvm")
llvm.toolchain(version = "17.0.6")
llvm.sysroot(
    name = "sysroot",
    path = "@bazel_sysroot_llvm_amd64//:sysroot",
)
use_repo(llvm)
```

### BUILD.bazel
```python
cc_binary(
    name = "hello",
    srcs = ["hello.cc"],
)
```

## Potential Issues to Watch For
1. Toolchain version compatibility
2. Sysroot path resolution
3. Repository visibility
4. Build configuration conflicts

## Success Criteria
1. Clean build of hello world target
2. Correct toolchain resolution
3. Proper sysroot integration
4. No custom toolchain configuration needed

## Next Steps
1. Remove custom toolchain files:
   - [ ] Delete toolchain.bzl
   - [ ] Delete toolchain_config.bzl
2. Test the new configuration
3. Document any issues or solutions
4. Update README with new approach