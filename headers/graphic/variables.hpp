const ui32 SEQUENTIAL_FRAMES = 3;

ui32 CURRENT_FRAME = 0;
f32 dt;

Window window;
Swapchain* _used_swapchain;

std::array<ui32, 2> WINDOW_SIZE {2200, 1200}, WINDOW_MAX_SIZE;
glm::vec2 WINDOW_CENTER_POSITION { WINDOW_SIZE[0] / 2, WINDOW_SIZE[1] / 2 };