# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL")
  file(MAKE_DIRECTORY "/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL")
endif()
file(MAKE_DIRECTORY
  "/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/HS_DECT_SHELL"
  "/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/_sysbuild/sysbuild/images/HS_DECT_SHELL-prefix"
  "/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/_sysbuild/sysbuild/images/HS_DECT_SHELL-prefix/tmp"
  "/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/_sysbuild/sysbuild/images/HS_DECT_SHELL-prefix/src/HS_DECT_SHELL-stamp"
  "/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/_sysbuild/sysbuild/images/HS_DECT_SHELL-prefix/src"
  "/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/_sysbuild/sysbuild/images/HS_DECT_SHELL-prefix/src/HS_DECT_SHELL-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/_sysbuild/sysbuild/images/HS_DECT_SHELL-prefix/src/HS_DECT_SHELL-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_SHELL/build/_sysbuild/sysbuild/images/HS_DECT_SHELL-prefix/src/HS_DECT_SHELL-stamp${cfgdir}") # cfgdir has leading slash
endif()
