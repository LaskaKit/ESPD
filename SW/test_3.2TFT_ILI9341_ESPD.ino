// ESPD 3,2"

// připojení knihoven
#include "SPI.h"
#include "Adafruit_ILI9341.h"
#include "XPT2046_Touchscreen.h"
#include "FS.h"
#include "SD.h"
#include <WiFi.h>

// nastavení propojovacích pinů
#define MOSI  13
#define MISO  12
#define SCK   14
#define TFT_CS    15
#define TFT_DC    32
#define TFT_RST   5
#define TOUCH_CS  27
#define SD_CS 4
#define BL 33

bool sd_card;

// inicializace LCD displeje z knihovny
Adafruit_ILI9341 displej = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
//inicializace řadiče dotykové vrstvy z knihovny
XPT2046_Touchscreen dotyk(TOUCH_CS);

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.path(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

void removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
}

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

void testFileIO(fs::FS &fs, const char * path){
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if(file){
        len = file.size();
        size_t flen = len;
        start = millis();
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    } else {
        Serial.println("Failed to open file for reading");
    }


    file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for(i=0; i<2048; i++){
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}

void setup() {
  pinMode(BL, OUTPUT);  
  // spuštění SPI sběrnice
  SPI.begin(SCK, MISO, MOSI);

  Serial.begin(115200);
  
  // zahájení komunikace s displejem a dotykovou vrstvou
  displej.begin();
  dotyk.begin();
  // pro otočení displeje stačí změnit číslo
  // v závorce v rozmezí 0-3
  displej.setRotation(0);
  // vyplnění displeje černou barvou
  displej.fillScreen(ILI9341_BLACK);
  // spuštění podsvícení
  digitalWrite(BL, HIGH);

  WiFi.begin("laskalab", "laskaLAB754125");
  Serial.print("Connecting to WiFi");
 
  displej.setCursor(0, 0);
  displej.setTextColor(ILI9341_WHITE);
  displej.setTextSize(2);
  displej.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    displej.print(".");
    delay(1000);
  }
  Serial.println();
  displej.println();
  Serial.println(WiFi.localIP());
  displej.println(WiFi.localIP());

  delay(2000);

  displej.fillScreen(ILI9341_BLACK);
  // nastavení kurzoru na souřadnice x, y
  displej.setCursor(0, 0);
  displej.setTextColor(ILI9341_WHITE);
  // velikost textu lze nastavit změnou čísla v závorce
  displej.setTextSize(1);
  // funkce pro výpis textu na displej,
  // print tiskne na řádek stále za sebou,
  // println na konci textu přeskočí na nový řádek
  displej.println("Dotykovy displej");
  displej.setTextSize(3);
  displej.println("ESPD 3.2\"");
  displej.setTextSize(2);
  displej.println("LaskaKit.cz");

  if(!SD.begin(SD_CS)){
    Serial.println("Card Mount Failed");
    sd_card = false;
  }else{
    sd_card = true;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
      Serial.println("No SD card attached");
      sd_card = false;
  }else{
    sd_card = true;
  }

  if (sd_card){
    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }
  
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
  
    listDir(SD, "/", 0);
    /*
    createDir(SD, "/mydir");
    listDir(SD, "/", 0);
    removeDir(SD, "/mydir");
    listDir(SD, "/", 2);
    writeFile(SD, "/hello.txt", "Hello ");
    appendFile(SD, "/hello.txt", "World!\n");
    readFile(SD, "/hello.txt");
    deleteFile(SD, "/foo.txt");
    renameFile(SD, "/hello.txt", "/foo.txt");
    readFile(SD, "/foo.txt");
    testFileIO(SD, "/test.txt");
    */
    Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
  }
}

void loop() {
  // pokud je detekován dotyk na displeji,
  // proveď následující
  if (dotyk.touched()) {
    // načti do proměnné bod souřadnice dotyku
    TS_Point bod = dotyk.getPoint();
    // funkce pro vykreslení plného obdélníku,
    // zadání (výchozí bod x, výchozí bod y,
    // velikost hrany na ose x, velikost hrany na ose y,
    // barva obdélníku) 
    displej.fillRect(115, 100, 100, 50, ILI9341_BLACK);
    // funkce pro vykreslení obrysu obdélníku,
    // zadání (výchozí bod x, výchozí bod y,
    // velikost hrany na ose x, velikost hrany na ose y,
    // barva obsysu obdélníku)
    displej.drawRect(20, 200, 100, 100, ILI9341_RED);
    // funkce pro vykreslení vodorovné čáry,
    // zadání (výchozí bod x, výchozí bod y,
    // délka čáry, barva čáry)
    displej.drawFastHLine(10, 90, 220, ILI9341_BLUE);
    // funkce pro vykreslení svislé čáry,
    // zadání (výchozí bod x, výchozí bod y,
    // délka čáry, barva čáry)
    displej.drawFastVLine(110, 90, 100, ILI9341_GREEN);
    displej.setCursor(0, 100);
    displej.setTextColor(ILI9341_WHITE);
    displej.setTextSize(2);
    // vypsání informací o souřadnicích posledního dotyku
    // včetně tlaku dotyku - tlak není přiliš přesný
    displej.print("   Tlak = ");
    displej.print(bod.z);
    displej.println(",");
    displej.print("Sour. x = ");
    displej.print(bod.x);
    displej.println(",");
    displej.print("Sour. y = ");
    displej.print(bod.y);
    displej.println();

    Serial.print("   Tlak = ");
    Serial.print(bod.z);
    Serial.println(",");
    Serial.print("Sour. x = ");
    Serial.print(bod.x);
    Serial.println(",");
    Serial.print("Sour. y = ");
    Serial.print(bod.y);
    Serial.println();
    Serial.println();
    
    // pokud je detekován stisk uvnitř červeného obdélníku,
    // vypiš do obdélníku text Stisk!,
    // jinak vyplň obdélník černou barvou
    if (bod.x > 580 & bod.x < 1660 & bod.y > 2100 & bod.y < 3500 ) {
      displej.setCursor(35, 240);
      displej.print("Stisk!");
    } else {
      displej.fillRect(35, 210, 80, 80, ILI9341_BLACK);
    }
  }
  delay(1);

}
