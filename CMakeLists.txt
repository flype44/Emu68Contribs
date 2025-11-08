# CPM BEGIN
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CPM)

set(CPM_SOURCE_CACHE "${CMAKE_CURRENT_SOURCE_DIR}/external")
CPMUsePackageLock(package-lock.cmake)

CPMGetPackage(CPMLicenses.cmake)
CPMGetPackage(CMakeAmigaCommon)

if(NOT TARGET write-licenses)
	cpm_licenses_create_disclaimer_target(
		write-licenses "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/third_party.txt" "${CPM_PACKAGES}"
	)
endif()
# CPM END

add_subdirectory(Emu68Info)
add_subdirectory(ListDeviceTree)
add_subdirectory(WaitUntilConnected)
