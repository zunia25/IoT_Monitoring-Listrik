#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <PZEM004Tv30.h>

// Update these with values suitable for your network.

const char* ssid = "ITS-ASRAMA-F";
const char* password = "itssurabaya";
const char* mqtt_server = "91.121.93.94";

// MQTT BROKER SETTINGS
#define INTOPIC "listrik-out-informatika"
#define POWERTOPIC "listrik-out-informatika/power"
#define ENERGYTOPIC "listrik-out-informatika/energy"
#define VOLTAGETOPIC "listrik-out-informatika/voltage"
#define CURRENTTOPIC "listrik-out-informatika/current"
#define FREQUENCYTOPIC "listrik-out-informatika/frequency"
#define PFTOPIC "listrik-out-informatika/pf"

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;


//inisialisasi objek untuk pzem (sensor arus)
PZEM004Tv30 pzem(14, 12, 0x01); // D5 = 14 , D6 = 12

//variabel penampung untuk nilai sesnsor arus
float Power, Energy, Voltage, Current, Frequency, Pf;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish(OUTTOPIC, "hello world");
      // ... and resubscribe
      // client.subscribe(INTOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();


  //baca nilai power (kW)
  Power = pzem.power();
  //jika gagal membaca power
  if(isnan(Power)){
    Serial.println("Gagal membaca Power");
  }
  else{
    Serial.print("Power :");
    Serial.print(Power);
    Serial.println("W");
  }

  // baca nilai Energy (Daya/jam)
  Energy = pzem.energy();
  //jika gagal membaca energy
  if(isnan(Energy)){
    Serial.println("Gagal membaca Energy");
  }
  else{
    Serial.print("Energy :");
    Serial.print(Energy);
    Serial.println("kWh");
  }

  // baca nilai Voltage (tegangan)
  Voltage = pzem.voltage();
  //jika gagal membaca voltage
  if(isnan(Voltage)){
    Serial.println("Gagal membaca Voltage");
  }
  else{
    Serial.print("Voltage :");
    Serial.print(Voltage);
    Serial.println("V");
  }

  // baca nilai Current (kuat arus)
  Current = pzem.current();
  //jika gagal membaca current
  if(isnan(Current)){
    Serial.println("Gagal membaca Current");
  }
  else{
    Serial.print("Current :");
    Serial.print(Current);
    Serial.println("A");
  }

  // baca nilai Frequency
  Frequency = pzem.frequency();
  //jika gagal membaca frequency
  if(isnan(Frequency)){
    Serial.println("Gagal membaca Frequency");
  }
  else{
    Serial.print("Frequency :");
    Serial.print(Frequency);
    Serial.println("Hz");
  }

  // baca nilai Power Factor
  Pf = pzem.pf();
  //jika gagal membaca power factor
  if(isnan(Pf)){
    Serial.println("Gagal membaca Power Factor");
  }
  else{
    Serial.print("Power Factor :");
    Serial.println(Pf);
  }

  snprintf (msg, MSG_BUFFER_SIZE, "%lf", Power);
  client.publish(POWERTOPIC, msg);
  snprintf (msg, MSG_BUFFER_SIZE, "%lf", Energy);
  client.publish(ENERGYTOPIC, msg);
  snprintf (msg, MSG_BUFFER_SIZE, "%lf", Current);
  client.publish(CURRENTTOPIC, msg);
  snprintf (msg, MSG_BUFFER_SIZE, "%lf", Voltage);
  client.publish(VOLTAGETOPIC, msg);
  snprintf (msg, MSG_BUFFER_SIZE, "%lf", Frequency);
  client.publish(FREQUENCYTOPIC, msg);
  snprintf (msg, MSG_BUFFER_SIZE, "%lf", Pf);
  client.publish(PFTOPIC, msg);


  delay(1000);
}
