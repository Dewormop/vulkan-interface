void Descriptor::create_pool() {
    std::vector<VkDescriptorPoolSize> sizes;
    for (auto& descriptor_resource : resources_info) {
        VkDescriptorPoolSize size {descriptor_resource->resource_type, (ui32) sets.size()};
        sizes.push_back(size);
    }

    VkDescriptorPoolCreateInfo info {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    
    info.maxSets       = sets.size();
    info.poolSizeCount = (ui32) sizes.size();
    info.pPoolSizes    = sizes.data();
    info.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    _check(vkCreateDescriptorPool, device, &info, nullptr, &pool);
}

void Descriptor::create_layout() {
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    for (i32 i = 0; i < resources_info.size(); ++i) {
        VkDescriptorSetLayoutBinding binding {};
        binding.binding         = i;
        binding.descriptorCount = 1;
        binding.descriptorType  = resources_info[i]->resource_type;
        binding.stageFlags      = resources_info[i]->resource_shader_stage;
        bindings.push_back(binding);
    }

    VkDescriptorSetLayoutCreateInfo info {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    info.bindingCount = (ui32) bindings.size();
    info.pBindings    = bindings.data();

    _check(vkCreateDescriptorSetLayout, device, &info, nullptr, &layout)
}

void Descriptor::allocate_sets() {
    create_pool();
    create_layout();

    pPipeline->descriptor_layouts.push_back(layout);

    resource_collector.append(this);

    std::vector<VkDescriptorSetLayout> layouts(sets.size(), layout);
    VkDescriptorSetAllocateInfo info {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    info.descriptorPool     = pool;
    info.descriptorSetCount = (ui32) sets.size();
    info.pSetLayouts        = layouts.data();
    
    _check(vkAllocateDescriptorSets, device, &info, sets.data());

    i32 sets_size = sets.size(), current_write = 0;
    std::vector<VkWriteDescriptorSet> writes(resources_info.size() * sets_size);

    for (auto& set : sets) {
        for (i32 i = 0; i < resources_info.size(); ++i) {
            writes[current_write].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writes[current_write].descriptorCount = 1;
            writes[current_write].descriptorType  = resources_info[i]->resource_type;
            writes[current_write].dstBinding      = i;
            writes[current_write].dstSet          = set;
            
            switch (resources_info[i]->type) {
                case ResourceInfo::Type::BUFFER: writes[current_write].pBufferInfo = &resources_info[i]->info.buffer_info; break;
                case ResourceInfo::Type::IMAGE:  writes[current_write].pImageInfo  = &resources_info[i]->info.image_info;  break;
                case ResourceInfo::Type::BUFFER_VIEW: writes[current_write].pTexelBufferView = &resources_info[i]->info.buffer_view_info;
            }

            current_write += 1;
        }
    }

    vkUpdateDescriptorSets(device, (ui32) writes.size(), writes.data(), 0, nullptr);
}

void Descriptor::destroy() {
    vkFreeDescriptorSets(device, pool, (ui32) sets.size(), sets.data());
    vkDestroyDescriptorPool(device, pool, nullptr);
    vkDestroyDescriptorSetLayout(device, layout, nullptr);
}

constexpr void Descriptor::add_resource(ResourceInfo* resource_info) {
    resource_info->resource_id = resources_info.size();
    resources_info.push_back(resource_info);
}

void Descriptor::update_resource(const ui64& resource_id) {
    ResourceInfo& resource_info = *resources_info[resource_id];

    std::vector<VkWriteDescriptorSet> writes(sets.size());

    for (i32 i = 0; i < sets.size(); ++i) {
        VkWriteDescriptorSet& write = writes[i];
        write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorCount = 1;
        write.descriptorType  = resource_info.resource_type;
        write.dstBinding      = resource_id;
        write.dstSet          = sets[i];
        
        switch (resource_info.type) {
            case ResourceInfo::Type::BUFFER: write.pBufferInfo = &resource_info.info.buffer_info; break;
            case ResourceInfo::Type::IMAGE:  write.pImageInfo  = &resource_info.info.image_info;  break;
            case ResourceInfo::Type::BUFFER_VIEW: write.pTexelBufferView = &resource_info.info.buffer_view_info;
        }
    }

    vkUpdateDescriptorSets(device, (ui32) writes.size(), writes.data(), 0, nullptr);
}

void Descriptor::bind(const VkCommandBuffer& command_buffer, ui32 i) {
    vkCmdBindDescriptorSets(command_buffer, pPipeline->bind_point, pPipeline->layout, 0, 1, &sets[i], 0, nullptr);
}