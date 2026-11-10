# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_Mesh")
  file(MAKE_DIRECTORY "/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_Mesh")
endif()
file(MAKE_DIRECTORY
  "/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_Mesh/build/HS_DECT_Mesh"
  "/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_Mesh/build/_sysbuild/sysbuild/images/HS_DECT_Mesh-prefix"
  "/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_Mesh/build/_sysbuild/sysbuild/images/HS_DECT_Mesh-prefix/tmp"
  "/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_Mesh/build/_sysbuild/sysbuild/images/HS_DECT_Mesh-prefix/src/HS_DECT_Mesh-stamp"
  "/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_Mesh/build/_sysbuild/sysbuild/images/HS_DECT_Mesh-prefix/src"
  "/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_Mesh/build/_sysbuild/sysbuild/images/HS_DECT_Mesh-prefix/src/HS_DECT_Mesh-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_Mesh/build/_sysbuild/sysbuild/images/HS_DECT_Mesh-prefix/src/HS_DECT_Mesh-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/ashuqullah-alizai/Documents/DECT_2020_GIT/HS_DECT_Mesh/build/_sysbuild/sysbuild/images/HS_DECT_Mesh-prefix/src/HS_DECT_Mesh-stamp${cfgdir}") # cfgdir has leading slash
endif()
