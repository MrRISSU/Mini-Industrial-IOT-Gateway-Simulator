idf_component_register(
    SRC_DIRS "src"
    INCLUDE_DIRS "include"
    REQUIRES nvs_flash "DeviceSimulator" "TagsRegistry" "Network" "Transport" "MQTT"
)