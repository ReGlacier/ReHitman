#pragma once

#ifndef ZASSERT
#include <Glacier/GlacierFWD.h>
#endif

// Test env (every assert makes exception)
#ifdef REHITMAN_TESTS
#   include <stdexcept>
#   include <string>

#   undef ZASSERT
#   define ZASSERT(expr) \
        do { \
            if (!(expr)) { \
                throw std::runtime_error("ZASSERT failed: " #expr " at " __FILE__ ":" + std::to_string(__LINE__)); \
            } \
        } while (0)

#   define ZHALT() throw std::runtime_error("HALT at " __FILE__ ":" + std::to_string(__LINE__))
#   define ZERROR(msg, ...) throw std::runtime_error("ZERROR: " + std::string(msg))
#   define ZWARN1(msg, ...) ((void)0)
#   define ZWARN2(msg, ...) ((void)0)
#   define ZWARN3(msg, ...) ((void)0)
#   define ZINFO(msg, ...)  ((void)0)
#   define ZMSG_CH(ch, msg, ...) ((void)0)

// Real Glacier-like macros
#elif defined(REHITMAN_USE_REAL_ZASSERT)
#   include <Glacier/Debug/sCall_u.h>

#   define ZASSERT_DEBUGBRK() __debugbreak()

// Internal
#   define ZASSERT_IMPL(expr, line) \
        do { \
            if (!(expr)) { \
                { \
                    sCall_u asc { line, __FILE__, ZDebug::eSEV_ERROR, false, false }; \
                    asc->(); \
                    asc._uPrint("%s(%d): ZASSERT(%s)", __FILE__, line, #expr); \
                } \
                { \
                    sCall_u asc { line, __FILE__, ZDebug::eSEV_ERROR, false, false }; \
                    asc->(); \
                    asc._uPrint("INT3 in %s at line %d", __FILE__, line); \
                } \
                ZASSERT_DEBUGBRK(); \
            } \
        } while (0)

#   define ZHALT_IMPL(line) \
        do { \
            sCall_u asc { line, __FILE__, ZDebug::eSEV_ERROR, false, false }; \
            asc->(); \
            asc._uPrint("INT3 in %s at line %d", __FILE__, line); \
            ZASSERT_DEBUGBRK(); \
        } while (0)

#   define ZLOG_GENERIC_IMPL(line, severity, fmt, ...) \
        do { \
            sCall_u asc { line, __FILE__, severity, false, false }; \
            asc->(); \
            asc._uPrint(fmt, ##__VA_ARGS__); \
        } while (0)

#   define ZLOG_CHANNEL_IMPL(line, severity, channel, fmt, ...) \
        do { \
            sCall_u asc { line, __FILE__, severity, false, false }; \
            asc->(); \
            asc._uMsg(channel, fmt, ##__VA_ARGS__); \
        } while (0)

// Public Glacier
#   define ZASSERT(expr)                    ZASSERT_IMPL(expr, __LINE__)
#   define ZHALT()                          ZHALT_IMPL(__LINE__)

#   define ZERROR(fmt, ...)                 ZLOG_GENERIC_IMPL(__LINE__, ZDebug::eSEV_ERROR, fmt, ##__VA_ARGS__)
#   define ZWARN1(fmt, ...)                 ZLOG_GENERIC_IMPL(__LINE__, ZDebug::eSEV_WARN1, fmt, ##__VA_ARGS__)
#   define ZWARN2(fmt, ...)                 ZLOG_GENERIC_IMPL(__LINE__, ZDebug::eSEV_WARN2, fmt, ##__VA_ARGS__)
#   define ZWARN3(fmt, ...)                 ZLOG_GENERIC_IMPL(__LINE__, ZDebug::eSEV_WARN3, fmt, ##__VA_ARGS__)
#   define ZINFO(fmt, ...)                  ZLOG_GENERIC_IMPL(__LINE__, ZDebug::eSEV_MSG, fmt, ##__VA_ARGS__)
#   define ZMSG_CH(channel, fmt, ...)       ZLOG_CHANNEL_IMPL(__LINE__, ZDebug::eSEV_MSG, channel, fmt, ##__VA_ARGS__)

// Default
#else
#   include <cassert>
#   include <cstdio>
#   include <cstdlib>

#   define ZASSERT(expr)                    assert(expr)
#   define ZHALT()                          do { std::fprintf(stderr, "ZHALT at %s:%d\n", __FILE__, __LINE__); std::abort(); } while(0)
#   define ZERROR(fmt, ...)                 std::fprintf(stderr, "[ERROR] (%s:%d): " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#   define ZWARN1(fmt, ...)                 std::fprintf(stderr, "[WARN1] (%s:%d): " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#   define ZWARN2(fmt, ...)                 std::fprintf(stderr, "[WARN2] (%s:%d): " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#   define ZWARN3(fmt, ...)                 std::fprintf(stderr, "[WARN3] (%s:%d): " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#   define ZINFO(fmt, ...)                  std::printf("[INFO] " fmt "\n", ##__VA_ARGS__)
#   define ZMSG_CH(channel, fmt, ...)       std::printf("[CH_%d] " fmt "\n", (int)channel, ##__VA_ARGS__)
#endif

// always safe for dtors and noexcept stubs
#define ZSAFE_ASSERT(expr) assert(expr)
