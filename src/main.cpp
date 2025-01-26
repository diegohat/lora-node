#include <SPI.h>
#include <LoRa.h>
#include <LowPower.h> // Biblioteca para modos de baixa energia

// Configuração dos pinos LoRa
#define SS 10   // Pino CS (Chip Select)
#define RST 9   // Pino RESET
#define DIO0 2  // Pino DIO0 (Interrupção)

// Configuração do sensor higrômetro
#define SENSOR_DIGITAL_PIN 3 // Pino digital do sensor higrômetro (DO)
#define SENSOR_ANALOG_PIN A0 // Pino analógico do sensor higrômetro (AO)

// Configuração do sensor de inclinação
#define TILT_SENSOR_PIN 4    // Pino digital do sensor de inclinação

// Configuração do LoRa
#define DESTINATION 0x01     // Endereço do destinatário

void setup()
{
  Serial.begin(9600);
  while (!Serial);

  // Configuração dos pinos
  pinMode(SENSOR_DIGITAL_PIN, INPUT);
  pinMode(TILT_SENSOR_PIN, INPUT);

  // Inicializa o módulo LoRa
  Serial.println("Iniciando LoRa no Arduino...");
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(915E6))
  {
    Serial.println("Falha ao inicializar o LoRa!");
    while (1);
  }

  LoRa.setSpreadingFactor(12); // Máxima sensibilidade
  LoRa.setTxPower(20);         // Potência máxima de transmissão
  LoRa.setCodingRate4(8);

  Serial.println("LoRa iniciado com sucesso!");
}

void loop()
{
  // Despertamos o rádio LoRa (se antes foi colocado em sleep)
  LoRa.idle();

  // Leitura do sensor higrômetro (digital)
  int digitalState = digitalRead(SENSOR_DIGITAL_PIN);
  String condition = (digitalState == HIGH) ? "SECO" : "MOLHADO";

  // Leitura do sensor higrômetro (analógico)
  int sensorValue = analogRead(SENSOR_ANALOG_PIN);
  float moisturePercentage = map(sensorValue, 0, 1023, 0, 100);

  // Leitura do sensor de inclinação
  int tiltState = digitalRead(TILT_SENSOR_PIN);
  String tiltCondition = (tiltState == HIGH) ? "NORMAL" : "INCLINADO";

  // Exibe informações no monitor serial
  Serial.print("Estado do Solo (Digital): ");
  Serial.println(condition);

  Serial.print("Umidade do Solo (Analógica): ");
  Serial.print(moisturePercentage);
  Serial.println("%");

  Serial.print("Estado do Sensor de Inclinação: ");
  Serial.println(tiltCondition);

  // Envia dados via LoRa
  Serial.println("Enviando dados via LoRa...");
  LoRa.beginPacket();
  LoRa.write(DESTINATION); // Endereço do destinatário
  LoRa.print("Solo=");
  LoRa.print(condition);
  LoRa.print(",Umidade=");
  LoRa.print(moisturePercentage);
  LoRa.print("%,Inclinacao=");
  LoRa.print(tiltCondition);
  LoRa.endPacket();
  LoRa.flush();

  Serial.println("Dados enviados!");
  Serial.flush();

  // Coloca o rádio em modo sleep
  LoRa.sleep();
  Serial.println("Arduino dormindo por 15 minutos...");
  Serial.flush();

  // Dorme 3 horas (10800s) => 1350 ciclos de 8s
  for (int i = 0; i < 1350; i++)
  {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }

  Serial.println("Arduino acordou!");
  Serial.flush();

  // Depois que acordar, o loop reinicia, chamando LoRa.idle() na próxima iteração
}
