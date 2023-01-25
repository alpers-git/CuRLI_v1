# CuRLI_v1
CS6610 Interactive Computer Graphics HW repository. Stands for **C**omp**U**ter **R**enders **L**ot of **I**mages. It is inteded as a toy renderer

### Building CuRLI for Windows
---
Building CuRLI for windows requires Cmake 3.0
#### Dependencies 
Most of these dependencies are included as submodules and compiles with CMake. Only dependency that does not exist as submodule is glad which can be downloaded from generated link obtained [glad web page](https://glad.dav1d.de/).
- MSVCv143 - VS2022 C++ or above
- [Cmake 3.0](https://cmake.org/)
- [GLM](https://github.com/g-truc/glm)
- [ImGui](https://github.com/ocornut/imgui)
- [GLFW](https://github.com/glfw/glfw) ~~FreeGlut~~
- [Glad](https://glad.dav1d.de/#language=c&specification=gl&api=gl%3D4.6&api=gles1%3Dnone&api=gles2%3Dnone&api=glsc2%3Dnone&profile=compatibility&loader=on)
- [cyCodeBase](http://www.cemyuksel.com/cyCodeBase/)(only for .obj importer ATM)

#### Building with Cmake
1. Clone or download the files(unzip the downloaded files).
2. Create a folder to *build* binaries.
3. Run CmakeGui or Cmake select source as the project root directory and where to build binaries as *build* folder.
4. Select Visual Studio 17 2022 as the generator.
5. Configure and Generate
6. Navigate to *build* folder and open curli.sln file with Visual Studio
7. Select Under Build>Build solution(F7)
8. Run the executable i.e. **./curli.exe [params]** from console

### Milestones
---
#### Project 1 - H*llo World
- Creating Window context
- Keyboard listeners where ‘esc’ is used to call glutLeaveMainLoop();
- Setting window size, position, name and clear color during initialization.
- Idle function where animation between two colors are generated using linear interpolation of sine value of time(ms).
Some Screenshots:
![](./images/pr1_1.jpg | width=100)
![alt text](./images/pr1_2.jpg "")