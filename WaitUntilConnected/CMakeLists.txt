cmake_minimum_required(VERSION 3.14.0)
project(WaitUntilConnected VERSION 1.0.0)
get_verstring(VERSTRING)
set_emu68_tools_policies()

add_link_options(-noixemul -m68040 -mhard-float)
add_compile_definitions(PRIVATE VERSION_STRING="${VERSTRING}")

enable_language(ASM_VASM)

file(GLOB_RECURSE SOURCE *.c)

add_executable(WaitUntilConnected ${SOURCE})


target_link_libraries(WaitUntilConnected devicetree)

target_compile_options(WaitUntilConnected PUBLIC $<$<COMPILE_LANGUAGE:C>:-noixemul -m68040 -mhard-float>)
target_compile_options(WaitUntilConnected PRIVATE $<$<COMPILE_LANGUAGE:ASM_VASM>:-m68040 -quiet -I${CMAKE_SYSROOT}/m68k-amigaos/ndk-include>)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/WaitUntilConnected DESTINATION ./${CONTRIB_PREFIX}/WaitUntilConnected/)
install(FILES README.md DESTINATION ./${CONTRIB_PREFIX}/WaitUntilConnected/)
#install(FILES WaitUntilConnected.info DESTINATION ./WaitUntilConnected/)
