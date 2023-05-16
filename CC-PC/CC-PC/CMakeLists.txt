# CMakeList.txt : CMake project for CC-PC, include source and define
# project specific logic here.
#


find_package(imgui REQUIRED)
find_package(libusb CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)



# Add source to this project's executable.
add_executable (CC-PC "CC-PC.cpp" "CC-PC.h" "../imgui/imgui_impl_glfw.cpp" "../imgui/imgui_impl_glfw.hpp"
"../imgui/imgui_impl_opengl3.cpp"
"../imgui/imgui_impl_opengl3.hpp"
"../imgui/imgui_impl_opengl3_loader.hpp"
"usb-comm.hpp"
"usb-comm.cpp"
)

target_include_directories(CC-PC PRIVATE ${LIBUSB_INCLUDE_DIRS})

target_link_libraries(CC-PC PRIVATE imgui::imgui OpenGL::GL glfw ${LIBUSB_LIBRARIES} nlohmann_json)


# TODO: Add tests and install targets if needed.
