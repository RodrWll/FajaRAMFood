#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

const char *ssid = "GOLDEN_NETWORK-2.4G";
const char *password = "bierko123";

// const char *ssid = "redpucp";
// const char *password = "C9AA28BA93";
String datosSerial = "";
WebServer server(80);
int direccion = 0;
int Mesa[4] = {0, 0, 0, 0};
int nroMesa = 0; // Esta será una variable auxiliar para guardar el numero de mesa que se esta usando
int CoincideMesas[4]={0,0, 0, 0};
int onMotor = 0;                // Solo habra un motor en la faja

// Codigo para un microcontrolador ESP32
// Se usaran 3 servomotores y 4 sensores PIR.
// Se usara un arreglo de 5 posiciones para representar la faja
//  0: posicion donde se coloca la faja
//  1: posicion del primer PIR en la faja para detectar un objeto
//  2: posicion del segundo PIR en la faja para detectar un objeto
//  3: posicion del tercer PIR en la faja para detectar un objeto
//  4: poscion final de la faja, donde debería quedarse en caso ningun brazo funcione
// Cada pir representa una mesa, y cada mesa tiene un numero de mesa.
// En este caso solo habran 3 mesas. la variable mesa[5] representa las mesas. En mesa[0] es donde se guardará la información donde se tiene que poner el plato. Es decir habra una variable direccion y sus posibles valores son 1,2,3 porque solo hay 3 mesas.
// Asignacion de pines:
// Motor

const int MOTOR = 2;//Cambiar pin
const int IN1 = 5; 
const int IN2 = 4;
const int MESA1 =32;
const int MESA2 = 33;
const int MESA3 = 25;
//const int pinMotorA[3] = { MOTOR, IN1, IN2 };
//const int waitTime = 2000;  //espera entre fases
const int speed = 150;    //velocidad de giro


int alerta = 0;
int errorMesa = 0;
// Instanciamos nuestro servo
int delayON=1000;
int delaySTOP = 1000;

/*void IRAM_ATTR pir1Interrupt()
{
    Serial.println("Se detecto un plato en el PIR1");
    SERVOM1.write(0);
    // Esperamos 1.5 segundos
    delay(1500);
    // Movemos el servo a 180 grados
    SERVOM1.write(90);
    // Esperamos 1.5 segundos
    delay(1500);
    if (PIR[1] == 1)
    {
        // Se detecto un plato, entonces el servo1 se debe mover
    }
}

*/
void setup()
{
    // Se configuran los pines de los PIR como entradas
    pinMode(MOTOR, OUTPUT);
    pinMode (IN1, OUTPUT);    // Input4 conectada al pin 4 
    pinMode (IN2, OUTPUT);
    pinMode (MESA1, OUTPUT);
    pinMode (MESA2, OUTPUT);
    pinMode (MESA3, OUTPUT);
    // Se configura el puerto serial
    Serial.begin(9600);
    delay(1000);
    // Conectar a la red WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Conectando a la red WiFi..");
    }
    // Imprimimos la direccion IP
    Serial.println("Conexión a la red WiFi establecida");
    Serial.print("Dirección IP: ");
    Serial.println(WiFi.localIP());
    // Configuramos las rutas del servidor
    server.on("/", HTTP_GET, handleRoot);
    // get si es que quiero saber el estado del motor
    // post si es que quiero cambiar el estado del motor
    server.on("/Motor", HTTP_GET, handleMotor);
    server.on("/Motor", HTTP_POST, handleMotor);
    // get para saber a que mesa se tiene que mover.
    // post para cambiar la mesa a la que se tiene que mover.
    server.on("/Mesa", HTTP_GET, handleMesa);
    server.on("/Mesa", HTTP_POST, handleMesa);

    // En caso el plato haya llegado al final de la faja, tenemos que detectarlo con el sesor PIR4 y tambien tenemos que conocer la mesa a la que le correspondia el plato.
    //  get para saber si el plato llego al final de la faja.
    // get para saber a que mesa le correspondia el plato.
    server.on("/ErrorFaja", HTTP_GET, handleFinal);

    // Iniciamos el servidor
    server.begin();
    Serial.println("Servidor web iniciado");
}
int listaTiempos[8]= {delayON,delayON,delayON,delayON,delayON,delayON,delayON,1.5*delayON};
int h=0;
void loop()
{

    // Manejar solicitudes del cliente
    server.handleClient();
    // Verificar si hay datos por el puerto serial
    while (Serial.available())
    {
        char c = Serial.read();
        if (c == '0' || c == '1')
        {
            datosSerial = c;
            // datosSerial += "\n";
            Serial.println(c);


            if(c=='1'){
              onMotor=1;
              setMotor(1);
            }else{
              onMotor=0;
              setMotor(0);
            }
            
        }else{

        }

        // Led();
    }
    if(onMotor){
      int delayActual= listaTiempos[h];     

      EncenderMotor();
      delay(delayActual);
      ApagarMotor();
      delay(delayActual);
      if(h>=7){
        h=0;
      }else{
        h++;
      }
      int aux1=0;
      int aux2=0;
      for(int i=0;i<4;i++){
        aux1=Mesa[i];
        if(i==0){
          Mesa[i]=0;

        }else{
          Mesa[i]= aux2;
        }
        
        aux2=aux1;

      }
      ShowValuesMesa();
      CheckCoincidencia();
      for(int j=3; j>0; j--){
        if(CoincideMesas[j]){
          switch(j){
          case 1:
            digitalWrite(MESA1, HIGH);

          break;
          
          case 2:
            digitalWrite(MESA2, HIGH);
        
          break;
          case 3:
            digitalWrite(MESA3, HIGH);
          break;      

          }
        
        }
      }
      delay(2000);
      digitalWrite(MESA1, LOW);
      digitalWrite(MESA2, LOW);
      digitalWrite(MESA3, LOW);

    }else{
      h=0;
    }



}

void handleRoot()
{
    // Enviar encabezado HTTP
    String html = "<html><head><style>";
    html += "h1 { color: blue; }";
    html += "p { font-size: 14px; }";
    html += "pre { background-color: #eee; padding: 10px; }";
    html += "form { margin-top: 20px; }";
    html += "label { font-weight: bold; }";
    html += "</style></head><body>";
    html += "<h1>RAMFood project</h1>";
    html += "<p>sus:</p>";
    html += "<p>Datos ingresados por el puerto serial:</p>";
    html += "<pre>" + datosSerial + "</pre>";
    html += "<br><br><form method='post' action='/Mesa'>";
    html += "<label for='nroMesa'>Indique el numero de mesa:</label><br>";
    html += "<input type='text' id='nroMesa' name='nroMesa'><br>";
    html += "<input type='submit' value='Enviar'>";
    html += "</form></body></html>";
    server.send(200, "text/html", html);

}

void handleMotor()
{
    if (server.method() == HTTP_GET)
    {
        // Enviar encabezado HTTP
        // Quiero que el cliente  consulte el dato de onMotor
        server.send(200, "text/plain", String(onMotor));
    }
    else if (server.method() == HTTP_POST)
    {
        String dato = server.arg("onMotor");
        if (dato == "1" || dato == "0")
        {
            Serial.println("Se ha cambiado el estado del motor");
            Serial.println(dato);
            onMotor = dato.toInt();
            server.send(200, "text/plain", "Dato recibido por la página web: " + dato);
            // Se enciende/apaga el motor
            setMotor(onMotor);
        }
        else
        {
            Serial.println("No se ha cambiado el estado del motor");
            Serial.println("Dato no valido.");
            Serial.println("Dato actual.");
            Serial.println(dato);
        }
    }
}

void handleMesa()
{
    if (server.method() == HTTP_GET)
    {
        // Enviar encabezado HTTP
        // Quiero que el cliente  consulte el dato de onMotor
        // server.send(200, "text/plain", String(mesa[0]));
        server.send(200, "text/plain", String(nroMesa));
    }
    else if (server.method() == HTTP_POST)
    {
        String dato = server.arg("nroMesa");
        if (dato == "1" || dato == "2" || dato == "3")
        {
            Serial.println("Se ha cambiado la mesa");
            Serial.println(dato);
            // mesa[0] = dato.toInt();
            nroMesa = dato.toInt();
            if (VerificaPrimerValorMesa())
            {
                Mesa[0] = nroMesa;
                ShowValuesMesa();
            }
            else
            {
                Serial.println("La faja esta ocupada");
                ShowValuesMesa();
            }
        }
        else
        {
            Serial.println("No se ha cambiado la mesa");
            Serial.println("Dato no valido.");
        }
    }
}
void handleFinal()
{
    // Solo sera un get
    if (server.method() == HTTP_GET)
    {
        // Enviar encabezado HTTP
        // Quiero que el cliente  consulte el dato de onMotor
        server.send(200, "text/plain", String(alerta));
        // Despues ver si se puede enviar a que mesa correspondia este plato
    }
}
//encender motor
void EncenderMotor(){
  digitalWrite (IN2, HIGH);
  digitalWrite (IN1, LOW);
  analogWrite (MOTOR, speed);
  //Enciende motor 
  //digitalWrite(MOTOR, HIGH);
  delay(delayON);
}
void ApagarMotor(){
  digitalWrite (IN1, LOW);
  digitalWrite (IN2, LOW); 
  analogWrite (MOTOR, LOW);
  delay(delaySTOP);
}



void setMotor(int onMotor)
{
    if (onMotor == 1)
    {
        EncenderMotor();
    }
    else
    {
        ApagarMotor();
    }
}


void ShowValuesMesa()
{
    Serial.println("Valores de la mesa:");
    for (int i = 0; i < 4; i++)
    {
        Serial.println(Mesa[i]);
    }
}
int BuscarPosicionProducto()
{
    int posicion;
    for (int posicion = 3; posicion < 0; posicion--)
    {
        if (Mesa[posicion] != 0)
        {
            return posicion;
            break;
        }
    }
    return 0;
}


int VerificaPrimerValorMesa()
{
    return Mesa[0] == 0; // Si esta vacio es porq no hay ningun plato a enviar al principio de la faja
}

void CheckCoincidencia()
{
    for (int i = 3; i >0; i--)
    {
        if (Mesa[i] == i)
        {
            CoincideMesas[i] =1 ;
        }else{
          CoincideMesas[i] =0;
        }
        
    }
}
