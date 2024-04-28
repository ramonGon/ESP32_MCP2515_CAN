#include <SPI.h>
#include <mcp2515.h>

struct can_frame canMsg;
struct can_frame canMsg1;
volatile bool interrupt = 0;
const int boton = 4;            // se difine el Pin del pushbotton
int estadoBoton = 0;           // variable donde se guarda si el  boton ya fue precionado
int valor = 0;                  // varibla donde se guarda la entrada del boton
int val_old = 0;

MCP2515 mcp2515(2);

void setup() {
  while (!Serial);
  Serial.begin(115200);
  SPI.begin();
  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);

  
  Serial.println("------- CAN Read ----------");
  Serial.println("ID  DLC   DATA");

/*  mcp2515.setConfigMode();
  mcp2515.setFilterMask(MCP2515::MASK0, false, 0x7F0);
  mcp2515.setFilterMask(MCP2515::MASK1, false, 0x7F0); 
  mcp2515.setFilter(MCP2515::RXF0, false, 0x7E1);
    mcp2515.setFilter(MCP2515::RXF1, false, 0x7DF); */
  mcp2515.setNormalMode();
    attachInterrupt(digitalPinToInterrupt(21), irqHandler, FALLING);
}

void irqHandler() {
    interrupt = 1;
}

/* void loop() {
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
   // if (( canMsg.can_id  == 0x280 ) &&
//(canMsg.can_dlc == 8)) {
  Serial.print(canMsg.can_id, HEX); // print ID
    Serial.print(" "); 
    Serial.print(canMsg.can_dlc, HEX); // print DLC
    Serial.print(" ");
    
    for (int i = 0; i<canMsg.can_dlc; i++)  {  // print the data
      Serial.print(canMsg.data[i],HEX);
      Serial.print(" ");
    }
//}
    Serial.println();      
  }
} */

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

canMsg1.can_id = 0x7DF;   //ID para solicitar datos (protocolo OBDII)
canMsg1.can_dlc = 8;
canMsg1.data[0] = 0x02;   // bytes adicionales(2)
canMsg1.data[1] = 0x01;   //Modo 1 obd2
canMsg1.data[2] = 0x05;   //PID temperatura de motor
canMsg1.data[3] = 0x00;
canMsg1.data[4] = 0x00;
canMsg1.data[5] = 0x00;
canMsg1.data[6] = 0x00;
canMsg1.data[7] = 0x00;
  
    if (interrupt) {
      interrupt = 0;
        uint8_t irq = mcp2515.getInterrupts();

        if (irq & MCP2515::CANINTF_RX0IF) {
            if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {

                // frame contains received from RXB0 message
            Serial.print(canMsg.can_id, HEX); // print ID
    Serial.print(" "); 
    Serial.print(canMsg.can_dlc, HEX); // print DLC
    Serial.print(" ");
    
    for (int i = 0; i<canMsg.can_dlc; i++)  {  // print the data
      Serial.print(canMsg.data[i],HEX);
      Serial.print(" ");
    } }
        Serial.println();  
        }

        if (irq & MCP2515::CANINTF_RX1IF) {
            if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
                // frame contains received from RXB1 message
            Serial.print(canMsg.can_id, HEX); // print ID
    Serial.print(" "); 
    Serial.print(canMsg.can_dlc, HEX); // print DLC
    Serial.print(" ");
    
    for (int i = 0; i<canMsg.can_dlc; i++)  {  // print the data
      Serial.print(canMsg.data[i],HEX);
      Serial.print(" ");
    }}
       Serial.println();
        }
delay(50);
mcp2515.sendMessage(&canMsg1);
delay(50);
         
 /* if (canMsg.data[2] = 0x05) {
     canMsg1.data[2] = 0x0C;      //PID RPM
     mcp2515.sendMessage(&canMsg1);
     delay(50);
  }
  if (canMsg.data[2]=0x0C) {
     canMsg1.data[2] = 0x0D;      //PID velocidad
     mcp2515.sendMessage(&canMsg1);
     delay(50);
  }
  if (canMsg.data[2]=0x0D) {
     canMsg1.data[2] = 0x0F;    //PID temperatura del colector de admisión
     mcp2515.sendMessage(&canMsg1);  
    delay(50);
    }
   if (canMsg.data[2]=0x0F) {
     canMsg1.data[2] = 0x10;    //PID MAF velocidad del flujo de aire
     mcp2515.sendMessage(&canMsg1);  
    delay(50);
  }
    if (canMsg.data[2]=0x0F) {
     canMsg1.data[2] = 0x10;    //PID MAF velocidad del flujo de aire
     mcp2515.sendMessage(&canMsg1);  
    delay(50);
  }
    if (canMsg.data[2]=0x10) {
     canMsg1.data[2] = 0x05;    //PID temperatura del motor
     mcp2515.sendMessage(&canMsg1);  
    delay(50);
  } */
  
   switch (canMsg.data[2]) {
   case 0x05:
    uint16_t Temp; 
    Temp = canMsg.data[3]-40;
    Serial.print("temp del liquido en ºC: ");
    Serial.println(Temp, DEC);
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
          delay(20);
     canMsg1.data[2] = 0x0D;      //PID velocidad
     mcp2515.sendMessage(&canMsg1);
     delay(50);
   break;
   case 0x0D:
       uint16_t Vel; 
    Vel = canMsg.data[3];
    Serial.print("velocidad en km/h ");
    Serial.println(Temp, DEC);
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
    delay(20);
     canMsg1.data[2] = 0x10;    //PID MAF velocidad del flujo de aire
     mcp2515.sendMessage(&canMsg1);  
     delay(50);
   break;
   case 0x10:
     uint16_t MAF; 
    MAF = ((256 * canMsg.data[3]) + canMsg.data[4]) / 100;
    Serial.print("(maf): ");
          Serial.println(MAF, DEC);
          delay(20);
     canMsg1.data[2] = 0x05;    //PID temperatura del motor
     mcp2515.sendMessage(&canMsg1);  
     delay(50);
   break;
   } 
 }
}
