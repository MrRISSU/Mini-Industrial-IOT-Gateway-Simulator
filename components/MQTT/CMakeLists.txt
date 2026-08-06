# Scan the src directory for all .cpp files
file(GLOB_RECURSE srcs "src/*.cpp")

# Register the component
idf_component_register(
    SRCS "${srcs}"
    INCLUDE_DIRS "include"
    REQUIRES esp_timer tcp_transport freertos mbedtls
)

# Force C++17 or C++20 standard for this component to use modern features like std::string_view
set_source_files_properties(${srcs} PROPERTIES COMPILE_FLAGS "-std=gnu++17")