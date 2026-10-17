#include "SPHSolver.hpp"

namespace sim {
    void SPHSolver::createParticleBuffer(VmaAllocator allocator, VkDevice device, VkQueue queue, VkCommandPool pool, const std::vector<Particle>& initialData) {
        VkDeviceSize bufferSize = sizeof(Particle) * initialData.size();

        // Create Host-Visible Staging Buffer
        VkBuffer stagingBuffer = VK_NULL_HANDLE;
        VmaAllocator stagingAlloc = VK_NULL_HANDLE;

        VkBufferCreateInfo stagingBufferInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        }
    }

    void SPHSolver::init(VkDevice device) {
        // 1. Descriptor Set Layout for the Particle SSBO
        VkDescriptorSetLayoutBinding ssboBinding {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
            .pImmutableSamplers = nullptr
        };

        VkDescriptorSetLayoutCreateInfo layoutInfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = 1,
            .pBindings = &ssboBinding
        };
        vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_descriptorSetLayout);

        // 2. Push Constant Range
        VkPushConstantRange pushConstantRange {
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
            .offset = 0,
            .size = sizeof(PushConstants)
        };

        // 3. Compute Pipeline Layout
        VkPipelineLayoutCreateInfo pipelineLayoutInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 1,
            .pSetLayouts = &m_descriptorSetLayout,
            .pushConstantRangeCount = 1,
            .pPushConstantRanges = &pushConstantRange
        };
        vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_descriptorSetLayout);
    }

    void SPHSolver::cleanup(VkDevice device) {
        if (m_pipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
        }
        if (m_descriptorSetLayout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(device, m_descriptorSetLayout, nullptr);
        }
    }
}