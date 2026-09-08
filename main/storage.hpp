// ============================================================================
//  Persistencia del estado de la mascota en NVS (flash).
//  Guarda el struct completo como blob. Sobrevive apagados y deep-sleep.
// ============================================================================
#pragma once
#include "pet.hpp"

// Inicializa el subsistema NVS. Llamar una vez al arrancar.
void storage_init();

// Carga la mascota guardada. Si no hay nada guardado (primer arranque),
// rellena `p` con los valores por defecto y devuelve false.
bool storage_load(PetState& p);

// Guarda la mascota. Puede bloquear unos ms mientras escribe flash:
// por eso se llama desde la tarea de lógica (core 0), nunca desde el render.
void storage_save(const PetState& p);
