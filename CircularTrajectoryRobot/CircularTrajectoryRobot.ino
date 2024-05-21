#include <Pn532NfcReader.h>
#include <PN532_HSU.h>

int8_t mIncidenceMatrix[] = { -1,  0,  0,  0,  1,
                              1, - 1,  0,  0,  0,
                              0,  1, -1,  0,  0,
                              0,  0,  1, -1,  0,
                              0,  0,  0,  1, -1};

uint16_t mStartingTokenVector[] = {1,0,0,0,0};

//Rotines related with the configuration of the RFID reader PN532
PN532_HSU pn532hsu(Serial2);
NfcAdapter nfc = NfcAdapter(pn532hsu);

//Creation of the reader and PNRD objects
Pn532NfcReader* reader = new Pn532NfcReader(&nfc);
Pnrd pnrd = Pnrd(reader,5,5, false, true);

uint32_t tagId = 0xFF;

void setup() {
  //Initialization of the communication with the reader and with the computer Serial bus
  Serial.begin(115200);
  delay(3000);
  reader->initialize();

  //Setting of the application Petri net approach
   pnrd.setTokenVector(mStartingTokenVector);
   pnrd.setIncidenceMatrix(mIncidenceMatrix);

  //Setting of the classic iPNRD approach
  pnrd.setAsTagInformation(PetriNetInformation::FIRE_VECTOR);
  pnrd.setAsTagInformation(PetriNetInformation::TAG_HISTORY);

  pnrd.setDeviceId(1);
  Serial.println("\nRobot 1: Clockwise moviment.");
}

void loop() {
  delay(1000);

  //In case of a successful reading
  while (pnrd.getData() != ReadError::NO_ERROR);

  //Checks if it's a new tag
  if (tagId != pnrd.getTagId()) {
    tagId = pnrd.getTagId();
    Serial.print("\nNew tag. Id code: ");
    Serial.println(tagId, HEX);

    //Realização do disparo contido na etiqueta
    FireError fireError = pnrd.fire();
    switch (fireError) {
      case (FireError::NO_ERROR):
        //Salvar a nova informação na etiqueta
        if(pnrd.saveData() == WriteError::NO_ERROR) Serial.println("Right path, keep going.");
        else Serial.println("Error in the tag update.");

        return;
      case (FireError::PRODUCE_EXCEPTION):
        Serial.println("Error: exception. Wrong path.");

        break;
    }
  }
}