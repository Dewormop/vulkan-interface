void CommandBuffers::allocate(const ui64& size) {
    buffers.resize(size);

    resource_collector.append(this);

    VkCommandBufferAllocateInfo info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    info.commandBufferCount = (ui32) size;
    info.commandPool        = this->pQueue->pool;
    info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    
    _check(vkAllocateCommandBuffers, device, &info, buffers.data());
}

void CommandBuffers::allocate() {
    buffers.resize(1);

    resource_collector.append(this);

    VkCommandBufferAllocateInfo info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    info.commandBufferCount = 1;
    info.commandPool        = this->pQueue->pool;
    info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    
    _check(vkAllocateCommandBuffers, device, &info, buffers.data());
}


void CommandBuffers::begin(const ui32& i) {
    VkCommandBufferBeginInfo begin_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    _check(vkBeginCommandBuffer, buffers[i], &begin_info);
}

void CommandBuffers::submit(const ui32& i) {
    VkSubmitInfo info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    info.commandBufferCount = 1;
    info.pCommandBuffers    = &buffers[i];
    
    check(vkQueueSubmit(*pQueue, 1, &info, VK_NULL_HANDLE), "submit command buffer to queue. Index: [", i, "]");
    check(vkQueueWaitIdle(*pQueue), "wait of submition of command buffer. Index: [", i, "]");
}

void Queue::create_command_pool() {
    VkCommandPoolCreateInfo info {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    
    info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    info.queueFamilyIndex = queue_indice;

    _check(vkCreateCommandPool, device, &info, nullptr, &pool);
}

void CommandBuffers::begin() {
    constexpr static VkCommandBufferBeginInfo begin_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    _check(vkBeginCommandBuffer, buffers[setted_command_buffer], &begin_info);
}

void CommandBuffers::submit() {
    static VkSubmitInfo info{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO, .commandBufferCount = 1};
    info.pCommandBuffers = &buffers[setted_command_buffer];
    
    check(vkQueueSubmit(*pQueue, 1, &info, VK_NULL_HANDLE), "submit command buffer to queue. Index: [", setted_command_buffer, "]");
    check(vkQueueWaitIdle(*pQueue), "wait of submition of command buffer. Index: [", setted_command_buffer, "]");
}

void CommandBuffers::submit(const VkSemaphore& wait_semaphore, const VkPipelineStageFlags& wait_mask, const VkSemaphore& signal_semaphore, const VkFence& external_fence) {
    static VkSubmitInfo info{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO, .waitSemaphoreCount = 1, .commandBufferCount = 1, .signalSemaphoreCount = 1};
    
    info.pSignalSemaphores    = &signal_semaphore;
    info.pWaitSemaphores      = &wait_semaphore;
    info.pWaitDstStageMask    = &wait_mask;
    info.pCommandBuffers      = &buffers[setted_command_buffer];
    
    check(vkQueueSubmit(*pQueue, 1, &info, external_fence), "submit command buffer to queue. Index: [", setted_command_buffer, "]");
}

ui32 find_memory_type(ui32 type, VkMemoryPropertyFlags memory_properties) {
    VkPhysicalDeviceMemoryProperties properties;
    vkGetPhysicalDeviceMemoryProperties(device, &properties);
    for (ui32 i = 0; i < properties.memoryTypeCount; ++i) if (type & (1 << i) && (properties.memoryTypes[i].propertyFlags & memory_properties) == memory_properties) return i;
    throw std::runtime_error("failed to find suitable memory type");
}

void CommandBuffers::destroy() {
    vkFreeCommandBuffers(device, pQueue->pool, (ui32) buffers.size(), buffers.data());
}
constexpr void CommandBuffers::early_destroy() {
    resource_collector.remove(resource_collector.find_resource(this));
}
