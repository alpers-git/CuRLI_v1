CMAKE_MINIMUM_REQUIRED(VERSION 3.0.0 FATAL_ERROR)
cmake_policy(VERSION 3.0.0)

project(CuRLI_v1)


#========== Global Configurations =============#
#----------------------------------------------#

# Set the C++ standard for all targets
# OPTIONAL:
#---------------------------------
set(CMAKE_CXX_STANDARD 17)     
set(CMAKE_VERBOSE_MAKEFILE ON)


#========= Dependency Configurations ==========#
find_package(OpenGL REQUIRED)

add_subdirectory(submodules/glm EXCLUDE_FROM_ALL)

#--------------------GLFW---------------------#
set(GLFW_BUILD_DOCS OFF CACHE STRING "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE STRING "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE STRING "" FORCE)
set(GLFW_INSTALL        OFF CACHE STRING "" FORCE)
add_subdirectory(submodules/glfw)
set_property(TARGET glfw PROPERTY FOLDER "submodules/GLFW3" )
#----------------------------------------------#

#--------------------ImGui---------------------#
# Set where the ImGui files are stored
set(IMGUI_PATH  submodules/imgui)

# Compile as static library 
file(GLOB IMGUI_SOURCES ${IMGUI_PATH}/*.cpp)
add_library("ImGui" STATIC ${IMGUI_SOURCES}
            ${IMGUI_PATH}/backends/imgui_impl_glfw.cpp
            ${IMGUI_PATH}/backends/imgui_impl_opengl3.cpp)
target_include_directories("ImGui" PUBLIC ${IMGUI_PATH} ${IMGUI_PATH}/backends glfw)
#----------------------------------------------#

include_directories(
    submodules/imgui
    submodules/glfw/include
    submodules/cyCodeBase
    curli
)
#========== Targets Configurations ===========#
add_executable(curli 
    curli/EntryPoint.cpp
    curli/GLFWHandler.cpp)

# Set executable dependency libraries
target_link_libraries(curli
    PRIVATE ${OPENGL_LIBRARIES}
    PRIVATE glfw
    PRIVATE glm::glm
    PUBLIC ImGui
)
