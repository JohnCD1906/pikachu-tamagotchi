idf_component_register(
    SRCS
        "main.cpp"
        "pet.cpp"
        "storage.cpp"
        "input.cpp"
        "render.cpp"
        "sprites.cpp"
    INCLUDE_DIRS "."
    REQUIRES nvs_flash driver esp_timer LovyanGFX
)
