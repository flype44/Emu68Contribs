cmake_minimum_required(VERSION 3.14.0)
project(ListDeviceTree VERSION 1.0.0)
get_verstring(VERSTRING)
set_emu68_tools_policies()

add_link_options(-noixemul -m68040 -mhard-float)
add_compile_definitions(PRIVATE VERSION_STRING="${VERSTRING}")

enable_language(ASM_VASM)

file(GLOB_RECURSE SOURCE *.c)

add_executable(ListDeviceTree ${SOURCE})


target_link_libraries(ListDeviceTree devicetree)

target_compile_options(ListDeviceTree PUBLIC $<$<COMPILE_LANGUAGE:C>:-noixemul -m68040 -mhard-float>)
target_compile_options(ListDeviceTree PRIVATE $<$<COMPILE_LANGUAGE:ASM_VASM>:-m68040 -quiet -I${CMAKE_SYSROOT}/m68k-amigaos/ndk-include>)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/ListDeviceTree DESTINATION ./${CONTRIB_PREFIX}/ListDeviceTree/)
#install(FILES Emu68Info.info DESTINATION ./Emu68Info/)
