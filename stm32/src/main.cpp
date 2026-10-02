#include <Arduino.h>

// ── Pins (Blue Pill STM32F103C8) ─────────────────────────────
constexpr uint32_t BTN_PIN = PB12;  // кнопка між PB12 і GND, INPUT_PULLUP (active LOW)
constexpr uint32_t LED_PIN = PC13;  // вбудований світлодіод, active LOW

// Serial1 = USART1: PA9 = TX -> ESP32 GPIO18, PA10 = RX <- ESP32 GPIO17

// ── UART protocol ────────────────────────────────────────────
constexpr uint32_t UART_BAUD  = 115200;
constexpr char     CMD_TOGGLE = 'T';  // "перемкни свій світлодіод"

constexpr uint32_t DEBOUNCE_MS = 50;

// ── State ────────────────────────────────────────────────────
bool     ledOn        = false;
bool     btnStable    = HIGH;
bool     btnLastRaw   = HIGH;
uint32_t btnChangedAt = 0;

static void setLed(bool on) {
    digitalWrite(LED_PIN, on ? LOW : HIGH);  // PC13 світиться при LOW
}

// Повертає true один раз на кожне натискання (перехід HIGH -> LOW після debounce)
static bool buttonPressed() {
    bool raw = digitalRead(BTN_PIN);
    if (raw != btnLastRaw) {
        btnLastRaw   = raw;
        btnChangedAt = millis();
    }
    if (millis() - btnChangedAt >= DEBOUNCE_MS && raw != btnStable) {
        btnStable = raw;
        return btnStable == LOW;
    }
    return false;
}

void setup() {
    Serial1.begin(UART_BAUD);

    pinMode(BTN_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    setLed(false);
}

void loop() {
    // 1. Кнопка ESP32 -> світлодіод STM32
    while (Serial1.available()) {
        if (Serial1.read() == CMD_TOGGLE) {
            ledOn = !ledOn;
            setLed(ledOn);
        }
    }

    // 2. Кнопка STM32 -> світлодіод ESP32
    if (buttonPressed()) {
        Serial1.write(CMD_TOGGLE);
    }
}
