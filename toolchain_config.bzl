#
# toolchain_config.bzl
#

def llvm_toolchain_config(repository_ctx):
    return {
        "cxx_builtin_include_directories": [
            "%{sysroot}/include",
            "%{sysroot}/usr/include",
        ],
        "extra_link_flags": [
            "-L%{sysroot}/lib",
            "-L%{sysroot}/usr/lib",
        ],
    }

# end