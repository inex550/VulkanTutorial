cmake_minimum_required(VERSION 3.12)

project(VulkanApplication)

set(TARGET_NAME VulkanApp)

find_package(Vulkan REQUIRED)

set(TARGET_SRC)
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

function (add_compileShaders_target TARGET_NAME)
    set(optionArgs)
    set(oneValueArgs)
    set(multiValueArgs FILES)

    cmake_parse_arguments(arg "${flagArgs}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(shaderSpvFiles)

    foreach(shaderFile ${arg_FILES})
        if (NOT EXISTS "${shaderFile}")
            message(FATAL_ERROR "Shader file \"${shaderFile}\" not exists")
        endif()

        get_filename_component(shaderFilename "${shaderFile}" NAME)

        set(shaderSpvFile "${CMAKE_CURRENT_BINARY_DIR}/assets/${shaderFilename}.spv")

        add_custom_command(OUTPUT ${shaderSpvFile}
            COMMAND Vulkan::glslc "${shaderFile}" -o "${shaderSpvFile}"
            DEPENDS "${shaderFile}"
        )
        list(APPEND shaderSpvFiles "${shaderSpvFile}")
    endforeach()

    add_custom_target(${TARGET_NAME} ALL DEPENDS ${shaderSpvFiles})
    message(${TARGET_NAME} ${shaderSpvFiles})
endfunction()

add_compileShaders_target(LearnVulkanShaders FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/triangle.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/triangle.frag
)
