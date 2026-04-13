Require: vulkan, glfw3, glm.
Something like:\
  \
  #include <vulkan/vulkan.h>\
  #include <glfw3.h>\
  #define GLM_ENABLE_EXPERIMENTAL\
  #define GLM_FORCE_DEPTH_ZERO_TO_ONE\
  #include <glm/glm.hpp>\
  #include <glm/gtx/transform.hpp>\
  #include <glm/gtc/matrix_transform.hpp>
