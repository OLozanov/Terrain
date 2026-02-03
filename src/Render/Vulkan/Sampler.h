#pragma once

#include <vulkan/vulkan.h>

namespace Render
{

class Sampler
{
    VkSampler m_sampler;

public:
    explicit Sampler(VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);
    ~Sampler();

    operator VkSampler() { return m_sampler; }
};

} // namespace Render
