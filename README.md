Codigos usados en mi proyecto final de la Universidad.
Se uso la placa ESP32 en conjunto con el modulo MCP2515 (Aunque el ESP32 cuenta ya con un controlador CAN para unicamente agregar un transceptorCAN. Al realizar las pruebas con la librería 
"ESP32CAN.h" no hubo respuesta).
Se ocupó la librería mcp2515.h para recibir mensaje de la red CAN del automovil.
Se realizó in codigo para solicitar informacion al vehiculo, como se especifica en la norma de la SAE, para luego alamacenar los datos en una base de datos en 
"Firebase" la cual se actualiza en tiempo real.
