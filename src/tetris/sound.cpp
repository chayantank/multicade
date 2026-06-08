#include <Arduino.h>
#include <pgmspace.h>
#include "sound.h"
#include "config.h"

namespace Tetris {


// ─── LEDC Configuration ──────────────────────────────
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  #define SOUND_USE_NEW_API
#endif

#define LEDC_CHANNEL  0
#define LEDC_RES      8

// ─── Melody System ────────────────────────────────────
struct ToneNote {
  uint16_t freq;
  uint16_t duration;
};

#define MAX_MELODY  5

static ToneNote melody[MAX_MELODY];
static uint8_t  melody_len = 0;
static uint8_t  melody_idx = 0;
static uint32_t tone_end_ms = 0;
static bool     tone_playing = false;

// ─── Background Theme System ──────────────────────────
static const ToneNote* current_song = nullptr;
static uint16_t song_len = 0;
static uint16_t song_idx = 0;
static bool     song_loop = false;

// Korobeiniki (Tetris Theme A)
const ToneNote tetris_theme[] PROGMEM = {
  {659, 200}, {494, 100}, {523, 100}, {587, 200}, {523, 100}, {494, 100},
  {440, 200}, {440, 100}, {523, 100}, {659, 200}, {587, 100}, {523, 100},
  {494, 200}, {494, 100}, {523, 100}, {587, 200}, {659, 200},
  {523, 200}, {440, 200}, {440, 400},
  // Part 2
  {587, 200}, {698, 100}, {880, 200}, {784, 100}, {698, 100},
  {659, 300}, {523, 100}, {659, 200}, {587, 100}, {523, 100},
  {494, 200}, {494, 100}, {523, 100}, {587, 200}, {659, 200},
  {523, 200}, {440, 200}, {440, 400}
};
#define THEME_LEN (sizeof(tetris_theme)/sizeof(tetris_theme[0]))

// ─── Low-Level Tone ───────────────────────────────────
static void _tone_on(uint16_t freq) {
#ifdef SOUND_USE_NEW_API
  ledcWriteTone(BUZZER_PIN, freq);
#else
  ledcWriteTone(LEDC_CHANNEL, freq);
#endif
}

static void _tone_off() {
#ifdef SOUND_USE_NEW_API
  ledcWriteTone(BUZZER_PIN, 0);
#else
  ledcWriteTone(LEDC_CHANNEL, 0);
#endif
}

static void _play_tone(uint16_t freq, uint16_t dur) {
  if (freq > 0) {
    _tone_on(freq);
  } else {
    _tone_off();
  }
  tone_end_ms = millis() + dur;
  tone_playing = true;
}

// ─── Init ─────────────────────────────────────────────
void sound_init() {
#ifdef SOUND_USE_NEW_API
  ledcAttach(BUZZER_PIN, 2000, LEDC_RES);
#else
  ledcSetup(LEDC_CHANNEL, 2000, LEDC_RES);
  ledcAttachPin(BUZZER_PIN, LEDC_CHANNEL);
#endif
  _tone_off();
}

// ─── Update (call each frame) ─────────────────────────
void sound_update() {
  if (!tone_playing) return;

  if (millis() >= tone_end_ms) {
    // Current note finished
    if (melody_len > 0) {
      if (melody_idx < melody_len - 1) {
        // Next note in SFX
        melody_idx++;
        _play_tone(melody[melody_idx].freq, melody[melody_idx].duration);
      } else {
        // SFX finished
        melody_len = 0;
        if (current_song) {
          // Resume song from current index
          ToneNote n;
          n.freq = pgm_read_word(&current_song[song_idx].freq);
          n.duration = pgm_read_word(&current_song[song_idx].duration);
          _play_tone(n.freq, n.duration);
        } else {
          _tone_off();
          tone_playing = false;
        }
      }
    } else if (current_song) {
      // Background song playing
      song_idx++;
      if (song_idx >= song_len) {
        if (song_loop) {
          song_idx = 0; // Loop
        } else {
          current_song = nullptr;
          _tone_off();
          tone_playing = false;
          return;
        }
      }
      ToneNote n;
      n.freq = pgm_read_word(&current_song[song_idx].freq);
      n.duration = pgm_read_word(&current_song[song_idx].duration);
      _play_tone(n.freq, n.duration);
    } else {
      // No melody, no song -> turn off!
      _tone_off();
      tone_playing = false;
    }
  }
}

// ─── Play Melody (SFX) ────────────────────────────────
static void _play_melody(const ToneNote* notes, uint8_t len) {
  if (len > MAX_MELODY) len = MAX_MELODY;
  for (uint8_t i = 0; i < len; i++) {
    melody[i] = notes[i];
  }
  melody_len = len;
  melody_idx = 0;
  _play_tone(melody[0].freq, melody[0].duration);
}

// ─── Theme Control ────────────────────────────────────
void sound_theme_start() {
  current_song = tetris_theme;
  song_len = THEME_LEN;
  song_idx = 0;
  song_loop = true;
  
  // Start playing if no SFX is currently overriding
  if (melody_len == 0) {
    ToneNote n;
    n.freq = pgm_read_word(&current_song[song_idx].freq);
    n.duration = pgm_read_word(&current_song[song_idx].duration);
    _play_tone(n.freq, n.duration);
  }
}

void sound_theme_stop() {
  current_song = nullptr;
  if (melody_len == 0) {
    _tone_off();
    tone_playing = false;
  }
}

// ─── Sound Effects ────────────────────────────────────
void sound_move() {
  melody_len = 0;
  _play_tone(200, 30);
}

void sound_rotate() {
  melody_len = 0;
  _play_tone(400, 30);
}

void sound_hard_drop() {
  const ToneNote notes[] = { {300, 40}, {150, 60} };
  _play_melody(notes, 2);
}

void sound_line_clear() {
  const ToneNote notes[] = { {523, 60}, {659, 60}, {784, 80} };
  _play_melody(notes, 3);
}

void sound_level_up() {
  const ToneNote notes[] = { {523, 50}, {659, 50}, {784, 50}, {1047, 100} };
  _play_melody(notes, 4);
}

void sound_game_over() {
  sound_theme_stop();
  const ToneNote notes[] = { {400, 150}, {300, 150}, {200, 300} };
  _play_melody(notes, 3);
}

} // namespace Tetris
