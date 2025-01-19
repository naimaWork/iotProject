#include <WiFi.h>
#include <PubSubClient.h>

// Configuration Wi-Fi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Configuration MQTT (ThingSpeak)
const char* mqttServer = "mqtt3.thingspeak.com";
const int mqttPort = 1883;
const char* mqttUser = ""; // Laissez vide
const char* mqttPassword = "DKLNUQ9HFU6U50Q2"; // Clé API d'écriture ThingSpeak
const char* topic = "channels/2805283/publish"; // Remplacez avec vos informations
//-- subscribed settings Virtuino command 1   
const char* subscribeTopicFor_Command_1="channels/2805283/subscribe/fields/field1";   //REPLACE THE NUMBER 114938 WITH YOUR channel ID  
const char* subscribeTopicFor_Command_2="channels/2805283/subscribe/fields/field2";
// Pins des capteurs
#define TRIG_PIN1 4
#define ECHO_PIN1 5
#define TRIG_PIN2 18
#define ECHO_PIN2 19

WiFiClient espClient;
PubSubClient client(espClient);

// Variables globales
long lastPublishMillis = 0;
int connectionDelay = 1;
int updateInterval = 15; // Intervalle de mise à jour (15 secondes)

void mqttSubscriptionCallback(char* topic, byte* payload, unsigned int length) {
  // Fonction pour traiter les messages entrants
  Serial.print("Message arrivé [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void mqttSubscribe(long subChannelID) {
  // S'abonner à un topic pour recevoir les mises à jour
  String myTopic = "channels/" + String(subChannelID) + "/subscribe";
  client.subscribe(myTopic.c_str());
}

void mqttPublish(long pubChannelID, String message) {
  // Publier un message sur le canal ThingSpeak
  String topicString = "channels/" + String(pubChannelID) + "/publish";
  client.publish(topicString.c_str(), message.c_str());
}

void connectWifi() {
  // Fonction pour connecter l'ESP32 au Wi-Fi
  Serial.print("Connexion à ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnecté au Wi-Fi");
}

void mqttConnect() {
  // Fonction pour connecter l'ESP32 au serveur MQTT
    while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
      //client.connect("cliend ID", "username","password") Replace with your Thingspeak MQTT Device Credentials
    if (client.connect("BgsCBxUXMDYGGzUJJDUgLAI", "BgsCBxUXMDYGGzUJJDUgLAI","FFoApbC7OKRBOHhSgkmgrIOp")) {  
      Serial.println("connected");
      client.subscribe(subscribeTopicFor_Command_1);   // subscribe the topics here
      client.subscribe(subscribeTopicFor_Command_2);   
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");   // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

long measureDistance(int trigPin, int echoPin) {
  // Mesurer la distance à l'aide du capteur ultrasonique
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  long distance = duration * 0.034 / 2;
  return distance;
}

void setup() {
  Serial.begin(9600);
  delay(3000); // Attendre que le moniteur série soit prêt

  // Configurer les broches des capteurs ultrasoniques
  pinMode(TRIG_PIN1, OUTPUT);
  pinMode(ECHO_PIN1, INPUT);
  pinMode(TRIG_PIN2, OUTPUT);
  pinMode(ECHO_PIN2, INPUT);

  // Connexion au Wi-Fi
  connectWifi();

  // Configurer le client MQTT
  client.setServer(mqttServer, mqttPort);
  client.setCallback(mqttSubscriptionCallback);
  client.setBufferSize(2048); // Taille du tampon pour les messages reçus
}

void loop() {
  // Vérification de la connexion Wi-Fi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi perdu. Tentative de reconnexion...");
    WiFi.reconnect();
    delay(1000);
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Wi-Fi reconnecté !");
    } else {
      Serial.println("Impossible de se reconnecter au Wi-Fi.");
      return;
    }
  }

  // Vérification de la connexion MQTT
  if (!client.connected()) {
    Serial.println("MQTT déconnecté. Tentative de reconnexion...");
    mqttConnect();
    if (!client.connected()) {
      Serial.print("Échec de la reconnexion MQTT. Statut : ");
      Serial.println(client.state());
      return;
    }
  }

  client.loop();

  // Mesurer les distances
  long distance1 = measureDistance(TRIG_PIN1, ECHO_PIN1);
  long distance2 = measureDistance(TRIG_PIN2, ECHO_PIN2);

  if (distance1 < 0 || distance2 < 0) {
    Serial.println("Erreur : distance négative détectée. Vérifiez les capteurs.");
    return;
  }



  // Préparer le payload
  String payload = "field1=" + String(distance1) + "&field2=" + String(distance2) + "&status=MQTTPUBLISH";
  Serial.print("Payload préparé : ");
  Serial.println(payload);

  // Publier les données
  if (client.publish(topic, payload.c_str())) {
    Serial.println("Données envoyées avec succès : " + payload);
  } else {
    Serial.println("Échec de l'envoi des données !");
    Serial.print("Statut MQTT : ");
    Serial.println(client.state());
  }

  // Attente avant la prochaine mise à jour
  delay(updateInterval * 1000);
}

