const VkMemoryPropertyFlags HOST_PROPERTY   = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
const VkMemoryPropertyFlags DEVICE_PROPERTY = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
const VkSampleCountFlagBits SAMPLE_COUNT    = VK_SAMPLE_COUNT_8_BIT;
const VkFormat DEPTH_FORMAT                 = VK_FORMAT_D24_UNORM_S8_UINT;

Device    device;
Instance  instance;
Queue     transfer_queue(VK_QUEUE_TRANSFER_BIT);

MainResourceCollector<
    Pipeline, Descriptor, BufferBase, Image, Fences, Semaphores, CommandBuffers, Queue
> resource_collector;