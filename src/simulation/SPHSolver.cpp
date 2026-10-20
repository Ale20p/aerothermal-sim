#include "SPHSolver.hpp"

namespace sim {
    void SPHSolver::createParticleBuffer(VmaAllocator allocator, VkDevice device, VkQueue queue, VkCommandPool pool, const std::vector<Particle>& initialData) {
        VkDeviceSize bufferSize = sizeof(Particle) * initialData.size();

        // Create Host-Visible Staging Buffer
        VkBuffer stagingBuffer = VK_NULL_HANDLE;
        VmaAllocation stagingAlloc = VK_NULL_HANDLE;

        VkBufferCreateInfo stagingBufferInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = bufferSize,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };

        VmaAllocationCreateInfo stagingAllocInfo{
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO
        };

        VmaAllocationInfo stagingAllocResult{};
        vmaCreateBuffer(allocator, &stagingBufferInfo, &stagingAllocInfo, &stagingBuffer, &stagingAlloc, &stagingAllocResult);

        // Map and copy initial particle data to stating buffer
        std::memcpy(stagingAllocResult.pMappedData, initialData.data(), bufferSize);

        // Create Device-Local SSBO with Storage + Vertex Buffer Usage
        VkBufferCreateInfo deviceBufferInfo {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = bufferSize,
            .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | 
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };

        VmaAllocationCreateInfo deviceAllocInfo {
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
        };

        vmaCreateBuffer(allocator, &deviceBufferInfo, &deviceAllocInfo, &m_particleBuffer, &m_particleBufferAlloc, nullptr);


        // Immediate GPU Copy: Staging -> Device Local
        VkCommandBufferAllocateInfo cmdAllocInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = pool,
            .level = VK_COMMAND_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };

        VkCommandBuffer cmd = VK_NULL_HANDLE;
        vkAllocateCommandBuffers(device, &cmdAllocInfo, &cmd);

        VkCommandBufferBeginInfo beginInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };
        vkBeginCommandBuffer(cmd, &beginInfo);

        VkBufferCopy copyRegion {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = bufferSize
        };
        vkCmdCopyBuffer(cmd, stagingBuffer, m_particleBuffer, 1, &copyRegion);

        vkEndCommandBuffer(cmd);

        VkSubmitInfo submitInfo {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &cmd
        };
        vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue);

        // Cleanup stating resources
        vkFreeCommandBuffers(device, pool, 1, &cmd);
        vmaDestroyBuffer(allocator, stagingBuffer, stagingAlloc);
    }

    void SPHSolver::init(VkDevice device, VmaAllocator allocator, VkQueue transferPool, uint32_t particleCount) {  
        m_allocator = allocator;
        m_particleCount = particleCount;
        
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
        vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout);

        m_particleCount = particleCount;

        std::vector<Particle> initialParticles(particleCount);
        int side = static_cast<int>(std::cbrt(particleCount));
        float spacing = 0.05f;

        for (uint32_t i = 0; i < particleCount; ++i) {
            float x = (i % side) * spacing;
            float y = ((i / side) % side) * spacing;
            float z = (i / (side * side)) * spacing;

            initialParticles[i].position_density = glm::vec4(x, y, z, 1000.0f);     // rest density ~1000 kg/m3
            initialParticles[i].velocity_pressure = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);  // initial zero velocity and pressure
            initialParticles[i].thermal_properties = glm::vec4(293.15f, 4184.0f, 0.0f, 0.0f);   // 20 deg C
        }

        createParticleBuffer(allocator, device, transferQueue, transferPool, initialParticles);
    }

    void SPHSolver::cleanup(VkDevice device) {
        if (m_particleBuffer != VK_NULL_HANDLE) {
            vmaDestroyBuffer(m_allocator, m_particleBuffer, m_particleBufferAlloc);
            m_particleBuffer = VK_NULL_HANDLE;
            m_particleBufferAlloc = VK_NULL_HANDLE;
        }
        if (m_pipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
            m_pipelineLayout = VK_NULL_HANDLE;
        }
        if (m_descriptorSetLayout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(device, m_descriptorSetLayout, nullptr);
            m_descriptorSetLayout = VK_NULL_HANDLE;
        }
    }
}