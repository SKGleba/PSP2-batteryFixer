cmake_minimum_required(VERSION 2.8)

set(CMAKE_SYSTEM_NAME "Generic")
set(CMAKE_C_COMPILER "arm-vita-eabi-gcc")
set(CMAKE_CXX_COMPILER "arm-vita-eabi-g++")

set(CMAKE_C_FLAGS "-Wl,-q -Wall -O3 -std=gnu99")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -fno-rtti -fno-exceptions")

link_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(bicr.elf
	main.c
)

target_link_libraries(bicr.elf
	taihenForKernel_stub
	SceThreadmgrForDriver_stub
	SceIofilemgrForDriver_stub
	taihenModuleUtils_stub
	)

set_target_properties(bicr.elf
	PROPERTIES LINK_FLAGS "-nostdlib"
	COMPILE_FLAGS "-D__VITA_KERNEL__"
)

add_custom_target(bicr.skprx ALL
	COMMAND vita-elf-create -e ${CMAKE_SOURCE_DIR}/bicr.yml bicr.elf bicr.velf
	COMMAND vita-make-fself -c bicr.velf bicr.skprx
)
add_dependencies(bicr.skprx bicr.elf)
