#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>

#include "Render/Vulkan/PhysicalDevice.h"
#include "Render/Vulkan/CommandList.h"

#ifdef NDEBUG
    #define ENABLE_VALIDATION_LAYERS false
#else
    #define ENABLE_VALIDATION_LAYERS true
#endif

struct Image;

namespace Render
{

class Buffer;

class VulkanInstance
{
    static const std::vector<const char*> ValidationLayers;
    static constexpr bool EnableValidationLayers = ENABLE_VALIDATION_LAYERS;

private:
    explicit VulkanInstance();
    ~VulkanInstance();

    VkInstance m_instance;
    std::vector<PhysicalDevice> m_physicalDevices;
    VkDevice m_device;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue = VK_NULL_HANDLE;

    VkDescriptorPool m_descriptorPool;

    VkCommandPool m_commandPool;
    CommandList m_commandList = VK_NULL_HANDLE;

    std::vector<const char*> m_validationLayers;

    std::vector<const char*> enumerateSupportedValidationLayers();
    std::vector<const char*> enumerateExtensions();

    void createInstance();
    void selectPhysicalDevice();
    void createLogicalDevice();
    void createDescriptorPool();
    void createCommandPool(VkCommandPool* commandPool);
    void createCommandList();

public:
    static VulkanInstance& GetInstance();

    VkCommandPool getCommandPool() { return m_commandPool; }

    void submit(VkCommandBuffer commandBuffer);

    uint32_t detectMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void swapChainSupportInfo(VkSurfaceKHR surface);

    VkSwapchainKHR createSwapChain(VkSurfaceKHR surface, VkExtent2D& imageExtent);
    void createTexture(Buffer& buffer, Image& image);
    void createBuffer(Buffer& buffer, const void* data, size_t size);

    void transitImageState(std::vector<VkImage>& images, VkImageLayout oldLayout, VkImageLayout newLayout);
    void transitImageState(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

    VkDescriptorPool descriptorPool() { return m_descriptorPool; }
    VkQueue presentQueue() { return m_presentQueue; }

    void waitIdle();
    VkDevice device() { return m_device; }

    operator VkInstance() { return m_instance; }

    friend class CommandList;
};

} // namespace Render
