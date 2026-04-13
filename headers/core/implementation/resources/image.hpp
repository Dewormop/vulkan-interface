void Image::create(const ui32& width, const ui32& height, Queue* queue, const VkSampleCountFlagBits& sample_count, const VkFormat& format, const VkImageUsageFlags& usage, const VkMemoryPropertyFlags& properties, bool is_external = false) {
    this->format = format;
    this->pQueue = queue;
    this->extent = {width, height, 1}; 
    if (!is_external) resource_collector.append(this);

    VkImageCreateInfo info {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    info.arrayLayers           = 1;
    info.extent                = extent;
    info.format                = format;
    info.imageType             = VK_IMAGE_TYPE_2D;
    info.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;
    info.mipLevels             = 1;
    info.queueFamilyIndexCount = 1;
    info.pQueueFamilyIndices   = &queue->queue_indice;
    info.samples               = sample_count;
    info.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    info.tiling                = VK_IMAGE_TILING_OPTIMAL;
    info.usage                 = usage;

    _check(vkCreateImage, device, &info, nullptr, &image);

    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(device, image, &requirements);

    VkMemoryAllocateInfo allocate_info {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocate_info.allocationSize  = requirements.size;
    allocate_info.memoryTypeIndex = find_memory_type(requirements.memoryTypeBits, properties);

    _check(vkAllocateMemory, device, &allocate_info, nullptr, &memory);
    _check(vkBindImageMemory, device, image, memory, 0);
}

void Image::create_view(VkImageAspectFlags aspect) {
    this->aspect = aspect;

    VkImageViewCreateInfo info {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    info.format           = format;
    info.image            = image;
    info.subresourceRange = {aspect, 0, 1, 0, 1};
    info.viewType         = VK_IMAGE_VIEW_TYPE_2D;
    
    _check(vkCreateImageView, device, &info, nullptr, &descriptor_resource.info.image_info.imageView);
}

void Image::destroy() {
    vkFreeMemory(device, memory, nullptr);
    vkDestroyImageView(device, get_view(),  nullptr);
    vkDestroyImage(device, image, nullptr);

    get_layout()   = VK_IMAGE_LAYOUT_UNDEFINED;
    access         = 0;
    pipeline_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
}

void Image::translate_to_layout(const VkImageLayout& new_layout, const VkAccessFlags& new_access, const VkPipelineStageFlags& new_stage) {    
    CommandBuffers command_buffer(&transfer_queue);
    command_buffer.allocate();
    command_buffer.set_buffer(0);
    command_buffer.begin();

    VkImageMemoryBarrier barrier {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier.image            = image;
    barrier.oldLayout        = get_layout();
    barrier.srcAccessMask    = access;
    barrier.newLayout        = new_layout;
    barrier.dstAccessMask    = new_access;
    barrier.subresourceRange = {aspect, 0, 1, 0, 1};

    vkCmdPipelineBarrier(command_buffer, pipeline_stage, new_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    command_buffer.end();
    command_buffer.submit();
    command_buffer.early_destroy();

    this->get_layout() = new_layout;
    access             = new_access;
    pipeline_stage     = new_stage;
}

constexpr void Image::set_descriptor(Descriptor* pDescriptor, const VkDescriptorType& type, const VkShaderStageFlags& shader_stage) {
    descriptor_resource.resource_shader_stage       = shader_stage;
    descriptor_resource.resource_type               = type;

    pDescriptor->add_resource(&descriptor_resource);
    this->pDescriptor = pDescriptor;
}

void Image::transfer(Image& image) {
    CommandBuffers command_buffer(&transfer_queue);
    command_buffer.allocate();
    command_buffer.set_buffer(0);
    command_buffer.begin();

    VkImageCopy copy_region {};
    copy_region.extent         = extent;
    copy_region.srcSubresource = {aspect, 0, 0, 1};
    copy_region.dstSubresource = {image.aspect, 0, 0, 1};
    vkCmdCopyImage(command_buffer, *this, this->get_layout(), image, image.get_layout(), 1, &copy_region);

    command_buffer.end();
    command_buffer.submit();
    command_buffer.early_destroy();
}

void Image::copy(Image& image) {
    CommandBuffers command_buffer(&transfer_queue);
    command_buffer.allocate();
    command_buffer.set_buffer(0);
    command_buffer.begin();

    VkImageCopy copy_region {};
    copy_region.extent         = image.extent;
    copy_region.srcSubresource = {image.aspect, 0, 0, 1};
    copy_region.dstSubresource = {aspect, 0, 0, 1};
    vkCmdCopyImage(command_buffer, image, image.get_layout(), *this, this->get_layout(), 1, &copy_region);

    command_buffer.end();
    command_buffer.submit();
    command_buffer.early_destroy();
}