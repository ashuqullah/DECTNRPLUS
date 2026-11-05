# Install script for directory: /home/ashuqullah-alizai/ncs/v3.1.1/zephyr/subsys

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
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/zephyr/subsys/canbus/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/zephyr/subsys/debug/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/zephyr/subsys/fb/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/zephyr/subsys/fs/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/zephyr/subsys/ipc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/zephyr/subsys/logging/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/zephyr/subsys/mem_mgmt/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/zephyr/subsys/mgmt/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/zephyr/subsys/modbus/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/zephyr/subsys/pm/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/zephyr/subsys/pmci/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/zephyr/subsys/portability/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/zephyr/subsys/random/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/zephyr/subsys/rtio/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/zephyr/subsys/sd/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/zephyr/subsys/stats/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/zephyr/subsys/storage/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/zephyr/subsys/task_wdt/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/zephyr/subsys/testsuite/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/zephyr/subsys/tracing/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/zephyr/subsys/usb/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/zephyr/subsys/settings/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL/zephyr/subsys/shell/cmake_install.cmake")
endif()

