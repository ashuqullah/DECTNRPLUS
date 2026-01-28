# src/CMakeLists.txt

# Add main.c to the app target
target_sources(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/main.c
)

# Include path for headers used by app
target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}                # src/*
    ${CMAKE_CURRENT_SOURCE_DIR}/hs_hello   # hs_hello.h
    ${CMAKE_CURRENT_SOURCE_DIR}/hs_core   # hs_core.h
    ${CMAKE_CURRENT_SOURCE_DIR}/hs_ping   # hs_ping.h
    ${CMAKE_CURRENT_SOURCE_DIR}/hs_scheduller   # hs_scheduller.h
    ${CMAKE_CURRENT_SOURCE_DIR}/hs_perf   # perf.h
)


# HSDECT subtree (each subfolder adds its own sources to app)
add_subdirectory(hs_hello)
add_subdirectory(hs_ping)
add_subdirectory(hs_scheduller)
add_subdirectory(hs_core)
add_subdirectory(hs_shell)
add_subdirectory(hs_perf)