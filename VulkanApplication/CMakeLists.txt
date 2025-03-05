cmake_minimum_required(VERSION 3.12)

project(VulkanApplication)

set(TARGET_NAME VulkanApp)
set(TARGET_SRC)

find_package(Vulkan REQUIRED)

aux_source_directory(src TARGET_SRC)

add_executable(${TARGET_NAME}
    ${TARGET_SRC}
)

target_link_libraries(${TARGET_NAME}
PRIVATE
    glfw
    glm::glm
    Vulkan::Vulkan
)
