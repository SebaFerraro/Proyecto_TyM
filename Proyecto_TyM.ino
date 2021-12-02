#include <DNSServer.h>
#include <ESPUI.h>
#include <WiFi.h>
#include <Preferences.h>
#include "time.h"
#include <Adafruit_MLX90614.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "MAX30100_PulseOximeter.h"
#include <Wire.h>

TaskHandle_t Tareacore0;
const byte DNS_PORT = 53;
IPAddress apIP( 192, 168, 1, 1 );
DNSServer dnsServer;
Preferences preference;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
double new_emissivity = 0.98;
PulseOximeter pox;

#define ANALOG_PIN_0 34 
#define REPORTING_PERIOD_TEMP_MS     10000
#define REPORTING_PERIOD_PULS_MS     2000
#define IRLEDCURR  MAX30100_LED_CURR_11MA


float beatsPerMinute=0;
int8_t spo2=0; //SPO2 value
int8_t sp02Avg = 0;
float beatAvg = 0  ;


//const char* ssid = "wifi";
//const char* password = "secret1703secret1703";
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
uint16_t cversion, cpulsostat, ctempstat, cwifistat, cntpstat, cmqttstat, csave;
uint16_t pmedid, pguardid,dtatempgid,dtapulsoid,dtaspoid,graficarid,wifissidid,wifipassid,mqttserverid,tokenid,usuarioid,passwordid;
int graphIdTemp, graphIdPulso, graphIdSPO;
String wmode;
String horaloc;
float valtemp=0;
float valtempamb=0;
float vcc=0;

const long  gmtOffset_sec = -3 * 60 * 60;
const int   daylightOffset_sec = 0;
const char* ntpServer = "pool.ntp.org";
bool SensorTemp = false;
bool SensorPulso = false;

WiFiClient espClient;
PubSubClient client(espClient);

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
    }}

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

void onBeatDetected()
{
   Serial.println("Pulso ! " + String(xPortGetCoreID()));
}

void send_mqtt(int32_t spo, int32_t hr, int spoav, int hrav, float v, float temp){
  Serial.println("sendmqtt Nucleo:" + String(xPortGetCoreID()));
  if(!client.connected()) {
      ESPUI.updateControlValue( cmqttstat, "Desconectado." );
      Serial.print("Conectando ThingsBoard node ...");
      if ( client.connect("BrazaleteSmart01", token.c_str(), NULL) ) {
        Serial.println( "[DONE]" );
        delay(1000);
      } else {
        Serial.print( "[FAILED] [ rc = " );
        Serial.print( client.state() );
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

      // Send payload
      char attributes[200];
      payload.toCharArray( attributes, 200 );
      int rsus=client.publish( "v1/devices/me/telemetry", attributes );
      Serial.print( "Publish : ");
      Serial.println(rsus);
      //client.publish(TEMP_TOPIC, msg);
      Serial.println( attributes );
    }
  }


float get_vcc(){
  Serial.println("getvcc Nucleo:" + String(xPortGetCoreID()));
  int steps=analogRead(ANALOG_PIN_0);
  Serial.print("Voltage Steps = ");
  Serial.println(steps);
  float VBAT = 3.90f * float(steps) / 4096.0f *2;  // LiPo battery
  return VBAT;
}


void correcore0( void * parameter) {
  Serial.println("Correcore0 Nucleo:" + String(xPortGetCoreID()));
  static long tsLastReport = 0;
  static long oldTMed = 0;
  int avail=0;
  bool buclepuls=true;
  while(true){
     if(SensorPulso){
        while(buclepuls){
          pox.update();
            if (millis() - tsLastReport > REPORTING_PERIOD_PULS_MS) {
               float bPMt=pox.getHeartRate();
               int spot=pox.getSpO2();
               Serial.print("Heart rate:");
               Serial.print(bPMt);
               Serial.print("bpm / SpO2:");
               Serial.print(spot);
               Serial.println("%");
               buclepuls=false;
               if(spot < 100 && spot > 70){
                  spo2 = spot;
                  sp02Avg = (sp02Avg + spo2)/2;
               }
               if(bPMt < 255 && bPMt > 38){
                  beatsPerMinute=bPMt;
                  beatAvg = (beatAvg + beatsPerMinute)/2  ;
               }  
               tsLastReport = millis();
            }
        }
     }
     if(SensorTemp){
         if ( millis() - oldTMed > REPORTING_PERIOD_TEMP_MS ) {
           //Serial.print("Sensor Temp Emisividad = "); Serial.println(mlx.readEmissivity());
           valtemp=mlx.readObjectTempC();
           valtempamb=mlx.readAmbientTempC();
           oldTMed = millis();
         }
     }
    buclepuls=true;
    vTaskDelay(10);
  }
}

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

//String usuario,String password,String ssid,String pass,String mqttserver,String token,unsigned long pmed,unsigned long pguard,uint16_t dtatempg,uint16_t dtapulso,uint16_t dtaspo,unsigned long version
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

bool get_conf(void) {
  preference.begin("configuracion", false);
  Serial.println("GetConf Nucleo:" + String(xPortGetCoreID()));
    
  Serial.print( "\nConfigurando .....\n" );
  
  if (preference.getUInt("version", 0) > 0) {
    usuario = preference.getString("usuario", "admin");
    password = preference.getString("password", "admin");
    ssid = preference.getString("ssid", "wifi");
    pass = preference.getString("pass", "secret1703secret1703");
    mqttserver = preference.getString("mqttserver", "192.168.0.1");
    token = preference.getString("token", "0000000000000000");
    pmed = preference.getULong("pmed", 30);
    pguard = preference.getULong("pguard", 60);
    dtatempg = preference.getUInt("dtatempg", 20);
    dtapulso = preference.getUInt("dtapulso", 20);
    dtaspo = preference.getUInt("dtaspo", 10);
    versionc = preference.getULong("version", 1);
    //Serial.println(usuario);
    //Serial.println(password);
    //Serial.println(ssid);
    //Serial.println(pass);
    return true;
  } else {
    conf_default();
    Serial.print( "\nConfiguracion por DEFECTO.\n" );
    return false;
  }
}

void muestra_ids(){
  Serial.println("MuestraIds Nucleo:" + String(xPortGetCoreID()));
  Serial.println(" cversion:" + String(cversion) + " cpulsostat:" + String(cpulsostat) + " ctempstat:" + String(ctempstat) + " cwifistat:" + String(cwifistat) + " cntpstat:" + String(cntpstat) + " cmqttstat:" + String(cmqttstat) + " pmedid:" + String(pmedid) + " pguardid:" + String(pguardid) + " dtatempgid:" + String(dtatempgid) + " dtapulsoid:" + String(dtapulsoid) + " dtaspoid:" + String(dtaspoid) + " graficarid:" + String(graficarid) + " wifissidid:" + String(wifissidid) + " wifipassid:" + String(wifipassid) + " mqttserverid:" + String(mqttserverid) + " tokenid:" + String(tokenid) + " usuarioid:" +  String(usuarioid) + " passwordid:" + String(passwordid) + " graphIdTemp:" + String(graphIdTemp) + " graphIdPulso:" + String(graphIdPulso) + " graphIdSPO:" + String(graphIdSPO));
}

void numberCall( Control* sender, int type ) {
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

void textCall( Control* sender, int type ) {
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

void slider( Control* sender, int type ) {
  Serial.println("Slider Nucleo:" + String(xPortGetCoreID()));
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

void buttonCallback( Control* sender, int type ) {
  Serial.println("buttonCallBack Nucleo:" + String(xPortGetCoreID()));
  Serial.print("Button: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.print( sender->value );
  Serial.print(", Type: ");
  Serial.println( type );
  switch ( type ) {
    case B_DOWN:
      Serial.println( "Button DOWN" );
      conf_save();
      ESPUI.updateControlValue( cversion, String(versionc) );
      break;

    case B_UP:
      Serial.println( "Button UP" );
      break;
  }
}

void switchExample( Control* sender, int value ) {
  Serial.println("SwitchExample Nucleo:" + String(xPortGetCoreID()));
  switch ( value ) {
    case S_ACTIVE:
      Serial.print( "Active:" );
      break;

    case S_INACTIVE:
      Serial.print( "Inactive" );
      break;
  }

  Serial.print( " " );
  Serial.println( sender->id );
}

void otherSwitchExample( Control* sender, int value ) {
  Serial.println("otherSwitchexample Nucleo:" + String(xPortGetCoreID()));
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


void setup( void ) {
  Serial.begin( 115200 );
  Serial.println("setup Nucleo:" + String(xPortGetCoreID()));
  WiFi.disconnect(true);
  delay(1000);
  pinMode(21,INPUT_PULLUP);
  pinMode(22,INPUT_PULLUP);
  delay(500);
  if (mlx.begin()) {
    SensorTemp=true;
    //mlx.writeEmissivity(new_emissivity);
    senstemp="Conectado";
    Serial.print("Sensor Temp Emisividad Temp TempAmb = "); Serial.print(mlx.readEmissivity());//Serial.print(mlx.readObjectTempC());Serial.println(mlx.readAmbientTempC());
  }else{
    Serial.println("Error el Sensor de Temperatura DESCONECTADO.");
    senstemp="Desconectado";
    SensorTemp=false;
  }
  delay(500);
  //Wire, I2C_SPEED_FAST
  if (pox.begin()){
    SensorPulso=true;
    senspuls="Conectado";
    pox.setIRLedCurrent(IRLEDCURR);
    pox.setOnBeatDetectedCallback(onBeatDetected);
    Serial.println("Sensor de PULSO Conectado.");
  }else{
    Serial.println("Error el Sensor de PULSO DESCONECTADO.");
    SensorPulso=false;
    senspuls="Desconectado";
  }
  delay(500);
  
  analogReadResolution(12); //12 bits
  analogSetAttenuation(ADC_11db);  //For all pins
  analogSetPinAttenuation(ANALOG_PIN_0, ADC_11db); //0db attenu
  delay(400);
  
  WiFi.onEvent(WiFiEvent);
  bool sal = get_conf();
  if (! sal ) {
    Serial.println("No habia configuracion.");
  }
  WiFi.setHostname( hostname );
  // try to connect to existing network
  WiFi.begin( ssid.c_str(), pass.c_str());
  Serial.print( "\n\nTry to connect to existing network" );

  uint8_t timeout = 10;

    // Wait for connection, 5s timeout
    do {
      delay( 500 );
      Serial.print( "." );
      timeout--;
    } while ( timeout && WiFi.status() != WL_CONNECTED );

    // not connected -> create hotspot
    if ( WiFi.status() != WL_CONNECTED ) {
      Serial.print( "\n\nCreating hotspot" );

      WiFi.mode( WIFI_AP );
      WiFi.softAPConfig( apIP, apIP, IPAddress( 255, 255, 255, 0 ) );
      WiFi.softAP( ssid.c_str() );

      timeout = 5;

      do {
        delay( 500 );
        Serial.print( "." );
        timeout--;
      } while ( timeout );
    }
  

  dnsServer.start( DNS_PORT, "*", apIP );

  Serial.print( "\n\nWiFi parameters:" );
  wmode=modo_wifi();
  Serial.println( wmode );
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  horaloc=tiempolocal();
  client.setServer(mqttserver.c_str(), 1884);
  
  xTaskCreatePinnedToCore(correcore0,"TareaCore0",10000,NULL,1,&Tareacore0,0);
  
  uint16_t tab1 = ESPUI.addControl( ControlType::Tab, "datos", "Datos" );
  uint16_t tab2 = ESPUI.addControl( ControlType::Tab, "conexion", "Conexion" );
  Serial.print("tab1 :");
  Serial.print(tab1);
  Serial.print(" tab2 :");
  Serial.println(tab2);
   
  // shown above all tabs
  cversion = ESPUI.addControl( ControlType::Label, "Conf Version:", String(versionc), ControlColor::Turquoise );
  cpulsostat = ESPUI.addControl( ControlType::Label, "Sensor Pulso:", senspuls, ControlColor::Turquoise );
  ctempstat = ESPUI.addControl( ControlType::Label, "Sensor Temperatura:", senstemp, ControlColor::Turquoise );
  cwifistat = ESPUI.addControl( ControlType::Label, "Modo Wifi:", wmode, ControlColor::Turquoise );
  cntpstat = ESPUI.addControl( ControlType::Label, "Servidor NTP:", horaloc, ControlColor::Turquoise );
  cmqttstat = ESPUI.addControl( ControlType::Label, "Servidor MQTT", "Desconectado", ControlColor::Turquoise );
  csave = ESPUI.addControl(ControlType::Button, "Configuracion", "Guardar", ControlColor::Turquoise, Control::noParent,&buttonCallback );
  
  dtatempgid=ESPUI.addControl( ControlType::Slider, "Dif Temp Guarda", String(dtatempg), ControlColor::Peterriver, tab1, &slider );
  dtapulsoid=ESPUI.addControl( ControlType::Slider, "Dif Pulso Guarda", String(dtapulso), ControlColor::Peterriver, tab1, &slider );
  dtaspoid=ESPUI.addControl( ControlType::Slider, "Dif SPO Guarda", String(dtaspo), ControlColor::Peterriver, tab1, &slider );
  pmedid=ESPUI.addControl( ControlType::Number, "Period Medicion:", String(pmed), ControlColor::Peterriver, tab1, &numberCall );
  pguardid=ESPUI.addControl( ControlType::Number, "Period Guardado:", String(pguard), ControlColor::Peterriver, tab1, &numberCall );
  graficarid=ESPUI.addControl( ControlType::Switcher, "Graficar", "", ControlColor::Peterriver, tab1, &otherSwitchExample );
 
  wifissidid=ESPUI.addControl( ControlType::Text, "Wifi SSID:", ssid, ControlColor::Emerald, tab2, &textCall );
  wifipassid=ESPUI.addControl( ControlType::Text, "Wifi PASS:", pass, ControlColor::Emerald, tab2, &textCall );
  mqttserverid=ESPUI.addControl( ControlType::Text, "MQTT Server:", mqttserver, ControlColor::Emerald, tab2, &textCall );
  tokenid=ESPUI.addControl( ControlType::Text, "Token:", token, ControlColor::Emerald, tab2, &textCall );
  usuarioid=ESPUI.addControl( ControlType::Text, "Usuario:", usuario, ControlColor::Emerald, tab2, &textCall );
  passwordid=ESPUI.addControl( ControlType::Text, "Contraseña:", password, ControlColor::Emerald, tab2, &textCall );
  
  graphIdTemp = ESPUI.addControl( ControlType::Graph, "Temperatura", "", ControlColor::Peterriver,tab1);
  graphIdPulso = ESPUI.addControl( ControlType::Graph, "Pulsaciones", "", ControlColor::Peterriver,tab1);
  graphIdSPO = ESPUI.addControl( ControlType::Graph, "SPO", "", ControlColor::Peterriver,tab1);

   
 
  /*
     .begin loads and serves all files from PROGMEM directly.
     If you want to serve the files from SPIFFS use ESPUI.beginSPIFFS
     (.prepareFileSystem has to be run in an empty sketch before)
  */

  // Enable this option if you want sliders to be continuous (update during move) and not discrete (update on stop)
  // ESPUI.sliderContinuous = true;

  /*
     Optionally you can use HTTP BasicAuth. Keep in mind that this is NOT a
     SECURE way of limiting access.
     Anyone who is able to sniff traffic will be able to intercept your password
     since it is transmitted in cleartext. Just add a string as username and
     password, for example begin("ESPUI Control", "username", "password")
  */


  ESPUI.begin("Brazalete SMART", usuario.c_str(), password.c_str());
  muestra_ids();
}

void loop( void ) {
  dnsServer.processNextRequest();

  static long oldTMed = 0;
  static bool switchi = false;

  if ( millis() - oldTMed >  (pmed * 1000 )) {
    //switchi = !switchi;
    //ESPUI.updateControlValue( switchOne, switchi ? "1" : "0" );
    if(SensorTemp){
      Serial.print("Ambient = "); Serial.print(valtempamb);
      Serial.print("*C\tObject = "); Serial.print(valtemp); Serial.println("*C");
      if (switchOne){
        ESPUI.addGraphPoint(graphIdTemp, valtemp);
      }
    }
           
    if(SensorPulso){
      Serial.print("heartrate/HRAv = "); Serial.print(beatsPerMinute);Serial.print("/");Serial.print(beatAvg);
      Serial.print("\t SPO2= "); Serial.print(spo2); ;Serial.print("/");Serial.print(sp02Avg);Serial.println("*C");
      if (switchOne){
        ESPUI.addGraphPoint(graphIdPulso, beatsPerMinute);
        ESPUI.addGraphPoint(graphIdSPO, spo2);
      }
    }
    Serial.println("loop Nucleo:" + String(xPortGetCoreID()));
    //int32_t spo, int32_t hr, int spoav, int hrav, float v, float temp
    vcc=get_vcc();
    send_mqtt(spo2,beatsPerMinute,sp02Avg,beatAvg,vcc,valtemp);
    oldTMed = millis();
  }
}
