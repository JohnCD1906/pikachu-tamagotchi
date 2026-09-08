#include "storage.hpp"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <string.h>

static const char* TAG       = "storage";
static const char* NVS_NS    = "tama";
static const char* KEY_PET   = "pet";

void storage_init() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

bool storage_load(PetState& p) {
    nvs_handle_t h;
    if (nvs_open(NVS_NS, NVS_READONLY, &h) != ESP_OK) {
        pet_init_defaults(p);
        return false;
    }
    size_t len = sizeof(PetState);
    esp_err_t err = nvs_get_blob(h, KEY_PET, &p, &len);
    nvs_close(h);

    if (err != ESP_OK || len != sizeof(PetState)) {
        ESP_LOGI(TAG, "Sin guardado previo, mascota nueva");
        pet_init_defaults(p);
        return false;
    }
    ESP_LOGI(TAG, "Mascota cargada (edad=%lu s)", (unsigned long)p.age_seconds);
    return true;
}

void storage_save(const PetState& p) {
    nvs_handle_t h;
    if (nvs_open(NVS_NS, NVS_READWRITE, &h) != ESP_OK) {
        ESP_LOGW(TAG, "No se pudo abrir NVS para escribir");
        return;
    }
    nvs_set_blob(h, KEY_PET, &p, sizeof(PetState));
    nvs_commit(h);
    nvs_close(h);
}
