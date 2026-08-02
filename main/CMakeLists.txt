idf_component_register(
    SRCS "TagsRegister.cpp" "MQTT.cpp" "network_manager.cpp" "main.cpp" "SensorSimulatorBase.cpp" "TempSensorSimulator.cpp"
    INCLUDE_DIRS "."
    REQUIRES nvs_flash esp_wifi esp_netif
)