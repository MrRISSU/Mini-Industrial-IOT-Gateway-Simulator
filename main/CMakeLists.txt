idf_component_register(
    SRCS "mqtt_client.cpp" "network_manager.cpp" "main.cpp" "SensorSimulatorBase.cpp" "TempSensorSimulator.cpp"
    INCLUDE_DIRS "."
    REQUIRES esp_wifi nvs_flash esp_netif
)