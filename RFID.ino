#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <U8g2lib.h> 

#define RST_PIN     9
#define SS_PIN      10
#define BUZZER_PIN  8 
#define BUTTON_PIN  2 

MFRC522 mfrc522(SS_PIN, RST_PIN);

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
 
 void displayInitialScreen() {
 u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_torussansbold8_8r);
  u8g2.drawStr(0, 35, "Place card near");
  u8g2.drawStr(0, 50, "reader...");
  u8g2.sendBuffer();
 }

void setup() {
  Serial.begin(9600);
  while (!Serial);
  u8g2.begin();

   u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr); // Set a medium font 
  u8g2.drawStr(0, 15, "RFID UID Reader");
  u8g2.setFont(u8g2_font_torussansbold8_8r); // Smaller font
  u8g2.drawStr(0, 35, "Place card near");
  u8g2.drawStr(0, 50, "reader...");
  u8g2.sendBuffer();
  delay(2000); //

  // --- MFRC522 Initialization ---
  SPI.begin();     
  mfrc522.PCD_Init(); 

  // byte version = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  // Serial.print(F("MFRC522 Version: 0x"));
  // Serial.println(version, HEX);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); 

void loop() {
 if (digitalRead(BUTTON_PIN) == LOW) {
  displayInitialScreen();
  delay(250);
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  return;
 }

  // Reset the loop if no new card present on the sensor/reader.
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards (read its UID)
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  digitalWrite(BUZZER_PIN, HIGH); 
  delay(200);                    
  digitalWrite(BUZZER_PIN, LOW);

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenR08_tr); // Smaller font for labels
  u8g2.drawStr(0, 10, "Scanned UID:");
  u8g2.setFont(u8g2_font_ncenB14_tr); // Larger font for the UID itself

  Serial.print(""); 

  // --- Display UID on OLED (and Serial) ---
  // The mfrc522.uid object contains:
  //   mfrc522.uid.uidByte[] : An array of bytes representing the UID
  //   mfrc522.uid.size     : The number of bytes in the UID (e.g., 4 or 7)

  if (mfrc522.uid.size == 4) {
    // If it's a 4-byte UID, convert it to a single unsigned long decimal
    unsigned long scannedUidDecimal = ((unsigned long)mfrc522.uid.uidByte[0] << 24)
                                    | ((unsigned long)mfrc522.uid.uidByte[1] << 16)
                                    | ((unsigned long)mfrc522.uid.uidByte[2] << 8)
                                    | ((unsigned long)mfrc522.uid.uidByte[3]);

    u8g2.setCursor(0, 35);
    u8g2.print(scannedUidDecimal, DEC); 

    Serial.println(scannedUidDecimal, DEC); 
  } else {
    u8g2.setFont(u8g2_font_ncenR08_tr); 
    u8g2.drawStr(0, 30, "UID (HEX):");
    u8g2.setCursor(0, 50); 

    for (byte i = 0; i < mfrc522.uid.size; i++) {
      if (mfrc522.uid.uidByte[i] < 0x10) {
        u8g2.print("0");
        Serial.print("0"); 
      }
      u8g2.print(mfrc522.uid.uidByte[i], HEX); 
      u8g2.print(" "); 
      Serial.print(mfrc522.uid.uidByte[i], HEX); 
      Serial.print(" ");
    }
    Serial.println(); 
  }

  u8g2.sendBuffer();      
 
  // Halt the currently selected PICC (tag)
  mfrc522.PICC_HaltA();
  // Stop encryption on the PCD (reader) if it was initiated (good practice)
  mfrc522.PCD_StopCrypto1();
}