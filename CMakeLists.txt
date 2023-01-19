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
add_subdirectory(submodules/freeglut submodules/freeglut/build EXCLUDE_FROM_ALL)
add_subdirectory(submodules/glm EXCLUDE_FROM_ALL)

#--------------------ImGui---------------------#
# Set where the ImGui files are stored
set(IMGUI_PATH  submodules/imgui)
    
# Compile as static library 
file(GLOB IMGUI_SOURCES ${IMGUI_PATH}/*.cpp)
add_library("ImGui" STATIC ${IMGUI_SOURCES} 
            ${IMGUI_PATH}/backends/imgui_impl_glut.cpp 
            ${IMGUI_PATH}/backends/imgui_impl_opengl3.cpp)
target_include_directories("ImGui" PUBLIC ${IMGUI_PATH} ${IMGUI_PATH}/backends )

include_directories(
    submodules/freeglut/include
    submodules/imgui
    curli
)
#========== Targets Configurations ====== "curli/GlutHandlers.h"======#
add_executable(curli curli/EntryPoint.cpp)

# Set executable dependency libraries
target_link_libraries(curli 
    PRIVATE  freeglut 
    PRIVATE  glm::glm
    PUBLIC ImGui)
