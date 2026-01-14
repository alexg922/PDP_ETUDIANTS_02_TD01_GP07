#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include "WiFiClientSecure.h"
#include <PubSubClient.h>

/*-----------------------------------------------------------------------------*/
/* Configuration WiFi & MQTT */

const char* wifi_ssid     = "MYWIFI";
const char* wifi_password = "0123456789";

const char* mqtt_server = "27cc61dbaffc4da08cd0081cabd8cf01.s2.eu.hivemq.cloud";
int mqtt_port = 8883;

const char* mqtt_user = "create_ece";
const char* mqtt_pass = "create123A";
const char* client_id = "TD01_GP07";

/*-----------------------------------------------------------------------------*/
/* Certificat CA pour la connexion TLS */

static const char ca_cert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

/*-----------------------------------------------------------------------------*/
/* Capteur DHT11 */

#define SENSOR 33
#define DHTTYPE DHT11
DHT_Unified dht(SENSOR, DHTTYPE);

/*-----------------------------------------------------------------------------*/
/* Clients WiFi et MQTT */

WiFiClientSecure client;
PubSubClient mqtt_client(client);

/*-----------------------------------------------------------------------------*/
/* Connexion WiFi */

void connect_wifi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }
  Serial.println("\nConnected.");
}

/*-----------------------------------------------------------------------------*/
/* Connexion MQTT */

bool connect_mqtt() {
  Serial.print("Connecting to MQTT");
  if (mqtt_client.connect(client_id, mqtt_user, mqtt_pass)) {
    Serial.println(" connected.");
    return true;
  } else {
    Serial.print(" failed, state=");
    Serial.println(mqtt_client.state());
    return false;
  }
}

/*-----------------------------------------------------------------------------*/
/* Setup */

void setup() {
  Serial.begin(9600);
  delay(100);

  // 1. Connexion au réseau WiFi
  connect_wifi();

  // 2. Configuration TLS et serveur MQTT
  client.setCACert(ca_cert);
  mqtt_client.setServer(mqtt_server, mqtt_port);

  // 3. Connexion au broker MQTT
  bool mqtt_ok = connect_mqtt();

  // 4. Lecture des mesures du capteur DHT11
  dht.begin();
  sensors_event_t event;

  float temp = NAN;
  float hum  = NAN;

  dht.temperature().getEvent(&event);
  if (!isnan(event.temperature)) {
    temp = event.temperature;
  }

  dht.humidity().getEvent(&event);
  if (!isnan(event.relative_humidity)) {
    hum = event.relative_humidity;
  }

  // 5. Publication MQTT des mesures
  if (mqtt_ok && !isnan(temp) && !isnan(hum)) {

    char topicTemp[32];
    char topicHum[32];
    char payload[16];

    snprintf(topicTemp, sizeof(topicTemp), "%s/temp", client_id);
    snprintf(topicHum, sizeof(topicHum), "%s/relhum", client_id);

    dtostrf(temp, 4, 2, payload);
    mqtt_client.publish(topicTemp, payload);

    dtostrf(hum, 4, 2, payload);
    mqtt_client.publish(topicHum, payload);
  }

  Serial.println("Going to sleep for 5 seconds...");
  delay(100);
  ESP.deepSleep(5e6);
}

void loop() {
  // Not used (deep sleep)
}
