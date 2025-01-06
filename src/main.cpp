#include <SPI.h>
#include <LoRa.h>
#include <LowPower.h> // Biblioteca para modos de baixa energia

// Configuração dos pinos LoRa
#define SS 10  // Pino CS (Chip Select)
#define RST 9  // Pino RESET
#define DIO0 2 // Pino DIO0 (Interrupção)

// Configuração do sensor higrômetro
#define SENSOR_DIGITAL_PIN 3 // Pino digital do sensor higrômetro (DO)
#define SENSOR_ANALOG_PIN A0 // Pino analógico do sensor higrômetro (AO)

// Configuração do sensor de inclinação
#define TILT_SENSOR_PIN 4 // Pino digital do sensor de inclinação

// Configuração do LoRa
#define DESTINATION 0x01 // Endereço do destinatário

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;

  // Configuração dos pinos
  pinMode(SENSOR_DIGITAL_PIN, INPUT);
  pinMode(TILT_SENSOR_PIN, INPUT); // Não precisa de resistor pull-up externo (integrado no módulo)

  // Inicializa o módulo LoRa
  Serial.println("Iniciando LoRa no Arduino...");
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(915E6))
  {
    Serial.println("Falha ao inicializar o LoRa!");
    while (1)
      ;
  }

  LoRa.setSpreadingFactor(12); // Maximum sensitivity
  LoRa.setTxPower(20);         // Maximum transmission power
  LoRa.setCodingRate4(8);

  Serial.println("LoRa iniciado com sucesso!");
}

void loop()
{
  // Leitura do sensor higrômetro
  int digitalState = digitalRead(SENSOR_DIGITAL_PIN);
  String condition = digitalState == HIGH ? "SECO" : "MOLHADO";
  int sensorValue = analogRead(SENSOR_ANALOG_PIN);
  float moisturePercentage = map(sensorValue, 0, 1023, 0, 100);

  // Leitura do sensor de inclinação
  int tiltState = digitalRead(TILT_SENSOR_PIN);
  String tiltCondition = tiltState == HIGH ? "NORMAL" : "INCLINADO";

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

  // Desliga o LoRa e entra em modo de baixo consumo
  LoRa.sleep();
  Serial.println("Arduino dormindo por 3 horas...");
  Serial.flush();

  // Dorme por 3 horas (10.800.000 ms)
  for (int i = 0; i < 10800; i++)
  {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF); // Dorme por 8 segundos (máximo permitido por ciclo)
    digitalWrite(LED_BUILTIN, HIGH);                // Gera consumo breve
    delay(10);                                      // Mantém ligado por 10ms
    digitalWrite(LED_BUILTIN, LOW);                 // Desativa o LED
  }

  Serial.println("Arduino acordado!");
  Serial.flush();
}
