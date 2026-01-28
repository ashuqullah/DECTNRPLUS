# src/hsdect/hello/CMakeLists.txt

# Add HELLO module sources directly to the app
target_sources(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/hello.c
    
)

# Include this folder for its own header
target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
