/**
 * Setup (Erase, Clean and Format) a Mifare Classic tags as an NDEF tag and store PNRD type
 * (PNDR and inverted PNRD) in the Tag.
*/

#define BTtype 13
#define BTdo 12

#if 1
  #define NDEF_DEBUG

  #include <LiquidCrystal.h>
  #include <Pn532NfcReader.h>
  #include <PN532_HSU.h>

  /** Input of Incidence Matrix
   * The size of Incidence Matrix must contain the number of transitions and the number of places in PetriNet
  */
  /*                           T0| T1| T2| T3| T4| T5| T6*/
  int8_t mIncidenceMatrix[] = {-1,  0,  0,  0,  0,  0,  1,  // P0
                                1, -1, -1,  0,  0,  0,  0,  // P1
                                0,  0,  1,  0,  0,  0,  0,  // P2
                                0,  1,  0, -1, -1,  0,  0,  // P3
                                0,  0,  0,  1,  0,  0,  0,  // P4
                                0,  0,  0,  0,  1, -1,  0,  // P5
                                0,  0,  0,  0,  0,  1, -1   // P6  
  };

  /** Input of Token Vector
   * The size of Token Vector must contain the number of places in PetriNet
  */
  uint16_t mStartingTokenVector[] = {1,0,0,0,0,0,0,0};

  /** Input of Fire Vector:
   * The Fire Vector must contain the number of transitions in the PetriNet
  */
  uint16_t mFireVector[] = {0,0,0,1,0};

  //Rotines related with the configuration of the RFID reader PN532
  PN532_HSU pn532hsu(Serial2);
  NfcAdapter nfc(pn532hsu);

  Pn532NfcReader* reader = new Pn532NfcReader(&nfc);
  Pnrd pnrd = Pnrd(reader,7,7); //leitor, no estados e no transicoes

  #define LCD_WIDTH 16
  #define LCD_HEIGHT 2
  #define RS 19
  #define Enable 18
  #define D4 5
  #define D5 4
  #define D6 2
  #define D7 15

  LiquidCrystal lcd(RS, Enable, D4, D5, D6, D7);
#endif

bool flag[2];
int count;
char text_to_print[LCD_WIDTH + 1];
char padding[LCD_WIDTH + 1];

void setup(void) {
  Serial.begin(115200);

  lcd.begin(LCD_WIDTH, LCD_HEIGHT);
  for (int i = 0; i <= LCD_WIDTH; i++) padding[i] = i != LCD_WIDTH? ' ':0;

  pinMode(BTtype, INPUT_PULLUP);
  pinMode(BTdo, INPUT_PULLUP);
}

void loop(void) {
  count = 0;
  flag[0] = false;
  flag[1] = true;
  lcd.clear();

  // Button to set the type of tag config
  while (digitalRead(BTdo)) {
    if (!flag[0] && !digitalRead(BTtype)) {
      flag[0] = true;
    } else if (flag[0] && digitalRead(BTtype)) {
      count++;
      count %= 2;

      flag[0] = false;
      flag[1] = true;
    }

    if(flag[1]) {
      lcd.setCursor(0, 0);
      if (count == 0) snprintf(text_to_print, LCD_WIDTH + 1, "Setup iPNRD Tag%s", padding);
      if (count == 1) snprintf(text_to_print, LCD_WIDTH + 1, "Setup PNRD Tag%s", padding);
      lcd.print(text_to_print);

      flag[1] = false;
    }
    delay(50);
  }

  // Preparing the tag information
  eraseRFID();
  cleanRFID();
  formatRFID();
  recordRFID();

  // Espera de 5 segundos
  delay(3000);
}

void eraseRFID() {
  PN532 tag(pn532hsu);

  nfc.begin();
  tag.begin();

  // Set the max number of retry attempts to read from a card
  // This prevents us from waiting forever for a card, which is
  // the default behaviour of the PN532.
  tag.setPassiveActivationRetries(0xFF);

  // configure board to read RFID tags
  tag.SAMConfig();

  uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
  uint8_t uidLength;
  String tagID = "";

  lcd.setCursor(0, 1);
  // lcd.print("Place the Tag");
  lcd.print("Insira a Tag");

  // Identifier if tag is present
  while (!nfc.tagPresent()) delay(500);
  lcd.setCursor(0, 1);
  snprintf(text_to_print, LCD_WIDTH + 1, "Tag Presented!!!%s", padding);
  lcd.print(text_to_print);
  delay(500);

  lcd.setCursor(0, 1);
  tag.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
  for(int i = 0; i < uidLength; i++) tagID += String(uid[i], HEX);
  tagID.toUpperCase();
  snprintf(text_to_print, LCD_WIDTH + 1, "TagId: %s%s", tagID, padding);
  lcd.print(text_to_print);
  delay(2000);

  // Erase the tag returning the success of erasing
  lcd.setCursor(0, 1);
  // snprintf(text_to_print, LCD_WIDTH + 1, "Erasing the Tag%s", padding);
  snprintf(text_to_print, LCD_WIDTH + 1, "Apagando a Tag%s", padding);
  lcd.print(text_to_print);
  bool success = nfc.erase();
  delay(500);

  // Return the message of erasing
  if (success) snprintf(text_to_print, LCD_WIDTH + 1, "Sucesso%s", padding);
  else snprintf(text_to_print, LCD_WIDTH + 1, "Tag ja apagada%s", padding);
  lcd.setCursor(0, 1);
  lcd.print(text_to_print);
  delay(500);
}

void cleanRFID() {
  nfc.begin();

  // Identifier if tag is present
  while (!nfc.tagPresent()) delay(500);

  // Erase the tag returning the success of erasing
  lcd.setCursor(0, 1);
  // snprintf(text_to_print, LCD_WIDTH + 1, "Factory Setup%s", padding);
  snprintf(text_to_print, LCD_WIDTH + 1, "Conf. de Fabrica%s", padding);
  lcd.print(text_to_print);
  bool success = nfc.clean();
  delay(500);

  if (success) snprintf(text_to_print, LCD_WIDTH + 1, "Sucesso%s", padding);
  else snprintf(text_to_print, LCD_WIDTH + 1, "Tag ja Conf.%s", padding);
  lcd.setCursor(0, 1);
  lcd.print(text_to_print);
  delay(500);
}

void formatRFID() {
  nfc.begin();

  // Identifier if tag is present
  while (!nfc.tagPresent()) delay(500);

  // Format the tag returning the success of formating
  lcd.setCursor(0, 1);
  // snprintf(text_to_print, LCD_WIDTH + 1, "Format the Tag%s", padding);
  snprintf(text_to_print, LCD_WIDTH + 1, "Formatando a Tag%s", padding);
  lcd.print(text_to_print);
  bool success = nfc.format();
  delay(500);

  if (success) snprintf(text_to_print, LCD_WIDTH + 1, "Sucesso%s", padding);
  else snprintf(text_to_print, LCD_WIDTH + 1, "Tag ja formatada%s", padding);
  lcd.setCursor(0, 1);
  lcd.print(text_to_print);
  delay(500);
}

void readRFID() {
  // Read the information of the Tag
  NfcTag tag = nfc.read();

  // Print the information of the Tag
  tag.print();
}

void recordRFID() {
  // Identifier PNRD type, PNRD(false) or iPNRD(true)
  if (!count) {
    /** Creation of the reader and PNRD objects
     * Pnrd(readerPointer, num_places, num_transitions, num_max_of_inputs, num_max_of_outputs, hasConditions, hasTagHistory)
    */

    // Initializing PN532reader
    reader->initialize();

    //Defining the Fire Vector to be recorded
    pnrd.setFireVector(mFireVector);

    //Setting of the classic iPNRD approach
    pnrd.setAsTagInformation(PetriNetInformation::FIRE_VECTOR);
    pnrd.setAsTagInformation(PetriNetInformation::TAG_HISTORY);
    Serial.print("\nInitial recording of iPNRD tags.");
  } else {
    // Initializing PN532reader
    reader->initialize();

    //Defining the Incidence Matrix and the Token Vector to be recorded
    pnrd.setIncidenceMatrix(mIncidenceMatrix);
    pnrd.setTokenVector(mStartingTokenVector);

    //Setting of the classic PNRD approach
    pnrd.setAsTagInformation(PetriNetInformation::TOKEN_VECTOR);
    pnrd.setAsTagInformation(PetriNetInformation::ADJACENCY_LIST);
    Serial.print("\nInitial recording of PNRD tags.");
  }

  // Identifier if tag is present
  while (!nfc.tagPresent()) delay(500);

  lcd.setCursor(0, 1);
  // snprintf(text_to_print, LCD_WIDTH + 1, "Storing the Tag%s", padding);
  snprintf(text_to_print, LCD_WIDTH + 1, "Gravando na Tag%s", padding);
  lcd.print(text_to_print);
  delay(500);

  // Cormfirming if data is saved
  if(pnrd.saveData() == WriteError::NO_ERROR) {
    if (count == 0) snprintf(text_to_print, LCD_WIDTH + 1, "iPNRD Gravada%s", padding);
    if (count == 1) snprintf(text_to_print, LCD_WIDTH + 1, "PNRD Gravada%s", padding);
    //Serial.println("\nTag configurated successfully.\n");
  } else snprintf(text_to_print, LCD_WIDTH + 1, "Erro de Gravacao%s", padding);
  lcd.setCursor(0, 1);
  lcd.print(text_to_print);

  // Read RFID tag
  //readRFID();
}