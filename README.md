Require: vulkan, glfw3, glm.
Something like:\n
  #include <vulkan/vulkan.h>\n
  #include <glfw3.h>\n
  #define GLM_ENABLE_EXPERIMENTAL\n
  #define GLM_FORCE_DEPTH_ZERO_TO_ONE\n
  #include <glm/glm.hpp>\n
  #include <glm/gtx/transform.hpp>\n
  #include <glm/gtc/matrix_transform.hpp>\n
