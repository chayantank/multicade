#pragma once

#include <Arduino.h>
#include "constants.h"

namespace Doom {


// =======================
// Original sound data
// =======================

constexpr uint8_t GET_KEY_SND_LEN = 90;
constexpr uint8_t SHOOT_SND_LEN = 6;
constexpr uint8_t HIT_WALL_SND_LEN = 8;
constexpr uint8_t WALK1_SND_LEN = 3;
constexpr uint8_t WALK2_SND_LEN = 3;
constexpr uint8_t MEDKIT_SND_LEN = 71;

static const uint8_t shoot_snd[] = {
  0x10,
  0x14,
  0x18,
  0x20,
  0x30,
  0x50
};

static const uint8_t hit_wall_snd[] = {
  0x83,0x83,0x82,0x8e,0x8a,0x89,0x86,0x84
};

static const uint8_t walk1_snd[] = {
  0x8f,0x8e,0x8e
};

static const uint8_t walk2_snd[] = {
  0x84,0x87,0x84
};

static const uint8_t medkit_snd[] = {
  0x55,0x20,0x3a,0x3a,0x3a,0x20,0x20,0x20,0x20,0x20,
  0x20,0x20,0x33,0x33,0x33,0x33,0x20,0x20,0x20,0x20,
  0x20,0x20,0x20,0x20,0x20,0x26,0x26,0x26,0x20,0x20,
  0x20,0x20,0x20,0x20,0x20,0x16,0x16,0x16,0x16,0x16,
  0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x16,
  0x16,0x16,0x16,0x16,0x16,0x20,0x20,0x20,0x20,0x20,
  0x20,0x20,0x20,0x20,0x15,0x15,0x15,0x15,0x15,0x15,
  0x15
};

static const uint8_t get_key_snd[] = {
  0x24,0x24,0x24,0x24,0x24,0x24,0x24,0x24,0x24,0x24,
  0x24,0x24,0x24,0x24,0x24,0x24,0x24,0x24,0x24,0x24,
  0x24,0x24,0x24,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
  0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,
  0x20,0x20,0x20,0x20,0x37,0x37,0x37,0x37,0x37,0x37,
  0x37,0x37,0x37,0x37,0x20,0x20,0x20,0x20,0x37,0x37,
  0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x20,0x20,
  0x20,0x20,0x20,0x20,0x19,0x19,0x19,0x19,0x19,0x19,
  0x19,0x19,0x19,0x19,0x19,0x19,0x19,0x19,0x19,0x19
};
// =======================
// Improved ESP32 Sound Engine
// =======================

static bool sound = false;
static const uint8_t* currentSound = nullptr;
static uint16_t currentIndex = 0;
static uint16_t currentLength = 0;
static uint32_t lastStep = 0;
static uint8_t soundStepMs = 8;

static bool gunShotActive = false;
static uint8_t gunShotStage = 0;

inline void sound_init()
{
    pinMode(SOUND_PIN, OUTPUT);
}

inline void playSound(const uint8_t* snd, uint8_t len)
{
    // Custom gunshot
    if (snd == shoot_snd)
    {
        gunShotActive = true;
        gunShotStage = 0;
        lastStep = 0;
        return;
    }

    currentSound = snd;
    currentLength = len;
    currentIndex = 0;
    sound = true;
    lastStep = 0;

    if (snd == walk1_snd || snd == walk2_snd)
        soundStepMs = 4;
    else
        soundStepMs = 8;
}

inline void off()
{
    noTone(SOUND_PIN);
    sound = false;
}

inline void updateSound()
{
    if (gunShotActive) 
    {
        if (millis() - lastStep < 12)
            return;

        lastStep = millis();

        switch (gunShotStage++)
        {
            case 0:
                tone(SOUND_PIN, 2200);
                return;

            case 1:
                tone(SOUND_PIN, 1600);
                return;

            case 2:
                tone(SOUND_PIN, 1000);
                return;

            case 3:
                tone(SOUND_PIN, 700);
                return;

            default:
                noTone(SOUND_PIN);
                gunShotActive = false;
                return;
        }
}
    if (!sound)
        return;

    if (millis() - lastStep < soundStepMs)
        return;

    lastStep = millis();

    if (currentIndex >= currentLength)
    {
        off();
        return;
    }

    uint8_t value = currentSound[currentIndex++];

    if (value == 0)
    {
        noTone(SOUND_PIN);
        return;
    }

    // Original Wolf3D / PC speaker conversion
    uint16_t freq = 1192030 / (60 * value);

    // Safety clamp for buzzer
    if (freq < 80)
        freq = 80;

    if (freq > 4000)
        freq = 4000;

    tone(SOUND_PIN, freq);
}
} // namespace Doom
