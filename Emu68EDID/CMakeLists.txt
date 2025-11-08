cmake_minimum_required(VERSION 3.14.0)
project(Emu68EDID VERSION 1.0.0)
get_verstring(VERSTRING)

add_link_options(-noixemul -m68040 -mhard-float)
add_compile_definitions(PRIVATE VERSION_STRING="${VERSTRING}")

enable_language(ASM_VASM)

add_executable(Emu68EDID
    src/Main.c
    src/EDID.c
    src/DeviceTree.c
    src/MailBox.c
)

add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/AsmFuncs.vasm
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/AsmFuncs.a ${PROJECT_BINARY_DIR}/AsmFuncs.vasm
  WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
  DEPENDS "${PROJECT_SOURCE_DIR}/src/AsmFuncs.a"
  VERBATIM
)

add_custom_target(AsmFuncs_gen_vasm${PROJECT_NAME} DEPENDS ${PROJECT_BINARY_DIR}/AsmFuncs.vasm)
add_dependencies(Emu68EDID AsmFuncs_gen_vasm${PROJECT_NAME})

target_link_libraries(Emu68EDID devicetree m)

target_sources(Emu68EDID PRIVATE ${PROJECT_BINARY_DIR}/AsmFuncs.vasm)

target_compile_options(Emu68EDID PUBLIC $<$<COMPILE_LANGUAGE:C>:-noixemul -m68040 -mhard-float>)
target_compile_options(Emu68EDID PRIVATE $<$<COMPILE_LANGUAGE:ASM_VASM>:-m68040 -quiet -I${CMAKE_SYSROOT}/m68k-amigaos/ndk-include>)

install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/Emu68EDID" DESTINATION "${CONTRIB_PREFIX}Emu68EDID/")
