#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <FirebaseESP8266.h>
#include "DHT.h"
#define DHTPIN D2
#define DHTTYPE DHT11   // DHT 11

FirebaseData firebaseData ;

#define FIREBASE_AUTH  "*******" // database secret key
#define FIREBASE_HOST "*********" // realtime database host
 
DHT dht(DHTPIN, DHTTYPE);
// Initialize Wifi connection to the router
char ssid[] = "************";     // Wifi Name
char password[] = "**********"; // Wifi Password
// Initialize Telegram BOT
#define BOTtoken "*********" // Bot token from telegram app
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
//Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

void handleNewMessages(int numNewMessages) {
      Serial.println("handleNewMessages");
      Serial.println(String(numNewMessages));
      for (int i=0; i<numNewMessages; i++) {
              String chat_id = String(bot.messages[i].chat_id);
              String text = bot.messages[i].text;
              String from_name = bot.messages[i].from_name;
              if (from_name == "") from_name = "Guest";
              
            if (text == "/temperature") {
                  int t = dht.readTemperature();
                  String temp = "Temperature : ";
                  temp += int(t);
                  temp +=" *C\n";
                  bot.sendMessage(chat_id,temp, "");
            }
             
            if (text == "/humidity") {
                int h = dht.readHumidity();
                String temp = "Humidity: ";
                temp += int(h);
                temp += " %";
                bot.sendMessage(chat_id,temp, "");
            }
             
            if (text == "/start") {
                String welcome = "Welcome  " + from_name + ".\n";
                welcome += "/temperature : Temperature reading\n";
                welcome += "/humidity : Humiditiy reading\n";
                bot.sendMessage(chat_id, welcome, "Markdown");
            }
      }
}
 
void setup() {
      Serial.begin(9600);
      dht.begin();
      // This is the simplest way of getting this working
      // if you are passing sensitive information, or controlling
      // something important, please either use certStore or at
      // least client.setFingerPrint
      client.setInsecure();
      // Set WiFi to station mode and disconnect from an AP if it was Previously
      // connected
      WiFi.mode(WIFI_STA);
      WiFi.disconnect();
      delay(100);
      // attempt to connect to Wifi network:
      Serial.print("Connecting Wifi: ");
      Serial.println(ssid);
      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
      }
 
Serial.println("");
Serial.println("WiFi connected");
Serial.print("IP address: ");
Serial.println(WiFi.localIP());
Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}
 
void loop() {
      int t = dht.readTemperature();
      int h = dht.readHumidity();
      if (isnan(h) || isnan(t)){                                   
        Serial.println(F("Failed to read from DHT sensor!"));
        return;
      } 
      Serial.print("Humidity: ");  
      Serial.print(h);
      String fireHumid = String(h) + String("%");                   
    
      Serial.print("%  Temperature: ");  
      Serial.print(t);  
      Serial.println("°C ");
      String fireTemp = String(t) + String("°C");
      //Checks for new measures every 5 second.                
      delay(5000);
      Firebase.pushString(firebaseData,"/DHT11/Humidity", fireHumid);            
      Firebase.pushString(firebaseData,"/DHT11/Temperature", fireTemp);
    
      if (millis() > lastTimeBotRan + botRequestDelay)  {
            int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
            while(numNewMessages) {
              Serial.println("got response");
              handleNewMessages(numNewMessages);
              numNewMessages = bot.getUpdates(bot.last_message_received + 1);
            }
      lastTimeBotRan = millis();
      }
}
