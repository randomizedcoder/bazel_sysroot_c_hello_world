# c_hello_world

## Introduction and Motivation

The aim of this repository is to demonstrate how to use Bazel to:
- compile a "hello world" c++ program [hello.cc](./hello.cc)
- compile the hello.cc using a nix created sysroot containing the llvm tools
- focusing on amd64 only for the moment
- absolute minimial bazel configuration

The reason to use a nix created sysroot is to ensure that the toolchain is extremely repeatable, and should require very little effort to updated.  All that will be require should be "nix flake update" and then "nix build .".  This will make tracking the latest llvm version very easy.

## NixOS and Bazel Undeclared Dependencies

The other unique thing about this repository is that because I use NixOS, a lot of the assumptions Bazel makes aren't true. [Read about NixOS here](https://edolstra.github.io/pubs/nixos-jfp-final.pdf). NixOS doesn't follow the traditional [Filesystem Hierarchy Standard](https://en.wikipedia.org/wiki/Filesystem_Hierarchy_Standard), so when Bazel tries to use undeclared dependencies, like mktemp and rm, it assumes they will be in the usual places like /bin/, but they are not there on NixOS. In fact, I've been heavily using Nix for over a year and am amazed by its correctness, and now looking at Bazel which claims to be "correct", it's just amazing how untrue this is. Nix and Bazel both use sandbox approaches to achieve "hermetic" builds, but Nix does a much more complete job.

"Hermeticity is very much a matter of degree, unfortunately." - David Sanderson (DWS), on Buildbarn Slack

## Bazel 8 on NixOS
Please note that NixOS users should use the bazelisk package, and not bazel_7. There is no bazel_8 package, but Bazel 8 works perfectly well with bazelisk.  Therefore, not using the bazel command, but only bazelisk.

Using bazelisk on NixOS requires the lld tricks, but doing this means Bazel version 8.2.1 works well.
```
cat .bazelversion
8.2.1
#
# .bazelversion
#
```

## Bazel Modules
This repository also aims to use the new-ish Bazel modules, rather than "workspaces".

I suspect the modules system isn't as widely used, as of May 2025, as workspaces, because there are still a lot of Bazel repositories talking about workspace configurations.


## How toolchain_llvm works

To create the required sysroot and then configure bazel to have a usable toolchain_llvm we need to understand more about how toolchain_llvm works.

The default toolchain_llvm bazel module essentially has phases:
1. Download and compiles llvm
2. Find all the executable binaries, some libs, and some includes, to make them usable by bazel.

### Phase 1: LLVM Compilation
The module downloads the LLVM source code and compiles it using options to use shared libraries. This makes all the compiled binaries available.

### Phase 2: Toolchain Structure
The toolchain_llvm expects a specific directory structure in the sysroot. The tool definitions come from multiple sources:

1. Core tool requirements are defined in [rules_cc's unix_cc_configure.bzl](https://github.com/bazelbuild/rules_cc/blob/main/cc/private/toolchain/unix_cc_configure.bzl#L68), which specifies the essential tools needed:
   ```python
   [
       "ar",           # Archiver
       "ld",           # Linker
       "llvm-cov",     # Coverage tool
       "llvm-profdata",# Profile data tool
       "cpp",          # C preprocessor
       "gcc",          # C compiler
       "dwp",          # DWARF packager
       "gcov",         # Coverage tool
       "nm",           # Symbol table dumper
       "objcopy",      # Object copier
       "objdump",      # Object dumper
       "strip",        # Symbol stripper
       "c++filt",      # C++ symbol demangler
   ]
   ```

2. Additional tools required by [toolchain_llvm's common.bzl](https://github.com/bazel-contrib/toolchains_llvm/blob/master/toolchain/internal/common.bzl#L35):
   ```python
   [
       "clang-cpp",    # C preprocessor
       "clang-format", # Code formatter
       "clang-tidy",   # Static analyzer
       "clangd",       # Language server
       "ld.lld",       # LLVM linker
       "llvm-ar",      # LLVM archiver
       "llvm-dwp",     # LLVM DWARF packager
       "llvm-profdata",# LLVM profile data tool
       "llvm-cov",     # LLVM coverage tool
       "llvm-nm",      # LLVM symbol table dumper
       "llvm-objcopy", # LLVM object copier
       "llvm-objdump", # LLVM object dumper
       "llvm-strip",   # LLVM symbol stripper
   ]
   ```

   Note: Since version 1.4.0, `toolchain_llvm` requires `clangd`, `clang-format`, and `clang-tidy` to be present in the distribution. This is documented in [issue #481](https://github.com/bazel-contrib/toolchains_llvm/issues/481).

3. Standard tool aliases are defined in [toolchain_llvm's aliases.bzl](https://github.com/bazel-contrib/toolchains_llvm/blob/master/toolchain/aliases.bzl), which maps standard tool names to their LLVM counterparts.

In the sysroot the are [symlinks setup](https://github.com/randomizedcoder/bazel_sysroot_llvm_amd64/blob/main/default.nix#L135C1-L151C27) to enure that tools like "gcc" are available, however the sysroot.BUILD.bazel.in file also sets up the very same aliases.  This is taking a belt and braces approach to ensuring the sysroot will support rules_cc ability to detect and use the c toolchain.

```
      # Create GNU tool symlinks
      cd $out/sysroot/bin
      ln -sf clang gcc
      ln -sf clang cc
      ln -sf clang++ c++
      ln -sf clang-cpp cpp
      ln -sf llvm-ar ar
      ln -sf llvm-ar ranlib
      ln -sf llvm-as as
      ln -sf ld.lld ld
      ln -sf llvm-nm nm
      ln -sf llvm-objcopy objcopy
      ln -sf llvm-objdump objdump
      ln -sf llvm-strip strip
      ln -sf llvm-dwp dwp
      ln -sf llvm-c++filt c++filt
      ln -sf llvm-cov gcov
```

4. Additional compiler tools and their patterns are defined in [rules_cc's cc_toolchain_config.bzl](https://github.com/bazelbuild/rules_cc/blob/master/cc/private/toolchain/cc_toolchain_config.bzl).

The sysroot must provide all these tools in the following structure:

```
sysroot/
├── bin/                    # All executable tools
│   ├── clang              # Main C/C++ compiler (aliased as 'gcc')
│   ├── clang-cpp          # C preprocessor (aliased as 'cpp')
│   ├── clang++            # C++ compiler (aliased as 'g++')
│   ├── clang-format       # Code formatter (required since toolchain_llvm 1.4.0)
│   ├── clang-tidy         # Static analyzer (required since toolchain_llvm 1.4.0)
│   ├── clangd             # Language server (required since toolchain_llvm 1.4.0)
│   ├── ld.lld             # LLVM linker (aliased as 'ld')
│   ├── llvm-ar            # LLVM archiver (aliased as 'ar')
│   ├── llvm-as            # LLVM assembler (aliased as 'as')
│   ├── llvm-nm            # LLVM symbol table dumper (aliased as 'nm')
│   ├── llvm-objcopy       # LLVM object copier (aliased as 'objcopy')
│   ├── llvm-objdump       # LLVM object dumper (aliased as 'objdump')
│   ├── llvm-readelf       # LLVM ELF reader (aliased as 'readelf')
│   ├── llvm-strip         # LLVM symbol stripper (aliased as 'strip')
│   ├── llvm-dwp           # LLVM DWARF packager (aliased as 'dwp')
│   ├── llvm-cov           # LLVM coverage tool
│   ├── llvm-profdata      # LLVM profile data tool
│   └── llvm-c++filt       # LLVM C++ symbol demangler
├── include/               # Header files (required by rules_cc)
│   ├── c++/              # C++ standard library headers
│   └── clang/            # Clang-specific headers
└── lib/                  # Library files (required by rules_cc)
    ├── libc++.a          # LLVM C++ standard library
    ├── libc++abi.a       # LLVM C++ ABI library
    └── libunwind.a       # LLVM unwinder library
```

The `include` and `lib` directories are required by `rules_cc` for:
- Finding system headers
- Linking against standard libraries
- Resolving compiler and linker dependencies

These paths are used by the toolchain configuration to set up the correct include paths and library search paths for the compiler and linker.

Please note that we are NOT putting a BUILD.bazel file into the sysroot.  We will keep the bazel configuration outside the sysroot.  The "build_file_content" contains the contents for hte BUILD.bazel file.

Specifically, we can see that the bazel MODULE.bazel is importing the build_file from the file [sysroot.BUILD.bazel.in](./sysroot.BUILD.bazel.in)

```
http_archive(
    name = "bazel_sysroot_llvm_amd64",
    urls = ["https://github.com/randomizedcoder/bazel_sysroot_llvm_amd64/archive/refs/heads/main.tar.gz"],
    sha256 = "31912d32edc85fd9e5386d53eea663726961a78661b0fdd313bb3f569815ffb1",
    strip_prefix = "bazel_sysroot_llvm_amd64-main/sysroot",
    build_file = "//:sysroot.BUILD.bazel.in",                        <------- BUILD.bazel is NOT in the sysroot
)
```

## Nix Created Sysroots

The design for the sysroots is as follows. Please note that "sysroots" are the Bazel term for a .tar.gz that you can unpack into the sandbox.

## Nix bazel_sysroot_llvm_amd64 created sysroot

The URL for the sysroot is:
https://github.com/randomizedcoder/bazel_sysroot_llvm_amd64/

The files themselves are in:
https://github.com/randomizedcoder/bazel_sysroot_llvm_amd64/tree/main/sysroot

The main nix file to generate the sysroot is:
https://github.com/randomizedcoder/bazel_sysroot_llvm_amd64/blob/main/default.nix

To make it easy to check what's in the sysroot, I've included the list of files in the file [bazel_sysroot_llvm_amd64_file_list](./bazel_sysroot_llvm_amd64_file_list) in this repo.

## Latest Bazel Modules in Use

In this repo, we are trying to use Bazel modules, and the latest versions of these, which are shown in the following table.

| Module | Version | URL |
|--------|---------|-----|
| rules_cc | 0.1.1 | [bazelbuild/rules_cc](https://github.com/bazelbuild/rules_cc/tags) |
| platforms | 1.0.0 | [bazelbuild/platforms](https://github.com/bazelbuild/platforms/tags) |
| toolchains_llvm | 1.4.0 | [bazel-contrib/toolchains_llvm](https://github.com/bazel-contrib/toolchains_llvm/tags) |
| gazelle | 0.43.0 | [bazel-contrib/bazel-gazelle](https://github.com/bazel-contrib/bazel-gazelle/tags) |
| rules_go | 0.54.1 | [bazel-contrib/rules_go](https://github.com/bazel-contrib/rules_go/tags) |
| toolchain_llvm | 1.4.0 | [bazel-contrib/toolchains_llvm](https://github.com/bazel-contrib/toolchains_llvm/tags) |
