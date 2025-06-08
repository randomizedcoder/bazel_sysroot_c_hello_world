#
# Makefile
#

all: clean build

clean:
	bazelisk clean --expunge

build:
	bazelisk build //:hello --verbose_failures --sandbox_debug

build-debug:
	bazelisk build //:hello --verbose_failures --sandbox_debug --experimental_skylark_debug --keep_state_after_build

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