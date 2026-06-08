#include "pet_main.h"
#include "pet_display.h"
#include "pet_input.h"
#include <Arduino.h>
#include <Preferences.h>
#include <Adafruit_SSD1306.h>

extern Adafruit_SSD1306 menuDisplay;

#define BUZZER_PIN 14

namespace Pet {

Preferences preferences;

int hunger = 100;
int happiness = 100;
int energy = 100;
int dirt = 0;

int petX = 50;
int petY = 20;

int menuSelected = 0; // 0=Food, 1=Play, 2=Clean, 3=Sleep
bool sleeping = false;
int animTimer = 0;
int petState = 0; // 0=idle, 1=eating, 2=sleeping, 3=sad
int animFrame = 0;

unsigned long lastTick = 0;

void saveStats() {
    preferences.begin("pocketpet", false);
    preferences.putInt("hunger", hunger);
    preferences.putInt("happiness", happiness);
    preferences.putInt("energy", energy);
    preferences.putInt("dirt", dirt);
    preferences.end();
}

void loadStats() {
    preferences.begin("pocketpet", true);
    hunger = preferences.getInt("hunger", 100);
    happiness = preferences.getInt("happiness", 100);
    energy = preferences.getInt("energy", 100);
    dirt = preferences.getInt("dirt", 0);
    preferences.end();
}

void setup() {
    display_setup();
    input_setup();
    loadStats();
    sleeping = false;
}

void loop() {
    unsigned long now = millis();
    
    // Stats decay every 5 seconds for demonstration (normally much slower)
    if (now - lastTick > 5000) {
        lastTick = now;
        if (!sleeping) {
            if (hunger > 0) hunger -= 2;
            if (energy > 0) energy -= 1;
            if (random(0, 100) < 10) dirt++;
        } else {
            if (energy < 100) energy += 10;
            if (hunger > 0) hunger -= 1;
        }
        
        if (hunger < 30 || dirt > 3) {
            happiness -= 5;
        } else {
            if (happiness < 100 && !sleeping) happiness += 1;
        }
        
        if (happiness < 0) happiness = 0;
        
        saveStats(); // Save periodically
    }
    
    // State logic
    if (sleeping) {
        petState = 2;
        if (input_action()) {
            sleeping = false; // wake up
            tone(BUZZER_PIN, 600, 100);
        }
    } else {
        if (happiness < 30 || dirt > 2) {
            petState = 3; // sad
        } else {
            petState = 0; // idle
        }
        
        // Input
        if (input_left()) {
            menuSelected--;
            if (menuSelected < 0) menuSelected = 3;
            tone(BUZZER_PIN, 800, 30);
        }
        if (input_right()) {
            menuSelected++;
            if (menuSelected > 3) menuSelected = 0;
            tone(BUZZER_PIN, 800, 30);
        }
        
        if (input_action()) {
            if (menuSelected == 0) { // Food
                if (hunger < 100) {
                    hunger += 20;
                    if (hunger > 100) hunger = 100;
                    dirt += 1; // eating makes poop
                    petState = 1;
                    animTimer = 20; // force eat anim
                    tone(BUZZER_PIN, 400, 50); delay(60); tone(BUZZER_PIN, 500, 50);
                }
            } else if (menuSelected == 1) { // Play
                if (energy > 10) {
                    happiness += 20;
                    if (happiness > 100) happiness = 100;
                    energy -= 15;
                    petY -= 10; // jump
                    tone(BUZZER_PIN, 800, 50); delay(60); tone(BUZZER_PIN, 1200, 100);
                }
            } else if (menuSelected == 2) { // Clean
                if (dirt > 0) {
                    dirt = 0;
                    happiness += 10;
                    tone(BUZZER_PIN, 1500, 100);
                }
            } else if (menuSelected == 3) { // Sleep
                sleeping = true;
                tone(BUZZER_PIN, 300, 200); delay(250); tone(BUZZER_PIN, 200, 400);
            }
            saveStats();
        }
    }
    
    // Animation
    animTimer--;
    if (animTimer <= 0) {
        animTimer = sleeping ? 40 : 20;
        animFrame = !animFrame;
        
        if (!sleeping && petState == 0) {
            // random roam
            petX += random(-5, 6);
            if (petX < 10) petX = 10;
            if (petX > 90) petX = 90;
        }
    }
    
    // Gravity (if jumped during play)
    if (petY < 20) {
        petY += 2;
    }
    
    // Render
    display_clear();
    
    // Menus
    drawIcon(16, 44, 0, menuSelected == 0);
    drawIcon(40, 44, 1, menuSelected == 1);
    drawIcon(64, 44, 2, menuSelected == 2);
    drawIcon(88, 44, 3, menuSelected == 3);
    
    // Dirt
    for(int i=0; i<dirt; i++) {
        int dx = 20 + i*15;
        drawRect(dx, 34, 4, 4, 1);
        drawLine(dx+1, 33, dx+2, 33, 1);
    }
    
    // Pet
    drawPet(petX, petY, petState, animFrame);
    
    // UI Bars
    drawLine(0, 0, 0, 40, 1);
    drawText(2, 2, "H"); fillRect(10, 2, hunger / 5, 4, 1);
    drawText(2, 10, "J"); fillRect(10, 10, happiness / 5, 4, 1);
    drawText(2, 18, "E"); fillRect(10, 18, energy / 5, 4, 1);
    
    // Night mode
    if (sleeping) {
        // Invert screen except for some stars
        menuDisplay.invertDisplay(true);
    } else {
        menuDisplay.invertDisplay(false);
    }
    
    display_render();
}

}
