idf_component_register(
    SRCS "priority_lock.cpp"
    INCLUDE_DIRS "."
    REQUIRES esp_log freertos
)
