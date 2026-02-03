#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace Render
{

struct InputLayout
{
    const std::vector<VkVertexInputBindingDescription> bindings;
    const std::vector<VkVertexInputAttributeDescription> attributes;
};

struct BindingLayout
{
    const std::vector<VkDescriptorSetLayoutBinding> bindings;
    const std::vector<VkPushConstantRange> pushranges;
};

struct PipelineParameters
{
    VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
    VkFrontFace frontFace = VK_FRONT_FACE_CLOCKWISE;
    VkPrimitiveTopology primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkBool32 depthTest = VK_TRUE;
    VkBool32 depthWrite = VK_TRUE;
    VkBool32 blend = VK_FALSE;
    bool dynamicCullMode = false;
};

class Pipeline
{
private:
    VkDescriptorSetLayout m_descriptorSetLayout;
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_graphicsPipeline;

    VkShaderModule loadShader(const char * name);
    VkShaderModule buildShader(const uint8_t* buffer, size_t size);

    void createDescriptorSetLayout(const BindingLayout& bindingLayout);

    void init(VkShaderModule vertShaderModule, 
              VkShaderModule fragShaderModule,
              const InputLayout& inputLayout,
              const BindingLayout& bindingLayout,
              const PipelineParameters& params);

public:

    Pipeline(const char * shader, 
             const InputLayout& inputLayout,
             const BindingLayout& bindingLayout,
             const PipelineParameters& params);

    Pipeline(const uint8_t* vertexShader, size_t vertexShaderSize,
             const uint8_t* fragmentShader, size_t fragmentShaderSize,
             const InputLayout& inputLayout,
             const BindingLayout& bindingLayout,
             const PipelineParameters& params);
    ~Pipeline();

    operator VkPipeline() const { return m_graphicsPipeline; }

    VkPipelineLayout pipelineLayout() const { return m_pipelineLayout; }
    VkDescriptorSetLayout descriptorLayout() const { return m_descriptorSetLayout; }
};

} // namespace Render
