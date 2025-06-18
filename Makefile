#
# Makefile
#

all: clean buildc buildcc

clean:
	bazelisk clean --expunge

# Build

build: build_hello build_sysroot_library_test_cc

build_hello: build_hello_c_shared build_hello_c_static build_hello_cc_shared build_hello_cc_static

build_hello_static: build_hello_c_static build_hello_cc_static
build_hello_shared: build_hello_c_shared build_hello_cc_shared

build_hello_c_shared:
	bazelisk build //:hello_c_shared --verbose_failures
build_hello_c_static:
	bazelisk build //:hello_c_static --verbose_failures

build_hello_cc_shared:
	bazelisk build //:hello_cc_shared --verbose_failures
build_hello_cc_static:
	bazelisk build //:hello_cc_static --verbose_failures

build_sysroot_library_test_cc: build_sysroot_library_test_cc_shared build_sysroot_library_test_cc_static

build_sysroot_library_test_cc_shared:
	bazelisk build //:sysroot_library_test_cc_shared --verbose_failures
build_sysroot_library_test_cc_static:
	bazelisk build //:sysroot_library_test_cc_static --verbose_failures

# Debugging builds

buildc:
	bazelisk build //:hello_c --verbose_failures --sandbox_debug

buildcc:
	bazelisk build //:hello_cc --verbose_failures --sandbox_debug

buildcc-debug:
	bazelisk build //:hello_cc --verbose_failures --sandbox_debug --experimental_skylark_debug --keep_state_after_build

# Run

run: run_hello run_sysroot_library_test_cc

run_hello: run_hello_c_shared run_hello_c_static run_hello_cc_shared run_hello_cc_static

run_hello_c_shared:
	bazelisk run //:hello_c_shared

run_hello_c_static:
	bazelisk run //:hello_c_static

run_hello_cc_shared:
	bazelisk run //:hello_cc_shared

run_hello_cc_static:
	bazelisk run //:hello_cc_static

run_sysroot_library_test_cc: run_sysroot_library_test_cc_shared run_sysroot_library_test_cc_static

run_sysroot_library_test_cc_shared:
	bazelisk run //:sysroot_library_test_cc_shared

run_sysroot_library_test_cc_static:
	bazelisk run //:sysroot_library_test_cc_static

# https://docs.stack.build/docs/cli/installation
install-bzl:
	curl -JLO https://get.bzl.io/linux_amd64/bzl

debug-adapter:
	./bzl debug adapter --make_default_workspace_content=false

query:
	bazelisk query @llvm_toolchain//:cc-toolchain-x86_64-linux

query_sysroot:
	bazelisk query '@bazel_sysroot_llvm_amd64//:sysroot'

query_deps:
	bazelisk query --output=build //:hello --noimplicit_deps --notool_deps

query_rules_cc:
	bazelisk query --output=build @rules_cc//...

output_base:
	bazelisk info output_base


calculate_checksum:
	curl -sSL https://github.com/randomizedcoder/bazel_sysroot_library_and_libs_amd64/archive/refs/heads/main.tar.gz | sha256sum

# [das@l:~/Downloads/c_hello_world]$ bazelisk query '@toolchains_llvm//...' --output=label_kind
# platform rule @toolchains_llvm//platforms:darwin-aarch64
# platform rule @toolchains_llvm//platforms:darwin-x86_64
# platform rule @toolchains_llvm//platforms:linux-aarch64
# platform rule @toolchains_llvm//platforms:linux-armv7
# platform rule @toolchains_llvm//platforms:linux-x86_64
# platform rule @toolchains_llvm//platforms:wasip1-wasm32
# platform rule @toolchains_llvm//platforms:wasip1-wasm64
# platform rule @toolchains_llvm//platforms:wasm32
# platform rule @toolchains_llvm//platforms:wasm64
# bool_flag rule @toolchains_llvm//toolchain/config:compiler-rt
# bool_flag rule @toolchains_llvm//toolchain/config:libunwind
# config_setting rule @toolchains_llvm//toolchain/config:use_compiler_rt
# config_setting rule @toolchains_llvm//toolchain/config:use_libunwind

# [das@l:~/Downloads/c_hello_world]$

# end
