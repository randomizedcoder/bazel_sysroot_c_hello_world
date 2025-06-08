#!/bin/bash

# Create a temporary directory
TEMP_DIR=$(mktemp -d)
echo "Created temporary directory: $TEMP_DIR"

# Function to download and calculate hash
download_and_hash() {
    local name=$1
    local url=$2
    local output_file="$TEMP_DIR/${name}.tar.gz"

    echo "Downloading $name..."
    curl -L "$url" -o "$output_file"

    if [ $? -eq 0 ]; then
        echo "Calculating SHA256 hash for $name..."
        sha256sum "$output_file" | awk '{print $1}'
    else
        echo "Failed to download $name"
        return 1
    fi
}

# List of sysroots to process
 declare -A SYSROOTS=(
     ["bazel_sysroot_llvm_amd64"]="https://github.com/randomizedcoder/bazel_sysroot_llvm_amd64/archive/refs/heads/main.tar.gz"
     ["bazel_sysroot_llvm_arm64"]="https://github.com/randomizedcoder/bazel_sysroot_llvm_arm64/archive/refs/heads/main.tar.gz"
 )

#declare -A SYSROOTS=(
#    ["bazel_sysroot_library"]="https://github.com/randomizedcoder/bazel_sysroot_library/archive/refs/heads/main.tar.gz"
#    ["bazel_sysroot_lib_amd64"]="https://github.com/randomizedcoder/bazel_sysroot_lib_amd64/archive/refs/heads/main.tar.gz"
#    ["bazel_sysroot_lib_arm64"]="https://github.com/randomizedcoder/bazel_sysroot_lib_arm64/archive/refs/heads/main.tar.gz"
#    ["bazel_sysroot_llvm_amd64"]="https://github.com/randomizedcoder/bazel_sysroot_llvm_amd64/archive/refs/heads/main.tar.gz"
#    ["bazel_sysroot_llvm_arm64"]="https://github.com/randomizedcoder/bazel_sysroot_llvm_arm64/archive/refs/heads/main.tar.gz"
#)

# Process each sysroot
echo "Processing sysroots..."
echo "----------------------------------------"
for name in "${!SYSROOTS[@]}"; do
    echo "$name:"
    hash=$(download_and_hash "$name" "${SYSROOTS[$name]}")
    if [ $? -eq 0 ]; then
        echo "SHA256: $hash"
        echo "----------------------------------------"
    fi
done

# Clean up
echo "Cleaning up temporary directory..."
rm -rf "$TEMP_DIR"
echo "Done!"
