void ComputePipeline::create() {
    create_layout();

    shader_module = create_shader_module("src/shaders/compute_shader.spv");

    VkPipelineShaderStageCreateInfo compute_stage {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    compute_stage.module = shader_module;
    compute_stage.pName  = "compute_main";
    compute_stage.stage  = VK_SHADER_STAGE_COMPUTE_BIT;

    VkComputePipelineCreateInfo info {VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
    info.layout = layout;
    info.stage  = compute_stage;
    
    _check(vkCreateComputePipelines, device, NULL, 1, &info, nullptr, &pipeline);
}