#pragma once
#include <cstdint>
#include <vulkan/vulkan.h>
#include <vector>
#include "Particle.hpp"
#include <vk_mem_alloc.h>

namespace sim {
    struct PushConstants {
        float dt;
        uint32_t particleCount;
    };

    class SPHSolver {
        public:
            void init(VkDevice device, VmaAllocator allocator, VkQueue transferQueue, VkCommadPool transferPool, uint32_t particleCount);
            void cleanup(VkDevice device, VmaAllocator allocator);

            VkBuffer getParticleBuffer() const { return m_particleBuffer };
            uint32_t getParticleCount() const { return m_particleCount };
        private:
            void createParticleBuffer(VmaAllocator allocator, VkDevice device, VkQueue queue, VkCommandPool pool, const std::vector<Particle>& initialData);

            uint32_t particleCount{0};

            // Buffer handles
            VkDescriptorSetLayout m_descriptorSetLayout{VK_NULL_HANDLE};
            VkPipelineLayout m_pipelineLayout{VK_NULL_HANDLE};

            // Pipeline and Descriptors
            VkDescriptorPool m_descriptorPool{VK_NULL_HANDLE};
            VkDescriptorSetLayout m_descriptorSetLayout{VK_NULL_HANDLE};
            VkDescriptorSet m_descriptorSet{VK_NULL_HANDLE};
            VkPipelineLayout m_pipelienLayout{VK_NULL_HANDLE};
    };
}