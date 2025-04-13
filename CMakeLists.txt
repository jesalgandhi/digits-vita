cmake_minimum_required(VERSION 3.2)

# vita toolchain
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

# project name
# project(digits_game C)
project(dijits_game)

# Vita helper macros
include("${VITASDK}/share/vita.cmake" REQUIRED)

# app config 
set(VITA_APP_NAME "Dijits")
set(VITA_TITLEID  "DGMG00001")   # Must be a unique 9-character string.
set(VITA_VERSION  "01.00")

# Add common compiler flags. We add -lm for the math library.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -lm")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Include directory for your source files.
include_directories(
  ../common
  # ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Add library directories if needed.
link_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

# Build the executable.
add_executable(${PROJECT_NAME}
  src/main.cpp
  # src/game_logic.c
)

# Link against the essential Vita libraries:
# - SceCtrl_stub: Input
# - SceDisplay_stub: Display functions
# - vita2d: Our 2D graphics library
# - SceGxm_stub: For GPU functions
# - SceAppMgr_stub: For functions like sceAppMgrGetBudgetInfo
# - SceFont_stub: For font and text operations inside vita2d
# - m: Math library (for cosf, sinf, tan)
target_link_libraries(${PROJECT_NAME}
  SceCtrl_stub
  vita2d
  SceDisplay_stub
  SceGxm_stub
  SceAppMgr_stub
  ScePgf_stub
  ScePvf_stub
  SceCommonDialog_stub
  freetype
  png
  z
  bz2
  m
)

## Create Vita files
vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME})
# The FILE directive lets you add additional files to the VPK, the syntax is
# FILE src_path dst_path_in_vpk. In this case, we add the LiveArea paths.
vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
)
