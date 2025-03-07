
#include "config.h"

int count = 0;

#define LED 2
#define RXD2 16  // RX2 en ESP32
#define TXD2 17  // TX2 en ESP32


#define IO_LOOP_DELAY 4000
unsigned long lastUpdate = 0;

// set up the 'counter' feed

AdafruitIO_Feed *feedHumedad = io.feed("humedad");
AdafruitIO_Feed *feedTemperatura = io.feed("temperatura");
AdafruitIO_Feed *feedLEDB = io.feed("ledb");
AdafruitIO_Feed *feedGas = io.feed("gas");
AdafruitIO_Feed *feedServo = io.feed("servo");
AdafruitIO_Feed *feedVentilador = io.feed("ventilador");
AdafruitIO_Feed *feedbuzzer = io.feed("buzzer");
void setup() {

  // start the serial connection
  
  Serial.begin(9600);  // Monitor serial en la PC
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);  // UART2 con formato correcto

  // wait for serial monitor to open
  while(! Serial);

  Serial.print("Connecting to Adafruit IO");

  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  // connect to io.adafruit.com
  io.connect(); // connect to my "server"

  //feedButton->onMessage(handleMessageButton);       //Recibe la señal de adafuit para encender la LED
  feedLEDB->onMessage(handleMessageLEDB);             // feed led
  feedHumedad->onMessage(handleMessageHumedad);       // feed humedad
  feedServo->onMessage(handleMessageServo);           // feed servo
  feedVentilador->onMessage(handleMessageventilador); // feed ventilador
  feedbuzzer->onMessage(handleMessagebuzzer);           // feed buzzer
  feedGas->onMessage(handleMessageGas);           // feed buzzer
  feedTemperatura->onMessage(handleMessageTemperatura);
  

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  

  // we are connected
  Serial.println();
  Serial.println(io.statusText());
  feedLEDB->get();    //takes data from my feed 
  feedServo->get();
  feedVentilador->get();
  feedbuzzer->get();
}

void loop() {
    io.run();
  


  if (millis() > (lastUpdate + IO_LOOP_DELAY)) {

     String data;
    
    if (Serial2.available()) {
        String data = Serial2.readStringUntil('\n');  // Leer una línea completa
        data.trim();  // Eliminar espacios en blanco y saltos de línea
        
        Serial.println("Datos recibidos: " + data);  // Verificar en monitor serial

        if (data.startsWith("H:")) {  // Verificar si la línea comienza con "H "
            String humedadStr = data.substring(2);  // Extraer el valor después de "Hum:"
            humedadStr.trim();  // Limpiar espacios en blanco
            
            float humedad = humedadStr.toFloat();  // Convertir a número
            
            Serial.print("Humedad extraída: ");
            Serial.println(humedad);
            
            feedHumedad->save(humedad);  // Enviar a Adafruit IO
        }

        if (data.startsWith("T:")){

            String temperaturaStr = data.substring(2);
            temperaturaStr.trim();

            float temperatura = temperaturaStr.toFloat();

            Serial.print("Temperatura extraida: ");
            Serial.println(temperatura);

            feedTemperatura->save(temperatura);


        }
    }

        if (data.startsWith("G:")){

            String gasStr = data.substring(2);
            gasStr.trim();

            float gas = gasStr.toFloat();

            Serial.print("% de Gas extraido: ");
            Serial.println(gas);

            feedGas->save(gas);


        }
    

    // after publishing, store the current time
    lastUpdate = millis();
  }
  
   

}




void handleMessageHumedad (AdafruitIO_Data *data){
  Serial.print("Recibido en feed humedad: ");
    Serial.println(data->toFloat());

}

void handleMessageTemperatura (AdafruitIO_Data *data){
  Serial.print("Recibido en feed temperatura: ");
    Serial.println(data->toFloat());

}

void handleMessageGas (AdafruitIO_Data *data){

    Serial.print("Recibido en feed gas: ");
    Serial.println(data->toFloat());

}

void handleMessageLEDB (AdafruitIO_Data *data){
 

Serial.println(data->value());

  char valor = data->value()[0];
  if (valor == '1') {
    digitalWrite(LED, HIGH); 
  } else if (valor == '0') {
    digitalWrite(LED, LOW);
  }
}

void handleMessageServo (AdafruitIO_Data *data){

  Serial.println(data->value());

  char actServo = data->value()[0];
  
  if(actServo == '0'){

     Serial.println("a");
     Serial2.println("a");
    delay(1000);
    
  } else if (actServo == '1'){
 
    Serial.println("A");
    Serial2.println("A");
    delay(1000);

  } 
 

}

void handleMessageventilador (AdafruitIO_Data *data){
  
  Serial.println(data->value());

  char actVentilador = data->value()[0];

  if (actVentilador == '1') {
    Serial.println("V");
    Serial2.println("V"); 
    delay(1000);
    
  } else if (actVentilador == '0') {
    Serial.println("v");
    Serial2.println("v");
    delay(1000);
  }

}




void handleMessagebuzzer (AdafruitIO_Data *data){

  Serial.println(data->value());

  char actBuzzer = data->value()[0];
  
  if(actBuzzer == '0'){

     Serial.println("b");
     Serial2.println("b");
    delay(1000);
    
  } else if (actBuzzer == '1'){
 
    Serial.println("B");
    Serial2.println("B");
    delay(1000);

  } 

}


/*#include "config.h"

int count = 0;

float gas = 0;
float temperatura = 0;
float humedad = 0;

#define LED 2
#define RXD2 16  // RX2 en ESP32
#define TXD2 17  // TX2 en ESP32


#define IO_LOOP_DELAY 4000
unsigned long lastUpdate = 0;

// set up the 'counter' feed

AdafruitIO_Feed *feedHumedad = io.feed("humedad");
AdafruitIO_Feed *feedTemperatura = io.feed("temperatura");
AdafruitIO_Feed *feedLEDB = io.feed("ledb");
AdafruitIO_Feed *feedGas = io.feed("gas");
AdafruitIO_Feed *feedServo = io.feed("servo");
AdafruitIO_Feed *feedVentilador = io.feed("ventilador");
AdafruitIO_Feed *feedbuzzer = io.feed("buzzer");
void setup() {

  // start the serial connection

  Serial.begin(9600);                           // Monitor serial en la PC
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);  // UART2 con formato correcto

  // wait for serial monitor to open
  while (!Serial)
    ;

  Serial.print("Connecting to Adafruit IO");

  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  // connect to io.adafruit.com
  io.connect();  // connect to my "server"

  //feedButton->onMessage(handleMessageButton);       //Recibe la señal de adafuit para encender la LED
  feedLEDB->onMessage(handleMessageLEDB);              // feed led
  feedHumedad->onMessage(handleMessageHumedad);        // feed humedad
  feedServo->onMessage(handleMessageServo);            // feed servo
  feedVentilador->onMessage(handleMessageventilador);  // feed ventilador
  feedbuzzer->onMessage(handleMessagebuzzer);          // feed buzzer
  feedGas->onMessage(handleMessageGas);                // feed buzzer
  feedTemperatura->onMessage(handleMessageTemperatura);


  // wait for a connection
  while (io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }


  // we are connected
  Serial.println();
  Serial.println(io.statusText());
  feedLEDB->get();  //takes data from my feed
  feedServo->get();
  feedVentilador->get();
  feedbuzzer->get();
}

void loop() {
  io.run();



  if (millis() > (lastUpdate + IO_LOOP_DELAY)) {

    feedGas->save(gas);
    feedTemperatura->save(temperatura);
    feedHumedad->save(humedad);
    // after publishing, store the current time
    lastUpdate = millis();
  }

  String data;

  if (Serial2.available()) {
    String data = Serial2.readStringUntil('\n');  // Leer una línea completa
    data.trim();                                  // Eliminar espacios en blanco y saltos de línea

    Serial.println("Datos recibidos: " + data);  // Verificar en monitor serial

    if (data.startsWith("H:")) {              // Verificar si la línea comienza con "H "
      String humedadStr = data.substring(2);  // Extraer el valor después de "Hum:"
      humedadStr.trim();                      // Limpiar espacios en blanco

      humedad = humedadStr.toFloat();  // Convertir a número

      Serial.print("Humedad extraída: ");
      Serial.println(humedad);

      //feedHumedad->save(humedad);  // Enviar a Adafruit IO
    }

    if (data.startsWith("T:")) {

      String temperaturaStr = data.substring(2);
      temperaturaStr.trim();

      temperatura = temperaturaStr.toFloat();

      Serial.print("Temperatura extraida: ");
      Serial.println(temperatura);

      //feedTemperatura->save(temperatura);
    }
  }

  if (data.startsWith("G:")) {

    String gasStr = data.substring(2);
    gasStr.trim();

    gas = gasStr.toFloat();

    Serial.print("% de Gas extraido: ");
    Serial.println(gas);

    //feedGas->save(gas);
  }
}




void handleMessageHumedad(AdafruitIO_Data *data) {
  Serial.print("Recibido en feed humedad: ");
  Serial.println(data->toFloat());
}

void handleMessageTemperatura(AdafruitIO_Data *data) {
  Serial.print("Recibido en feed temperatura: ");
  Serial.println(data->toFloat());
}

void handleMessageGas(AdafruitIO_Data *data) {

  Serial.print("Recibido en feed gas: ");
  Serial.println(data->toFloat());
}

void handleMessageLEDB(AdafruitIO_Data *data) {


  Serial.println(data->value());

  char valor = data->value()[0];
  if (valor == '1') {
    digitalWrite(LED, HIGH);
  } else if (valor == '0') {
    digitalWrite(LED, LOW);
  }
}

void handleMessageServo(AdafruitIO_Data *data) {

  Serial.println(data->value());

  char actServo = data->value()[0];

  if (actServo == '0') {

    Serial.println("a");
    Serial2.println("a");
    delay(1000);

  } else if (actServo == '1') {

    Serial.println("A");
    Serial2.println("A");
    delay(1000);
  }
}

void handleMessageventilador(AdafruitIO_Data *data) {

  Serial.println(data->value());

  char actVentilador = data->value()[0];

  if (actVentilador == '1') {
    Serial.println("V");
    Serial2.println("V");
    delay(1000);

  } else if (actVentilador == '0') {
    Serial.println("v");
    Serial2.println("v");
    delay(1000);
  }
}




void handleMessagebuzzer(AdafruitIO_Data *data) {

  Serial.println(data->value());

  char actBuzzer = data->value()[0];

  if (actBuzzer == '0') {

    Serial.println("b");
    Serial2.println("b");
    delay(1000);

  } else if (actBuzzer == '1') {

    Serial.println("B");
    Serial2.println("B");
    delay(1000);
  }
}*/
