#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <Adafruit_NeoPixel.h>

// Pin configuration
#define LED_PIN 6
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

// LED strip
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Default colors for each section
uint32_t sectionColors[NUM_SECTIONS] = {
  strip.Color(255, 0, 0),    // Red
  strip.Color(0, 255, 0),    // Green
  strip.Color(0, 0, 255),    // Blue
  strip.Color(128, 0, 128),  // Purple
  strip.Color(255, 255, 255) // White
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

void setSectionState(int sectionIndex, bool state) {
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

void processPacket(char* packet) {
  // Split packet by ';'
  char* sectionStr = strtok(packet, ";");
  char* stateStr = strtok(NULL, ";");

  if (sectionStr && stateStr) {
    int sectionIndex = parseSection(sectionStr);
    bool state = parseState(stateStr);

    if (sectionIndex >= 0 && sectionIndex < NUM_SECTIONS) {
      setSectionState(sectionIndex, state);
    }
  }
}

void setup() {
  // Initialize LED strip
  strip.begin();
  strip.show();

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
