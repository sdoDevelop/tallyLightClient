#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <Adafruit_NeoPixel.h>

// Pin configuration
#define LED_PIN_1 6
#define LED_PIN_2 7
#define LED_PIN_3 8
#define NUM_LEDS_PER_SECTION 14
#define NUM_SECTIONS 5
#define NUM_LEDS (NUM_LEDS_PER_SECTION * NUM_SECTIONS)

// Ethernet configuration
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(10, 75, 140, 5);
unsigned int localPort = 5005;

// UDP
EthernetUDP Udp;
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];

// LED strips
Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(NUM_LEDS, LED_PIN_1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(NUM_LEDS, LED_PIN_2, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip3 = Adafruit_NeoPixel(NUM_LEDS, LED_PIN_3, NEO_GRB + NEO_KHZ800);

// Default colors for each section
uint32_t sectionColors[NUM_SECTIONS] = {
  strip1.Color(255, 0, 0),    // Red
  strip1.Color(0, 255, 0),    // Green
  strip1.Color(0, 0, 255),    // Blue
  strip1.Color(128, 0, 128),  // Purple
  strip1.Color(255, 255, 255) // White
};

int parseSection(const char* sectionStr) {
  if (strncmp(sectionStr, "LED", 3) == 0) {
    return atoi(sectionStr + 3) - 1;
  }
  return -1;
}

bool parseState(const char* stateStr) {
  return (strcmp(stateStr, "On") == 0);
}

void setSectionState(Adafruit_NeoPixel& strip, int sectionIndex, bool state) {
  uint32_t color = state ? sectionColors[sectionIndex] : strip.Color(0, 0, 0);

  int start = sectionIndex * NUM_LEDS_PER_SECTION;
  int end = start + NUM_LEDS_PER_SECTION;

  for (int i = start; i < end; i++) {
    strip.setPixelColor(i, color);
  }

  strip.show();
}

void setSectionColor(int sectionIndex, uint32_t color) {
  if (sectionIndex >= 0 && sectionIndex < NUM_SECTIONS) {
    sectionColors[sectionIndex] = color;
  }
}

void processControlMessage(char* packet) {
  // Split packet by ';'
  char* sectionStr = strtok(packet, ";");
  char* stateStr = strtok(NULL, ";");

  if (sectionStr && stateStr) {
    int sectionIndex = parseSection(sectionStr);
    bool state = parseState(stateStr);

    if (sectionIndex >= 0 && sectionIndex < NUM_SECTIONS) {
      setSectionState(strip1, sectionIndex, state);
      setSectionState(strip2, sectionIndex, state);
      setSectionState(strip3, sectionIndex, state);
    }
  }
}

void processSetupMessage(char* packet) {
  // Split packet by ';'
  char* args[4];
  for (int i = 0; i < 4; i++) {
    args[i] = strtok(i == 0 ? packet : NULL, ";");
  }

  if (args[0] && strcmp(args[0], "Brightness") == 0) {
    int stripNumber = atoi(args[1]);
    int sectionNumber = atoi(args[2]);
    int brightness = atoi(args[3]);

    if (stripNumber >= 1 && stripNumber <= 3 && sectionNumber >= 1 && sectionNumber <= 5) {
      Adafruit_NeoPixel* strip;
      if (stripNumber == 1) strip = &strip1;
      else if (stripNumber == 2) strip = &strip2;
      else strip = &strip3;

      int sectionIndex = sectionNumber - 1;
      int start = sectionIndex * NUM_LEDS_PER_SECTION;
      int end = start + NUM_LEDS_PER_SECTION;

      strip->setBrightness(map(brightness, 0, 100, 0, 255));
      for (int i = start; i < end; i++) {
        strip->setPixelColor(i, sectionColors[sectionIndex]);
      }
      strip->show();
    }
  }
}

void processPacket(char* packet) {
  int delimiterCount = 0;
  for (char* p = packet; *p; ++p) {
    if (*p == ';') delimiterCount++;
  }

  if (delimiterCount == 4) {
    processControlMessage(packet);
  } else if (delimiterCount == 3) {
    processSetupMessage(packet);
  } else {
    Serial.println("Invalid packet format");
  }
}

void setup() {
  // Initialize LED strips
  strip1.begin();
  strip1.setBrightness(255); // Set default brightness to 100%
  strip1.show();

  strip2.begin();
  strip2.setBrightness(255); // Set default brightness to 100%
  strip2.show();

  strip3.begin();
  strip3.setBrightness(255); // Set default brightness to 100%
  strip3.show();

  // Start Ethernet and UDP
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);

  Serial.begin(9600);
  Serial.println("WS2812B Controller Ready");
}

void loop() {
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    packetBuffer[packetSize] = '\0';
    Serial.println(packetBuffer);

    processPacket(packetBuffer);
  }
}
