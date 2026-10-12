#pragma once
#include <cstdint>
#include <vulkan/vulkan.h>

namespace sim {
    struct PushConstants {
        float dt;
        uint32_t particleCount;
    };

    class SPHSolver {
        public:
            void init(VkDevice device);
            void cleanup(VkDevice device);

        private:
            VkDescriptorSetLayout m_descriptorSetLayout{VK_NULL_HANDLE};
            VkPipelineLayout m_pipelineLayout{VK_NULL_HANDLE};
    };
}