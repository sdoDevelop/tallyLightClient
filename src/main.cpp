#include <Adafruit_NeoPixel.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

// Configuration
#define NUM_STRIPS 5           // Number of LED strips
#define LED_COUNT 14           // Number of LEDs per strip
#define LED_PINS {6, 7, 8, 9, 10} // Pins connected to the WS2812B strips

// Static IP, Gateway, and Subnet Configuration
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  // MAC address
IPAddress ip(10, 75, 140, 5);  // Static IP address
IPAddress gateway(10, 75, 140, 1);  // Gateway IP address
IPAddress subnet(255, 255, 255, 0);  // Subnet mask

// Define an array of colors to cycle through (Red, Green, Blue, Purple, White)
uint32_t colorCycle[] = {
  0xFF0000, // Red
  0x00FF00, // Green
  0x0000FF, // Blue
  0x800080, // Purple
  0xFFFFFF  // White
};

int currentColorIndex = 0; // To track the current color in the cycle

// Create an array of NeoPixel objects
Adafruit_NeoPixel strips[NUM_STRIPS] = {
  Adafruit_NeoPixel(LED_COUNT, 6, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(LED_COUNT, 7, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(LED_COUNT, 8, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(LED_COUNT, 9, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(LED_COUNT, 10, NEO_GRB + NEO_KHZ800)
};

// Ethernet and UDP objects
EthernetUDP Udp;
unsigned int localPort = 5005;  // Port to listen on

// Initialize all strips
void initializeStrips() {
  for (int i = 0; i < NUM_STRIPS; i++) {
    strips[i].begin();
    strips[i].show(); // Ensure all LEDs are off initially
  }
}

// Function to turn all strips on with a specified color
void turnAllStripsOn(uint32_t color) {
  for (int i = 0; i < NUM_STRIPS; i++) {
    for (int j = 0; j < LED_COUNT; j++) {
      strips[i].setPixelColor(j, color);
    }
    strips[i].show();
  }
}

// Function to change the color to the next in the cycle
void changeColor() {
  currentColorIndex = (currentColorIndex + 1) % (sizeof(colorCycle) / sizeof(colorCycle[0]));  // Move to the next color
  turnAllStripsOn(colorCycle[currentColorIndex]); // Apply the new color to all strips
}

// Setup function
void setup() {
  // Start Serial Monitor for debugging
  Serial.begin(9600);
  
  // Initialize Ethernet with static IP, subnet, and gateway
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to initialize Ethernet.");
    while (true);  // Stay here forever if Ethernet initialization fails
  }
  
  // Set static IP, gateway, and subnet
  Ethernet.begin(mac, ip, subnet, gateway);
  
  // Print the IP address, gateway, and subnet to Serial
  Serial.print("IP Address: ");
  Serial.println(Ethernet.localIP());
  Serial.print("Gateway: ");
  Serial.println(gateway);
  Serial.print("Subnet Mask: ");
  Serial.println(subnet);

  // Start listening for UDP packets
  Udp.begin(localPort);
  
  // Initialize the NeoPixel strips
  initializeStrips();
  
  // Set the initial color
  turnAllStripsOn(colorCycle[currentColorIndex]);
}

// Loop function
void loop() {
  int packetSize = Udp.parsePacket(); // Check for incoming UDP packets

  if (packetSize) {
    // If a packet is received, change the color of the strips
    Serial.println("Packet received. Changing color.");
    changeColor();  // Change color to the next one in the cycle
  }
}
