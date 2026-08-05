idf_component_register(
    SRCS "network_manager.cpp"
    INCLUDE_DIRS "."
    REQUIRES esp_wifi esp_netif
)