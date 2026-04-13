void Fences::create(const ui64& size) {
    fences.resize(size);
    resource_collector.append(this);

    VkFenceCreateInfo info {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for (auto& fence : fences) vkCreateFence(device, &info, nullptr, &fence);
}

bool Fences::wait(const ui64& i) {
    if (vkWaitForFences(device, 1, &fences[i], true, 0) == VK_SUCCESS) {
        vkResetFences(device, 1, &fences[i]);
        return true;
    } else return false;
}

void Fences::destroy() {
    for (auto& fence : fences) vkDestroyFence(device, fence, nullptr);
}

void Semaphores::create(const ui64& size) {
    semaphores.resize(size);
    resource_collector.append(this);
    
    VkSemaphoreCreateInfo info {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    for (auto& semaphore : semaphores) vkCreateSemaphore(device, &info, nullptr, &semaphore);
}

void Semaphores::destroy() {
    for (auto& semaphore : semaphores) vkDestroySemaphore(device, semaphore, nullptr);
}