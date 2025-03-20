set(SOURCES "BLE.c")
set(INCLUDES "include")

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS ${INCLUDES}
    REQUIRES bt nvs_flash
)