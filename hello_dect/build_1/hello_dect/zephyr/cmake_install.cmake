# Install script for directory: /home/ashuqullah-alizai/ncs/v3.3.0/zephyr

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

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/home/ashuqullah-alizai/ncs/toolchains/911f4c5c26/opt/zephyr-sdk/arm-zephyr-eabi/bin/arm-zephyr-eabi-objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/zephyr/arch/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/zephyr/lib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/zephyr/soc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/zephyr/boards/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/zephyr/subsys/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/zephyr/drivers/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/nrf/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/mcuboot/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/mbedtls/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/trusted-firmware-m/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/cjson/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/azure-sdk-for-c/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/cirrus-logic/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/openthread/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/memfault-firmware-sdk/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/hostap/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/canopennode/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/chre/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/cmsis/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/cmsis-dsp/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/cmsis-nn/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/cmsis_6/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/fatfs/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/hal_nordic/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/hal_st/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/hal_tdk/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/hal_wurthelektronik/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/liblc3/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/libmetal/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/libsbc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/littlefs/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/loramac-node/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/lvgl/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/mipi-sys-t/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/nanopb/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/nrf_wifi/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/open-amp/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/percepio/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/picolibc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/segger/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/uoscore-uedhoc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/zcbor/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/nrfxlib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/nrf_hw_models/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/modules/connectedhomeip/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/zephyr/kernel/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/zephyr/cmake/flash/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/zephyr/cmake/usage/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/zephyr/cmake/reports/cmake_install.cmake")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/ashuqullah-alizai/Documents/DECT_2020_GIT/hello_dect/build_1/hello_dect/zephyr/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
