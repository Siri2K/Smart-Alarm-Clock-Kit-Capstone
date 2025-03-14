set(SOURCES "bulb.c")
set(INCLUDES "include")

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS ${INCLUDES}
    REQUIRES driver freertos esp_http_client mbedtls
)