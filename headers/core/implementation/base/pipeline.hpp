constexpr std::vector<char> read_file(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) throw std::runtime_error(collect("failed to open:", filename));
    
    std::vector<char> buffer(file.tellg());
    
    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    file.close();
    
    return buffer;
}

[[nodiscard]] VkShaderModule create_shader_module (const std::string& shader_path) {
    VkShaderModule mdl;

    auto code = read_file(shader_path);

    VkShaderModuleCreateInfo moduleCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = code.size() * sizeof(char),
        .pCode = reinterpret_cast<const ui32*>(code.data()) 
    };
    _check(vkCreateShaderModule, device, &moduleCreateInfo, nullptr, &mdl);

    return mdl;
}

void Pipeline::destroy() {
    vkDestroyPipeline(device, pipeline, nullptr);
    vkDestroyPipelineLayout(device, layout, nullptr);
    vkDestroyShaderModule(device, shader_module, nullptr);
}

void Pipeline::create_layout() {
    resource_collector.append(this);

    VkPipelineLayoutCreateInfo info {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    info.setLayoutCount         = (ui32) descriptor_layouts.size();
    info.pSetLayouts            = descriptor_layouts.data();
    info.pushConstantRangeCount = (ui32) push_constants.size();
    info.pPushConstantRanges    = push_constants.data();
    _check(vkCreatePipelineLayout, device, &info, nullptr, &layout);
}

void Pipeline::bind(const VkCommandBuffer& command_buffer) {
    vkCmdBindPipeline(command_buffer, bind_point, pipeline);
}