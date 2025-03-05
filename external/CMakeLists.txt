# Configuration macro
function (configure_external_library LIB_DIRNAME)
    if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${LIB_DIRNAME})
        add_subdirectory(${LIB_DIRNAME})
        include(${LIB_DIRNAME}Conf.cmake)
    else()
        message(FATAL_ERROR "External lib \"${LIB_DIRNAME}\" was not found. Try to \"git submodule update\"")
    endif()
endfunction()

# External libraries configuration
configure_external_library(glfw)

configure_external_library(glm)
