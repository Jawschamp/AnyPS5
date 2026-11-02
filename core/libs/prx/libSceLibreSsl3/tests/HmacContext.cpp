#include "prx/libSceLibreSsl3/libSceLibreSsl3.hpp"
#include "prx/libc/libc.hpp"
#include "prx/libc/include/general/VabiMacros.hpp"
#include "prx/libc/include/ApplicationHeap.hpp"
#include <array>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <source_location>
#include <stdexcept>
#include <unordered_map>

namespace {

constexpr std::size_t ContextSize = 304;

void Require(bool condition, std::source_location location = std::source_location::current()) {
    if (condition) return;
    std::fprintf(stderr, "LibreSSL3 HMAC_CTX check failed at line %u\n", location.line());
    std::abort();
}

std::unordered_map<void*, std::size_t> allocations;
std::size_t callocCalls = 0;
std::size_t freeCalls = 0;
bool failAllocate = false;

void* APS5_VABI UnexpectedAllocate_nid_no_patch(std::size_t) { throw std::runtime_error("unexpected malloc"); }

void* APS5_VABI Calloc_nid_no_patch(std::size_t count, std::size_t size) {
    ++callocCalls;
    if (failAllocate) return nullptr;
    void* result = std::calloc(count, size);
    Require(result != nullptr);
    allocations.emplace(result, count * size);
    return result;
}

void APS5_VABI Free_nid_no_patch(void* pointer) {
    if (!pointer) return;
    Require(allocations.erase(pointer) == 1);
    ++freeCalls;
    std::free(pointer);
}

void* APS5_VABI UnexpectedReallocate_nid_no_patch(void*, std::size_t) { throw std::runtime_error("unexpected realloc"); }
void* APS5_VABI UnexpectedAlign_nid_no_patch(std::size_t, std::size_t) { throw std::runtime_error("unexpected aligned allocation"); }
void* APS5_VABI UnexpectedRealign_nid_no_patch(void*, std::size_t, std::size_t) { throw std::runtime_error("unexpected aligned reallocation"); }
int APS5_VABI UnexpectedPosixAlign_nid_no_patch(void**, std::size_t, std::size_t) { throw std::runtime_error("unexpected posix_memalign"); }

bool AllZero(const void* data, std::size_t size) {
    const auto* bytes = static_cast<const unsigned char*>(data);
    for (std::size_t index = 0; index < size; ++index) {
        if (bytes[index] != 0) return false;
    }
    return true;
}

void CreateAndFree() {
    callocCalls = 0;
    void* const context = HMAC_CTX_new_nid_postfix();
    Require(context != nullptr && callocCalls == 1);
    Require(allocations.size() == 1 && allocations.at(context) == ContextSize);
    Require(AllZero(context, ContextSize));
    void* const second = HMAC_CTX_new_nid_postfix();
    Require(second != nullptr && second != context && allocations.size() == 2);
    const std::size_t freesBefore = freeCalls;
    HMAC_CTX_free_nid_postfix(context);
    Require(freeCalls == freesBefore + 1 && allocations.size() == 1 && allocations.contains(second));
    HMAC_CTX_free_nid_postfix(second);
    Require(freeCalls == freesBefore + 2 && allocations.empty());
}

void FreeNull() {
    const std::size_t freesBefore = freeCalls;
    HMAC_CTX_free_nid_postfix(nullptr);
    Require(freeCalls == freesBefore);
}

void AllocationFailure() {
    failAllocate = true;
    Require(HMAC_CTX_new_nid_postfix() == nullptr && allocations.empty());
    failAllocate = false;
}

void FreeUsedContext() {
    for (const std::size_t offset : {std::size_t {0}, std::size_t {8}, std::size_t {55}, std::size_t {103}, std::size_t {151}}) {
        auto* const context = static_cast<unsigned char*>(HMAC_CTX_new_nid_postfix());
        Require(context != nullptr);
        context[offset] = 0x5a;
        const std::size_t freesBefore = freeCalls;
        bool thrown = false;
        try {
            HMAC_CTX_free_nid_postfix(context);
        } catch (const std::runtime_error&) {
            thrown = true;
        }
        Require(thrown && freeCalls == freesBefore && allocations.contains(context) && context[offset] == 0x5a);
        context[offset] = 0;
        HMAC_CTX_free_nid_postfix(context);
        Require(allocations.empty());
    }
    auto* const keyed = static_cast<unsigned char*>(HMAC_CTX_new_nid_postfix());
    Require(keyed != nullptr);
    std::memset(keyed + 152, 0xa5, ContextSize - 152);
    HMAC_CTX_free_nid_postfix(keyed);
    Require(allocations.empty());
}

}

int main() {
    std::array<void*, 10> api{
        reinterpret_cast<void*>(UnexpectedAllocate_nid_no_patch), reinterpret_cast<void*>(Free_nid_no_patch), reinterpret_cast<void*>(Calloc_nid_no_patch),
        reinterpret_cast<void*>(UnexpectedReallocate_nid_no_patch), reinterpret_cast<void*>(UnexpectedAlign_nid_no_patch), reinterpret_cast<void*>(UnexpectedRealign_nid_no_patch),
        reinterpret_cast<void*>(UnexpectedPosixAlign_nid_no_patch)};
    ApplicationHeapRegister_nid_no_patch(api.data());
    CreateAndFree();
    FreeNull();
    AllocationFailure();
    FreeUsedContext();
    std::puts("libSceLibreSsl3 HMAC_CTX tests passed");
}
