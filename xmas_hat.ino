// this is for My Xmas HAT lights !!
//
#include <Adafruit_NeoPixel.h>

#include <bluefruit.h>

#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// define analog 1 as the pin to drive the neopixel
#define PIN 3

//using a strand for xmas hat project
// version in hat of 36 pixels
#define NUMPIXELS 36

// create pixels.
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// BLE Service
BLEDfu  bledfu;  // OTA DFU service
BLEDis  bledis;  // device information
BLEUart bleuart; // uart over ble
BLEBas  blebas;  // battery

// Current colour
uint32_t cur_colour = pixels.Color(255, 0, 0);


/* Bluefruit utilities */

// callback invoked when central connects
void connect_callback(uint16_t conn_handle) {
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));

  Serial.print("Connected to ");
  Serial.println(central_name);
}

/**
 * Callback invoked when a connection is dropped
 * @param conn_handle connection where this event happens
 * @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
 */
void disconnect_callback(uint16_t conn_handle, uint8_t reason) {
  (void) conn_handle;
  (void) reason;

  Serial.println();
  Serial.print("Disconnected, reason = 0x"); Serial.println(reason, HEX);
}

void start_adv(void) {
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  // Include bleuart 128-bit uuid
  Bluefruit.Advertising.addService(bleuart);

  // Secondary Scan Response packet (optional)
  // Since there is no room for 'Name' in Advertising packet
  Bluefruit.ScanResponse.addName();
  
  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);   // for nrf52840 with native usb

  // begin the pixels
  pixels.begin();
  // clear pixels if required
  pixels.clear();
  // show the pixels are working at start-up
  for (int i = 0; i < pixels.numPixels(); i++) {
    // set pixels in a row to show green as start-up sequence
    pixels.setPixelColor(i, pixels.Color(0, 255, 0));
    pixels.show();
    delay(25);
  }
  for (int i = 0; i < NUMPIXELS ; i++) {
    // set pixels in a row to show green as start-up sequence
    pixels.setPixelColor(i, pixels.Color(0, 255, 0));
    pixels.show();
    delay(25);
  }
  pixels.clear();
  pixels.show();
  // Setup the BLE LED to be enabled on CONNECT
  // Note: This is actually the default behaviour, but provided
  // here in case you want to control this LED manually via PIN 19
  Bluefruit.autoConnLed(true);

  // Config the peripheral connection with maximum bandwidth 
  // more SRAM required by SoftDevice
  // Note: All config***() function must be called before begin()
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);

  Bluefruit.begin();
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values
  Bluefruit.setName("XMAS HAT V1");
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  // To be consistent OTA DFU should be added first if it exists
  bledfu.begin();

  // Configure and Start Device Information Service
  bledis.setManufacturer("Adafruit Industries");
  bledis.setModel("Bluefruit Feather52");
  bledis.begin();

  // Configure and Start BLE Uart Service
  bleuart.begin();

  // Start BLE Battery Service
  blebas.begin();
  blebas.write(100);

  // Set up and start advertising
  start_adv();
}

void loop() {
  while (bleuart.available()) {
    uint8_t ch = (uint8_t) bleuart.read();
    
    switch (ch) {
      // Set the current colour
      case 's':
      {
        // Wait for next 3 values on serial - they are RGB
        while (!bleuart.available()) {}
        uint8_t r = (uint8_t) bleuart.read();
  
        while (!bleuart.available()) {}
        uint8_t g = (uint8_t) bleuart.read();
  
        while (!bleuart.available()) {}
        uint8_t b = (uint8_t) bleuart.read();
  
        cur_colour = pixels.Color(r, g, b);
        break;
      }
      
      // Fill with solid colour
      case 'f':
        solid_colour(cur_colour);
        break;

      // Rainbow party effect
      case 'p':
        rainbow(3);
        break;

      // Chase/Run effect (current colour or rainbow)
      case 'r':
      {
        // Wait for next value on serial, 1 for rainbow, 0 for not
        while (!bleuart.available()) {}
        uint8_t rainbow = (uint8_t) bleuart.read();
        theatreChase(cur_colour, 50, rainbow);
        break;
      }

      // Trail effect
      case 't':
      {
        // Wait for next value on serial, it is trail size
        while (!bleuart.available()) {}
        uint8_t size = (uint8_t) bleuart.read();
        trail(cur_colour, size);
        break;
      }

      // Clear the hat
      case 'c':
        pixels.clear();
        pixels.show();
        break;

      default: // Do nothing
        break;
    }
  }
}


/* Utility/Effect functions */

void solid_colour(uint32_t colour) {
  for(int i = 0; i < pixels.numPixels(); i++) {
    pixels.setPixelColor(i, colour);
  }
  pixels.show();
}

void trail(uint32_t colour, int trail_size) {
  bool first = true;
  int i = 0;
  while (true) {
    if (bleuart.available()) return;

    pixels.setPixelColor(i, colour);  
    if (i >= trail_size || !first) {
      pixels.setPixelColor(i, colour);
      int turn_off = i - trail_size < 0 ? pixels.numPixels() + i - trail_size : i - trail_size;
      pixels.setPixelColor(turn_off, pixels.Color(0, 0, 0));
    }

    pixels.show();
    delay(25);

    i++;
    if (i >= pixels.numPixels()) {
      first = false;
      i = 0;  
    }
  }  
}

void theatreChase(uint32_t colour, int speed_delay, bool rainbow) {
  while (true) {
    for (int j = 0; j < 256; j++) {     //cycle all 256 colours in the wheel
      for (int q = 0; q < 3; q++) {
        if (bleuart.available()) return;
        
        for (int i = 0; i < pixels.numPixels(); i = i + 3) {
          uint32_t c;
          if (rainbow) {
            c = wheel(i + j % 255);  
          } else {
            c = colour;  
          }
          pixels.setPixelColor(i + q, c);    //turn every third pixel on
        }
        pixels.show();
       
        delay(speed_delay);
       
        for (int i=0; i < pixels.numPixels(); i=i+3) {
          pixels.setPixelColor(i+q, pixels.Color(0, 0, 0));        //turn every third pixel off
        }
      }
    }
  }
}
// Helper for theatreChase
uint32_t wheel(int wheel_pos) {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  if (wheel_pos < 85) {
   r = wheel_pos * 3;
   g = 255 - wheel_pos * 3;
   b = 0;
  } else if (wheel_pos < 170) {
   wheel_pos -= 85;
   r = 255 - wheel_pos * 3;
   g = 0;
   b = wheel_pos * 3;
  } else {
   wheel_pos -= 170;
   r = 0;
   g = wheel_pos * 3;
   b = 255 - wheel_pos * 3;
  }

  return pixels.Color(r, g, b);
}

void rainbow(int wait) {
  int fade_val = 0, fade_max = 100;

  // Hue of first pixel runs 'rainbowLoops' complete loops through the color
  // wheel. Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to rainbowLoops*65536, using steps of 256 so we
  // advance around the wheel at a decent clip.
  uint32_t first_pixel_hue = 0;
  while(true) {

    if (bleuart.available()) {
      return;
    }

    for(int i = 0; i < pixels.numPixels(); i++) { // For each pixel in strip...

      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      uint32_t pixel_hue = first_pixel_hue + (i * 65536L / pixels.numPixels());

      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the three-argument variant, though the
      // second value (saturation) is a constant 255.
      pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(pixel_hue, 255,
        255 * fade_val / fade_max)));
    }

    pixels.show();
    delay(wait);

    if(first_pixel_hue < 65536) {                              // First loop,
      if(fade_val < fade_max) fade_val++;                       // fade in
    } else {
      fade_val = fade_max; // Interim loop, make sure fade is at max
    }

    first_pixel_hue += 256;
  }
}
