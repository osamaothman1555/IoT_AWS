/* ESP32 AWS IoT
 *  
 * Simplest possible example (that I could come up with) of using an ESP32 with AWS IoT.
 *  
 * Author: Anthony Elder 
 * License: Apache License v2
 * Anothny Elder Sketch Modified by Stephen Borsay for www.udemy.com
 * https://github.com/sborsay
 * Author of This Sketch: Osama Othman
 * Add in Char buffer utilizing sprintf to dispatch JSON data to AWS IoT Core
 * Use and replace your own SID, PW, AWS Account Endpoint, Client cert, private cert, x.509 CA root Cert
 */
#include <WiFiClientSecure.h>
#include <PubSubClient.h> // install with Library Manager, I used v2.6.0
#include <dht.h>

#define dht_pin 27

char incoming_Msg[500];

const char* awsEndpoint = "insert-endpoint-here";

/* Update the two certificate strings below. Paste in the text of your AWS 
 * device certificate and private key. Add a quote character at the start
 * of each line and a backslash, n, quote, space, backslash at the end 
 * of each line. 
 * For the rest start with quote and end with quote, space and backslash:
 */

// xxxxxxxxxx-certificate.pem.crt
const char* certificate_pem_crt = \

"-----BEGIN CERTIFICATE-----\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" 
"-----END CERTIFICATE-----\n";

// xxxxxxxxxx-private.pem.key
const char* private_pem_key = \

"-----BEGIN RSA PRIVATE KEY-----\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" 
"-----END RSA PRIVATE KEY-----\n";

/* root CA can be downloaded in:
  https://www.symantec.com/content/en/us/enterprise/verisign/roots/VeriSign-Class%203-Public-Primary-Certification-Authority-G5.pem
*/

const char* rootCA = \
"-----BEGIN CERTIFICATE-----\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" 
"-----END CERTIFICATE-----\n";

WiFiClientSecure wiFiClient;
void msgReceived(char* topic, byte* payload, unsigned int len);
PubSubClient pubSubClient(awsEndpoint, 8883, msgReceived, wiFiClient); 

void setup() {
  Serial.begin(115200); delay(50); Serial.println();
  Serial.println("ESP32 AWS IoT Example");
  Serial.printf("SDK version: %s\n", ESP.getSdkVersion());

  Serial.println("Please input SSID (wifi name): ");
  while (Serial.available() == 0) 
  {/*Wait for User Input*/ }
  const char* ssid = (Serial.readString()).c_str();
  Serial.println("Please input wifi password: ");
  while (Serial.available() == 0) 
  {/*Wait for User Input*/ }
  const char* password = (Serial.readString()).c_str();

  Serial.print("Connecting to "); Serial.print(ssid);
  WiFi.begin(ssid, password);
  WiFi.waitForConnectResult();
  Serial.print(", WiFi connected, IP address: "); Serial.println(WiFi.localIP());

  wiFiClient.setCACert(rootCA);
  wiFiClient.setCertificate(certificate_pem_crt);
  wiFiClient.setPrivateKey(private_pem_key);

  pinMode(27,INPUT);
  
}

unsigned long lastPublish;
int msgCount;
dht DHT_0;
char* subTopic = "mytemptopic"; //topic to subscribe to in order to send to AWS
char* pubTopic = "inTopic"; //topic to recieve data from AWS

void loop() {

  pubSubCheckConnect(pubTopic);

   //If you need to increase buffer size, you need to change MQTT_MAX_PACKET_SIZE in PubSubClient.h
   char realData[128];

  float temperature = 0; 
  float humidity =  0;
  
  if (millis() - lastPublish > 900000)   //Checks temperature and humidity every 15 minutes
  {
    DHT_0.read11(dht_pin);
    temperature = DHT_0.temperature;
    humidity = DHT_0.humidity;
    sprintf(realData,  "{\"uptime\":%lu,\"temperature\":%f,\"humid\":%f}", millis() / 1000, temperature, humidity);   //Packages data into a JSON string
    boolean rc = pubSubClient.publish(subTopic, realData);
    Serial.print("Published, rc="); Serial.print( (rc ? "OK: " : "FAILED: ") );
    Serial.println(realData);
    lastPublish = millis();  
  }
}

/*
 * Prints out message payload received from topic subscribed
 * @param topic The topic to subscribe to receive message.
 * @return The area of the circle.
*/
void msgReceived(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received on "); Serial.print(topic); Serial.print(": ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    incoming_Msg[i] = payload[i];
  }
  Serial.println();
}

/*
 * Connects ESP32 device to AWSIoT. 
 * Also checks for incoming payloads from subcribed topic
 * @param topic The topic to subscribe to receive message.
*/

void pubSubCheckConnect(char* topic) {
  if ( ! pubSubClient.connected()) {
    Serial.print("PubSubClient connecting to: "); Serial.print(awsEndpoint);
    while ( ! pubSubClient.connected()) {
      Serial.print(".");
      pubSubClient.connect("ESPthingXXXX");
      delay(1000);
    }
    Serial.println(" connected");
    pubSubClient.subscribe(topic);
  }
  pubSubClient.loop();
}
