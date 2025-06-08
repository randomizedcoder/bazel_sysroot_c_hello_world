# Bazel LLVM Toolchain Diagnostic Plan

#
# diagnostic_plan.md
#

## Introduction

We are facing challenges in setting up a hermetic Bazel build using LLVM tools from a Nix-created sysroot. The core issue appears to be around toolchain registration and resolution, with errors consistently indicating that Bazel cannot find the expected toolchain targets.

The key symptoms we've observed:
1. Toolchain target resolution failures
2. Inconsistent toolchain registration
3. Potential issues with sysroot integration
4. Oscillating between different toolchain target names without success

This suggests we need a systematic approach to diagnose the root cause rather than making incremental changes to the configuration.

## Current Status
- Project: C++ Hello World using Bazel with Nix-created sysroot
- Issue: Build failures related to toolchain configuration
- Environment: Linux, using LLVM toolchain

## Diagnostic Steps

### 1. Verify Sysroot Download and Extraction ✅
- **Status**: COMPLETED
- **Findings**:
  - Sysroot is properly downloaded and extracted
  - Contains all necessary LLVM/Clang components
  - Repository structure is correct with bin/, lib/, and include/ directories
- **Next Steps**: None needed, sysroot is properly configured

### 2. Check Repository Rules and Visibility ✅
- **Status**: COMPLETED
- **Findings**:
  - Repository is properly configured in MODULE.bazel
  - All necessary filegroups are defined in the sysroot's BUILD.bazel
  - Components are properly exposed with public visibility
- **Next Steps**: None needed, repository rules are correct

### 3. Inspect Bazel Cache and Repository Rules ✅
- **Status**: COMPLETED
- **Findings**:
  - Repository is properly recognized by Bazel
  - All tool components are available through filegroup targets
  - No direct toolchain target is defined in the sysroot
- **Next Steps**: None needed, cache and rules are properly configured

### 4. Analyze Toolchain Configuration 🔄
- **Status**: IN PROGRESS
- **Findings**:
  - We've been trying to create a custom toolchain configuration instead of using toolchains_llvm
  - The example in toolchains_llvm/test/MODULE.bazel shows a simpler approach
  - Our sysroot is already properly structured for toolchains_llvm
  - We should use toolchains_llvm's extension system instead of custom configuration
- **Next Steps**:
  1. Revert our custom toolchain configuration
  2. Use toolchains_llvm's extension system
  3. Configure the sysroot using llvm.sysroot() extension

## Key Findings (updated)
1. The sysroot is present and its BUILD.bazel is correct.
2. We've been overcomplicating the solution by creating a custom toolchain.
3. The toolchains_llvm module already provides the functionality we need.
4. Our sysroot structure matches what toolchains_llvm expects.
5. We should use toolchains_llvm's extension system instead of custom configuration.

## Next Steps (revised)
1. Revert our custom toolchain configuration:
   - Remove custom toolchain.bzl
   - Remove custom toolchain_config.bzl
   - Simplify BUILD.bazel

2. Use toolchains_llvm's extension system:
   - Add toolchains_llvm dependency to MODULE.bazel
   - Use llvm.toolchain() extension
   - Use llvm.sysroot() extension to point to our sysroot
   - Register the toolchain using toolchains_llvm's registration

3. Test the new configuration:
   - Build with the new configuration
   - Verify toolchain resolution
   - Check for any remaining issues

4. Document the working solution:
   - Update README with the correct approach
   - Document any gotchas or special considerations
   - Add examples for future reference

## Current Status

1. Sysroot Integration (Step 1):
   - ✅ Sysroot download and structure verified
   - ✅ Build file application confirmed
   - ✅ All necessary tools and libraries present

2. Toolchain Configuration (Step 2):
   - 🔄 Need to revert custom configuration
   - 🔄 Need to implement toolchains_llvm approach
   - 🔄 Need to configure sysroot integration

3. Build Process (Step 3):
   - 🔄 Need to test with new configuration
   - 🔄 Need to verify toolchain resolution
   - 🔄 Need to check for any remaining issues

4. Module Configuration (Step 4):
   - 🔄 Need to update module configuration
   - 🔄 Need to add toolchains_llvm dependency
   - 🔄 Need to configure toolchain extensions

## Expected Outcomes

For each step, we should:
1. Document the actual output
2. Compare with expected output
3. Identify any discrepancies
4. Note any error messages or warnings

## Next Steps

After completing the diagnostic steps:
1. Review all collected information
2. Identify the root cause of the toolchain resolution issues
3. Propose specific configuration changes
4. Test changes incrementally
5. Document successful configuration

## New Error and Current Status (as of latest build attempt)

**Observed Error:**
```
ERROR: /home/das/Downloads/bazel_sysroot_c_hello_world/BUILD.bazel:7:22: //:llvm_cc_toolchain: invalid label '@bazel_sysroot_llvm_amd64:sysroot' in attribute 'all_files' of 'cc_toolchain': invalid repository name 'bazel_sysroot_llvm_amd64:sysroot': repo names may contain only A-Z, a-z, 0-9, '-', '_', '.' and '+'
...
```
- This error is fundamental: Bazel is rejecting the way we reference the sysroot repository in our toolchain definition.
- The sysroot is present and its BUILD.bazel is correct, but our toolchain.bzl and BUILD.bazel are referencing it with a label that Bazel does not accept.
- All attempts to enable toolchain resolution debugging in .bazelrc have failed due to Bazel's flag parsing.
- The root cause appears to be a miswiring of repository names/labels in the toolchain registration.

## Key Findings (updated)
1. The sysroot is present and its BUILD.bazel is correct.
2. Our custom toolchain.bzl and BUILD.bazel are attempting to reference the sysroot using a repository name that Bazel does not accept.
3. Bazel's label syntax for external repositories is stricter than expected; the label format we are using is not valid.
4. All attempts to set up toolchain resolution debugging in .bazelrc have failed due to Bazel's flag parsing.
5. The toolchain registration and label wiring is likely the root cause of the build failure.

## Next Steps (revised)
1. Investigate the correct way to reference the sysroot repository in toolchain definitions:
   - Review Bazel documentation on label syntax and external repository naming.
   - Check if the sysroot repository is being created with a name that is not valid for label references, and if so, how to work around this.
2. Research Bazel's rules for repository naming and label syntax, especially for external repositories created by http_archive or similar.
3. Consider using Bazel's label canonicalization or repository mapping features if available.
4. Review examples from Bazel and rules_cc/rules_toolchains documentation for sysroot/toolchain integration.
5. If needed, simplify the toolchain definition to a minimal working example and incrementally add sysroot integration.
6. Document any new findings and update the plan as we proceed.

