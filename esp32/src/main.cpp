#include <Arduino.h>

// ── Pins ─────────────────────────────────────────────────────
constexpr uint8_t BTN_PIN     = 0;   // BOOT button, INPUT_PULLUP (active LOW)
constexpr uint8_t LED_PIN     = 2;   // зовнішній світлодіод
constexpr uint8_t UART_TX_PIN = 17;  // -> STM32 PA10 (RX1)
constexpr uint8_t UART_RX_PIN = 18;  // <- STM32 PA9  (TX1)

// ── UART protocol ────────────────────────────────────────────
constexpr uint32_t UART_BAUD  = 115200;
constexpr char     CMD_TOGGLE = 'T';  // "перемкни свій світлодіод"

constexpr uint32_t DEBOUNCE_MS = 50;

// ── State ────────────────────────────────────────────────────
bool     ledOn        = false;
bool     btnStable    = HIGH;
bool     btnLastRaw   = HIGH;
uint32_t btnChangedAt = 0;

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
    Serial.begin(115200);                                            // лог у USB
    Serial1.begin(UART_BAUD, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);  // лінк зі STM32

    pinMode(BTN_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    Serial.println("=== ESP32 <-> STM32 UART ===");
}

void loop() {
    // 1. Кнопка ESP32 -> світлодіод STM32
    if (buttonPressed()) {
        Serial1.write(CMD_TOGGLE);
        Serial.println("BTN -> sent 'T' to STM32");
    }

    // 2. Кнопка STM32 -> світлодіод ESP32
    while (Serial1.available()) {
        char c = Serial1.read();
        if (c == CMD_TOGGLE) {
            ledOn = !ledOn;
            digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
            Serial.printf("RX 'T' from STM32 -> LED %s\n", ledOn ? "ON" : "OFF");
        }
    }
}
