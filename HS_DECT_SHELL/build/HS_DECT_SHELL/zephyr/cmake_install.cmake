# Install script for directory: /home/ashuqullah-alizai/ncs/v3.1.1/zephyr

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/home/ashuqullah-alizai/ncs/toolchains/b2ecd2435d/opt/zephyr-sdk/arm-zephyr-eabi/bin/arm-zephyr-eabi-objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/zephyr/arch/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/zephyr/lib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/zephyr/soc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/zephyr/boards/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/zephyr/subsys/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/zephyr/drivers/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/nrf/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/hostap/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/mcuboot/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/mbedtls/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/trusted-firmware-m/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/cjson/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/azure-sdk-for-c/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/cirrus-logic/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/openthread/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/memfault-firmware-sdk/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/canopennode/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/chre/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/lz4/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/nanopb/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/zscilib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/cmsis/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/cmsis-dsp/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/cmsis-nn/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/cmsis_6/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/fatfs/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/hal_nordic/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/hal_st/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/hal_tdk/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/hal_wurthelektronik/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/liblc3/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/libmetal/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/littlefs/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/loramac-node/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/lvgl/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/mipi-sys-t/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/nrf_wifi/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/open-amp/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/percepio/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/picolibc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/segger/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/tinycrypt/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/uoscore-uedhoc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/zcbor/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/nrfxlib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/nrf_hw_models/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/modules/connectedhomeip/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/zephyr/kernel/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/zephyr/cmake/flash/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/zephyr/cmake/usage/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/zephyr/cmake/reports/cmake_install.cmake")
endif()

