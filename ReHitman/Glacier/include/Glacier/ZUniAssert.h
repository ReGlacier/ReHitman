#pragma once

// INCLUDE ONLY IN CPP AND ONLY AFTER ALL HEADERS!!!
#ifdef REHITMAN_TESTS
#   include <stdexcept>
#   include <string>
#   undef ZASSERT
#   define ZASSERT(expr) \
        if (!(expr)) { \
            throw std::runtime_error("ZASSERT failed: " #expr " at " __FILE__ ":" + std::to_string(__LINE__)); \
        }
#endif