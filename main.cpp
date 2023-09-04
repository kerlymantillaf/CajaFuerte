#include <Arduino.h>
#include <LiquidCrystal_I2C.h> // Incluir la libreria LiquidCrystal_I2C
#include <Keypad.h>// importa libreria Keypad
#include <ESP32Servo.h>//Agregamos las libreriras del servo
#include <Wire.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
// Wifi network station credentials
#define WIFI_SSID "Netlife-Home Plus"//"demonstoke"//"Microcontroladores"//"FIEC-AUTO"//"iPhone LB"//
#define WIFI_PASSWORD "0955682307"//"bob12345"//"raspy123"//"FIEC-c4r2021"//"040597luis"//
// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "5772149955:AAEOTIgI1e9DKbj5io_roD64VYS2o3zoKa8"

const unsigned long BOT_MTBS = 1000; // mean time between scan messages

int ledAmarillo = 14; // Pin del LED amarillo
int ledPin = 27; //pin del LED rojo
int buzzer = 26; 
//unsigned long ledStartTime = 0; // Tiempo de inicio del LED
//const unsigned long ledDuration = 180000; // 3 minutos en milisegundos

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime;          // last time messages' scan has been done
bool Start = false;
/* String nombreClave;  */
String chat_id;
String text;
bool activeChat = false;
int numProcess = 0;

Servo servo;//Instanciamos nuestro servo
int pinServo=13; //Declaramos el pin del servomotor

const uint8_t ROWS = 4; // define numero de filas
const uint8_t COLS = 4; // define numero de columnas

char keys[ROWS][COLS] = { // define la distribucion de teclas
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

uint8_t colPins[COLS] = { 16, 4, 2, 15 }; // pines correspondientes a las filas
uint8_t rowPins[ROWS] = { 12, 18, 5, 17 }; // pines correspondientes a las columnas

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS); // crea objeto con los prametros creados previamente
String pass="999"; //Password que se debe ingresar
String newNum="1234";
int numErrores=0;
String passIngresado; //Pasword que se esta ingresando en el teclado


#pragma GCC optimize ("O3")   //code optimisation controls - "O2" & "O3" code performance, "Os" code size

//LCD
#define COLUMS           16   //LCD columns
#define ROWSLCD             2    //LCD rows
#define LCD_SPACE_SYMBOL 0x20 //space symbol from LCD ROM, see p.9 of GDM2004D datasheet

LiquidCrystal_I2C lcd(PCF8574_ADDR_A21_A11_A01, 4, 5, 6, 16, 11, 12, 13, 14, POSITIVE);




void handleNewMessages(int numNewMessages)
{
  Serial.println("manejarNuevosMensajes");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "")// Si el remitente no tiene un nombre, se establece como "Guest" (invitado).
      {
        from_name = "Guest";
      }
      /**voy a crear codigo de prueba aqui**/
      Serial.println(numProcess + "numproceso");


      if(numProcess == 0){
          if(text == "/abrir"){
              bot.sendChatAction(chat_id, "typing");
              bot.sendMessage(chat_id, "Ingresa el nombre clave:");
              numProcess = 1;
             
            }else{
                    String welcome = "Bienvenido(a), " + from_name + ".\n";
                    welcome += "Me llamo KeyGuardian. Soy tu Bot para abrir la caja fuerte.\n\n";
                    welcome+="/abrir: ingresa este comando para iniciar el proceso de abrir la caja fuerte\n";
                    bot.sendMessage(chat_id, welcome);
                    pass=random(1000, 9999);

            }
      }
      else if (numProcess == 1)
      {
          if (text =="kermanfie")
            {
              bot.sendChatAction(chat_id, "typing");
              delay(1000);
              bot.sendMessage(chat_id, "Ingresa el numero clave:");
              numProcess = 2;
            }
            else
              {
                bot.sendMessage(chat_id, "Nombre Incorrecto. Intenta nuevamente");
              }
      }
      else if (numProcess == 2)
      {
        if (text == newNum) // Replace "1234" with the actual secret number
        {
          bot.sendChatAction(chat_id, "typing");
          delay(3000);
          bot.sendMessage(chat_id, "¿Deseas cambiar el numero clave?");
          numProcess=3;       
        }    
        else
        {
          bot.sendMessage(chat_id, "Numero clave incorrecto. Intenta nuevamente.");         
        }
      }

      else if(numProcess == 3){       
        if (text=="si"){
          bot.sendMessage(chat_id, "Registra el NUEVO numero clave:");
          delay(2000);
          numProcess=4;
        }
        else{
          bot.sendMessage(chat_id, "Acceso concedido!!");
          delay(3000);
          int randomCode = random(1000, 9999); 
          pass=randomCode;
          bot.sendMessage(chat_id, "Abre a la caja fuerte con el numero: " +  String(pass));
          bot.sendMessage(chat_id, "Solo tienes 3 minutos!!" );
          delay(60000);//1 min para demostración                          
          pass = randomCode;
          bot.sendMessage(chat_id, "Se acabo el tiempo." );
          numProcess = 0; 
        }              

      }

      else if(numProcess == 4){ //Cambiar el numero clave
        newNum=text;
        bot.sendMessage(chat_id, "Ingresa el NUEVO numero clave:");
        numProcess = 2;   
      }

     else{}
     

  }
}

void setup() {
  Serial.begin(115200);
  while (lcd.begin(COLUMS, ROWSLCD, LCD_5x8DOTS) != 1) //colums, rows, characters size
  {
    Serial.println(F("PCF8574 is not connected or lcd pins declaration is wrong. Only pins numbers: 4,5,6,16,11,12,13,14 are legal."));
    delay(5000);   
  }
  //Inicializamos la posicion del servo
  servo.attach(pinServo, 500, 2500);
  servo.write(0);
  
  Serial.println();

  // attempt to connect to Wifi network:
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(5000);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("Retrieving time: ");
  configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
  time_t now = time(nullptr);
  while (now < 24 * 3600)
  {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println(now);

  pinMode(ledPin, OUTPUT); // Configura el pin del LED como salida
  digitalWrite(ledPin, LOW); // Apaga el LED al inicio

  pinMode(ledAmarillo, OUTPUT); // Configura el pin del LED como salida
  digitalWrite(ledAmarillo, LOW); // Apaga el LED al inicio

  pinMode(buzzer, OUTPUT); // Configura el pin del LED como salida
  digitalWrite(buzzer, LOW); // Apaga el LED al inicio



}

void loop() {
  //Alamacenamos la tecla que se preciono
  //keypad.waitForKey();
  char key=keypad.getKey();
  
      //Serial.println(key + "tecla inicio");
  //Si se preciono una tecla
  if(key){


    //Serial.println(numErrores);
    //Si la tecla es entre 0 y 9
    if(key>='0' && key<='9' ){
      Serial.println("hola");
      //Agregamos esa tecla al string
      passIngresado=passIngresado+key;
      Serial.println(passIngresado);
      lcd.print(key);         // envia al LCD la tecla presionada
      delay(30);
      
    }
    //Si precionamos las teclas A B C D * #
    else if(key>='A' || key<='B'  || key<='C'  || key<='D'  || key<='*' || key<='#'  ){
      //Si el texto ingresado tiene texto
      if(passIngresado.length()>0){
        //Si el texto ingresado es igual al pass
        if( passIngresado==pass ){
          lcd.clear( ); 
          Serial.println(" Abrir");
          lcd.print("Abierto");  // imprime en el LCD que esta abierta
          digitalWrite(ledAmarillo, HIGH);
          //Abrimos el servomotor
          servo.write(90);
          delay(10000);
          //Cerramos el servomotor
          servo.write(0);
          Serial.println(" Cerrar");
          digitalWrite(ledAmarillo, LOW);
          lcd.clear( ); 
          lcd.print("Cerrado");
          delay(2000);
          lcd.clear( );
          numErrores=0;
        }
     
        else if(numErrores==2){
          // Encendemos el LED y apagamos despues de "3min"
          digitalWrite(ledPin, HIGH);
          digitalWrite(buzzer,HIGH);
          lcd.clear( );
          lcd.print("ALARMA ENCENDIDA");         
          delay(5000);
          digitalWrite(ledPin, LOW);
          digitalWrite(buzzer, LOW);
          lcd.clear( );
          numErrores=0;
        }
        

        //Si la contraseña es incorrecta
        else{
          Serial.println("Acceso denegado");
          numErrores++;
          lcd.clear( );
          lcd.print("Acceso denegado");
          //Cerramos el servomotor
          servo.write(0);
          delay(2000);
          lcd.clear( ); 
          lcd.print("Tienes "+ String(3-numErrores) + " intentos");
          delay(2000);//para que se muestre el mensaje y no se borre rapido
          lcd.clear( );
        }
        //Reseteamos la contraseña ingresada
        passIngresado="";
      }
      else{
        Serial.println(" Acceso denegado");
        lcd.clear( );
        lcd.print("Acceso denegado");
        //Cerramos el servomotor
        servo.write(0);
        delay(2000);
        lcd.clear( ); 
      }
    }

}

    
    if (millis() - bot_lasttime > BOT_MTBS)
    {
      int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      while (numNewMessages)
      {
        Serial.println("obtuve respuesta");
        handleNewMessages(numNewMessages);
              Serial.println(numNewMessages);

        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        Serial.println(newNum);
        Serial.println(pass);

      }

      bot_lasttime = millis();
    }


}

//-------------------LCD---------------------------
//SDA 23
//SCL 22
//-----------------TECLADO-------------------------
//16, 4, 2, 15 // pines correspondientes a las filas
//12, 18, 5, 17 //COLUMNAS

//BUZZER PIN 26
//LED VERDE PIN 14
//LED AMARILLO PIN 27
