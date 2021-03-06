#include <DNSServer.h>
#include <ESPUI.h>
#include <WiFi.h>
#include <Preferences.h>
#include "time.h"
#include <Adafruit_MLX90614.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "MAX30100_PulseOximeter.h"
#include "FS.h"
#include "SPIFFS.h"
#include <ESPAsyncWebServer.h>

TaskHandle_t Tareacore0;
const byte DNS_PORT = 53;
IPAddress apIP( 192, 168, 1, 1 );
DNSServer dnsServer;
Preferences preference;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
double new_emissivity = 0.98;
PulseOximeter pox;
AsyncWebServer server(81);

#define ANALOG_PIN_0 34 //Define el pin donde medimos tension de bateria
#define POXLEDIC MAX30100_LED_CURR_20_8MA  //define la corriente de los leds de Max30100
#define LEDPULSO 2 //Define pin de led de destello de latido

float PPM=0;
int32_t SPO=0; //SPO2 value
int PPMAv = 0, SPOAv = 0; //stores the average BPM and SPO2 

//variables de configuracion del dispositivo
const char* hostname = "espui";
String usuario;
String password;
String ssid;
String pass;
String mqttserver;
String token;
String senstemp;
String senspuls;
unsigned long pmed;
unsigned long pguard;
uint16_t dtatempg;
uint16_t dtapulso;
uint16_t dtaspo;
unsigned long versionc;
uint16_t button1;
bool switchOne;
bool guarda=false;
bool valid=false;
bool spifs=true;
uint16_t cversion, cpulsostat, ctempstat, cwifistat, cntpstat, cmqttstat, csave, cleearch, cborrarch, cmensaj;
uint16_t pmedid, pguardid,dtatempgid,dtapulsoid,dtaspoid,graficarid,wifissidid,wifipassid,mqttserverid,tokenid,usuarioid,passwordid;
int graphIdTemp, graphIdPulso, graphIdSPO;
String wmode;
String horaloc;
String mensaje;
float valtemp=0;
float valtempamb=0;
float vcc=0;
static long oldTMed = 0;
static long oldTGuard = 0;
long tledpuls = 0;
const long  gmtOffset_sec = -3 * 60 * 60;
const int   daylightOffset_sec = 0;
const char* ntpServer = "pool.ntp.org";
bool SensorTemp = false;
bool SensorPulso = false;

WiFiClient espClient;
PubSubClient client(espClient);

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

//Describe eventos wifi
void WiFiEvent(WiFiEvent_t event){
    Serial.printf("[WiFi-event] event: %d\n", event);
    Serial.println("WiFiEvent Nucleo:" + String(xPortGetCoreID()));
    switch (event) {
        case ARDUINO_EVENT_WIFI_READY: 
            Serial.println("WiFi interface ready");
            break;
        case ARDUINO_EVENT_WIFI_SCAN_DONE:
            Serial.println("Completed scan for access points");
            break;
        case ARDUINO_EVENT_WIFI_STA_START:
            Serial.println("WiFi client started");
            break;
        case ARDUINO_EVENT_WIFI_STA_STOP:
            Serial.println("WiFi clients stopped");
            break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.println("Connected to access point");
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.println("Disconnected from WiFi access point");
            break;
        case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
            Serial.println("Authentication mode of access point has changed");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.print("Obtained IP address: ");
            Serial.println(WiFi.localIP());
            break;
        case ARDUINO_EVENT_WIFI_STA_LOST_IP:
            Serial.println("Lost IP address and IP address is reset to 0");
            break;
        case ARDUINO_EVENT_WPS_ER_SUCCESS:
            Serial.println("WiFi Protected Setup (WPS): succeeded in enrollee mode");
            break;
        case ARDUINO_EVENT_WPS_ER_FAILED:
            Serial.println("WiFi Protected Setup (WPS): failed in enrollee mode");
            break;
        case ARDUINO_EVENT_WPS_ER_TIMEOUT:
            Serial.println("WiFi Protected Setup (WPS): timeout in enrollee mode");
            break;
        case ARDUINO_EVENT_WPS_ER_PIN:
            Serial.println("WiFi Protected Setup (WPS): pin code in enrollee mode");
            break;
        case ARDUINO_EVENT_WIFI_AP_START:
            Serial.println("WiFi access point started");
            break;
        case ARDUINO_EVENT_WIFI_AP_STOP:
            Serial.println("WiFi access point  stopped");
            break;
        case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
            Serial.println("Client connected");
            break;
        case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
            Serial.println("Client disconnected");
            break;
        case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
            Serial.println("Assigned IP address to client");
            break;
        case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
            Serial.println("Received probe request");
            break;
        case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
            Serial.println("AP IPv6 is preferred");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
            Serial.println("STA IPv6 is preferred");
            break;
        case ARDUINO_EVENT_ETH_GOT_IP6:
            Serial.println("Ethernet IPv6 is preferred");
            break;
        case ARDUINO_EVENT_ETH_START:
            Serial.println("Ethernet started");
            break;
        case ARDUINO_EVENT_ETH_STOP:
            Serial.println("Ethernet stopped");
            break;
        case ARDUINO_EVENT_ETH_CONNECTED:
            Serial.println("Ethernet connected");
            break;
        case ARDUINO_EVENT_ETH_DISCONNECTED:
            Serial.println("Ethernet disconnected");
            break;
        case ARDUINO_EVENT_ETH_GOT_IP:
            Serial.println("Obtained IP address");
            break;
        default: break;
 
    }
 }

//Muestra en que modo esta el dispositivo para mostrarlo en la web
String modo_wifi(void){
  int wm= WiFi.getMode();
  String salida="";
  String ip = WiFi.localIP().toString().c_str();
  Serial.println("ModoWiFi Nucleo:" + String(xPortGetCoreID()));
    
  if( wm == WIFI_AP ){
    salida="Access Point:";
  }
  if ( wm == WIFI_STA ){
    salida="Cliente:";
  }
  if ( wm == WIFI_AP_STA ){
    salida="AP Cliente:";
  }
  if ( wm == WIFI_OFF ){
    salida="Apagado:";
  }
  salida+=ip;
  return salida;
}

//Guarda datos en filesystem flash
void guarda_datos(String timestamp ,int32_t spo, int32_t hr, int spoav, int hrav, float v, float temp){
  String dat = timestamp + ";" + String(spo) + ";" + String(hr) + ";"+ String(spoav) + ";"+ String(hrav) + ";"+ String(v) + ";"+ String(temp) + "\n";
  appendFile(SPIFFS, "/datos.txt", dat.c_str());
  Serial.print("Guarda Datos en datos.txt:" + dat);
}

//Envia datos por MQTT
void send_mqtt(int32_t spo, int32_t hr, int spoav, int hrav, float v, float temp){
  Serial.println("sendmqtt Nucleo:" + String(xPortGetCoreID()));
  if(!client.connected()) {
      ESPUI.updateControlValue( cmqttstat, "Desconectado." );
      Serial.print("Conectando ThingsBoard node ...");
      if ( client.connect("BrazaleteSmart01", token.c_str(), NULL) ) {
        Serial.println( "[OK]" );
        delay(1000);
      } else {
        Serial.print( "[ERROR] [ rc = " );
        Serial.println( client.state() );
      }
    }  
    if (client.connected()) {
      ESPUI.updateControlValue( cmqttstat, "Conectado." );
      String spo2 = String(spo);
      String pulso = String(hr);
      String promspo2 = String(spoav);
      String prompulso= String(hrav);
      String temperatura = String(temp);
      String vbat = String(v);
      
      String payload = "{";
      payload += "\"spo2\":";
      payload += spo2;
      payload += ",";
      payload += "\"pulso\":";
      payload += pulso;
      payload += ",";
      payload += "\"promspo2\":";
      payload += promspo2;
      payload += ",";
      payload += "\"prompulso\":";
      payload += prompulso;
      payload += ",";
      payload += "\"temperatura\":";
      payload += temperatura;
      payload += ",";
      payload += "\"vbat\":";
      payload += vbat;
      payload += "}";

      char attributes[200];
      payload.toCharArray( attributes, 200 );
      int rsus=client.publish( "v1/devices/me/telemetry", attributes );
      Serial.print( "Publish : ");
      Serial.println(rsus);
      Serial.println( attributes );
    }
  }

//Lee tension de bateria
float get_vcc(){
  Serial.println("getvcc Nucleo:" + String(xPortGetCoreID()));
  int steps=analogRead(ANALOG_PIN_0);
  Serial.print("Voltage Steps = ");
  Serial.println(steps);
  //float VBAT = 3.90f * float(steps) / 4096.0f *2;  // LiPo battery
  float VBAT = 4.91f * float(steps) * 4.35f /4096.0f ;  // LiPo battery
  
  return VBAT;
}

//Callback por latido destella led
void onBeatDetected()
{
    //Serial.println("Pulso! Nucleo:" + String(xPortGetCoreID()));
    digitalWrite(LEDPULSO,HIGH);
    tledpuls=millis();
}

//Otro hilo de ejecucion para asegurar muestreo de 100hz de Max30100
void correcore0( void * parameter) {
  Serial.println("Correcore0 Nucleo:" + String(xPortGetCoreID()));
  static long oldTMed = 0;
  while(true){
    if(SensorPulso){
      if ( millis() - oldTMed > 10 ) {
          pox.update();
      }
      if (millis() - tledpuls > 100){
        digitalWrite(LEDPULSO,LOW);
      }
      vTaskDelay(3);
    }else{
      vTaskDelay(200);
    }   
  }
}

//Pone conf por defecto
void conf_default(void) {
  preference.putString("usuario", "admin");
  preference.putString("password", "admin");
  preference.putString("ssid", "wifi");
  preference.putString("pass", "admin");
  preference.putString("mqttserver", "192.168.0.1");
  preference.putString("token", "0000000000000000");
  preference.putULong("pmed", 30);
  preference.putULong("pguard", 60);
  preference.putUInt("dtatempg", 20);
  preference.putUInt("dtapulso", 20);
  preference.putUInt("dtaspo", 10);
  preference.putULong("version", 1);
}

//Guarda Configuracion
void conf_save(void) {
  Serial.println("ConfSave Nucleo:" + String(xPortGetCoreID()));
  preference.putString("usuario",usuario );
  preference.putString("password", password);
  preference.putString("ssid", ssid);
  preference.putString("pass", pass);
  preference.putString("mqttserver", mqttserver);
  preference.putString("token", token);
  preference.putULong("pmed", pmed);
  preference.putULong("pguard", pguard);
  preference.putUInt("dtatempg", dtatempg);
  preference.putUInt("dtapulso", dtapulso);
  preference.putUInt("dtaspo", dtaspo);
  versionc++;
  preference.putULong("version", versionc);
}

//Carga Configuracion
bool get_conf(void) {
  preference.begin("configuracion", false);
  Serial.println("GetConf Nucleo:" + String(xPortGetCoreID()));
  Serial.print( "\nConfigurando .....\n" );
  if (preference.getUInt("version", 0) > 0) {
    usuario = preference.getString("usuario", "admin");
    password = preference.getString("password", "admin");
    ssid = preference.getString("ssid", "wifi");
    pass = preference.getString("pass", "secret");
    mqttserver = preference.getString("mqttserver", "192.168.0.1");
    token = preference.getString("token", "0000000000000000");
    pmed = preference.getULong("pmed", 30);
    pguard = preference.getULong("pguard", 60);
    dtatempg = preference.getUInt("dtatempg", 20);
    dtapulso = preference.getUInt("dtapulso", 20);
    dtaspo = preference.getUInt("dtaspo", 10);
    versionc = preference.getULong("version", 1);
    return true;
  } else {
    conf_default();
    Serial.print( "\nConfiguracion por DEFECTO.\n" );
    return false;
  }
}

//Muestra ids de controles web
void muestra_ids(){
  Serial.println("MuestraIds Nucleo:" + String(xPortGetCoreID()));
  Serial.println(" cversion:" + String(cversion) + " cpulsostat:" + String(cpulsostat) + " ctempstat:" + String(ctempstat) + " cwifistat:" + String(cwifistat) + " cntpstat:" + String(cntpstat) + " cmqttstat:" + String(cmqttstat) + " pmedid:" + String(pmedid) + " pguardid:" + String(pguardid) + " dtatempgid:" + String(dtatempgid) + " dtapulsoid:" + String(dtapulsoid) + " dtaspoid:" + String(dtaspoid) + " graficarid:" + String(graficarid) + " wifissidid:" + String(wifissidid) + " wifipassid:" + String(wifipassid) + " mqttserverid:" + String(mqttserverid) + " tokenid:" + String(tokenid) + " usuarioid:" +  String(usuarioid) + " passwordid:" + String(passwordid) + " graphIdTemp:" + String(graphIdTemp) + " graphIdPulso:" + String(graphIdPulso) + " graphIdSPO:" + String(graphIdSPO));
}

//Funciones que son llamdas por los controles web
void numero_func( Control* sender, int type ) {
  Serial.println("NumberCall Nucleo:" + String(xPortGetCoreID()));
  Serial.print("Num: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.print( sender->value );
  Serial.print(", Type: ");
  Serial.println( type );
  if(sender->id == pmedid){
    pmed=(sender->value).toInt();
  }
  if(sender->id == pguardid){
    pguard=(sender->value).toInt();
  }
}

void texto_func( Control* sender, int type ) {
  Serial.println("textCall Nucleo:" + String(xPortGetCoreID()));
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.print( sender->value );
  Serial.print(", Type: ");
  Serial.println( type );
  if(sender->id == wifissidid){
    ssid=sender->value;
  }
  if(sender->id == wifipassid){
    pass=sender->value;
  }
  if(sender->id == mqttserverid){
    mqttserver=sender->value;
  }
  if(sender->id == tokenid){
    token=sender->value;
  }
  if(sender->id == usuarioid){
    usuario=sender->value;
  }
  if(sender->id == passwordid){
    password=sender->value;
  }
}

void slider_func( Control* sender, int type ) {
  Serial.println("Slider_func Nucleo:" + String(xPortGetCoreID()));
  Serial.print("Slider: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.print( sender->value );
  Serial.print(", Type: ");
  Serial.println( type );
  if(sender->id == dtatempgid){
    dtatempg=(sender->value).toInt();
  }
  if(sender->id == dtapulsoid){
    dtapulso=(sender->value).toInt();
  }
  if(sender->id == dtaspoid){
    dtaspo=(sender->value).toInt();
  }
}

void boton_func( Control* sender, int type ) {
  Serial.println("boton_func Nucleo:" + String(xPortGetCoreID()));
  Serial.print("Boton: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.print( sender->value );
  Serial.print(", Type: ");
  Serial.println( type );
  switch ( type ) {
    case B_DOWN:
      Serial.println( "Boton DOWN" );
      conf_save();
      ESPUI.updateControlValue( cversion, String(versionc) );
      break;
    case B_UP:
      Serial.println( "Boton UP" );
      break;
  }
}

void borrarch_func( Control* sender, int type ) {
  Serial.println("borrarch_func Nucleo:" + String(xPortGetCoreID()));
  Serial.print("Boton: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.print( sender->value );
  Serial.print(", Type: ");
  Serial.println( type );
  switch ( type ) {
    case B_DOWN:
      Serial.println( "Borrach DOWN" );
      deleteFile(SPIFFS, "/datos.txt");
      conf_save();
      ESPUI.updateControlValue( cmensaj, "Archivo datos.txt eliminado." );
      break;
    case B_UP:
      Serial.println( "Borrarch UP" );
      break;
  }
}

void leearch_func( Control* sender, int type ) {
  Serial.println("leearch_func Nucleo:" + String(xPortGetCoreID()));
  Serial.print("Boton: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.print( sender->value );
  Serial.print(", Type: ");
  Serial.println( type );
  size_t len=0;
  String sal;
  switch ( type ) {
    case B_DOWN:
      Serial.println( "leearch DOWN" );
      len=sizeFile(SPIFFS, "/datos.txt");
      sal="Tama??o:" + String(len) +"bytes\nDescarga http://" + WiFi.localIP().toString() + ":81/datos.txt";
      Serial.println(sal);
      ESPUI.updateControlValue( cmensaj, sal );
      break;
    case B_UP:
      Serial.println( "leearch UP" );
      break;
  }
}

void switch_func( Control* sender, int value ) {
  Serial.println("switch_func Nucleo:" + String(xPortGetCoreID()));
  switch ( value ) {
    case S_ACTIVE:
      Serial.print( "Active:" );
      switchOne=true;
      break;
    case S_INACTIVE:
      Serial.print( "Inactive" );
      switchOne=false;
      break;
  }
  Serial.print( " " );
  Serial.println( sender->id );
}

//Formatea fecha para web
String tiempolocal(){
  Serial.println("tiempolocal Nucleo:" + String(xPortGetCoreID()));
  struct tm timeinfo;
  char salhora[23];
  String sh;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Error al obtener la fecha y hora!");
    return sh;
  }
  strftime(salhora,23, "%F %T", &timeinfo);
  sh=String(salhora);
  Serial.print("Fecha: ");
  Serial.println(sh);
  return sh;
}

//devuelve el tama??o del archivo de datos
size_t sizeFile(fs::FS &fs, const char * path){
    Serial.printf("Leyendo archivo: %s\n", path);
    size_t len =0;
    File file = fs.open(path);
    if(!file){
        Serial.println("Error abriendo archivo.");
        return len;
    }

    Serial.print("Archivo leido: ");
    len=file.size();
    file.close();
    return len;
}


//Agrega datos al archivo      
void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Agregando al archivo: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Error Abriendo el archivo!");
        return;
    }else{
      if(!file.isDirectory()){
        size_t len =0;
        len = file.size();
        Serial.print("Tama??o Archivo datos.txt:");
        Serial.println(len);
        if(len < 10000 ){   
          if(file.print(message)){
             Serial.println("Dato Agregado");
          } else {
             Serial.println("Error agregando Datos.");
          }
        }else{
          Serial.println("Error Archivo supera el tama??o permitido.");
        }
      }else{
        Serial.println("Error Es un directorio.");
      }
      file.close();
    }
}

//borra archivo de datos
void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("Archivo borrado.");
    } else {
        Serial.println("Error borrando archivo.");
    }
}

void setup( void ) {
  Serial.begin( 115200 );
  Serial.println("setup Nucleo:" + String(xPortGetCoreID()));
  WiFi.disconnect(true);
  delay(1000);
  pinMode(21,INPUT_PULLUP);
  pinMode(22,INPUT_PULLUP);
  pinMode(LEDPULSO,OUTPUT);
  //Wire.begin();
  //Wire.setClock(100000);
  //Monta filesystem flash
  if(!SPIFFS.begin()){
        Serial.println("Error al montar Filesystem.");
        spifs=false;
  }
  Serial.print("Estado Filesystem:");
  Serial.println(String(spifs));
  delay(500);
  //Inicializa sensor Max30100 y marca estado en bandera SensorPulso
  if (pox.begin()){
    SensorPulso=true;
    senspuls="Conectado";
    pox.setIRLedCurrent(POXLEDIC);
    pox.setOnBeatDetectedCallback(onBeatDetected);
    Serial.println("Sensor de PULSO Conectado.");
  }else{
    Serial.println("Error el Sensor de PULSO DESCONECTADO.");
    SensorPulso=false;
    senspuls="Desconectado";
  }
  delay(500);
  //Inicializa sensor mlx90614 y marca estado en bandera SensorTemp
  if (mlx.begin()) {
    SensorTemp=true;
    //mlx.writeEmissivity(new_emissivity);
    senstemp="Conectado";
    Serial.print("Sensor Temp Emisividad Temp TempAmb = ");
    Serial.println(mlx.readEmissivity());
    //Serial.print(mlx.readObjectTempC());Serial.println(mlx.readAmbientTempC());
  }else{
    Serial.println("Error el Sensor de Temperatura DESCONECTADO.");
    senstemp="Desconectado";
    SensorTemp=false;
  }
  delay(500);
  //Wire, I2C_SPEED_FAST
  //Configura adc para medir tension de bateria
  analogReadResolution(12); //12 bits
  analogSetAttenuation(ADC_11db);  //For all pins
  analogSetPinAttenuation(ANALOG_PIN_0, ADC_11db); //0db attenu
  delay(400);
  WiFi.onEvent(WiFiEvent);
  //carga configuracion
  bool sal = get_conf();
  if (! sal ) {
    Serial.println("No habia configuracion.");
  }
  //Inicia WIFI
  WiFi.setHostname( hostname );
  WiFi.begin( ssid.c_str(), pass.c_str());
  Serial.print( "\n\nTry to connect to existing network" );
  uint8_t timeout = 10;
  do {
    delay( 500 );
    Serial.print( "." );
    timeout--;
  } while ( timeout && WiFi.status() != WL_CONNECTED );
  if ( WiFi.status() != WL_CONNECTED ) {
    //No se pudo conectar a la se??al configurada inicia modo AP
    Serial.print( "\n\nCreando hotspot" );
    WiFi.mode( WIFI_AP );
    WiFi.softAPConfig( apIP, apIP, IPAddress( 255, 255, 255, 0 ) );
    WiFi.softAP("BrazaleteSmart"); //ssid.c_str()
    timeout = 5;
    do {
      delay( 500 );
      Serial.print( "." );
      timeout--;
    } while ( timeout );
  }
  dnsServer.start( DNS_PORT, "*", apIP );
  Serial.print( "\n\nWiFi parametros:" );
  wmode=modo_wifi();
  Serial.println( wmode );
  //Configura la fecha y hora por protocolo NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  horaloc=tiempolocal();
  //Configura cliente MQTT
  client.setServer(mqttserver.c_str(), 1884);
  //Arranca 2do hilo de ejecucion
  xTaskCreatePinnedToCore(correcore0,"TareaCore0",10000,NULL,1,&Tareacore0,1);
  //Configura 3 pesta??as en la interface web
  uint16_t tab1 = ESPUI.addControl( ControlType::Tab, "datos", "Datos" );
  uint16_t tab2 = ESPUI.addControl( ControlType::Tab, "conexion", "Conexion" );
  uint16_t tab3 = ESPUI.addControl( ControlType::Tab, "archivos", "Archivos" );
  Serial.print("tab1 :");
  Serial.print(tab1);
  Serial.print(" tab2 :");
  Serial.print(tab2);
  Serial.print(" tab3 :");
  Serial.println(tab3);
  //Agrega todos los controles a la interface web
  cversion = ESPUI.addControl( ControlType::Label, "Conf Version:", String(versionc), ControlColor::Turquoise );
  cpulsostat = ESPUI.addControl( ControlType::Label, "Sensor Pulso:", senspuls, ControlColor::Turquoise );
  ctempstat = ESPUI.addControl( ControlType::Label, "Sensor Temperatura:", senstemp, ControlColor::Turquoise );
  cwifistat = ESPUI.addControl( ControlType::Label, "Modo Wifi:", wmode, ControlColor::Turquoise );
  cntpstat = ESPUI.addControl( ControlType::Label, "Servidor NTP:", horaloc, ControlColor::Turquoise );
  cmqttstat = ESPUI.addControl( ControlType::Label, "Servidor MQTT", "Desconectado", ControlColor::Turquoise );
  csave = ESPUI.addControl(ControlType::Button, "Configuracion", "Guardar", ControlColor::Turquoise, Control::noParent,&boton_func );
  
  cborrarch = ESPUI.addControl(ControlType::Button, "Borrar Archivo", "Borrar", ControlColor::Turquoise, tab3,&borrarch_func );
  cleearch = ESPUI.addControl(ControlType::Button, "Leer Archivo", "Leer", ControlColor::Turquoise, tab3,&leearch_func );
  cmensaj = ESPUI.addControl( ControlType::Label, "Status:", mensaje, ControlColor::Turquoise, tab3 );
  
  dtatempgid=ESPUI.addControl( ControlType::Slider, "Dif Temp Guarda", String(dtatempg), ControlColor::Peterriver, tab1, &slider_func );
  dtapulsoid=ESPUI.addControl( ControlType::Slider, "Dif Pulso Guarda", String(dtapulso), ControlColor::Peterriver, tab1, &slider_func );
  dtaspoid=ESPUI.addControl( ControlType::Slider, "Dif SPO Guarda", String(dtaspo), ControlColor::Peterriver, tab1, &slider_func );
  pmedid=ESPUI.addControl( ControlType::Number, "Period Medicion:", String(pmed), ControlColor::Peterriver, tab1, &numero_func );
  pguardid=ESPUI.addControl( ControlType::Number, "Period Guardado:", String(pguard), ControlColor::Peterriver, tab1, &numero_func );
  graficarid=ESPUI.addControl( ControlType::Switcher, "Graficar", "", ControlColor::Peterriver, tab1, &switch_func );
 
  wifissidid=ESPUI.addControl( ControlType::Text, "Wifi SSID:", ssid, ControlColor::Emerald, tab2, &texto_func );
  wifipassid=ESPUI.addControl( ControlType::Text, "Wifi PASS:", pass, ControlColor::Emerald, tab2, &texto_func );
  mqttserverid=ESPUI.addControl( ControlType::Text, "MQTT Server:", mqttserver, ControlColor::Emerald, tab2, &texto_func );
  tokenid=ESPUI.addControl( ControlType::Text, "Token:", token, ControlColor::Emerald, tab2, &texto_func );
  usuarioid=ESPUI.addControl( ControlType::Text, "Usuario:", usuario, ControlColor::Emerald, tab2, &texto_func );
  passwordid=ESPUI.addControl( ControlType::Text, "Contrase??a:", password, ControlColor::Emerald, tab2, &texto_func );
  
  graphIdTemp = ESPUI.addControl( ControlType::Graph, "Temperatura", "", ControlColor::Peterriver,tab1);
  graphIdPulso = ESPUI.addControl( ControlType::Graph, "Pulsaciones", "", ControlColor::Peterriver,tab1);
  graphIdSPO = ESPUI.addControl( ControlType::Graph, "SPO", "", ControlColor::Peterriver,tab1);
  ESPUI.begin("Brazalete SMART", usuario.c_str(), password.c_str());
  muestra_ids();
  //Arranca un servidor web asincrono en puerto 81 para descarga de archivo de datos
  server.on("/datos.txt", HTTP_GET, [](AsyncWebServerRequest *request){
  Serial.println(" /datos.txt pedidos ...");
  AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/datos.txt", "text/plain", true);
  request->send(response);
});
  server.onNotFound(notFound);
  server.begin();
}

void loop( void ) {
  dnsServer.processNextRequest();
  static bool switchi = false;
  float ppmi=0;
  float valtempi=0;
  float valtempambi=0;
  float dtemp=0;
  float dppm=0;
  float dspo=0;
  float apa=0;
  int32_t spoi=0; //SPO2 value
  valid=false;
  guarda=false;
  //Conprueba periodo configurado de medicion  
  if ( millis() - oldTMed >  (pmed * 1000 )) {
    if(SensorTemp){
      //lee valores del sensor Temp
      valtempi=mlx.readObjectTempC();
      valtempambi=mlx.readAmbientTempC();
      if(!isnan(valtempi)){
        if(valtemp != 0){
          dtemp=abs(1-(valtempi/valtemp));
          if(dtemp > float(dtatempg)/100 ){
            Serial.print("Guarda TRUE TEMP");
            guarda=true;
          }
        }
        valtemp=valtempi;
      }
      if(!isnan(valtempambi)){
        valtempamb=valtempambi;
      }
      Serial.print("Ambiente = "); Serial.print(valtempamb);
      Serial.print("*C\tObjeto = "); Serial.print(valtemp); Serial.println("*C");
      if (switchOne){
        //
        ESPUI.addGraphPoint(graphIdTemp, valtemp);
      }
    }
    if(SensorPulso){
      ppmi=pox.getHeartRate();
      spoi=pox.getSpO2();
      if (ppmi < 255 && ppmi > 30){
        valid=true;
        if(PPM != 0){
           dppm=abs(1-(ppmi/PPM));
           if(dppm > float(dtapulso)/100 ){
             Serial.print("Guarda TRUE PPM");
             guarda=true;
           }
        }
        PPMAv=(PPMAv + ppmi)/2;
        PPM=ppmi;
      }
      if (spoi < 101 && spoi > 70){
        valid=true;
        if(SPO != 0){
          dspo=abs(1-(float(spoi)/float(SPO)));
          if(dspo > float(dtaspo)/100 ){
            Serial.print("Guarda TRUE SPO");
            guarda=true;
          }
        }
        SPOAv=(SPOAv + spoi)/2;
        SPO=spoi;
      }
      Serial.print("PPM:");
      Serial.print(ppmi);
      Serial.print(" / SPO:");
      Serial.print(spoi);
      Serial.println("%");
      if (switchOne){
        ESPUI.addGraphPoint(graphIdPulso, PPM);
        ESPUI.addGraphPoint(graphIdSPO, SPO);
      }
    }
    //Serial.println("loop Nucleo:" + String(xPortGetCoreID()));
    vcc=get_vcc();
    if (SPO != 0 || PPM != 0 || valtemp != 0){
       send_mqtt(SPO,PPM,SPOAv,PPMAv,vcc,valtemp);
    }
    if(!client.connected()) {
      Serial.print("Gurda ? tiempo:");
      Serial.print(millis() - oldTGuard);
      Serial.print(" dta:");
      Serial.println(guarda);
      if(((millis() - oldTGuard >  (pguard * 1000 )) || (guarda)) && spifs){
         horaloc=tiempolocal();
         guarda_datos(horaloc,SPO,PPM,SPOAv,PPMAv,vcc,valtemp);
         oldTGuard = millis();    
      }
      
    }
    vTaskDelay(40);
    oldTMed = millis();
  }else{
    vTaskDelay(200); 
  }
}
