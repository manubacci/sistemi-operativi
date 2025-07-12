cmake_minimum_required(VERSION 3.3)
project(
	Panda+
	VERSION 3.0
	LANGUAGES C ASM
)

add_library(crtso STATIC ${UMPS_SRC}/crtso.S)
add_library(libumps STATIC ${UMPS_SRC}/libumps.S)

include_directories("lib")
add_subdirectory("lib")
add_subdirectory("phase1")

add_subdirectory("phase1/p1test")

add_custom_target(
	kernel.core.umps ALL
	COMMAND umps3-elf2umps -k ${PROJECT_BINARY_DIR}/kernel
	BYPRODUCTS kernel.core.umps kernel.stab.umps
	DEPENDS kernel
)