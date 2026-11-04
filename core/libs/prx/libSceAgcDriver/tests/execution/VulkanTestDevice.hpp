#ifndef CORE_LIBS_PRX_LIBSCEAGCDRIVER_TESTS_VULKANTESTDEVICE_HPP
#define CORE_LIBS_PRX_LIBSCEAGCDRIVER_TESTS_VULKANTESTDEVICE_HPP

#include "prx/libSceAgcDriver/Execution/include/VulkanDevice.hpp"
#include <spirv/unified1/spirv.hpp>
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <memory>
#ifdef __linux__
#include <fcntl.h>
#include <unistd.h>
#endif

constexpr int VulkanTestSkipped = 77;

inline bool DmaBufImportsUnavailable() {
#ifdef __linux__
    const int device = open("/dev/udmabuf", O_RDWR | O_CLOEXEC);
    if (device < 0) return true;
    close(device);
#endif
    return false;
}

inline bool TargetHasCapability(const ShaderRecompiler::SpirvTarget& target, spv::Capability capability) {
    const auto& capabilities = target.supportedCapabilities;
    return std::find(capabilities.begin(), capabilities.end(), static_cast<std::uint32_t>(capability)) != capabilities.end();
}

inline std::unique_ptr<AgcDriver::VulkanDevice> OpenVulkanTestDevice() {
    try {
        return std::make_unique<AgcDriver::VulkanDevice>();
    } catch (const AgcDriver::VulkanUnavailable& error) {
        if (std::getenv("ANYPS5_REQUIRE_VULKAN") != nullptr) throw;
        std::printf("skipped, no usable Vulkan device: %s\n", error.what());
        return nullptr;
    }
}

inline std::uint32_t PixelPushOffset(std::uint32_t vertexPush, const ShaderRecompiler::SpirvTarget& target) {
    return AgcDriver::Graphics::StagePushOffset(vertexPush, ShaderRecompiler::ShaderStage::Fragment, target.fixedPushSlots);
}

inline ShaderRecompiler::BindingLayout PixelPushLayout(std::uint32_t vertexPush, const ShaderRecompiler::SpirvTarget& target) {
    const auto offset = PixelPushOffset(vertexPush, target);
    return {0, 0, offset, AgcDriver::Graphics::PipelinePushSlotBytes - offset % AgcDriver::Graphics::PipelinePushSlotBytes};
}

#endif
