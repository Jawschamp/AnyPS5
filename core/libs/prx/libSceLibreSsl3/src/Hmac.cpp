#include "prx/libSceLibreSsl3/include/Api.hpp"
#include "prx/libc/include/Api.hpp"
#include "prx/libc/include/General.hpp"
#include <cstddef>

namespace {

constexpr std::size_t HmacContextSize = 304;
constexpr std::size_t HmacUsedPrefixSize = 152;

bool Untouched(const unsigned char* context) {
    for (std::size_t offset = 0; offset < HmacUsedPrefixSize; ++offset) {
        if (context[offset] != 0) return false;
    }
    return true;
}

}

extern "C" {

void* APS5_VABI HMAC_CTX_new_nid_postfix() {
    return calloc_nid_postfix(1, HmacContextSize);
}

void APS5_VABI HMAC_CTX_free_nid_postfix(void* context) {
    if (!context) return;
    auto* const bytes = static_cast<unsigned char*>(context);
    if (!Untouched(bytes)) NotImplemented_nid_no_patch(__func__);
    free_nid_postfix(bytes);
}

}
