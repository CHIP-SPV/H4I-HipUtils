# Copyright 2021-2023 UT-Battelle
# See LICENSE.txt in the root of the source distribution for license info.

# Specify how to install the library.
include(GNUInstallDirs)

# Right now, this is a header-only library.
# But we need a target to be defined to install 
# the CMake support files.
add_library(hiputils INTERFACE)
target_include_directories(hiputils
    INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include;${CMAKE_BINARY_DIR}/include>"
		$<INSTALL_INTERFACE:include>)
install(TARGETS hiputils
    EXPORT hiputils
)

# Specify how to install the library headers.
install(DIRECTORY
    ${CMAKE_SOURCE_DIR}/include/h4i
    TYPE INCLUDE)

install(EXPORT hiputils
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/hiputils
    NAMESPACE H4I::
)

include(CMakePackageConfigHelpers)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/../CMake/HipUtilsConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/hiputilsConfig.cmake
    PATH_VARS CMAKE_INSTALL_INCLUDEDIR
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/hiputils
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/hiputilsConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/hiputils
)

