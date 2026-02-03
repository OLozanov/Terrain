#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif

#define VK_USE_PLATFORM_WIN32_KHR

#include "VulkanInstance.h"

#include "Render/Vulkan/Buffer.h"
#include "Resources/Image.h"

#include <iostream>
#include "string.h"

namespace Render
{

const std::vector<const char*> VulkanInstance::ValidationLayers =
{
    "VK_LAYER_KHRONOS_validation",
    "VK_LAYER_LUNARG_core_validation"
};

VulkanInstance::VulkanInstance()
{
    createInstance();
    selectPhysicalDevice();
    createLogicalDevice();
    createDescriptorPool();
    createCommandPool(&m_commandPool);
    createCommandList();

    CommandList::LoadExtFunctions(m_device);
}

VulkanInstance::~VulkanInstance()
{
    vkDeviceWaitIdle(m_device);

    vkFreeCommandBuffers(m_device, m_commandPool, 1, &m_commandList);
    m_commandList = VK_NULL_HANDLE;

    vkDestroyCommandPool(m_device, m_commandPool, nullptr);
    vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);

    vkDestroyDevice(m_device, nullptr);

    vkDestroyInstance(m_instance, nullptr);
}

VulkanInstance& VulkanInstance::GetInstance()
{
    static VulkanInstance instance;

    return instance;
}

std::vector<const char*> VulkanInstance::enumerateSupportedValidationLayers()
{
    std::vector<const char*> supportedLayers;

    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for(const char * layer : ValidationLayers)
    {
        for(const auto& layerProperties : availableLayers)
        {
            //std::cout << layerProperties.layerName << std::endl;

            if(strcmp(layer, layerProperties.layerName) == 0)
            {
                std::cout << "Found validation layer: " << layerProperties.layerName << std::endl;
                supportedLayers.push_back(layer);
            }
        }
    }

    return supportedLayers;
}

std::vector<const char*> VulkanInstance::enumerateExtensions()
{
    std::vector<const char*> extensionNames;

    uint32_t extensionCount;
    if(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr) != VK_SUCCESS)
        throw std::runtime_error("Extension enumeration failure");

    std::vector<VkExtensionProperties> extensions(extensionCount);
    if(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data()) != VK_SUCCESS)
        throw std::runtime_error("Extension enumeration failure");

    for(const auto& extension : extensions)
    {
        if(!strcmp(VK_KHR_SURFACE_EXTENSION_NAME, extension.extensionName))
        {
            extensionNames.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
        }

        if(!strcmp(VK_KHR_WIN32_SURFACE_EXTENSION_NAME, extension.extensionName))
        {
            extensionNames.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
        }
    }

    return extensionNames;
}

void VulkanInstance::createInstance()
{
    std::vector<const char*> extensions = enumerateExtensions();

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan App";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();

    m_validationLayers = enumerateSupportedValidationLayers();

    if(EnableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
        createInfo.ppEnabledLayerNames = m_validationLayers.data();
    }

    if(vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS)
    {
        std::runtime_error("Error creating Vulkan instance");
    }

    std::cout << "Vulkan instance created" << std::endl;
}

void VulkanInstance::selectPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

    if(deviceCount == 0) throw std::runtime_error("failed to find GPUs with Vulkan support!");

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

    std::cout << "Physical devices:" << std::endl;

    for(const auto& device : devices)
    {
        PhysicalDevice physicalDevice(device);

        if(physicalDevice.isSuitable()) m_physicalDevices.push_back(physicalDevice);
    }

    if(m_physicalDevices.empty()) throw std::runtime_error("failed to find a suitable GPU!");
}

void VulkanInstance::createLogicalDevice()
{
    float queuePriority = 1.0f;

    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = m_physicalDevices[0].graphicsFamilyIndex();
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceVulkan14Features vulkan14Features = {};
    vulkan14Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES;
    vulkan14Features.pushDescriptor = VK_TRUE;

    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT dynamicStateFeatures = {};
    dynamicStateFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT;
    dynamicStateFeatures.extendedDynamicState3PolygonMode = VK_TRUE;
    dynamicStateFeatures.extendedDynamicState3DepthClipEnable = VK_TRUE;

    VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures = {};
    dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
    dynamicRenderingFeatures.dynamicRendering = VK_TRUE;
    dynamicRenderingFeatures.pNext = &dynamicStateFeatures;
    //dynamicRenderingFeatures.pNext = &vulkan14Features;

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.fillModeNonSolid = VK_TRUE;
    deviceFeatures.shaderClipDistance = VK_TRUE;

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = &dynamicRenderingFeatures;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(PhysicalDevice::DeviceExtensions.size());
    createInfo.ppEnabledExtensionNames = PhysicalDevice::DeviceExtensions.data();

    if(EnableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
        createInfo.ppEnabledLayerNames = m_validationLayers.data();

    } else createInfo.enabledLayerCount = 0;

    if(vkCreateDevice(m_physicalDevices[0], &createInfo, nullptr, &m_device) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device!");
    }

    std::cout << "Logical device created" << std::endl;

    vkGetDeviceQueue(m_device, m_physicalDevices[0].graphicsFamilyIndex(), 0, &m_graphicsQueue);
}

void VulkanInstance::swapChainSupportInfo(VkSurfaceKHR surface)
{
    m_physicalDevices[0].getSwapChainSupportInfo(surface);
}

VkSwapchainKHR VulkanInstance::createSwapChain(VkSurfaceKHR surface, VkExtent2D& imageExtent)
{
    VkSwapchainKHR swapchain;

    VkPresentModeKHR presentMode = m_physicalDevices[0].getSurfacePresentMode(surface);

    m_physicalDevices[0].findPresentationFamily(surface);

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevices[0], surface, &surfaceCapabilities);

    imageExtent = surfaceCapabilities.currentExtent;

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    
    createInfo.minImageCount = surfaceCapabilities.minImageCount + 1;

    if (surfaceCapabilities.maxImageCount > 0 && createInfo.minImageCount > surfaceCapabilities.maxImageCount)
        createInfo.minImageCount = surfaceCapabilities.maxImageCount;

    createInfo.imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
    createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageExtent = surfaceCapabilities.currentExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    if(m_physicalDevices[0].graphicsFamilyIndex() != m_physicalDevices[0].presentationFamilyIndex())
    {
        uint32_t queueFamilyIndices[] = {m_physicalDevices[0].graphicsFamilyIndex(),
                                        m_physicalDevices[0].presentationFamilyIndex()};

        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = surfaceCapabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if(vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &swapchain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain!");
    }

    if (m_presentQueue == VK_NULL_HANDLE)
    {
        vkGetDeviceQueue(m_device, m_physicalDevices[0].presentationFamilyIndex(), 0, &m_presentQueue);
    }

    /*waitIdle();

    m_commandList.begin();
    m_commandList.barrier(image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    m_commandList.copyBufferToImage(buffer, image.image, image.width, image.height);
    m_commandList.barrier(image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    m_commandList.finish();

    submit(m_commandList);
    waitIdle();

    m_commandList.reset();*/

    return swapchain;
}

void VulkanInstance::createTexture(Buffer& buffer, Image& image)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(image.width);
    imageInfo.extent.height = static_cast<uint32_t>(image.height);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = image.mipmaps;
    imageInfo.arrayLayers = 1;
    imageInfo.format = image.format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    if (vkCreateImage(m_device, &imageInfo, nullptr, &image.image) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_device, image.image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = detectMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(m_device, &allocInfo, nullptr, &image.imageMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(m_device, image.image, image.imageMemory, 0);

    waitIdle();

    m_commandList.begin();
    m_commandList.barrier(image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    m_commandList.copyBufferToImage(buffer, image.image, image.width, image.height, image.mipmaps);
    m_commandList.barrier(image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    m_commandList.finish();

    submit(m_commandList);
    waitIdle();

    // ImageView
    VkImageViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = image.image;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = image.format;

    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = image.mipmaps;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(m_device, &createInfo, nullptr, &image.imageView) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image views!");
    }
}

void VulkanInstance::createBuffer(Buffer& buffer, const void* data, size_t size)
{
    Buffer staging(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, size);

    void* mem = staging.map(size);
    memcpy(mem, data, size);
    staging.unmap();

    waitIdle();

    m_commandList.begin();
    m_commandList.copyBuffer(staging, buffer, size);
    m_commandList.finish();

    submit(m_commandList);
    waitIdle();
}

void VulkanInstance::transitImageState(std::vector<VkImage>& images, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    waitIdle();

    m_commandList.begin();

    for (VkImage image : images)
    {
        m_commandList.barrier(image, oldLayout, newLayout);
    }

    m_commandList.finish();

    submit(m_commandList);
    waitIdle();
}

void VulkanInstance::transitImageState(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    waitIdle();

    m_commandList.begin();
    m_commandList.barrier(image, oldLayout, newLayout);
    m_commandList.finish();

    submit(m_commandList);
    waitIdle();
}

void VulkanInstance::createDescriptorPool()
{
    VkDescriptorPoolSize poolSize[2];
    poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize[0].descriptorCount = 16;
    poolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize[1].descriptorCount = 16;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSize;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 16;

    if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void VulkanInstance::createCommandPool(VkCommandPool* commandPool)
{
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = m_physicalDevices[0].graphicsFamilyIndex();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if(vkCreateCommandPool(m_device, &poolInfo, nullptr, commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }

    std::cout << "Command Pool Created" << std::endl;
}

void VulkanInstance::createCommandList()
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(m_device, &allocInfo, &m_commandList) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void VulkanInstance::submit(VkCommandBuffer commandBuffer)
{
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    if(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }
}

uint32_t VulkanInstance::detectMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevices[0], &memProperties);

    for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }
}

void VulkanInstance::waitIdle()
{
    vkQueueWaitIdle(m_graphicsQueue);
}

} // namespace Render