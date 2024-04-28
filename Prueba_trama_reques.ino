#include <SPI.h>            //Librería para usar SPI
#include <mcp2515.h>        //Librería para la comunicaión CAN
#include <Arduino.h>                                                
#include <WiFi.h>
#include <FirebaseESP32.h>                                  // librerías necesarias para usar el modulo wi-fi inorporado en el ESP32, Arduino y comunicación con FireBase
#include <addons/TokenHelper.h>                               //libreria para Proporcionar la información del proceso de generación del token.
#include <addons/RTDBHelper.h>                                //libreria para Proporcionar información de impresión de carga útil RTDB y otras funciones auxiliares.

#define WIFI_SSID "BUAP_Estudiantes"                      //"Totalplay-80A5_EXT_plus"
#define WIFI_PASSWORD "f85ac21de4"                    //"80A5F733Mn38yzXp"                    //se definen las credeciales para la cenexion wifi. Nombre y contraseña
#define API_KEY "AIzaSyA7nRZppsJCu1tMV7UmT1_WFTnfca5vY_E"   //Se define la llave API del usuario
#define DATABASE_URL "esp32-canbus-default-rtdb.firebaseio.com"       //Se define la URL de la base de datos con la siguiente estructura
                                                              //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
#define USER_EMAIL "ramonigh.15@gmail.com"    
#define USER_PASSWORD "ESPBUAP123"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;                                    //Declaramos los "Firebase Data object"

unsigned long sendDataPrevMillis = 0;
struct can_frame canMsg;
struct can_frame canMsg1;   //Se defime la estructura de los mensajes
                            //como una trama CAN-bus
volatile int interrupt = 0;      //se define la variable de interrupcion
const int boton = 4;            // se difine el Pin del pushbotton
int estadoBoton = 0;           // variable donde se guarda si el  boton ya fue precionado
int valor = 0;                  // varibla donde se guarda la entrada del boton
int val_old = 0;

     MCP2515 mcp2515(2);                  // CS del MCP2515 al pin 10

void setup() {
  while (!Serial);
  Serial.begin(115200);
   SPI.begin();
   
WiFi.begin(WIFI_SSID, WIFI_PASSWORD);                 //Iniciamos el modulo WiFi
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;                         //Asignamos las variables ya definidas a la configuracion estalecida en el coigo de la librerías de FireBaseESP32
  config.token_status_callback = tokenStatusCallback;         //configuracion para llamar el estado del token

  //Asegúrese de que el espacio libre del dispositivo no sea inferior a 80 k para ESP32 y 10 k para ESP8266.
  // de lo contrario la conexión SSL fallará
  //////////////////////////////////////////////////////////////////////////////////////////////

  Firebase.begin(&config, &auth);                       //Inicia Firebase
  // Comente o pase un valor falso cuando la reconexión WiFi se controle mediante su código o biblioteca de terceros
  Firebase.reconnectWiFi(true);
  Firebase.setDoubleDigits(5);                        //digitos despues del punto en variables tipo doble

  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);      //configura el CAN a velocidad de 500KBPS 
                                                //y la frecuencua del ozcilador en 8MHz
  Serial.println("------- Receptor  CAN ----------");
  Serial.println("ID  DLC   DATA");
pinMode(boton, INPUT);                              // de define "boton" como entrada

//  mcp2515.setConfigMode();
//  mcp2515.setFilterMask(MCP2515::MASK0, false, 0x7FF);      // Primer marcara 
//  mcp2515.setFilterMask(MCP2515::MASK1, false, 0x7FF);      // Segunda marcara 
//  mcp2515.setFilter(MCP2515::RXF0, false, 0x280);           // Filtro 1
//    mcp2515.setFilter(MCP2515::RXF1, false, 0x0F6);       // Filtro 2 
//    
  mcp2515.setNormalMode();
    attachInterrupt(digitalPinToInterrupt(21), irqHandler, FALLING);   //Funcción para activar la interrupción
    //El primer termino es el pin de interrupción, 
    // Función de la interrupción
    // Activa la funcion de la interrupcion, cuando hay un cambio de alto a bajo
}

void irqHandler() {                               // Funcion implemenatada por la interrupcion
    interrupt=1;                              // variable interrucion a positivo
  }

void loop() {

valor= digitalRead(boton); // lee el estado del Boton
if ((valor == HIGH) && (val_old == LOW)){
estadoBoton=1-estadoBoton;    // Se tiene un valor de 0 o 1 si el boton ya fue presionado
delay(10);
}
val_old = valor; // valor del antiguo estado
if (estadoBoton==1){
interrupt = 1; // inicia la comunicación CAN 
 // canMsg1.data[2] = 0x0c;
}
else{
interrupt = 0; // apagar la comunicación CAN
}  

canMsg1.can_id = 0x7DF;
canMsg1.can_dlc = 8;
canMsg1.data[0] = 0x02;
canMsg1.data[1] = 0x01;
canMsg1.data[2] = 0x05;
canMsg1.data[3] = 0x00;
canMsg1.data[4] = 0x00;
canMsg1.data[5] = 0x00;
canMsg1.data[6] = 0x00;
canMsg1.data[7] = 0x00;

  if (interrupt) {              // si interrupcion es verdadero se leen los mensajes del CAN  
        interrupt = 0;
        uint8_t irq = mcp2515.getInterrupts();
        
if (irq & MCP2515::CANINTF_RX0IF) {
            if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {      // Recibe los mensajes de RXB0
              
            Serial.print(canMsg.can_id, HEX);                         // imprime ID
    Serial.print(" "); 
    Serial.print(canMsg.can_dlc, HEX);                                // imprime DLC
    Serial.print(" ");
    
    for (int i = 0; i<canMsg.can_dlc; i++)  {                          // imprime los datos
      Serial.print(canMsg.data[i],HEX);
      Serial.print(" ");
    }
    Serial.println();  
        }
            } 
 
 if (irq & MCP2515::CANINTF_RX1IF) {
            if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {      // Recibe los ensajes de RXB1 
              
            Serial.print(canMsg.can_id, HEX); 
    Serial.print(" "); 
    Serial.print(canMsg.can_dlc, HEX); 
    Serial.print(" ");
    
    for (int i = 0; i<canMsg.can_dlc; i++)  {  
      Serial.print(canMsg.data[i],HEX);
      Serial.print(" ");
    }   }
       Serial.println();
        }
delay(100);
mcp2515.sendMessage(&canMsg1);

   switch (canMsg.data[2]) {
   case 0x05:
    uint16_t Temp; 
    Temp = canMsg.data[3]-40;
    Serial.print("temp del liquido en ºC: ");
    Serial.println(Temp, DEC);
       if (Firebase.ready() )
  {
    Firebase.setFloat(fbdo, F("/PID/temperatura motor"), (Temp));
  }

    delay(20);
     canMsg1.data[2] = 0x0C;      //PID RPM
     mcp2515.sendMessage(&canMsg1);
     delay(50);
   break;
   case 0x0C:
   float RPM; 
    RPM = ((256 * canMsg.data[3]) + canMsg.data[4]) / 4;
     Serial.print("rpm: ");
          Serial.println(RPM, DEC);
    if (Firebase.ready())
  {
    Firebase.setFloat(fbdo, F("/PID/rpm"), (RPM - random(5)));
    delay (100);
    //Firebase.push(fbdo, F("/PID/rpm2"), (RPM ));
  }

          delay(20);
     canMsg1.data[2] = 0x0D;      //PID velocidad
     mcp2515.sendMessage(&canMsg1);
     delay(50);
   break;
   case 0x0D:
      int Vel; 
    Vel = canMsg.data[3];
    Serial.print("velocidad en km/h ");
    Serial.println(Vel);
        if (Firebase.ready())
  {
    Firebase.setFloat(fbdo, F("/PID/velocidad"), (Vel));
  }
    delay(20);
     canMsg1.data[2] = 0x0F;    //PID temperatura del colector de admisión
     mcp2515.sendMessage(&canMsg1);  
     delay(50);
   break;
   case 0x0F:
       uint16_t Temp1; 
    Temp1 = canMsg.data[3]-40;
    Serial.print("temp del colector de admision en ºC: ");
    Serial.println(Temp1, DEC);
        if (Firebase.ready())
  {
    Firebase.setFloat(fbdo, F("/PID/temperatura del colector"), (Temp1));
  }
    delay(20);
     canMsg1.data[2] = 0x10;    //PID Presión del combustible
     mcp2515.sendMessage(&canMsg1);  
     delay(50);
   break;
   case 0x10:
     uint16_t Pre; 
    Pre = 3 * canMsg.data[3];
    Serial.print("(maf): ");
          Serial.println(Pre, DEC);
        if (Firebase.ready())
  {
    Firebase.setFloat(fbdo, F("/PID/vel flujo de aire"), (Pre));
  }
          delay(20);
     canMsg1.data[2] = 0x05;    //PID temperatura del motor
     mcp2515.sendMessage(&canMsg1);  
     delay(50);
   break;
   }
  } 
}   
