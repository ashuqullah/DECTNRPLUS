
cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(HS_DECT)

# Let the src/ directory manage its own sources
add_subdirectory(src)
