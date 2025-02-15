set(SOURCES "WIFI.c")
set(INCLUDES "include")

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS ${INCLUDES}
)
