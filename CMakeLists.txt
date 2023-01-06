cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(batteryFixer)
set(TITLE_ID "SKGB4TF1X")
set(TITLE_NAME "batteryFixer")

include("${VITASDK}/share/vita.cmake" REQUIRED)

enable_language(C ASM)

set(CMAKE_C_FLAGS "-Wl,-q -Wall -O3 -std=gnu99")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -fno-rtti -fno-exceptions")

include_directories(
)

link_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(app.elf
	app/main.c
	app/debugScreenFont.c
)

target_link_libraries(app.elf
	taihen_stub
	SceDisplay_stub
	SceCtrl_stub
)

add_custom_target(app.velf ALL
	COMMAND vita-elf-create app.elf app.velf
)

add_custom_target(eboot.bin ALL
	COMMAND vita-make-fself -c app.velf eboot.bin
)

add_custom_target(${PROJECT_NAME}.vpk ALL
	COMMAND vita-mksfoex -s TITLE_ID=${TITLE_ID} "${TITLE_NAME}" param.sfo
	COMMAND vita-pack-vpk -s param.sfo -b eboot.bin -a bicr.skprx=bicr.skprx 
	-a res/icon0.png=sce_sys/icon0.png ${PROJECT_NAME}.vpk
)

add_dependencies(app.velf app.elf)
add_dependencies(eboot.bin app.velf)
add_dependencies(${PROJECT_NAME}.vpk eboot.bin)

add_executable(bicr
  kernel/main.c
  kernel/blit.c
  kernel/font.c
)

target_link_libraries(bicr
  SceDebugForDriver_stub
  SceSysmemForDriver_stub
  SceSysrootForDriver_stub
  SceKernelUtilsForDriver_stub
  SceCpuForDriver_stub
  SceThreadmgrForDriver_stub
  SceIofilemgrForDriver_stub
  SceDisplayForDriver_stub
  SceSysclibForDriver_stub
  SceSysrootForKernel_stub
  SceDipswForDriver_stub
  SceSysconForDriver_stub
  SceCpuForDriver_stub
  ScePervasiveForDriver_stub
  ScePowerForDriver_stub
  k
  gcc
)

set_target_properties(bicr
  PROPERTIES LINK_FLAGS "-nostdlib"
)

vita_create_self(bicr.skprx bicr
  CONFIG kernel/exports.yml
  UNSELF
)