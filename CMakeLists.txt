cmake_minimum_required(VERSION 3.10)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(DrawApp C)

include("${VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "DrawApp")
set(VITA_TITLEID  "DRAW00001")
set(VITA_VERSION  "01.00")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2")

add_executable(${PROJECT_NAME}
  src/main.c
  src/canvas.c
  src/ui.c
  src/input.c
  src/colors.c
)

target_include_directories(${PROJECT_NAME} PRIVATE src)

target_link_libraries(${PROJECT_NAME}
  vita2d
  SceDisplay_stub
  SceCtrl_stub
  SceTouch_stub
  SceSysmodule_stub
  SceGxm_stub
  SceCommonDialog_stub
  ScePgf_stub
  ScePvf_stub
  SceSharedFb_stub
  SceAppMgr_stub
  freetype
  png
  jpeg
  z
  m
  c
)

vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME})

vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
)
