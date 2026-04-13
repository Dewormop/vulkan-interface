template<typename Type>
void Buffer<Type>::create(const ui64& count, const VkBufferUsageFlags& usage, const VkMemoryPropertyFlags& properties) {
    resource_collector.append(this);
    
    VkBufferCreateInfo info {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.size        = sizeof(Type) * count;
    info.usage       = usage;

    descriptor_resource.info.buffer_info.range = info.size;
    
    _check(vkCreateBuffer, device, &info, nullptr, &descriptor_resource.info.buffer_info.buffer);

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(device, *this, &requirements);
    
    VkMemoryAllocateInfo allocate_info {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocate_info.allocationSize  = requirements.size;
    allocate_info.memoryTypeIndex = find_memory_type(requirements.memoryTypeBits, properties);

    _check(vkAllocateMemory, device, &allocate_info, nullptr, &memory);
    _check(vkBindBufferMemory, device, *this, memory, 0);

    if (properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT || properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) map_memory();
}

void BufferBase::destroy() {
    vkFreeMemory(device, memory, nullptr);
    vkDestroyBuffer(device, descriptor_resource.info.buffer_info.buffer, nullptr);
}
void BufferBase::early_destroy() {
    resource_collector.remove(resource_collector.find_resource(this));
}

template<typename Type>
void Buffer<Type>::create_stage_buffer(const ui64& count) {
    create(count, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, HOST_PROPERTY);
}

template<typename Type>
void Buffer<Type>::create_stage_buffer(const ui64& count, const Type* data) {
    create(count, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, HOST_PROPERTY);
    memory_copy(get_size(), data);
}

template<typename Type>
void Buffer<Type>::create_stage_buffer(Buffer& buffer) {
    create_stage_buffer(buffer.get_count());
}

template<typename Type>
void Buffer<Type>::map_memory() {
    is_mapped = true;
    _check(vkMapMemory, device, memory, 0, get_size(), 0, (void**) &memory_data);
}

template<typename Type>
void Buffer<Type>::unmap_memory() {
    is_mapped = false;
    vkUnmapMemory(device, memory);
}

template<typename Type>
constexpr void Buffer<Type>::memory_copy(const ui64& count, const Type* data) {
    if (!is_mapped) log_error("buffer's memory doesn't mapped");
    size_t src_size = sizeof(Type) * count;
    memcpy(memory_data, data, ( this->get_size() > src_size  ? src_size : this->get_size() ));
}

template<typename Type>
void Buffer<Type>::gpu_copy(const ui64& src_size, const Type* src_data) {
    Buffer stage_buffer;
    stage_buffer.create_stage_buffer(src_size, src_data);

    gpu_copy(stage_buffer);

    stage_buffer.resource_collector.remove_back();
}

template<typename Type>
constexpr void Buffer<Type>::set_descriptor(Descriptor* pDescriptor, const VkDescriptorType& type, const VkShaderStageFlags& shader_stage) {
    descriptor_resource.resource_shader_stage = shader_stage;
    descriptor_resource.resource_type         = type;

    pDescriptor->add_resource(&descriptor_resource);
    this->pDescriptor = pDescriptor;
}

template<typename Type>
void Buffer<Type>::gpu_copy(Buffer& src_buffer) {
    CommandBuffers command_buffer(&transfer_queue);
    command_buffer.allocate();
    command_buffer.set_buffer(0);
    command_buffer.begin();

    VkBufferCopy region {.size = ( src_buffer.get_size() > this->get_size() ? this->get_size() : src_buffer.get_size() )};
    vkCmdCopyBuffer(command_buffer, src_buffer, *this, 1, &region);

    command_buffer.end();
    command_buffer.submit();
    command_buffer.early_destroy();
}

template<typename Type>
void Buffer<Type>::gpu_insert(Buffer& src_buffer, const std::vector<ui32>& dst_indices) {
    CommandBuffers command_buffers(&transfer_queue);
    command_buffers.allocate();
    command_buffers.set_buffer(0);
    command_buffers.begin();

    std::vector<VkBufferCopy> copies(src_buffer.get_count(), VkBufferCopy{.size = sizeof(Type)});

    for (ui32 i = 0; i < src_buffer.get_count(); ++i) {
        copies[i].srcOffset = copies[i].size * i;
        copies[i].dstOffset = copies[i].size * dst_indices[i];
    }

    vkCmdCopyBuffer(command_buffers, src_buffer, *this, copies.size(), copies.data());

    command_buffers.end();
    command_buffers.submit();
    command_buffers.early_destroy();
}
