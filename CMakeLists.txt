project(Emu68Contribs VERSION 1.0.0)

subdir_list(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})

foreach(subdir ${SUBDIRS})
	add_subdirectory(${subdir})
endforeach()