cmake \
-DCMAKE_FIND_ROOT_PATH="/opt/adtools;/opt/adtools/ppc-amigaos/SDK/clib4;/opt/adtools;/opt/adtools/ppc-amigaos/SDK/local/clib4;/opt/adtools/ppc-amigaos/SDK/local/common" \
-DCMAKE_LIBRARY_PATH="/opt/adtools/ppc-amigaos/SDK/clib4/lib;/opt/adtools/ppc-amigaos/SDK/local/clib4/lib" \
-DCMAKE_INCLUDE_PATH="/opt/adtools/ppc-amigaos/SDK/clib4/include;/opt/adtools/ppc-amigaos/SDK/local/clib4/include" \
-DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
-DCMAKE_SYSTEM_NAME="AmigaOS" \
-DCMAKE_SYSTEM_PROCESSOR="PowerPC" \
-DCMAKE_C_COMPILER="ppc-amigaos-gcc" \
-DCMAKE_CXX_COMPILER="ppc-amigaos-g++" \
-DCMAKE_ASM_COMPILER="ppc-amigaos-as" \
-DCMAKE_MAKE_PROGRAM="make" \
-DCMAKE_CXX_FLAGS_INIT="-mcrt=clib4 -athread=native" \
-DCMAKE_C_FLAGS_INIT="-mcrt=clib4 -athread=native" \
-DCMAKE_EXE_LINKER_FLAGS="-use-dynld -mcrt=clib4 -athread=native" \
-DUNIX=1 -DAMIGA=1 \
-DCMAKE_INSTALL_PREFIX="/qt6-amiga" \
-DCMAKE_PREFIX_PATH="/qt6-amiga" \
-DCMAKE_BUILD_TYPE=Release \
-DQT_HOST_PATH="/usr/local/Qt-6.2.0" \
-DQT_QMAKE_TARGET_MKSPEC=amiga-g++ \
-DQT_INSTALL_PREFIX="/qt6-amiga" \
-DQT_FEATURE_dlopen=ON \
-DQT_FEATURE_thread=ON \
-DQT_FEATURE_network=ON \
-DQT_FEATURE_ssl=ON \
-DQT_FEATURE_openssl=ON \
-DQT_FEATURE_openssl_linked=OFF \
-DQT_FEATURE_library=ON \
-DQT_FEATURE_concurrent=ON \
-DQT_FEATURE_sql=ON \
-DQT_FEATURE_future=ON \
-DQT_FEATURE_process=ON \
-DQT_FEATURE_processenvironment=ON \
-DQT_FEATURE_systemsemaphore=ON \
-DQT_FEATURE_brotli=ON \
-DQT_FEATURE_libudev=OFF \
-DQT_FEATURE_evdev=OFF \
-DQT_BUILD_TOOLS_WHEN_CROSSCOMPILING=ON \
-DPNG_PNG_INCLUDE_DIR="/opt/adtools/ppc-amigaos/SDK/local/clib4/include/libpng16" \
-DLIBXML2_LIBRARY="/opt/adtools/ppc-amigaos/SDK/local/clib4/lib/libxml2.a" \
-DJPEG_LIBRARY="/opt/adtools/ppc-amigaos/SDK/local/clib4/lib/libjpeg.a" \
..

# -DSQLite3_LIBRARY="/opt/adtools/ppc-amigaos/SDK/local/clib4/lib/libsqlite3.a" \
# -DSQLite3_INCLUDE_DIR="/opt/adtools/ppc-amigaos/SDK/local/clib4/include" \
# -DFREETYPE_LIBRARY="/opt/adtools/ppc-amigaos/SDK/local/clib4/lib/libfreetype.a" \
# -DFREETYPE_INCLUDE_DIRS="/opt/adtools/ppc-amigaos/SDK/local/clib4/include/freetype2" \
# -DHarfBuzz_LIBRARY="/opt/adtools/ppc-amigaos/SDK/local/clib4/lib/libharfbuzz.a" \
# -DHarfBuzz_INCLUDE_DIR="/opt/adtools/ppc-amigaos/SDK/local/clib4/include/harfbuzz" \
# -DHarfBuzz_ICU_LIBRARY="/opt/adtools/ppc-amigaos/SDK/local/clib4/lib/libharfbuzz-icu.a" \
# -DHarfBuzz_ICU_INCLUDE_DIR="/opt/adtools/ppc-amigaos/SDK/local/clib4/include/harfbuzz" \
# -DJPEG_LIBRARY="/opt/adtools/ppc-amigaos/SDK/local/clib4/lib/libjpeg.a" \
# -DJPEG_INCLUDE_DIR="/opt/adtools/ppc-amigaos/SDK/local/clib4/include" \
# -DPNG_PNG_INCLUDE_DIR="/opt/adtools/ppc-amigaos/SDK/local/clib4/include/libpng16" \
# -DPNG_LIBRARY="/opt/adtools/ppc-amigaos/SDK/local/clib4/lib/libpng16.a" \
# -DZLIB_LIBRARY="/opt/adtools/ppc-amigaos/SDK/local/clib4/lib/libz.a" \
# -DZLIB_INCLUDE_DIR="/opt/adtools/ppc-amigaos/SDK/local/clib4/include" \
# -DICU_INCLUDE_DIR="/opt/adtools/ppc-amigaos/SDK/local/clib4/include" \
# -DICU_LIBRARY="/opt/adtools/ppc-amigaos/SDK/local/clib4/lib/libicudata.a;/opt/adtools/ppc-amigaos/SDK/local/clib4/lib/libicui18n.a;/opt/adtools/ppc-amigaos/SDK/local/clib4/lib/libicuuc.a" \
# -DICU_DATA_LIBRARY_RELEASE="/opt/adtools/ppc-amigaos/SDK/local/clib4/lib/libicudata.a" \
# -DICU_I18N_LIBRARY_RELEASE="/opt/adtools/ppc-amigaos/SDK/local/clib4/lib/libicui18n.a" \
# -DICU_UC_LIBRARY_RELEASE="/opt/adtools/ppc-amigaos/SDK/local/clib4/lib/libicuuc.a" \
# -DLIBXML2_LIBRARY="/opt/adtools/ppc-amigaos/SDK/local/clib4/lib/libxml2.a" \
# -DLIBXML2_INCLUDE_DIR="/opt/adtools/ppc-amigaos/SDK/local/clib4/include/libxml2" \
# -DLIBXSLT_LIBRARY="/opt/adtools/ppc-amigaos/SDK/local/clib4/lib/libxslt.a" \
# -DLIBXSLT_INCLUDE_DIR="/opt/adtools/ppc-amigaos/SDK/local/clib4/include/libxslt" \
# -DWebP_LIBRARY="/opt/adtools/ppc-amigaos/SDK/local/clib4/lib/libwebp.a" \
# -DWebP_DEMUX_LIBRARY="/opt/adtools/ppc-amigaos/SDK/local/clib4/lib/libwebpdemux.a" \
# -DWebP_INCLUDE_DIR="/opt/adtools/ppc-amigaos/SDK/local/clib4/include/webp" \
