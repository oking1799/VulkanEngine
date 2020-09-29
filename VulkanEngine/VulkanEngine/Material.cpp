#include "pch.h"
#include "Material.h"

#include "VulkanManager.h"
#include "FileManager.h"
#include "SwapChain.h"

#define logicalDevice VulkanManager::GetInstance()->GetLogicalDevice()
#define swapChainImages SwapChain::GetInstance()->GetImages()

#pragma region Memory Management

Material::Material(std::string vertexShaderPath, std::string fragmentShaderPath, bool wireframe)
{
	this->vertexShaderPath = vertexShaderPath;
	this->fragmentShaderPath = fragmentShaderPath;
	this->wireframe = wireframe;

	pipelineLayout = VkPipelineLayout();
	pipeline = VkPipeline();

	descriptorPool = VkDescriptorPool();
	descriptorSetLayout = VkDescriptorSetLayout();
	descriptorSets = std::vector<VkDescriptorSet>();
}

void Material::Init()
{
	CreateDescriptorSetLayout();

	CreateGraphicsPipeline();

	CreateDescriptorPool();

	CreateDescriptorSets();
}

void Material::CreateGraphicsPipeline()
{
	//Read in shader code
	auto vertexShaderCode = FileManager::ReadFile(vertexShaderPath);
	auto fragmentShaderCode = FileManager::ReadFile(fragmentShaderPath);

	//Create shader module
	VkShaderModule vertexShaderModule = CreateShaderModule(vertexShaderCode);
	VkShaderModule fragmentShaderModule = CreateShaderModule(fragmentShaderCode);

	//Setup shader stages
	VkPipelineShaderStageCreateInfo vertexStageCreateInfo = {};
	vertexStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexStageCreateInfo.module = vertexShaderModule;
	vertexStageCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragmentStageCreateInfo = {};
	fragmentStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentStageCreateInfo.module = fragmentShaderModule;
	fragmentStageCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = {
		vertexStageCreateInfo,
		fragmentStageCreateInfo
	};

	//Setup the Vertex input
	std::array<VkVertexInputAttributeDescription, 4> vertexDescriptions = Vertex::GetAttributeDescriptions();
	std::array<VkVertexInputAttributeDescription, 4> transformDescriptions = TransformData::GetAttributeDescriptions();
	std::array<VkVertexInputAttributeDescription, 8> attributeDescriptions = {
		vertexDescriptions[0],
		vertexDescriptions[1],
		vertexDescriptions[2],
		vertexDescriptions[3],
		transformDescriptions[0],
		transformDescriptions[1],
		transformDescriptions[2],
		transformDescriptions[3]
	};

	std::array<VkVertexInputBindingDescription, 2> bindingDescriptions = {
		Vertex::GetBindingDescription(),
		TransformData::GetBindingDescription()
	};

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
	vertexInputCreateInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
	vertexInputCreateInfo.pVertexBindingDescriptions = bindingDescriptions.data();

	//Setup the Input Assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
	inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

	//Setup the Viewport
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)SwapChain::GetInstance()->GetExtents().width;
	viewport.height = (float)SwapChain::GetInstance()->GetExtents().height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	//Setup the Scissor Rectangle
	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = SwapChain::GetInstance()->GetExtents();

	//Setup Viewport State
	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
	viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.pViewports = &viewport;
	viewportStateCreateInfo.scissorCount = 1;
	viewportStateCreateInfo.pScissors = &scissor;

	//Setup Rasterizer
	VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {};
	rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerCreateInfo.depthClampEnable = VK_FALSE;
	rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	
	if (wireframe) {
		rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_LINE;
	}
	else {
		rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	}

	rasterizerCreateInfo.lineWidth = 1.0f;
	rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizerCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizerCreateInfo.depthBiasEnable = VK_FALSE;
	rasterizerCreateInfo.depthBiasConstantFactor = 0.0f;
	rasterizerCreateInfo.depthBiasClamp = 0.0f;
	rasterizerCreateInfo.depthBiasSlopeFactor = 0.0f;

	//Setup Multisampling
	VkPipelineMultisampleStateCreateInfo multisampleCreateInfo = {};
	multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
	multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleCreateInfo.minSampleShading = 0.0f;
	multisampleCreateInfo.pSampleMask = nullptr;
	multisampleCreateInfo.alphaToCoverageEnable = VK_FALSE;
	multisampleCreateInfo.alphaToOneEnable = VK_FALSE;

	//Setup Color Blending
	VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
	colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachmentState.blendEnable = VK_FALSE;
	colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo = {};
	colorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendCreateInfo.logicOpEnable = VK_FALSE;
	colorBlendCreateInfo.logicOp = VK_LOGIC_OP_COPY;
	colorBlendCreateInfo.attachmentCount = 1;
	colorBlendCreateInfo.pAttachments = &colorBlendAttachmentState;
	colorBlendCreateInfo.blendConstants[0] = 0.0f;
	colorBlendCreateInfo.blendConstants[1] = 0.0f;
	colorBlendCreateInfo.blendConstants[2] = 0.0f;
	colorBlendCreateInfo.blendConstants[3] = 0.0f;

	//Setup depth stencil
	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
	depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilCreateInfo.depthTestEnable = VK_TRUE;
	depthStencilCreateInfo.depthWriteEnable = VK_TRUE;
	depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilCreateInfo.minDepthBounds = 0.0f;
	depthStencilCreateInfo.maxDepthBounds = 1.0f;
	depthStencilCreateInfo.stencilTestEnable = VK_FALSE;
	depthStencilCreateInfo.front = {};
	depthStencilCreateInfo.back = {};

	/*
	//Setup Dynamic states
	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
	dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.dynamicStateCount = 2;
	dynamicStateCreateInfo.pDynamicStates = dynamicStates;
	*/

	//Setup Pipeline Layout
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

	//Create the pipeline layout
	if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Graphics Pipeline Layout!");
	}

	//Setup graphics pipeline create info
	VkGraphicsPipelineCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	createInfo.stageCount = 2;
	createInfo.pStages = shaderStages;
	createInfo.pVertexInputState = &vertexInputCreateInfo;
	createInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
	createInfo.pViewportState = &viewportStateCreateInfo;
	createInfo.pRasterizationState = &rasterizerCreateInfo;
	createInfo.pMultisampleState = &multisampleCreateInfo;
	createInfo.pDepthStencilState = &depthStencilCreateInfo;
	createInfo.pColorBlendState = &colorBlendCreateInfo;
	createInfo.pDynamicState = nullptr;
	createInfo.layout = pipelineLayout;
	createInfo.renderPass = SwapChain::GetInstance()->GetRenderPass();
	createInfo.subpass = 0;
	createInfo.basePipelineHandle = VK_NULL_HANDLE;
	createInfo.basePipelineIndex = -1;

	//Create graphics pipeline
	if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Graphics Pipeline!");
	}

	//Cleanup shader modules
	vkDestroyShaderModule(logicalDevice, vertexShaderModule, nullptr);
	vkDestroyShaderModule(logicalDevice, fragmentShaderModule, nullptr);
}

void Material::CreateDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> bindings(1);

	bindings[0] = {};
	bindings[0].binding = 0;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[0].descriptorCount = 1;
	bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	bindings[0].pImmutableSamplers = nullptr;

	/// <summary>
	/// newwwww
	/// </summary>
	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	std::array<VkDescriptorSetLayoutBinding, 2> bdgs = { bindings[0], samplerLayoutBinding };
	/// <summary>
	/// /new end
	/// </summary>

	VkDescriptorSetLayoutCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	createInfo.bindingCount = 1;
	createInfo.pBindings = bdgs.data();


	/// <summary>
	/// new
	/// </summary>
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

	layoutInfo.pBindings = &bindings[0];
	layoutInfo.bindingCount = static_cast<uint32_t>(bdgs.size());
	layoutInfo.pBindings = bdgs.data();
	/// <summary>
	/// end
	/// </summary>
	if (vkCreateDescriptorSetLayout(logicalDevice, &createInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor set layout");
	}
}

void Material::CreateDescriptorPool()
{
	VkDescriptorPoolSize poolSize = {};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = static_cast<uint32_t>(swapChainImages.size());

	VkDescriptorPoolCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.poolSizeCount = 1;
	createInfo.pPoolSizes = &poolSize;
	createInfo.maxSets = static_cast<uint32_t>(swapChainImages.size());

	if (vkCreateDescriptorPool(logicalDevice, &createInfo, nullptr, &descriptorPool)) {
		throw std::runtime_error("Failed to create Descriptor Pool!");
	}
}

void Material::CreateDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size(), descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocateInfo.descriptorPool = descriptorPool;
	allocateInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
	allocateInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(swapChainImages.size());

	if (vkAllocateDescriptorSets(logicalDevice, &allocateInfo, descriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate Descriptor Sets!");
	}

	for (size_t i = 0; i < swapChainImages.size(); i++) {
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = SwapChain::GetInstance()->GetUniformBuffers()[i].GetBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSets[i];
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;
		descriptorWrite.pImageInfo = nullptr;
		descriptorWrite.pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(logicalDevice, 1, &descriptorWrite, 0, nullptr);
	}
}

void Material::Cleanup()
{
	vkDestroyPipeline(logicalDevice, pipeline, nullptr);
	vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
	// VkResult t = 
	// vkFreeDescriptorSets(logicalDevice, descriptorPool, descriptorSets.size(), &(descriptorSets[0]));
	vkDestroyDescriptorPool(logicalDevice, descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetLayout, nullptr);
	printf("We at descriptor sets");
}

#pragma endregion

#pragma region Accessors

VkPipelineLayout Material::GetPipelineLayout()
{
	return pipelineLayout;
}

VkPipeline Material::GetPipeline()
{
	return pipeline;
}

std::vector<VkDescriptorSet> Material::GetDescriptorSets()
{
	return descriptorSets;
}

#pragma endregion

#pragma region Helper Methods

VkShaderModule Material::CreateShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Shader Module!");
	}

	return shaderModule;
}

#pragma endregion