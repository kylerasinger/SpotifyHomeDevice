#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <FS.h>
#include <SPIFFS.h>
#include <SpotifyArduino.h>
#include <ArduinoJson.h>
#include <JPEGDEC.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

#include "Config.h"
#include "Helper.h"
#include "Display.h"
#include "Touchscreen.h"
#include "Spotify.h"
#include "Connections.h"
#include "SPIFFS_Methods.h"
#include "SerialPrint.h"

#define FORMAT_SPIFFS_IF_FAILED true

TFT_eSPI tft = TFT_eSPI();
JPEGDEC jpeg;

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

WiFiClientSecure client;
SpotifyArduino spotify(client, clientId, clientSecret, SPOTIFY_REFRESH_TOKEN);

int timeOffset = -4 * 3600;
String currentTime;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", timeOffset, 60000);

// Touchscreen coordinates: (x, y) and pressure (z)
uint16_t x, y, z;
const uint16_t CENTERX = SCREEN_WIDTH / 2;
const uint16_t CENTERY = SCREEN_HEIGHT / 2;

bool playing = true;

unsigned long delayBetweenRequests = 30000; // Time between spotify requests (30 seconds)
unsigned long delayBetweenClockUpdates = 60000; // Time betwen clock updates (1 minute)
unsigned long delayBetweenProgressUpdates = 3000; // Time between track progress updates (3 seconds)
unsigned long requestDueTime;
unsigned long clockDueTime;
unsigned long progressDueTime;

long trackDurationMs = 100;
long trackProgressMs = 1;

String previousTrack = "empty";

int JPEGDraw(JPEGDRAW *pDraw)
{
  // Stop further decoding as image is running off bottom of screen
  if (pDraw->y >= tft.height()) {
    Serial.println("Image running off the screen");
    return 0;
  }

  // Serial.printf("Drawing image at x: %d, y: %d, width: %d, height: %d\n", pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight);
  tft.pushImage(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pDraw->pPixels);
  return 1;
}

void drawImageFromFile(const String filename){
  unsigned long lTime = millis();
  lTime = millis();

  File file = SPIFFS.open(filename, FILE_READ);
  if (!file) {
    Serial.println("Failed to open image file for verification");
    return;
  }

  jpeg.open(file, JPEGDraw);
  jpeg.setPixelType(1);
  uint16_t imagePositionX = 15;
  uint16_t decodeStatus = jpeg.decode(imagePositionX, 15, JPEG_AUTO_ROTATE);
  jpeg.close();

  Serial.print("Time taken to decode and display Image (ms): ");
  Serial.println(millis() - lTime);
}

void downloadImage(const char* url, String filename) {
  drawWait(CENTERX, CENTERY, tft, true);

  Serial.println("Starting to download image: ");
  Serial.println(url);

  HTTPClient http; // !! Would be better to use WifiClientSecure, but I am unable to set up TLS
  http.begin(url, root_ca);

  uint16_t httpCode = http.GET();
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {

      // Open file for writing
      File file = SPIFFS.open(filename, FILE_WRITE);
      if (!file) {
        Serial.println("Failed to open file for writing");
        return;
      }

      // Get the payload (image data)
      WiFiClient *stream = http.getStreamPtr();
      size_t written = stream->available();
      uint8_t buff[512] = { 0 };
      while (stream->available() > 0) {
        size_t size = stream->available();
        size_t len = stream->readBytes(buff, (size > sizeof(buff) ? sizeof(buff) : size));
        file.write(buff, len);
      }
      file.close();
      
      Serial.println("Image downloaded successfully");

      drawImageFromFile(filename);
    }
  } else {
    Serial.printf("Failed to download image, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end(); 

  drawWait(CENTERX, CENTERY, tft, false);
}

void handleCurrentlyPlaying(CurrentlyPlaying currentlyPlaying)
{ 
  playing = currentlyPlaying.isPlaying;
  if(!playing){
    drawPlay(CENTERX, CENTERY, tft, true);
  }else{
    drawPause(CENTERX, CENTERY, tft, true);
  }

  String filename = "/" + cleanFilename(String(currentlyPlaying.trackName));
  Serial.println("Previously playing song is");
  Serial.println(previousTrack);

  Serial.println("Currently playing song is");
  Serial.println(filename);

  if (filename != previousTrack) { //new track
    Serial.println("New track, deleting old art, downloading new. ");

    SPIFFS.remove(previousTrack);
    downloadImage(currentlyPlaying.albumImages[2].url, filename);
  }else{
    Serial.println("Old track, skipping art download and draw. ");
  }

  previousTrack = filename;

  String tempText = String(currentlyPlaying.trackName);
  uint16_t featIndex = tempText.indexOf("(feat.");
  if (featIndex != -1) {
      tempText = tempText.substring(0, featIndex);
      tempText.trim();
  }
  featIndex = tempText.indexOf("(with");
  if (featIndex != -1) {
      Serial.println("Found a (with");
      tempText = tempText.substring(0, featIndex);
      tempText.trim();
  }

  tft.fillRect(CENTERX - 75, CENTERY - 105, 240, 60, TFT_BLACK);
  tft.drawString(tempText, CENTERX - 65, CENTERY - 90, FONT_SIZE);
  tempText = String(currentlyPlaying.artists[0].artistName);
  tft.drawString(tempText, CENTERX - 65, CENTERY - 70, FONT_SIZE);

  trackProgressMs = currentlyPlaying.progressMs;
  trackDurationMs = currentlyPlaying.durationMs;

  drawProgress(CENTERX, CENTERY, tft, trackProgressMs, trackDurationMs); //updateProgress
}

void handleNextSong(){
  Serial.println("Next song\n");
  drawNext(CENTERX, CENTERY, tft, false);
  delay(300);
  drawNext(CENTERX, CENTERY, tft, true);

  spotify.nextTrack();

  uint16_t status = spotify.getCurrentlyPlaying(handleCurrentlyPlaying, SPOTIFY_MARKET);
}

void handlePrevSong(){
  //prevent auto request for 30 seconds
  requestDueTime = millis() + delayBetweenRequests;

  Serial.println("Prev song\n");
  drawPrev(CENTERX, CENTERY, tft, false);
  delay(300);
  drawPrev(CENTERX, CENTERY, tft, true);

  spotify.previousTrack();

  uint16_t status = spotify.getCurrentlyPlaying(handleCurrentlyPlaying, SPOTIFY_MARKET);
}

void handlePause(){
  //prevent auto request for 30 seconds
  requestDueTime = millis() + delayBetweenRequests;

  Serial.println("Pausing song\n");
  drawPlay(CENTERX, CENTERY, tft, false);
  delay(300);
  drawPlay(CENTERX, CENTERY, tft, true);

  spotify.pause();
}

void handlePlay(){
  //prevent auto request for 30 seconds
  requestDueTime = millis() + delayBetweenRequests;

  Serial.println("Playing song\n");
  drawPause(CENTERX, CENTERY, tft, false);
  delay(300);
  drawPause(CENTERX, CENTERY, tft, true);

  spotify.play();
}

void setup() {
  Serial.begin(115200);

  setupConnections(ssid, password);  
	setupSpotify(client, spotify);

  // Mount SPIFFS
  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
    Serial.println("An error has occurred while mounting SPIFFS");
    return;
  }

  initTouchscreen(touchscreen, touchscreenSPI);
  initDisplay(tft);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);

  tft.fillScreen(TFT_BLACK);
  tft.drawRect(15, 15, 64, 64, TFT_WHITE);

  timeClient.begin();
  
  //Draw buttons
  drawButtons(CENTERX, CENTERY, tft);
}

void loop() {
  // Clock display, accurate within a minute
  if(millis() > clockDueTime){
    timeClient.update();
    currentTime = timeClient.getFormattedTime();
    currentTime = currentTime.substring(0,5);
    tft.fillRect(CENTERX - 50, CENTERY - 30, 100, 20, TFT_BLACK);
    tft.drawCentreString(currentTime, CENTERX, CENTERY - 30, 4);

    clockDueTime = millis() + delayBetweenClockUpdates;
  }

  // Query of spotify
  if(millis() > requestDueTime){
    uint16_t status = spotify.getCurrentlyPlaying(handleCurrentlyPlaying, SPOTIFY_MARKET);

    if (status == 200)
    {
      Serial.println("Successfully got currently playing");
    }
    else if (status == 204)
    {
      Serial.println("Doesn't seem to be anything playing");
    }
    else
    {
      Serial.print("Error, HTTP Code: ");
      Serial.println(status);
    }

    requestDueTime = millis() + delayBetweenRequests;
  }

  // Progress bar
  if(millis() > progressDueTime && playing){
    Serial.println("Updating progress");
    trackProgressMs += delayBetweenProgressUpdates;    
    updateProgress(CENTERX, CENTERY, tft, trackProgressMs, trackDurationMs);

    if (trackProgressMs > trackDurationMs) {
      trackProgressMs = trackDurationMs; 
      tft.fillCircle(CENTERX + 144 - 3, CENTERY + 90 + 6, 5, TFT_WHITE);
      uint16_t status = spotify.getCurrentlyPlaying(handleCurrentlyPlaying, SPOTIFY_MARKET);
    }

    progressDueTime = millis() + delayBetweenProgressUpdates;
  }

  // Button logic
  if (touchscreen.tirqTouched() && touchscreen.touched()) {
    getTouchCoordinates(touchscreen, x, y ,z);


    if(40 < x && x < 110 && 140 < y && y < 200){
      handlePrevSong();
    }else if(110 < x && x < 210 && 140 < y && y < 200){
      if(playing){
        handlePause();
        playing = false;
      }else{
        handlePlay();
        playing = true;
      }
    }else if(210 < x && x < 280 && 140 < y && y < 200){
      handleNextSong();
    }

    delay(300);
  }
}