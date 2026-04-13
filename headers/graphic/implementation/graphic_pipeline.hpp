struct VertexInfo {
    std::vector<VkVertexInputAttributeDescription> attributes;
    VkVertexInputBindingDescription binding;
};

struct VertexData {
    std::vector<VkVertexInputAttributeDescription> attribures;
    std::vector<VkVertexInputBindingDescription>   bindings;
};

struct VertexBase {
    inline static ui32 location = 0, binding = 0;
};

template<typename Vertex, typename... Vertices>
constexpr void get_vertex_data(VertexData& vertex_data) {
    auto _vertex_data = Vertex::get_info();

    vertex_data.bindings.push_back(_vertex_data.binding);
    vertex_data.attribures.insert(vertex_data.attribures.end(), _vertex_data.attributes.begin(), _vertex_data.attributes.end());

    if constexpr (sizeof...(Vertices) > 0) get_vertex_data<Vertices...>(vertex_data);
}

template<typename... Vertices>
void GraphicPipeline<Vertices...>::create() {
    create_layout();

    shader_module = create_shader_module("src/shaders/graphic_shader.spv");

    VkPipelineShaderStageCreateInfo vertex_stage {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    vertex_stage.module = shader_module;
    vertex_stage.pName  = "vertex_main";
    vertex_stage.stage  = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineShaderStageCreateInfo fragment_stage {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    fragment_stage.module = shader_module;
    fragment_stage.pName  = "fragment_main";
    fragment_stage.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkPipelineShaderStageCreateInfo pipeline_stages[] = { vertex_stage, fragment_stage };

    VkPipelineColorBlendAttachmentState color_blend_attachment{};

    VkPipelineColorBlendStateCreateInfo color_blend_state {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    color_blend_state.attachmentCount = 1;
    color_blend_state.logicOpEnable   = false;
    color_blend_state.pAttachments    = &color_blend_attachment;

    VkPipelineDepthStencilStateCreateInfo depth_state {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    depth_state.depthCompareOp   = VK_COMPARE_OP_LESS;
    depth_state.depthTestEnable  = true;
    depth_state.depthWriteEnable = true;

    std::vector<VkDynamicState> dynamic_states {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY
    };
    VkPipelineDynamicStateCreateInfo dynamic_state {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dynamic_state.dynamicStateCount = (ui32) dynamic_states.size();
    dynamic_state.pDynamicStates    = dynamic_states.data();

    VkPipelineInputAssemblyStateCreateInfo input_assembly_state {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineMultisampleStateCreateInfo multisample_state {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    multisample_state.rasterizationSamples = SAMPLE_COUNT;

    VkPipelineRenderingCreateInfo rendering_info {VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
    rendering_info.depthAttachmentFormat = DEPTH_FORMAT;

    VkPipelineRasterizationStateCreateInfo rasterization_state {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rasterization_state.cullMode    = VK_CULL_MODE_NONE;
    rasterization_state.frontFace   = VK_FRONT_FACE_CLOCKWISE;
    rasterization_state.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization_state.lineWidth   = 1;

    VkPipelineViewportStateCreateInfo viewport_state {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    viewport_state.scissorCount  = 1;
    viewport_state.viewportCount = 1;
    viewport_state.pScissors     = &pRenderPass->pSwapchain->scissor;
    viewport_state.pViewports    = &pRenderPass->pSwapchain->viewport;

    VertexData vertex_data;
    get_vertex_data<Vertices...>(vertex_data);

    VkPipelineVertexInputStateCreateInfo vertex_input_state {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vertex_input_state.vertexAttributeDescriptionCount = (ui32) vertex_data.attribures.size();
    vertex_input_state.vertexBindingDescriptionCount   = (ui32) vertex_data.bindings.size();
    vertex_input_state.pVertexBindingDescriptions      = vertex_data.bindings.data();
    vertex_input_state.pVertexAttributeDescriptions    = vertex_data.attribures.data();

    VkGraphicsPipelineCreateInfo info {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, &rendering_info};
    info.layout              = layout;
    info.pColorBlendState    = &color_blend_state;
    info.pDepthStencilState  = &depth_state;
    info.pDynamicState       = &dynamic_state;
    info.pInputAssemblyState = &input_assembly_state;
    info.pMultisampleState   = &multisample_state;
    info.pRasterizationState = &rasterization_state;
    info.pViewportState      = &viewport_state;
    info.stageCount          = 2;
    info.pStages             = pipeline_stages;
    info.pVertexInputState   = &vertex_input_state;
    info.renderPass          = *pRenderPass;

    check(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &info, nullptr, &pipeline), "creation of graphic pipeline");
}