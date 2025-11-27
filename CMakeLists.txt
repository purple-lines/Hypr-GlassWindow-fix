cmake_minimum_required(VERSION 3.19)

project(glasswindow
    DESCRIPTION "Glass window effect plugin for Hyprland"
    VERSION 1.0.0
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the Hyprland plugin API (automatically exposed via pkg-config)
find_package(PkgConfig REQUIRED)
pkg_check_modules(Hyprland REQUIRED IMPORTED_TARGET hyprland)

# Find pixman-1 using pkg-config
pkg_check_modules(PIXMAN REQUIRED pixman-1)

# Find libdrm using pkg-config
pkg_check_modules(LIBDRM REQUIRED libdrm)

# Create plugin library
add_library(glasswindow SHARED
    src/glasswindow.cpp
)

# Set include dirs, add libdrm include dirs here
target_include_directories(glasswindow PRIVATE
    include/
    ${Hyprland_INCLUDE_DIRS}
    ${PIXMAN_INCLUDE_DIRS}
    ${LIBDRM_INCLUDE_DIRS}   # Add libdrm include dirs here
)

# Link against Hyprland plugin API, pixman, and libdrm
target_link_libraries(glasswindow PRIVATE
    PkgConfig::Hyprland
    ${PIXMAN_LIBRARIES}
    ${LIBDRM_LIBRARIES}     # Link libdrm libs here
)

# Add compile flags from pixman and libdrm
target_compile_options(glasswindow PRIVATE
    ${PIXMAN_CFLAGS_OTHER}
    ${LIBDRM_CFLAGS_OTHER}   # Add libdrm compile flags here
    -Wall -Wextra -Wno-unused-parameter
)

# Define WLR_USE_UNSTABLE if needed
target_compile_definitions(glasswindow PRIVATE WLR_USE_UNSTABLE)

# Correct plugin properties
set_target_properties(glasswindow PROPERTIES
    PREFIX ""
    SUFFIX ".so"
)

# Optional install target
