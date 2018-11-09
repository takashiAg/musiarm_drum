#include <SPI.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define PDLTHD    4000

#define SPI1_CLK  25
#define SPI1_MISO 33
#define SPI1_MOSI 32
#define SPI1_SS   26

#define SPI2_CLK  14
#define SPI2_MISO 12
#define SPI2_MOSI 13
#define SPI2_SS   27

#define BASS  35
#define SNARE 38
#define TOM1  50
#define TOM2  48
#define TOM3  47
#define TOM4  45
#define FTOM1 43
#define FTOM2 41
#define HHPDL 44
#define HHOPN 46
#define HHCLZ 42
#define CRSHR 57
#define CRSHL 49
#define RIDE  51

#define HH    999
#define HHCTL 1000

#define PAD0  BASS
#define PAD1  TOM3
#define PAD2  TOM1
#define PAD3  CRSHR
#define PAD4  CRSHR
#define PAD5  TOM3
#define PAD6  HH
#define PAD7  RIDE
#define PAD8  CRSHL
#define PAD9  CRSHL
#define PAD10 HHPDL
#define PAD11 BASS
#define PAD12 TOM1
#define PAD13 SNARE
#define PAD14 SNARE
#define PAD15 HH

// set SPI freqency 1MHz
#define SPI_CLK 1000000

#define SERVICE_UUID        "03b80e5a-ede8-4b33-a751-6ce34ec4c700"
#define CHARACTERISTIC_UUID "7772e5db-3868-4112-a1a9-f2669d106bf3"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

uint8_t midiPacket[] = {
  0x80,  // header
  0x80,  // timestamp, not implemented
  0x00,  // status
  0x3c,  // 0x3c == 60 == middle c
  0x00   // velocity
};
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class Hysteresis {
  public:
    Hysteresis(int t1, int t2) {
      th1 = t1;
      th2 = t2;
      status = false;
    };
    boolean update(int value) {
      if (value > th2) {
        status = true;
      }
      if (value < th1) {
        status = false;
      }
      return status;
    };
    boolean status;
    int th1, th2;
  private:
};

Hysteresis th0(40, 100);
Hysteresis th1(40, 100);
Hysteresis th2(40, 100);
Hysteresis th3(40, 100);
Hysteresis th4(40, 100);
Hysteresis th5(40, 100);
Hysteresis th6(40, 100);
Hysteresis th7(40, 100);
Hysteresis th8(40, 100);
Hysteresis th9(40, 100);
Hysteresis thA(40, 100);
Hysteresis thB(40, 100);
Hysteresis thC(40, 100);
Hysteresis thD(40, 100);
Hysteresis thE(40, 100);
Hysteresis thF(40, 100);

bool hihat_open = false;
//uninitalised pointers to SPI objects
SPIClass SPI1(HSPI);
//SPIClass SPI2(VSPI);
SPISettings spiSettings = SPISettings(SPI_CLK, SPI_MSBFIRST, SPI_MODE1);

void setup() {
  if (PAD0 == HHPDL) {
    th0.th1 = PDLTHD;
    th0.th2 = PDLTHD;
  }
  if (PAD1 == HHPDL) {
    th1.th1 = PDLTHD;
    th1.th2 = PDLTHD;
  }
  if (PAD2 == HHPDL) {
    th2.th1 = PDLTHD;
    th2.th2 = PDLTHD;
  }
  if (PAD3 == HHPDL) {
    th3.th1 = PDLTHD;
    th3.th2 = PDLTHD;
  }
  if (PAD4 == HHPDL) {
    th4.th1 = PDLTHD;
    th4.th2 = PDLTHD;
  }
  if (PAD5 == HHPDL) {
    th5.th1 = PDLTHD;
    th5.th2 = PDLTHD;
  }
  if (PAD6 == HHPDL) {
    th6.th1 = PDLTHD;
    th6.th2 = PDLTHD;
  }
  if (PAD7 == HHPDL) {
    th7.th1 = PDLTHD;
    th7.th2 = PDLTHD;
  }
  if (PAD8 == HHPDL) {
    th8.th1 = PDLTHD;
    th8.th2 = PDLTHD;
  }
  if (PAD9 == HHPDL) {
    th9.th1 = PDLTHD;
    th9.th2 = PDLTHD;
  }
  if (PAD10 == HHPDL) {
    thA.th1 = PDLTHD;
    thA.th2 = PDLTHD;
  }
  if (PAD11 == HHPDL) {
    thB.th1 = PDLTHD;
    thB.th2 = PDLTHD;
  }
  if (PAD12 == HHPDL) {
    thC.th1 = PDLTHD;
    thC.th2 = PDLTHD;
  }
  if (PAD13 == HHPDL) {
    thD.th1 = PDLTHD;
    thD.th2 = PDLTHD;
  }
  if (PAD14 == HHPDL) {
    thE.th1 = PDLTHD;
    thE.th2 = PDLTHD;
  }
  if (PAD15 == HHPDL) {
    thF.th1 = PDLTHD;
    thF.th2 = PDLTHD;
  }

  Serial.begin(115200);

  BLEDevice::init("DRM");

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(BLEUUID(SERVICE_UUID));

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      BLEUUID(CHARACTERISTIC_UUID),
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_WRITE_NR
                    );

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  //pAdvertising->addServiceUUID(pService->getUUID());
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();

  SPI1.begin(SPI1_CLK, SPI1_MISO, SPI1_MOSI, SPI1_SS);
  //  SPI2.begin(SPI2_CLK, SPI2_MISO, SPI2_MOSI, SPI2_SS);

  pinMode(SPI1_SS, OUTPUT);
  pinMode(SPI1_CLK, OUTPUT);
  pinMode(SPI1_MISO, INPUT);
  pinMode(SPI1_MOSI, OUTPUT);
  pinMode(SPI2_SS, OUTPUT);
  //  pinMode(SPI2_CLK, OUTPUT);
  //  pinMode(SPI2_MISO, INPUT);
  //  pinMode(SPI2_MOSI, OUTPUT);

  digitalWrite(SPI1_SS, HIGH) ;
  digitalWrite(SPI2_SS, HIGH) ;
}

void loop() {

  bool oldstatus , new_status ;

  int a0 = adc_read(0);
  int a1 = adc_read(1);
  int a2 = adc_read(2);
  int a3 = adc_read(3);
  int a4 = adc_read(4);
  int a5 = adc_read(5);
  int a6 = adc_read(6);
  int a7 = adc_read(7);
  int a8 = adc_read(8);
  int a9 = adc_read(9);
  int aA = adc_read(10);
  int aB = adc_read(11);
  int aC = adc_read(12);
  int aD = adc_read(13);
  int aE = adc_read(14);
  int aF = adc_read(15);

  play(th0.status, th0.update(a0), a0, PAD0);
  play(th1.status, th1.update(a1), a1, PAD1);
  play(th2.status, th2.update(a2), a2, PAD2);
  play(th3.status, th3.update(a3), a3, PAD3);
  play(th4.status, th4.update(a4), a4, PAD4);
  play(th5.status, th5.update(a5), a5, PAD5);
  play(th6.status, th6.update(a6), a6, PAD6);
  play(th7.status, th7.update(a7), a7, PAD7);
  play(th8.status, th8.update(a8), a8, PAD8);
  play(th9.status, th9.update(a9), a9, PAD9);
  play(thA.status, thA.update(aA), aA, PAD10);
  play(thB.status, thB.update(aB), aB, PAD11);
  play(thC.status, thC.update(aC), aC, PAD12);
  play(thD.status, thD.update(aD), aD, PAD13);
  play(thE.status, thE.update(aE), aE, PAD14);
  play(thF.status, thF.update(aF), aF, PAD15);

  Serial.print('\t');

  Serial.print(a0);
  Serial.print('\t');
  Serial.print(a1);
  Serial.print('\t');
  Serial.print(a2);
  Serial.print('\t');
  Serial.print(a3);
  Serial.print('\t');
  Serial.print(a4);
  Serial.print('\t');
  Serial.print(a5);
  Serial.print('\t');
  Serial.print(a6);
  Serial.print('\t');
  Serial.print(a7);
  Serial.print('\t');
  Serial.print(a8);
  Serial.print('\t');
  Serial.print(a9);
  Serial.print('\t');
  Serial.print(aA);
  Serial.print('\t');
  Serial.print(aB);
  Serial.print('\t');
  Serial.print(aC);
  Serial.print('\t');
  Serial.print(aD);
  Serial.print('\t');
  Serial.print(aE);
  Serial.print('\t');
  Serial.print(aF);
  Serial.print('\n');
  delay(1);
}

void play(bool oldstatus, bool new_status, int value, int PAD) {
  if (PAD == HH) {
    if (hihat_open) {
      PAD = HHOPN;
    } else {
      PAD = HHCLZ;
    }
  }
  //  if (PAD == HHPDL) {
  //    if (value > PDLTHD) {
  //      hihat_open = false;
  //    } else {
  //      hihat_open = true;
  //    }
  //  }
  if (oldstatus ^ new_status) {
    if (new_status) {
      if (PAD == HHPDL)
        hihat_open = true;
      playnote(PAD, value);
    } else {
      if (PAD == HHPDL)
        hihat_open = false;
      releasenote(PAD);
    }
  }
}

int adc_read(uint8_t channel) {
  if (channel <= 7) {
    SPI1.beginTransaction(spiSettings);
    digitalWrite(SPI1_SS, LOW);
    SPI1.transfer(0x06 | (channel >> 2));
    int d1 = SPI1.transfer(channel << 6) ;
    int d2 = SPI1.transfer(0x00) ;
    digitalWrite(SPI1_SS, HIGH);
    SPI1.endTransaction();
    return ((d1 & 0x1F) * 256 + d2);
  } else {
    channel -= 8;
    SPI1.beginTransaction(spiSettings);
    digitalWrite(SPI2_SS, LOW);
    SPI1.transfer(0x06 | (channel >> 2));
    int d1 = SPI1.transfer(channel << 6) ;
    int d2 = SPI1.transfer(0x00) ;
    digitalWrite(SPI2_SS, HIGH);
    SPI1.endTransaction();
    return ((d1 & 0x1F) * 256 + d2);
  }

}

void releasenote(int index) {
  if (!deviceConnected)
    return ;
  midiPacket[2] = 0x80; // note up, channel 0
  midiPacket[3] = index; // note up, channel 0
  midiPacket[4] = 0;    // velocity
  pCharacteristic->setValue(midiPacket, 5); // packet, length in bytes)
  pCharacteristic->notify();
}
void playnote(int index, int power) {
  if (!deviceConnected)
    return ;
  midiPacket[2] = 0x90; // note up, channel 0
  midiPacket[3] = index; // note up, channel 0
  midiPacket[4] = uint8_t((power - 1000) / 100) + 40; // velocity
  pCharacteristic->setValue(midiPacket, 5); // packet, length in bytes)
  pCharacteristic->notify();
}



