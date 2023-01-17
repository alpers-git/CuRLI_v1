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

include_directories(
    submodules/freeglut/include
)
#========== Targets Configurations ============#
add_executable(curli curli/EntryPoint.cpp)

# Set executable dependency libraries
target_link_libraries(curli 
    PRIVATE  freeglut 
    PRIVATE  glm::glm)
