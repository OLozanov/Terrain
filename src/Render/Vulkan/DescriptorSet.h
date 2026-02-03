#pragma once

#include <vulkan/vulkan.h>

namespace Render
{

class DescriptorSet
{
    VkDescriptorSet m_descriptorSet;
public:

    DescriptorSet(VkDescriptorSetLayout layout);

    void bind(uint32_t binding, VkBuffer buffer, VkDeviceSize size);
    void bind(uint32_t binding, VkImageView image, VkSampler sampler);
    void bind(uint32_t binding, uint32_t index, VkImageView image, VkSampler sampler);

    operator VkDescriptorSet() const { return m_descriptorSet; }
};

} // namespace Render
