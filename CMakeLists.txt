cmake_minimum_required(VERSION 3.14.0)
project(Emu68Contribs)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CPM-init)

if(NOT TARGET devicetree)
    CPMGetPackage(devicetree.resource)
endif()

add_subdirectory(Emu68EDID)
add_subdirectory(Emu68Info)
add_subdirectory(ListDeviceTree)
add_subdirectory(WaitUntilConnected)
