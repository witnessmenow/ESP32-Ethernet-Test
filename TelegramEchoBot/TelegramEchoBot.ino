/*******************************************************************
    Using Telegram on the ESP32 with an ENC28J60 Ethernet Module

    If you find what I do useful and would like to support me,
    please consider becoming a sponsor on Github
    https://github.com/sponsors/witnessmenow/


    Written by Brian Lough
    YouTube: https://www.youtube.com/brianlough
    Tindie: https://www.tindie.com/stores/brianlough/
    Twitter: https://twitter.com/witnessmenow
 *******************************************************************/


#include <ESP32-ENC28J60.h>
//https://github.com/tobozo/ESP32-ENC28J60

#define SPI_HOST       1
#define SPI_CLOCK_MHZ  8

//---------------------------
// SPI Pin definitions
// - I'm using weird ones cause im using the Trinity
// - See the library for more standard examples
//---------------------------
#define INT_GPIO  34

#define MISO_GPIO 32
#define MOSI_GPIO 21
#define SCLK_GPIO 33
#define CS_GPIO   22

#define RST_GPIO   2


static bool eth_connected = false;


void WiFiEvent(WiFiEvent_t event)
{
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      Serial.println("ETH Started");
      //set eth hostname here
      ETH.setHostname("esp32-ethernet");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.print("ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
      if (ETH.fullDuplex()) {
        Serial.print(", FULL_DUPLEX");
      }
      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      eth_connected = true;
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;
    case ARDUINO_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;
    default:
      break;
  }
}

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "XXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"

const unsigned long BOT_MTBS = 1000; // mean time between scan messages

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime; // last time messages' scan has been done

void handleNewMessages(int numNewMessages)
{
  for (int i = 0; i < numNewMessages; i++)
  {
    bot.sendMessage(bot.messages[i].chat_id, bot.messages[i].text, "");
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println();

  WiFi.onEvent( WiFiEvent );

  //Resting the Device, maybe not needed?
  pinMode(RST_GPIO, INPUT);
  digitalWrite(RST_GPIO, LOW);
  delay(100);
  digitalWrite(RST_GPIO, HIGH);

  ETH.begin( MISO_GPIO, MOSI_GPIO, SCLK_GPIO, CS_GPIO, INT_GPIO, SPI_CLOCK_MHZ, SPI_HOST );

  while ( !eth_connected) {
    Serial.println("Connecting...");
    delay( 1000 );
  }
  Serial.print("\nETH connected. IP address: ");
  Serial.println(ETH.localIP());

  secured_client.setInsecure();
}

void loop()
{
  if (millis() - bot_lasttime > BOT_MTBS)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages)
    {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }
}
