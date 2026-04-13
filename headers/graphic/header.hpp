#include "structures.hpp"
#include "variables.hpp"

struct GraphicInititate {
    GraphicInititate() {
        window_init = Window::_init;
        graphic_destroy = Swapchain::_destroy;
    }
} _graphic_init;

#include "implementation/window.hpp"
#include "implementation/swapchain.hpp"
#include "implementation/graphic_pipeline.hpp"
#include "implementation/render_pass.hpp"
