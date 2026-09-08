#include "pet.hpp"

// ---- Ritmos de decaimiento: 1 punto cada N segundos ----
// Ajusta estos números para hacer la mascota más o menos exigente.
static const uint16_t HUNGER_EVERY  = 45;   // pierde hambre cada 45 s
static const uint16_t HAPPY_EVERY   = 70;   // pierde felicidad cada 70 s
static const uint16_t ENERGY_EVERY  = 90;   // pierde energía cada 90 s

static inline uint8_t clamp8(int v) {
    if (v < 0)   return 0;
    if (v > 100) return 100;
    return (uint8_t)v;
}

void pet_init_defaults(PetState& p) {
    p.hunger       = 80;
    p.happiness    = 80;
    p.energy       = 100;
    p.health       = 100;
    p.age_seconds  = 0;
    p.last_update  = 0;
    p.mood         = PetMood::IDLE;
    p.forced_sleep = false;
    p.acc_hunger   = 0;
    p.acc_happy    = 0;
    p.acc_energy   = 0;
}

PetMood pet_compute_mood(const PetState& p) {
    if (p.health == 0)                 return PetMood::DEAD;
    if (p.health < 30)                 return PetMood::SICK;
    if (p.forced_sleep || p.energy == 0) return PetMood::SLEEPING;
    if (p.happiness > 80)              return PetMood::HAPPY;
    return PetMood::IDLE;
}

void pet_apply_elapsed(PetState& p, uint32_t seconds) {
    if (seconds == 0 || p.mood == PetMood::DEAD) return;

    p.age_seconds += seconds;

    // Si está durmiendo, recupera energía en vez de gastarla y decae más lento.
    bool sleeping = p.forced_sleep || p.energy == 0;

    // --- Hambre ---
    p.acc_hunger += seconds;
    while (p.acc_hunger >= HUNGER_EVERY) {
        p.acc_hunger -= HUNGER_EVERY;
        p.hunger = clamp8((int)p.hunger - 1);
    }

    // --- Felicidad --- (más lenta si duerme)
    p.acc_happy += sleeping ? seconds / 2 : seconds;
    while (p.acc_happy >= HAPPY_EVERY) {
        p.acc_happy -= HAPPY_EVERY;
        p.happiness = clamp8((int)p.happiness - 1);
    }

    // --- Energía --- (duerme = recupera; despierto = gasta)
    p.acc_energy += seconds;
    while (p.acc_energy >= ENERGY_EVERY) {
        p.acc_energy -= ENERGY_EVERY;
        p.energy = clamp8((int)p.energy + (sleeping ? +3 : -1));
    }
    // Si ya recuperó del todo durmiendo por orden del usuario, se despierta.
    if (p.forced_sleep && p.energy >= 100) p.forced_sleep = false;

    // --- Salud --- : baja cuando hambre o felicidad están en el suelo; si no,
    // se recupera despacio.
    int health_delta = 0;
    if (p.hunger    == 0) health_delta -= (int)seconds / 30;
    if (p.happiness == 0) health_delta -= (int)seconds / 45;
    if (p.hunger > 40 && p.happiness > 40) health_delta += (int)seconds / 120;
    if (health_delta != 0) p.health = clamp8((int)p.health + health_delta);

    p.mood = pet_compute_mood(p);
}

void pet_feed(PetState& p) {
    if (p.mood == PetMood::DEAD) return;
    p.hunger    = clamp8((int)p.hunger + 25);
    p.happiness = clamp8((int)p.happiness + 3);
    p.mood      = PetMood::EATING;   // el render la deja "comiendo" un ratito
}

void pet_play(PetState& p) {
    if (p.mood == PetMood::DEAD) return;
    if (p.energy < 10) return;       // demasiado cansada para jugar
    p.happiness = clamp8((int)p.happiness + 20);
    p.energy    = clamp8((int)p.energy - 8);
    p.hunger    = clamp8((int)p.hunger - 3);
    p.mood      = PetMood::HAPPY;
}

void pet_toggle_sleep(PetState& p) {
    if (p.mood == PetMood::DEAD) return;
    p.forced_sleep = !p.forced_sleep;
    p.mood = pet_compute_mood(p);
}
