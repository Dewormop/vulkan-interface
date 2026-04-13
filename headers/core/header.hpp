#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <array>
#include <ranges>
#include <fstream>
#include <cinttypes>
#include <queue>

#include "functions.hpp"
#include "structures.hpp"
#include "variables.hpp"

void null_function() {}
void (*window_init)() = null_function;
void (*graphic_destroy)() = null_function;

void vulkan_init() {
    window_init();

    instance.create();
    device.create();

    Queue::resource_collector.start_action([] (Queue* queue) {
        queue->create_command_pool();
    });
}

struct VulkanDestructor {
    ~VulkanDestructor() {
        graphic_destroy();

        resource_collector.destroy();

        device.destroy();
        instance.destroy();
    }
} _vulkan_destructor;

#include "implementation/base/header.hpp"
#include "implementation/resources/header.hpp"
