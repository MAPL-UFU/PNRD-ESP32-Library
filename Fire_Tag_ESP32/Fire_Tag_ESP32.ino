#include <Pn532NfcReader.h>
#include <PN532_HSU.h>

//Definição da comunicação com o leitor RFID PN532
PN532_HSU pn532hsu(Serial2);
NfcAdapter nfc = NfcAdapter(pn532hsu);

//Criação dos objetos de leitor e PNRD
Pn532NfcReader* reader = new Pn532NfcReader(&nfc);
Pnrd pnrd = Pnrd(reader, 7, 7);

uint32_t tagId = 0xFFFFFFFF;

void setup() {
  //Iniciação do leitor e da UART
  Serial.begin(115200);

  delay(3000);

  reader->initialize();

  //Configuração para a abordagem PNRD clássica
  pnrd.setAsTagInformation(PetriNetInformation::TOKEN_VECTOR);
  pnrd.setAsTagInformation(PetriNetInformation::ADJACENCY_LIST);
}

void loop() {
  //Tentativa de leitura da etiqueta
  ReadError readError = pnrd.getData();
  uint32_t a = pnrd.getTagId();
  uint32_t b = uint32_t(readError);
  //Serial.print("tagID = ");
  //Serial.print(a, HEX);
  //Serial.print("\treadError = ");
  //Serial.println(b, HEX);

  if (readError == ReadError::NO_ERROR) {
    //Verifica se é uma nova etiqueta
    if (tagId != pnrd.getTagId()) {
      tagId = pnrd.getTagId();
      Serial.print('\n');
      Serial.print("Leitura da etiqueta de código: ");
      Serial.println(tagId, HEX);
    } else {
      tagId = pnrd.getTagId();
      Serial.print('\n');
      Serial.print("Leitura da etiqueta de código: ");
      Serial.println(tagId, HEX);
      return;
    };

    //Realização do disparo da transição de índice 0
    FireError fireError = pnrd.fire(0);

    switch (fireError) {
      case FireError::NO_ERROR:
        Serial.println("Disparo bem sucedido.");

        //Mostrar o vetor de marcações resultante do disparo
        pnrd.printTokenVector();

        //Salvar a nova informação na etiqueta
        if (pnrd.saveData() == WriteError::NO_ERROR) {
          Serial.println("Informação atualizada.");
        } else {
          Serial.println("Erro na atualização da etiqueta.");
        };
        break;

      case FireError::PRODUCE_EXCEPTION:
        Serial.println("Erro: disparo gerou exceção.");
        break;

      case FireError::CONDITIONS_ARE_NOT_APPLIED:
        Serial.println("Erro: condições não são satisfeitas.");
        break;
    }
  }
  delay(5000);
}