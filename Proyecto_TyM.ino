#include <DNSServer.h>
#include <ESPUI.h>
#include <WiFi.h>
#include <Preferences.h>


const byte DNS_PORT = 53;
IPAddress apIP( 192, 168, 1, 1 );
DNSServer dnsServer;
Preferences preference;


//const char* ssid = "wifi";
//const char* password = "secret1703secret1703";
const char* hostname = "espui";
String usuario;
String password;
String ssid;
String pass;
String mqttserver;
String token;
unsigned long pmed;
unsigned long pguard;
uint16_t dtatempg;
uint16_t dtapulso;
uint16_t dtaspo;
unsigned long versionc;
uint16_t button1;
uint16_t switchOne;
uint16_t cversion, cpulsostat, ctempstat, cwifistat, cntpstat, cmqttstat, csave;
uint16_t pmedid, pguardid,dtatempgid,dtapulsoid,dtaspoid,graficarid,wifissidid,wifipassid,mqttserverid,tokenid,usuarioid,passwordid;
int graphIdTemp, graphIdPulso, graphIdSPO;


void conf_default(void) {
  preference.putString("usuario", "admin");
  preference.putString("password", "admin");
  preference.putString("ssid", "wifi");
  preference.putString("pass", "admin");
  preference.putString("mqttserver", "192.168.0.1");
  preference.putString("token", "0000000000000000");
  preference.putULong("pmed", 30000);
  preference.putULong("pguard", 600000);
  preference.putUInt("dtatempg", 20);
  preference.putUInt("dtapulso", 20);
  preference.putUInt("dtaspo", 10);
  preference.putULong("version", 1);
}

//String usuario,String password,String ssid,String pass,String mqttserver,String token,unsigned long pmed,unsigned long pguard,uint16_t dtatempg,uint16_t dtapulso,uint16_t dtaspo,unsigned long version
void conf_save(void) {
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
  preference.putULong("version", versionc);
}

bool get_conf(void) {
  preference.begin("configuracion", false);
  Serial.print( "\nConfigurando .....\n" );
  
  if (preference.getUInt("version", 0) > 0) {
    usuario = preference.getString("usuario", "admin");
    password = preference.getString("password", "admin");
    ssid = preference.getString("ssid", "wifi");
    pass = preference.getString("pass", "secret1703secret1703");
    mqttserver = preference.getString("mqttserver", "192.168.0.1");
    token = preference.getString("token", "0000000000000000");
    pmed = preference.getULong("pmed", 30000);
    pguard = preference.getULong("pguard", 600000);
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
  Serial.println(" cversion:" + String(cversion) + " cpulsostat:" + String(cpulsostat) + " ctempstat:" + String(ctempstat) + " cwifistat:" + String(cwifistat) + " cntpstat:" + String(cntpstat) + " cmqttstat:" + String(cmqttstat) + " pmedid:" + String(pmedid) + " pguardid:" + String(pguardid) + " dtatempgid:" + String(dtatempgid) + " dtapulsoid:" + String(dtapulsoid) + " dtaspoid:" + String(dtaspoid) + " graficarid:" + String(graficarid) + " wifissidid:" + String(wifissidid) + " wifipassid:" + String(wifipassid) + " mqttserverid:" + String(mqttserverid) + " tokenid:" + String(tokenid) + " usuarioid:" +  String(usuarioid) + " passwordid:" + String(passwordid) + " graphIdTemp:" + String(graphIdTemp) + " graphIdPulso:" + String(graphIdPulso) + " graphIdSPO:" + String(graphIdSPO));
}

void numberCall( Control* sender, int type ) {
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
      break;

    case B_UP:
      Serial.println( "Button UP" );
      break;
  }
}

void switchExample( Control* sender, int value ) {
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


void setup( void ) {
  Serial.begin( 115200 );
  bool sal = get_conf();
  if (! sal ) {
    Serial.println("No habia configuracion.");
  }
  WiFi.setHostname( hostname );
  // try to connect to existing network
  WiFi.begin( ssid.c_str(), pass.c_str());
  Serial.print( "\n\nTry to connect to existing network" );

  {
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
  }

  dnsServer.start( DNS_PORT, "*", apIP );

  Serial.println( "\n\nWiFi parameters:" );
  Serial.print( "Mode: " );
  Serial.println( WiFi.getMode() == WIFI_AP ? "Station" : "Client" );
  Serial.print( "IP address: " );
  Serial.println( WiFi.getMode() == WIFI_AP ? WiFi.softAPIP() : WiFi.localIP() );

  uint16_t tab1 = ESPUI.addControl( ControlType::Tab, "datos", "Datos" );
  uint16_t tab2 = ESPUI.addControl( ControlType::Tab, "conexion", "Conexion" );
  Serial.print("tab1 :");
  Serial.print(tab1);
  Serial.print(" tab2 :");
  Serial.println(tab2);
   
  // shown above all tabs
  cversion = ESPUI.addControl( ControlType::Label, "Conf Version:", String(versionc), ControlColor::Turquoise );
  cpulsostat = ESPUI.addControl( ControlType::Label, "Sensor Pulso:", "Desconectado", ControlColor::Turquoise );
  ctempstat = ESPUI.addControl( ControlType::Label, "Sensor Temperatura:", "Desconectado", ControlColor::Turquoise );
  cwifistat = ESPUI.addControl( ControlType::Label, "Modo Wifi:", "AP", ControlColor::Turquoise );
  cntpstat = ESPUI.addControl( ControlType::Label, "Servidor NTP:", "Desconectado", ControlColor::Turquoise );
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


  ESPUI.begin("Brazalete SMART", "admin", "admin");
  muestra_ids();
}

void loop( void ) {
  dnsServer.processNextRequest();

  static long oldTime = 0;
  static bool switchi = false;

  if ( millis() - oldTime > 5000 ) {
    //switchi = !switchi;
    //ESPUI.updateControlValue( switchOne, switchi ? "1" : "0" );
    uint16_t temp=random(1, 50);
    Serial.print( "Temperatura:" );
    Serial.println( temp);
    ESPUI.addGraphPoint(graphIdTemp, temp);
    //ESPUI.addGraphPoint(graphIdPulso, random(1, 50));
    //ESPUI.addGraphPoint(graphIdSPO, random(1, 50));

    oldTime = millis();
  }
}
