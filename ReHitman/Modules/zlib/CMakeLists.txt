cmake_minimum_required(VERSION 4.0)
project(zlib)

set(ZLIB_SOURCES
    impl/adler32.c
    impl/compress.c
    impl/crc32.c
    impl/deflate.c
    impl/gzio.c
    impl/infblock.c
    impl/infcodes.c
    impl/inffast.c
    impl/inflate.c
    impl/inftrees.c
    impl/infutil.c
    impl/trees.c
    impl/uncompr.c
    impl/zutil.c
)

add_library(zlib STATIC ${ZLIB_SOURCES})
target_include_directories(zlib PUBLIC impl)

set_target_properties(zlib PROPERTIES COMPILE_WARNING_AS_ERROR OFF)
if(MSVC)
    target_compile_options(zlib PRIVATE /W0)
else()
    target_compile_options(zlib PRIVATE -w)
endif()