set(SOURCES "Wifi.c")
set(INCLUDES "include")

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS ${INCLUDES}
    REQUIRES driver freertos esp_wifi nvs_flash 
)
