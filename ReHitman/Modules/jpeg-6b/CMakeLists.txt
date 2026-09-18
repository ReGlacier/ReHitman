cmake_minimum_required(VERSION 3.16)
project(jpeg-6b LANGUAGES C)

# Independent JPEG Group's JPEG library (release 6b). Clean C sources, no external deps.

set(JPEG6B_SOURCES
    jcapimin.c
    jcapistd.c
    jccoefct.c
    jccolor.c
    jcdctmgr.c
    jchuff.c
    jcinit.c
    jcmainct.c
    jcmarker.c
    jcmaster.c
    jcomapi.c
    jcparam.c
    jcphuff.c
    jcprepct.c
    jcsample.c
    jctrans.c
    jdapimin.c
    jdapistd.c
    jdatadst.c
    jdatasrc.c
    jdcoefct.c
    jdcolor.c
    jddctmgr.c
    jdhuff.c
    jdinput.c
    jdmainct.c
    jdmarker.c
    jdmaster.c
    jdmerge.c
    jdphuff.c
    jdpostct.c
    jdsample.c
    jdtrans.c
    jerror.c
    jfdctflt.c
    jfdctfst.c
    jfdctint.c
    jidctflt.c
    jidctfst.c
    jidctint.c
    jidctred.c
    jmemansi.c
    jmemmgr.c
    jmemnobs.c
    jquant1.c
    jquant2.c
    jutils.c
)

add_library(jpeg STATIC ${JPEG6B_SOURCES})

target_include_directories(jpeg PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(jpeg PRIVATE /W3)
    # libjpeg 6b is written for older C dialects; silence harmless C4244 (loss of data in
    # conversions) warnings that MSVC emits throughout the codec.
    target_compile_options(jpeg PRIVATE /wd4244 /wd4305)
endif()