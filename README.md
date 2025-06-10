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

## MaterializeInc toolchain

There was a recommendation to use MaterializeInc's toolchain, however this uses shared libraries at runtime.

https://github.com/MaterializeInc/toolchains
```
(exec env - \
    LD_LIBRARY_PATH=/nix/store/rl94syzwgdwi54wp66gpx4hywcsxrkyb-cursor-0.50.5-extracted/usr/lib/:/nix/store/rl94syzwgdwi54wp66gpx4hywcsxrkyb-cursor-0.50.5-extracted/usr/lib32/:/nix/store/rl94syzwgdwi54wp66gpx4hywcsxrkyb-cursor-0.50.5-extracted/usr/lib64/:/nix/store/rl94syzwgdwi54wp66gpx4hywcsxrkyb-cursor-0.50.5-extracted/lib/:/nix/store/rl94syzwgdwi54wp66gpx4hywcsxrkyb-cursor-0.50.5-extracted/lib/i386-linux-gnu/:/nix/store/rl94syzwgdwi54wp66gpx4hywcsxrkyb-cursor-0.50.5-extracted/lib/x86_64-linux-gnu/:/nix/store/rl94syzwgdwi54wp66gpx4hywcsxrkyb-cursor-0.50.5-extracted/lib/aarch64-linux-gnu/:/nix/store/rl94syzwgdwi54wp66gpx4hywcsxrkyb-cursor-0.50.5-extracted/lib32/:/nix/store/rl94syzwgdwi54wp66gpx4hywcsxrkyb-cursor-0.50.5-extracted/lib64/: \
    PATH=/home/das/.cache/bazelisk/downloads/sha256/7ff2b6a675b59a791d007c526977d5262ade8fa52efc8e0d1ff9e18859909fc0/bin:/run/wrappers/bin:/usr/bin:/usr/sbin:/nix/store/rl94syzwgdwi54wp66gpx4hywcsxrkyb-cursor-0.50.5-extracted/usr/bin/:/nix/store/rl94syzwgdwi54wp66gpx4hywcsxrkyb-cursor-0.50.5-extracted/usr/sbin/:/nix/store/rl94syzwgdwi54wp66gpx4hywcsxrkyb-cursor-0.50.5-extracted/usr/games/:/nix/store/rl94syzwgdwi54wp66gpx4hywcsxrkyb-cursor-0.50.5-extracted/bin/:/nix/store/rl94syzwgdwi54wp66gpx4hywcsxrkyb-cursor-0.50.5-extracted/sbin/:/nix/store/1q9lw4r2mbap8rsr8cja46nap6wvrw2p-bash-interactive-5.2p37/bin:/nix/store/v63bxfiacw082c7ijshf60alvvrpfxsq-binutils-2.44/bin:/nix/store/87fck6hm17chxjq7badb11mq036zbyv9-coreutils-9.7/bin:/nix/store/fcyn0dqszgfysiasdmkv1jh3syncajay-gawk-5.3.2/bin:/nix/store/a15rc9v3f5zb0wdxll7mxcidbvp78nny-libarchive-3.7.8/bin:/nix/store/95c0yh4a1jgw5sfg404sfd4v26h8vr1z-pv-1.9.31/bin:/nix/store/5liqs188bhx6cxfwd7rfhsgq7aj2v6ix-squashfs-4.6.1/bin:/run/wrappers/bin:/usr/bin:/usr/sbin:/run/wrappers/bin:/home/das/.nix-profile/bin:/nix/profile/bin:/home/das/.local/state/nix/profile/bin:/etc/profiles/per-user/das/bin:/nix/var/nix/profiles/default/bin:/run/current-system/sw/bin:/nix/store/wqa75pw63x1ab9ci3x3shrc3ychc06ja-ghostty-1.1.3/bin \
    PWD=/proc/self/cwd \
    TMPDIR=/tmp \
    ZERO_AR_DATE=1 \
  /home/das/.cache/bazel/_bazel_das/install/772f324362dbeab9bc869b8fb3248094/linux-sandbox -t 15 -w /dev/shm -w /home/das/.cache/bazel/_bazel_das/a84d885b8a0d42e22ab01a9042c210c2/sandbox/linux-sandbox/2/execroot/_main -w /tmp -M /home/das/.cache/bazel/_bazel_das/a84d885b8a0d42e22ab01a9042c210c2/sandbox/linux-sandbox/2/_hermetic_tmp -m /tmp -S /home/das/.cache/bazel/_bazel_das/a84d885b8a0d42e22ab01a9042c210c2/sandbox/linux-sandbox/2/stats.out -D /home/das/.cache/bazel/_bazel_das/a84d885b8a0d42e22ab01a9042c210c2/sandbox/linux-sandbox/2/debug.out -- /bin/sh -i)
ERROR: /home/das/Downloads/bazel_sysroot_c_hello_world/BUILD.bazel:5:10: Linking hello failed: (Exit 1): linux-sandbox failed: error executing CppLink command
  (cd /home/das/.cache/bazel/_bazel_das/a84d885b8a0d42e22ab01a9042c210c2/sandbox/linux-sandbox/2/execroot/_main && \
  exec env - \
    LD_LIBRARY_PATH=/nix/store/rl94syzwgdwi54wp66gpx4hywcsxrkyb-cursor-0.50.5-extracted/usr/lib/:/nix/store/rl94syzwgdwi54wp66gpx4hywcsxrkyb-cursor-0.50.5-extracted/usr/lib32/:/nix/store/rl94syzwgdwi54wp66gpx4hywcsxrkyb-cursor-0.50.5-extracted/usr/lib64/:/nix/store/rl94syzwgdwi54wp66gpx4hywcsxrkyb-cursor-0.50.5-extracted/lib/:/nix/store/rl94syzwgdwi54wp66gpx4hywcsxrkyb-cursor-0.50.5-extracted/lib/i386-linux-gnu/:/nix/store/rl94syzwgdwi54wp66gpx4hywcsxrkyb-cursor-0.50.5-extracted/lib/x86_64-linux-gnu/:/nix/store/rl94syzwgdwi54wp66gpx4hywcsxrkyb-cursor-0.50.5-extracted/lib/aarch64-linux-gnu/:/nix/store/rl94syzwgdwi54wp66gpx4hywcsxrkyb-cursor-0.50.5-extracted/lib32/:/nix/store/rl94syzwgdwi54wp66gpx4hywcsxrkyb-cursor-0.50.5-extracted/lib64/: \
    PATH=/home/das/.cache/bazelisk/downloads/sha256/7ff2b6a675b59a791d007c526977d5262ade8fa52efc8e0d1ff9e18859909fc0/bin:/run/wrappers/bin:/usr/bin:/usr/sbin:/nix/store/rl94syzwgdwi54wp66gpx4hywcsxrkyb-cursor-0.50.5-extracted/usr/bin/:/nix/store/rl94syzwgdwi54wp66gpx4hywcsxrkyb-cursor-0.50.5-extracted/usr/sbin/:/nix/store/rl94syzwgdwi54wp66gpx4hywcsxrkyb-cursor-0.50.5-extracted/usr/games/:/nix/store/rl94syzwgdwi54wp66gpx4hywcsxrkyb-cursor-0.50.5-extracted/bin/:/nix/store/rl94syzwgdwi54wp66gpx4hywcsxrkyb-cursor-0.50.5-extracted/sbin/:/nix/store/1q9lw4r2mbap8rsr8cja46nap6wvrw2p-bash-interactive-5.2p37/bin:/nix/store/v63bxfiacw082c7ijshf60alvvrpfxsq-binutils-2.44/bin:/nix/store/87fck6hm17chxjq7badb11mq036zbyv9-coreutils-9.7/bin:/nix/store/fcyn0dqszgfysiasdmkv1jh3syncajay-gawk-5.3.2/bin:/nix/store/a15rc9v3f5zb0wdxll7mxcidbvp78nny-libarchive-3.7.8/bin:/nix/store/95c0yh4a1jgw5sfg404sfd4v26h8vr1z-pv-1.9.31/bin:/nix/store/5liqs188bhx6cxfwd7rfhsgq7aj2v6ix-squashfs-4.6.1/bin:/run/wrappers/bin:/usr/bin:/usr/sbin:/run/wrappers/bin:/home/das/.nix-profile/bin:/nix/profile/bin:/home/das/.local/state/nix/profile/bin:/etc/profiles/per-user/das/bin:/nix/var/nix/profiles/default/bin:/run/current-system/sw/bin:/nix/store/wqa75pw63x1ab9ci3x3shrc3ychc06ja-ghostty-1.1.3/bin \
    PWD=/proc/self/cwd \
    TMPDIR=/tmp \
    ZERO_AR_DATE=1 \
  /home/das/.cache/bazel/_bazel_das/install/772f324362dbeab9bc869b8fb3248094/linux-sandbox -t 15 -w /dev/shm -w /home/das/.cache/bazel/_bazel_das/a84d885b8a0d42e22ab01a9042c210c2/sandbox/linux-sandbox/2/execroot/_main -w /tmp -M /home/das/.cache/bazel/_bazel_das/a84d885b8a0d42e22ab01a9042c210c2/sandbox/linux-sandbox/2/_hermetic_tmp -m /tmp -S /home/das/.cache/bazel/_bazel_das/a84d885b8a0d42e22ab01a9042c210c2/sandbox/linux-sandbox/2/stats.out -D /home/das/.cache/bazel/_bazel_das/a84d885b8a0d42e22ab01a9042c210c2/sandbox/linux-sandbox/2/debug.out -- external/toolchains_llvm++llvm+llvm_toolchain/bin/cc_wrapper.sh @bazel-out/k8-fastbuild/bin/hello-0.params)
external/toolchains_llvm++llvm+llvm_toolchain_llvm/bin/ld.lld: error while loading shared libraries: libxml2.so.2: cannot open shared object file: No such file or directory
clang: error: unable to execute command: No such file or directory
clang: error: linker command failed due to signal (use -v to see invocation)
Target //:hello failed to build
INFO: Elapsed time: 12.355s, Critical Path: 0.77s
INFO: 10 processes: 9 internal, 1 linux-sandbox.
ERROR: Build did NOT complete successfully
make: *** [Makefile:11: build] Error 1

[das@l:~/Downloads/bazel_sysroot_c_hello_world]$
```

## Monogon Bazel hermetic LLVM toolchain

https://review.monogon.dev/plugins/gitiles/toolchain_cc

```
[das@l:~/Downloads]$ git clone "https://review.monogon.dev/toolchain_cc"
Cloning into 'toolchain_cc'...
remote: Counting objects: 26, done
remote: Finding sources: 100% (26/26)
remote: Total 26 (delta 2), reused 26 (delta 2)
Receiving objects: 100% (26/26), 23.57 KiB | 23.57 MiB/s, done.
Resolving deltas: 100% (2/2), done.
```