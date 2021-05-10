#pragma once
#include "Device.h"
#include <vector>

namespace vke
{
    struct PipelineConfigInfo 
    {
        VkViewport viewport;
        VkRect2D scissor;
        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };

    class VkePipeline
    {
    public:
        VkePipeline(VkeDevice& device, const std::string& vertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo);
        ~VkePipeline();

        VkePipeline(const VkePipeline&) = delete;
        VkePipeline& operator=(const VkePipeline&) = delete;

        static PipelineConfigInfo DefaultPipelineConfigInfo(u32 width, u32 height);

    private:
        static std::vector<char> ReadFile(const std::string& filepath);

        void CreateGraphicsPipeline(const std::string& vertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo);
        void CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

        VkeDevice& m_Device;
        VkPipeline m_GraphicsPipeline;
        VkShaderModule m_VertShaderModule;
        VkShaderModule m_FragShaderModule;
    };
}