cmake_minimum_required(VERSION 3.16)

# Change this to the name of your plugin #
project(StackAbusePC)

# Add your source files here #
add_library(${PROJECT_NAME} SHARED
        source/StackAbuse.cpp
        source/usb_comm.cpp
)

###########link_directories(COLDCLEAR libcold_clear.a ${PROJECT_NAME}/lib)
###########target_link_libraries(${PROJECT_NAME} LINK_PUBLIC ${COLDCLEAR})
find_package(nlohmann_json CONFIG REQUIRED)

# Add additional include directories here #
target_include_directories(${PROJECT_NAME} PRIVATE include)
# Add additional libraries here #
target_link_libraries(${PROJECT_NAME} PRIVATE ${FMT_LIBRARIES} usb-1.0 libimhex)

# ---- No need to change anything from here downwards unless you know what you're doing ---- #

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_SHARED_LIBRARY_SUFFIX ".hexplug")
add_compile_definitions(IMHEX_PROJECT_NAME=\"${PROJECT_NAME}\")
if (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc -Wl,--allow-multiple-definition -fvisibility=hidden")
endif()

add_compile_definitions(IMHEX_PROJECT_NAME="${PROJECT_NAME}")
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
setupCompilerWarnings(${PROJECT_NAME})


set(LIBROMFS_RESOURCE_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/romfs)
set(LIBROMFS_PROJECT_NAME ${PROJECT_NAME})
add_subdirectory(../../lib/external/libromfs ${CMAKE_CURRENT_BINARY_DIR}/libromfs)
set_target_properties(${LIBROMFS_LIBRARY} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBROMFS_LIBRARY})
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/install/plugins)
