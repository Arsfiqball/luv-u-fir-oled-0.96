#include <SPI.h>
#include <Wire.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <cstdlib>

#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 64     // OLED display height, in pixels
#define REFRESH_MS 20        // Game screen refresh interval (ms)
#define PLAYER_WIDTH 12      // Girl image width (checkout player_bmp)
#define PLAYER_HEIGHT 16     // Girl image height (checkout player_bmp)
#define PLAYER_SPEED 4       // Girl movement speed in pixel per refresh interval
#define CPU_WIDTH 12         // Boy image width (checkout cpu_bmp)
#define CPU_HEIGHT 18        // Boy image height (checkout cpu_bmp)
#define CPU_SPEED 1          // Boy movement speed in pixel per refresh interval
#define NUM_TEXT 24          // Number of random text available in setText(i)
#define PIN_PUSH_RIGHT D6    // Digital pin for right side push button
#define PIN_PUSH_LEFT D7     // Digital pin for left side push button
#define I2C_ADDR_OLED 0x3C   // I2C address (SDA & SCL) for OLED 128x64 Display
#define OLED_RESET -1        // OLED Reset Pin

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

static const bool player_bmp[] = {
  0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
  0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0,
  0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0,
  0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
  1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1,
  1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1,
  1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
  0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
  0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0,
  0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

static const bool cpu_bmp[] = {
  1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0,
  1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1,
  1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 1,
  1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1,
  0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0,
  0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0,
  0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
  0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 0, 0,
  0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0,
};

uint16_t state_playerPosX = 10; // initially on the left
uint16_t state_cpuPosX = SCREEN_WIDTH - CPU_WIDTH - 10; // initially on the right
bool state_playerFlipped = true; // facing right 
bool state_cpuFlipped = false; // facing left
bool state_changeText = false; // initially no text changing state
unsigned int state_randomIndex = 1000; // initally using random number outside NUM_TEXT

void drawPlayer (uint16_t playerX, bool flipped);
void drawCPU (uint16_t posX, bool flipped);
void setText (unsigned int i);

void setup() {
  Serial.begin(9600);

  pinMode(PIN_PUSH_RIGHT, INPUT);
  pinMode(PIN_PUSH_LEFT, INPUT);

  if(!display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDR_OLED)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  display.display();
  delay(2000);
}

void loop() {
  bool pushRight = digitalRead(PIN_PUSH_RIGHT);
  bool pushLeft = digitalRead(PIN_PUSH_LEFT);

  display.clearDisplay();
  display.drawLine(0, SCREEN_HEIGHT - 1, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, SSD1306_WHITE);
  display.setCursor(0, 0);
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(true);

  if (pushRight && state_playerPosX < (SCREEN_WIDTH - PLAYER_WIDTH)) {
    state_playerPosX += state_playerPosX < ((SCREEN_WIDTH - PLAYER_WIDTH) - PLAYER_SPEED) ? PLAYER_SPEED : 0;
    state_playerFlipped = true;
  } else if (pushLeft && state_playerPosX > 0) {
    state_playerPosX -= state_playerPosX > PLAYER_SPEED ? PLAYER_SPEED : 0;
    state_playerFlipped = false;
  } else {
    if (state_playerPosX > state_cpuPosX) {
      state_playerFlipped = false;
    } else {
      state_playerFlipped = true;
    }
  }

  if (state_playerPosX > (state_cpuPosX + PLAYER_WIDTH + 4)) {
    state_cpuPosX += CPU_SPEED;
    state_cpuFlipped = true;
    state_changeText = true;
  } else if (state_playerPosX < state_cpuPosX - PLAYER_WIDTH - 4) {
    state_changeText = true;
    state_cpuPosX -= CPU_SPEED;
    state_cpuFlipped = false;
  } else {
    if (state_playerPosX > state_cpuPosX) {
      state_cpuFlipped = true;
    } else {
      state_cpuFlipped = false;
    }

    if (state_changeText) {
      state_changeText = false;
      state_randomIndex = rand() % NUM_TEXT;
    }
  }

  setText(state_randomIndex);
  drawPlayer(state_playerPosX, state_playerFlipped);
  drawCPU(state_cpuPosX, state_cpuFlipped);

  display.display();
  delay(REFRESH_MS);
}

void drawPlayer (uint16_t playerX, bool flipped) {
  uint16_t playerY = 46;

  for (uint16_t y = 0; y < PLAYER_HEIGHT; y++) {
    for (uint16_t x = 0; x < PLAYER_WIDTH; x++) {
      if (player_bmp[y * PLAYER_WIDTH + x] == 1) {
        display.drawPixel(playerX + (flipped ? PLAYER_WIDTH - x : x), playerY + y, SSD1306_WHITE);
      }
    }
  }
}

void drawCPU (uint16_t posX, bool flipped) {
  uint16_t posY = 44;

  for (uint16_t y = 0; y < CPU_HEIGHT; y++) {
    for (uint16_t x = 0; x < CPU_WIDTH; x++) {
      if (cpu_bmp[y * PLAYER_WIDTH + x] == 1) {
        display.drawPixel(posX + (flipped ? PLAYER_WIDTH - x : x), posY + y, SSD1306_WHITE);
      }
    }
  }
}

void setText (unsigned int i) {
  switch (i) {
    case 0:
      display.println(F("Hmm... Fir, nyari kemanapun juga ga ada yang semanis kamu."));
      break;
    case 1:
      display.println(F("Miss u fir..."));
      break;
    case 2:
      display.println(F("Fir, ga capek apah di dunia nyata ketemu, di mimpi ketemu juga..."));
      break;
    case 3:
      display.println(F("Hai"));
      break;
    case 4:
      display.println(F("Fir..."));
      break;
    case 5:
      display.println(F("Love u"));
      break;
    case 6:
      display.println(F("Weh, yang dulu baper mulu..."));
      break;
    case 7:
      display.println(F("Heh, yang sering ngechat garagara kangen..."));
      break;
    case 8:
      display.println(F("Hati aku udah 100% buat kamu"));
      break;
    case 9:
      display.println(F("Silverqueen sih fir, penasaran kenapa kalo dikasih kamu malah lebih manis."));
      break;
    case 10:
      display.println(F("Fir, kalo ngambil hati aku, bilang dulu sih..."));
      break;
    case 11:
      display.println(F("Heh, yang kangen digambarin. Nih aku bikinin game sekalian."));
      break;
    case 12:
      display.println(F("Sayang..."));
      break;
    case 13:
      display.println(F("Beb..."));
      break;
    case 14:
      display.println(F("Nih game nya lucu amat sih..."));
      break;
    case 15:
      display.println(F("Heh..."));
      break;
    case 16:
      display.println(F("Kangen..."));
      break;
    case 17:
      display.println(F("Cubit nih..."));
      break;
    case 18:
      display.println(F("Tanggal 31 Juli sih fir, ucapin selamat ulang tahun"));
      break;
    case 19:
      display.println(F("Udah makan belum, makan biar ga sakit"));
      break;
    case 20:
      display.println(F("Fira..."));
      break;
    case 21:
      display.println(F("Fir, ajarin lagi sih cara ngegombalin kamu..."));
      break;
    case 22:
      display.println(F("Mau kamu, Eh"));
      break;
    case 23:
      display.println(F("Nih teksnya malumaluin, Tapi biarinlah ama fira mah ga usah jaim..."));
      break;
    case 24:
      display.println(F(":D"));
      break;
  }
}
