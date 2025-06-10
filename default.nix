# Create symlinks to the original Nix GCC paths
mkdir -p $out/sysroot/usr/lib/gcc/x86_64-unknown-linux-gnu/14.2.1
ln -sf ${pkgs.gcc-unwrapped.out}/lib/gcc/x86_64-unknown-linux-gnu/14.2.1/include $out/sysroot/usr/lib/gcc/x86_64-unknown-linux-gnu/14.2.1/
ln -sf ${pkgs.gcc-unwrapped.out}/lib/gcc/x86_64-unknown-linux-gnu/14.2.1/include-fixed $out/sysroot/usr/lib/gcc/x86_64-unknown-linux-gnu/14.2.1/

# Copy fundamental C headers
echo "Copying fundamental C headers..."
mkdir -p $out/sysroot/usr/include
cp --dereference --recursive ${pkgs.glibc.dev}/include/* $out/sysroot/usr/include/
cp --dereference --recursive ${pkgs.glibc.dev}/include/bits $out/sysroot/usr/include/
cp --dereference --recursive ${pkgs.glibc.dev}/include/x86_64-unknown-linux-gnu $out/sysroot/usr/include/

# Copy core libraries
echo "Copying core libraries..."
if [ -d "${pkgs.glibc}/lib" ]; then
  for lib in ${pkgs.glibc}/lib/*.so*; do
    if [ -f "$lib" ]; then
      cp --dereference --recursive "$lib" $out/sysroot/usr/lib/ || true
    fi
  done
fi
if [ -d "${pkgs.glibc.dev}/lib" ]; then
  for lib in ${pkgs.glibc.dev}/lib/*.so*; do
    if [ -f "$lib" ]; then
      cp --dereference --recursive "$lib" $out/sysroot/usr/lib/ || true
    fi
  done
fi
if [ -d "${pkgs.glibc.static}/lib" ]; then
  for lib in ${pkgs.glibc.static}/lib/*.a; do
    if [ -f "$lib" ]; then
      cp --dereference --recursive "$lib" $out/sysroot/usr/lib/ || true
    fi
  done
fi
if [ -d "${pkgs.gcc-unwrapped.lib}/lib" ]; then
  for lib in ${pkgs.gcc-unwrapped.lib}/lib/*.so*; do
    if [ -f "$lib" ]; then
      cp --dereference --recursive "$lib" $out/sysroot/usr/lib/ || true
    fi
  done
fi
if [ -d "${pkgs.gcc-unwrapped.out}/lib" ]; then
  for lib in ${pkgs.gcc-unwrapped.out}/lib/*.so*; do
    if [ -f "$lib" ]; then
      cp --dereference --recursive "$lib" $out/sysroot/usr/lib/ || true
    fi
  done
fi

# Copy other libraries
for pkg in ${pkgs.zlib} ${pkgs.zlib.dev} ${pkgs.zlib.static} \
           ${pkgs.bzip2} ${pkgs.bzip2.dev} \
           ${pkgs.xz} ${pkgs.xz.dev} \
           ${pkgs.libxml2} ${pkgs.libxml2.dev} ${pkgs.libxml2.out} \
           ${pkgs.expat} ${pkgs.expat.dev} ${pkgs.expat.out} \
           ${pkgs.openssl} ${pkgs.openssl.dev} ${pkgs.openssl.out} \
           ${pkgs.curl} ${pkgs.curl.dev} ${pkgs.curl.out} \
           ${pkgs.pcre} ${pkgs.pcre.dev} ${pkgs.pcre.out} \
           ${pkgs.pcre2} ${pkgs.pcre2.dev} ${pkgs.pcre2.out} \
           ${pkgs.jansson} ${pkgs.jansson.dev} ${pkgs.jansson.out} \
           ${pkgs.sqlite} ${pkgs.sqlite.dev} ${pkgs.sqlite.out} \
           ${pkgs.libpng} ${pkgs.libpng.dev} ${pkgs.libpng.out} \
           ${pkgs.libjpeg} ${pkgs.libjpeg.dev} ${pkgs.libjpeg.out} \
           ${pkgs.util-linux} ${pkgs.util-linux.dev} ${pkgs.util-linux.out}; do
  if [ -d "$pkg/lib" ]; then
    for lib in $pkg/lib/*.so* $pkg/lib/*.a; do
      if [ -f "$lib" ]; then
        cp --dereference --recursive "$lib" $out/sysroot/usr/lib/ || true
      fi
    done
  fi
done