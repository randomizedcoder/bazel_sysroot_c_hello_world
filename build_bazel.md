# BUILD.bazel Design Document

## Overview
This document describes the design of the BUILD.bazel file, which builds three source files with both static and shared linking variants. Each target is designed to have minimal dependencies, using only what is absolutely necessary for the code to compile and run.

## Source Files

### 1. hello.c
A minimal C hello world program that only uses basic C standard library functions.
```c
#include <stdio.h>
int main() {
    printf("Hello, World!\n");
    return 0;
}
```

### 2. hello.cc
A minimal C++ hello world program that only uses basic C++ standard library features.
```cpp
#include <iostream>
int main() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
```

### 3. sysroot_library_test.cc
A comprehensive test program that exercises various libraries from the sysroot:
- Compression libraries:
  - bzip2 (libbz2)
  - zstd (libzstd)
  - zlib (libz)
- XML processing:
  - libxml2
- JSON handling:
  - jansson
- Cryptography:
  - OpenSSL (libcrypto)
- Regular expressions:
  - PCRE
  - PCRE2
  - RE2
- Image processing:
  - libpng16
  - libjpeg
- System libraries:
  - libc (C standard library)
  - libgcc (GCC support library)
  - libstdc++ (C++ standard library)
  - libsupc++ (C++ support library)

## Build Targets

### C Hello World Targets

#### hello_c_shared
- **Purpose**: Build hello.c with shared linking
- **Dependencies**:
  - libc.so (shared C standard library)
  - libgcc_s.so (shared GCC support library)
- **Compiler Options**: Basic C compiler flags
- **Linker Options**: Minimal shared library linking

#### hello_c_static
- **Purpose**: Build hello.c with static linking
- **Dependencies**:
  - libc.a (static C standard library)
  - libgcc.a (static GCC support library)
- **Compiler Options**: Basic C compiler flags
- **Linker Options**: Minimal static library linking

### C++ Hello World Targets

#### hello_cc_shared
- **Purpose**: Build hello.cc with shared linking
- **Dependencies**:
  - libc.so (shared C standard library)
  - libgcc_s.so (shared GCC support library)
  - libstdc++.so (shared C++ standard library)
  - libsupc++.a (static C++ support library)
- **Compiler Options**: C++ compiler flags + C++ standard library headers
- **Linker Options**: Shared library linking for C++

#### hello_cc_static
- **Purpose**: Build hello.cc with static linking
- **Dependencies**:
  - libc.a (static C standard library)
  - libgcc.a (static GCC support library)
  - libstdc++.a (static C++ standard library)
  - libsupc++.a (static C++ support library)
- **Compiler Options**: C++ compiler flags + C++ standard library headers
- **Linker Options**: Static library linking for C++

### Sysroot Library Test Targets

#### sysroot_library_test_cc_shared
- **Purpose**: Build sysroot_library_test.cc with shared linking
- **Dependencies**:
  - All C++ shared dependencies
  - Additional shared libraries:
    - libbz2.so
    - libzstd.so
    - libxml2.so
    - libjansson.so
    - libcrypto.so
    - libpcre.so
    - libpcre2-8.so
    - libre2.so
    - libpng16.so
    - libjpeg.so
    - libz.so
- **Compiler Options**: C++ compiler flags + all third-party headers
- **Linker Options**: Full shared library linking

#### sysroot_library_test_cc_static
- **Purpose**: Build sysroot_library_test.cc with static linking
- **Dependencies**:
  - All C++ static dependencies:
    - libc.a (static C standard library)
    - libgcc.a (static GCC support library)
    - libstdc++.a (static C++ standard library)
    - libsupc++.a (static C++ support library)
  - Additional shared libraries (static versions not available in sysroot):
    - libbz2.so.1 (bzip2 compression)
    - libzstd.so.1 (zstd compression)
    - libxml2.so.2 (XML processing)
    - libjansson.so.4 (JSON handling)
    - libcrypto.so.3 (OpenSSL cryptography)
    - libpcre.so.1 (PCRE regular expressions)
    - libpcre2-8.so.0 (PCRE2 regular expressions)
    - libre2.so.11 (RE2 regular expressions)
    - libpng16.so.16 (PNG image processing)
    - libjpeg.so.62 (JPEG image processing)
    - libz.so.1 (zlib compression)
- **Compiler Options**: C++ compiler flags + all third-party headers
- **Linker Options**: Mixed static/shared library linking
  - Core libraries (libc, libgcc, libstdc++) are linked statically
  - Third-party libraries are linked as shared libraries since static versions are not available in the sysroot

## Key Design Points

1. **Minimal Dependencies**
   - Each target only includes the dependencies it actually needs
   - Hello world targets have minimal dependencies
   - Sysroot test target includes all required libraries

2. **Static vs Shared Linking**
   - Static builds use .a files where available
   - Shared builds use .so files
   - Some libraries are only available as shared libraries

3. **Compiler Options**
   - Common options for all targets
   - C++-specific options for C++ targets
   - Additional include paths for sysroot test

4. **Linker Options**
   - Different linking strategies for static vs shared builds
   - Proper library ordering to avoid symbol conflicts
   - Special handling for C++ support libraries

5. **Include Path Resolution**
   - The compiler needs to find system headers in the correct order
   - C++ standard library headers must come BEFORE C standard library headers
   - This is because C++ headers use `#include_next` to find C headers
   - Required include path order:
     1. GCC internal headers
        ```
        -isystem <sysroot>/lib/gcc/x86_64-unknown-linux-gnu/14.2.1/include
        ```
     2. C++ standard library headers
        ```
        -isystem <sysroot>/include/c++/14.2.1.20250322
        -isystem <sysroot>/include/c++/14.2.1.20250322/x86_64-unknown-linux-gnu
        ```
     3. C standard library headers
        ```
        -isystem <sysroot>/include
        ```
     4. Architecture-specific headers
        ```
        -isystem <sysroot>/include/x86_64-unknown-linux-gnu
        -isystem <sysroot>/include/linux
        -isystem <sysroot>/include/asm
        -isystem <sysroot>/include/asm-generic
        -isystem <sysroot>/include/drm
        -isystem <sysroot>/include/uapi
        -isystem <sysroot>/include/uapi/asm-generic
        -isystem <sysroot>/include/uapi/linux
        -isystem <sysroot>/include/uapi/x86_64-linux-gnu
        -isystem <sysroot>/include/x86_64-linux-gnu
        ```
     5. Additional system headers
        ```
        -isystem <sysroot>/include/bits
        -isystem <sysroot>/include/gnu
        -isystem <sysroot>/include/sys
        ```
     6. Additional C++ standard library headers
        ```
        -isystem <sysroot>/include/c++/14.2.1.20250322/backward
        -isystem <sysroot>/include/c++/14.2.1.20250322/bits
        ```
   - Note: The order of include paths is critical for C++ builds because:
     - C++ standard library headers use `#include_next` to find C standard library headers
     - If the include paths are not in the correct order, the compiler will fail to find required headers
     - This is especially important for headers like `stdlib.h` which are needed by the C++ standard library
   - Common pitfalls:
     - Missing or incorrect architecture-specific paths
     - Incorrect order of C and C++ standard library paths (C++ must come before C)
     - Missing system header paths (bits, gnu, sys)
     - Including paths that don't exist in the sysroot

## Implementation Notes

1. The BUILD.bazel file should be organized to make these dependencies clear and maintainable
2. Common options and dependencies should be defined as variables
3. Each target should explicitly list its dependencies
4. The sysroot test target should be clearly separated from the hello world targets
5. Static builds should be marked with `linkstatic = True`
6. Include paths must be carefully ordered to ensure proper header resolution
7. C++ targets need both C and C++ standard library include paths

## Common Pitfalls

1. **Missing System Headers**
   - Error: `fatal error: 'stdlib.h' file not found`
   - Cause: Include paths not properly ordered or missing
   - Solution: Ensure all required include paths are present and in the correct order

2. **C++ Standard Library Dependencies**
   - The C++ standard library depends on the C standard library
   - C++ headers use `#include_next` to find C headers
   - Include paths must be ordered to support this mechanism

3. **Architecture-Specific Headers**
   - Some headers are architecture-specific
   - Must include both generic and architecture-specific paths
   - Example: x86_64-unknown-linux-gnu specific headers

4. **Third-Party Library Headers**
   - Additional include paths needed for each third-party library
   - Must be added after system and standard library paths
   - Example: libxml2, jansson, etc.

5. **LLVM libc Implementation and C++ Libraries**
   - **Problem**: LLVM's libc implementation (`libc.a`) contains C++ components (`.cpp.o` files)
   - **Symptom**: C programs fail to link with undefined symbols like `malloc`, `free`, `realloc` from `file.cpp.o` and `new.cpp.o`
   - **Root Cause**: The LLVM toolchain automatically adds C++ libraries (`libc++.a`, `libc++abi.a`, `libunwind.a`) even for C programs
   - **Solution**: Use specific linker flags to prevent unnecessary library linking:
     ```bash
     # Prevent C++ libraries from being linked for C programs
     "-Wl,--as-needed",        # Only link libraries that are actually needed
     "-Wl,--gc-sections",      # Remove unused sections during linking
     # Explicitly specify runtime library
     "-rtlib=compiler-rt",     # Use LLVM's compiler-rt for memory management functions
     ```
   - **Key insight**: The memory management functions (`malloc`, `free`, `realloc`) should come from the LLVM runtime library, not from C++ libraries
   - **Library Order**: LLVM runtime must be linked before `libc.a` to ensure memory management functions are available when needed

6. **C++ Static Linking Challenges**
   - **Problem**: C++ programs require many more system functions than C programs
   - **Symptom**: C++ static builds fail with undefined symbols for locale, threading, and system functions
   - **Required Libraries**: C++ static builds need additional system libraries:
     ```bash
     "-l:libm.so.6",        # Math functions (__strtof_l, __strtod_l) - shared version
     "-l:libpthread.so.0",  # Threading support (pthread_cond_wait) - shared version
     "-l:libdl.so.2",       # Dynamic loading (dladdr, dl_iterate_phdr) - shared version
     ```
   - **Challenge**: Some system libraries are only available as shared libraries in the sysroot
   - **Solution**: Use mixed static/shared linking for C++ programs:
     - **Important**: Do NOT use `-static` flag for C++ programs (causes "attempted static link of dynamic object" error)
     - Use `-static-libgcc` and `-static-libstdc++` instead to statically link core libraries
     - Core libraries (libc.a, libgcc.a, libstdc++.a, libsupc++.a) linked statically
     - System libraries (libm.so.6, libpthread.so.0, libdl.so.2) linked as shared
   - **Key insight**: C++ standard library has many dependencies on system functions that aren't needed for simple C programs
   - **Result**: Partially static binaries that require shared system libraries at runtime