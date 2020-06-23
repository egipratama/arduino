#include <MKRGSM.h>
#include <PubSubClient.h>
// PIN Number
const char PINNUMBER[]     = "1234";
// APN data
const char GPRS_APN[]      = "3gprs";
const char GPRS_LOGIN[]    = "3gprs";
const char GPRS_PASSWORD[] = "3gprs";

const char* mqtt_server = "mqtt.mapid.io";

// initialize the library instance
GSMClient gsmClient;
GPRS gprs;
GSM gsmAccess;

PubSubClient client(gsmClient);

long lastMsg = 0;
char msg[50];
int value = 0;

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
    digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
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
      Serial.println("connected mqtt");
      // Once connected, publish an announcement...
      client.publish("public/outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("public/inTopic");
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
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Starting Arduino web client.");
  bool connected = false;
  while (!connected) {
    if ((gsmAccess.begin(PINNUMBER) == GSM_READY) &&
        (gprs.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD) == GPRS_READY)) {
      connected = true;
    } else {
      Serial.println("Not connected");
      delay(1000);
    }
  }
  Serial.println("connecting...");
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 10000) {
    lastMsg = now;
    ++value;
    snprintf (msg, 50, "key=1e204563cd95b5354e1678affacd56b7&var1=%ld", value);
    client.publish("private/egi", msg);
    Serial.print("Publish message: ");
    Serial.print(msg);
    Serial.print("\n");
  }
}
