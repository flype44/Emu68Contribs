cmake_minimum_required(VERSION 3.14.0)
project(Emu68Info VERSION 0.1.1)
get_verstring(VERSTRING)
set_emu68_tools_policies()

add_link_options(-noixemul -m68040 -mhard-float)
add_compile_definitions(PRIVATE VERSION_STRING="${VERSTRING}")

enable_language(ASM_VASM)

add_executable(Emu68Info
    Emu68Info.c
    DeviceTree.c
    MailBox.c
    Modules.c
    VC4Msg.c
    VC4.c
)

add_custom_command(
  TARGET Emu68Info POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E rm ${PROJECT_BINARY_DIR}/AsmFuncs.vasm
  VERBATIM)

add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/AsmFuncs.vasm
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/AsmFuncs.a ${PROJECT_BINARY_DIR}/AsmFuncs.vasm
  WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
  DEPENDS "${PROJECT_SOURCE_DIR}/AsmFuncs.a"
  VERBATIM
)

add_custom_target(AsmFuncs.vasm DEPENDS ${PROJECT_BINARY_DIR}/AsmFuncs.vasm)
add_dependencies(Emu68Info AsmFuncs.vasm)

target_link_libraries(Emu68Info devicetree)

target_sources(Emu68Info PRIVATE ${PROJECT_BINARY_DIR}/AsmFuncs.vasm)

target_compile_options(Emu68Info PUBLIC $<$<COMPILE_LANGUAGE:C>:-noixemul -m68040 -mhard-float>)
target_compile_options(Emu68Info PRIVATE $<$<COMPILE_LANGUAGE:ASM_VASM>:-m68040 -quiet -I${CMAKE_SYSROOT}/m68k-amigaos/ndk-include>)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/Emu68Info DESTINATION ./${CONTRIB_PREFIX}/Emu68Info/)
#install(FILES Emu68Info.info DESTINATION ./Emu68Info/)
