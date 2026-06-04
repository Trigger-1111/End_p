#include <Wire.h>
#include <U8g2lib.h>

// ── 버튼 / LED ───────────────────────────────────────────
#define BTN_START  34
#define BTN_STOP   35
#define BTN_MODE   16
#define LED_STATUS  2

// ── TB6612 공통 ──────────────────────────────────────────
#define STBY_PIN   15

// TB6612 #1 (액추에이터 1·2)
#define A1_IN1  25
#define A1_IN2  26
#define A1_PWM  32
#define A2_IN1  27
#define A2_IN2  14
#define A2_PWM  33

// TB6612 #2 (액추에이터 3·4)
#define A3_IN1  13
#define A3_IN2  19
#define A3_PWM   4
#define A4_IN1  18
#define A4_IN2  23
#define A4_PWM   5

// ── I2C / OLED ───────────────────────────────────────────
#define SDA_PIN 21
#define SCL_PIN 22
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R2, U8X8_PIN_NONE, SCL_PIN, SDA_PIN);

// ── 상태 ─────────────────────────────────────────────────
enum Mode     { MODE_WOK = 0, MODE_SPIN, MODE_AUTO };
enum AppState { STATE_IDLE, STATE_SELECTING, STATE_RUNNING };

volatile bool          modePressed  = false;
volatile unsigned long lastModeISR  = 0;
void IRAM_ATTR modeISR() {
    unsigned long now = millis();
    if (now - lastModeISR > 200) { modePressed = true; lastModeISR = now; }
}

AppState appState  = STATE_IDLE;
AppState prevState = STATE_IDLE;
Mode     selMode   = MODE_WOK;
Mode     runMode   = MODE_WOK;

unsigned long selectStart  = 0;
unsigned long blinkTimer   = 0;
unsigned long oledRefreshT = 0;
bool blinkVisible = true;

// ── 액추에이터 ────────────────────────────────────────────

struct ActPins { uint8_t in1, in2, pwm; };
const ActPins ACTS[4] = {
    {A1_IN1, A1_IN2, A1_PWM},
    {A2_IN1, A2_IN2, A2_PWM},
    {A3_IN1, A3_IN2, A3_PWM},
    {A4_IN1, A4_IN2, A4_PWM},
};

void actForward(int i)  { digitalWrite(ACTS[i].in1, HIGH); digitalWrite(ACTS[i].in2, LOW);  digitalWrite(ACTS[i].pwm, HIGH); }
void actBackward(int i) { digitalWrite(ACTS[i].in1, LOW);  digitalWrite(ACTS[i].in2, HIGH); digitalWrite(ACTS[i].pwm, HIGH); }
void actBrake(int i)    { digitalWrite(ACTS[i].in1, LOW);  digitalWrite(ACTS[i].in2, LOW);  digitalWrite(ACTS[i].pwm, LOW);  }

void stopAll() {
    digitalWrite(STBY_PIN, LOW);
    for (int i = 0; i < 4; i++) actBrake(i);
    digitalWrite(LED_STATUS, LOW);
}

// 테스트: 전체 액추에이터 전진·후진 1초 교대
void runTestTick() {
    static bool fwd = false;   // !fwd → true(전진) 으로 첫 동작 시작
    static unsigned long t = 0;
    if (millis() - t < 1000) return;
    t = millis();
    fwd = !fwd;
    Serial.printf("[ACT] %s | STBY=%d IN1=%d IN2=%d PWM=%d\n",
        fwd ? "FORWARD" : "BACKWARD",
        digitalRead(STBY_PIN),
        digitalRead(ACTS[0].in1), digitalRead(ACTS[0].in2), digitalRead(ACTS[0].pwm));
    for (int i = 0; i < 4; i++) {
        if (fwd) actForward(i);
        else     actBackward(i);
    }
}

// ── OLED 아이콘 ──────────────────────────────────────────

void drawIconWok() {
    u8g2.drawTriangle(14, 28,  26, 20,  26, 36);
    u8g2.drawTriangle(114, 28, 102, 20, 102, 36);
    u8g2.drawRFrame(34, 18, 60, 26, 5);
    u8g2.setFont(u8g2_font_7x13B_tr);
    u8g2.drawStr(49, 58, "WOK");
}

void drawIconSpin() {
    u8g2.drawCircle(64, 28, 18,
        U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_LOWER_LEFT);
    u8g2.drawTriangle(78, 40, 72, 47, 84, 47);
    u8g2.setFont(u8g2_font_7x13B_tr);
    u8g2.drawStr(45, 58, "SPIN");
}

void drawIconAuto() {
    u8g2.drawRFrame(28, 14, 72, 36, 4);
    u8g2.drawCircle(64, 32, 12);
    u8g2.drawDisc(64, 32, 5);
    u8g2.drawBox(50, 10, 28, 6);
    u8g2.setFont(u8g2_font_7x13B_tr);
    u8g2.drawStr(45, 58, "AUTO");
}

void showMode(Mode m) {
    u8g2.clearBuffer();
    switch (m) { case MODE_WOK: drawIconWok(); break; case MODE_SPIN: drawIconSpin(); break; case MODE_AUTO: drawIconAuto(); break; }
    u8g2.sendBuffer();
}

void showReady() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB18_tr);
    const char* msg = "READY";
    u8g2.drawStr((128 - u8g2.getStrWidth(msg)) / 2, 40, msg);
    u8g2.sendBuffer();
}

void showStop() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB18_tr);
    const char* msg = "STOP";
    u8g2.drawStr((128 - u8g2.getStrWidth(msg)) / 2, 40, msg);
    u8g2.sendBuffer();
}

// ── Setup ────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);

    pinMode(LED_STATUS, OUTPUT); digitalWrite(LED_STATUS, LOW);
    pinMode(STBY_PIN,   OUTPUT); digitalWrite(STBY_PIN,   LOW);

    pinMode(BTN_START, INPUT);       // 외부 10k 풀업 필수
    pinMode(BTN_STOP,  INPUT);
    pinMode(BTN_MODE,  INPUT_PULLUP);
    attachInterrupt(BTN_MODE, modeISR, FALLING);

    for (int i = 0; i < 4; i++) {
        pinMode(ACTS[i].in1, OUTPUT); digitalWrite(ACTS[i].in1, LOW);
        pinMode(ACTS[i].in2, OUTPUT); digitalWrite(ACTS[i].in2, LOW);
        pinMode(ACTS[i].pwm, OUTPUT); digitalWrite(ACTS[i].pwm, LOW);
    }

    Wire.begin(SDA_PIN, SCL_PIN);
    delay(100);
    bool oledOk = u8g2.begin();
    Serial.printf("OLED init: %s\n", oledOk ? "OK" : "FAIL");
    showReady();
}

// ── Loop ─────────────────────────────────────────────────

void loop() {
    bool startBtn = digitalRead(BTN_START) == LOW;
    bool stopBtn  = digitalRead(BTN_STOP)  == LOW;

    // MODE 버튼
    if (modePressed) {
        modePressed = false;
        if (appState == STATE_SELECTING) {
            selMode = (Mode)((selMode + 1) % 3);
        } else {
            prevState = appState;
            selMode   = MODE_WOK;
        }
        appState     = STATE_SELECTING;
        selectStart  = millis();
        blinkTimer   = millis();
        blinkVisible = true;
        showMode(selMode);
    }

    // ── SELECTING ──
    if (appState == STATE_SELECTING) {
        if (millis() - blinkTimer >= 1000) {
            blinkVisible = !blinkVisible;
            blinkTimer   = millis();
            if (blinkVisible) showMode(selMode);
            else { u8g2.clearBuffer(); u8g2.sendBuffer(); }
        }

        if (startBtn) {
            runMode      = selMode;
            appState     = STATE_RUNNING;
            oledRefreshT = millis();
            stopAll();
            digitalWrite(STBY_PIN,   HIGH);
            digitalWrite(LED_STATUS, HIGH);
            showMode(runMode);
            Serial.printf("[START] mode=%d STBY=%d\n", runMode, digitalRead(STBY_PIN));
            delay(300);
            return;
        }

        if (stopBtn) {
            appState = (prevState == STATE_RUNNING) ? STATE_RUNNING : STATE_IDLE;
            if (appState == STATE_RUNNING) { showMode(runMode); }
            else { stopAll(); showReady(); }
            delay(300);
            return;
        }

        if (millis() - selectStart >= 10000) {
            appState = (prevState == STATE_RUNNING) ? STATE_RUNNING : STATE_IDLE;
            if (appState == STATE_RUNNING) { oledRefreshT = millis(); showMode(runMode); }
            else showReady();
        }
    }

    // ── RUNNING ──
    if (appState == STATE_RUNNING) {

        // OLED 5초마다 갱신 (액추에이터 전원 노이즈로 리셋될 경우 대비)
        if (millis() - oledRefreshT >= 5000) {
            showMode(runMode);
            oledRefreshT = millis();
        }

        if (stopBtn) {
            stopAll();
            appState = STATE_IDLE;
            showStop();
            delay(1000);
            showReady();
            delay(300);
            return;
        }

        // 테스트: 모든 액추에이터 전진·후진 반복
        // TODO: 검증 후 아래 switch로 교체
        runTestTick();
        /*
        switch (runMode) {
            case MODE_WOK:  runWokTick();  break;
            case MODE_SPIN: runSpinTick(); break;
            case MODE_AUTO: runAutoTick(); break;
        }
        */
    }
}
