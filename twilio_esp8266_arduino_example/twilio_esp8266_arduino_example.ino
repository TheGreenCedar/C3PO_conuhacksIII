/*
   Twilio SMS and MMS on ESP8266 Example.
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "twilio.hpp"

// Use software serial for debugging?
#define USE_SOFTWARE_SERIAL 0

// Your network SSID and password
const char* ssid = "AndroidAP";
const char* password = "1234567890";

// Find the api.twilio.com SHA1 fingerprint, this one was valid as
// of January 2017.
const char* fingerprint = "CF C4 40 77 88 75 5F BD 07 0F 88 3F 1C BD 95 3B DD AE 5F 13";

// Twilio account specific details, from https://twilio.com/console
// Please see the article:
// https://www.twilio.com/docs/guides/receive-and-reply-sms-and-mms-messages-esp8266-c-and-ngrok

// If this device is deployed in the field you should only deploy a revocable
// key. This code is only suitable for prototyping or if you retain physical
// control of the installation.
const char* account_sid = "AC6e371da2296f6f9acca081abd72ceff7";
const char* auth_token = "de38c1ceb50431674570b6b68b24694a";

// Details for the SMS we'll send with Twilio.  Should be a number you own
// (check the console, link above).
String to_number    = "+15149696840";
String from_number  = "+15149005554";
String message_body    = "Hello from Twilio and the ESP8266!";

// The 'authorized number' to text the ESP8266 for our example
String master_number    = "+15149696840";

// Optional - a url to an image.  See 'MediaUrl' here:
// https://www.twilio.com/docs/api/rest/sending-messages
String media_url = "";

// Global twilio objects
Twilio *twilio;
ESP8266WebServer twilio_server(8000);

/*
   Callback function when we hit the /message route with a webhook.
   Use the global 'twilio_server' object to respond.
*/
void handle_message() {
#if USE_SOFTWARE_SERIAL == 0
  Serial.println("Incoming connection!  Printing body:");
#endif
  bool authorized = false;
  char command = '\0';

  // Parse Twilio's request to the ESP
  for (int i = 0; i < twilio_server.args(); ++i) {
#if USE_SOFTWARE_SERIAL == 0
    Serial.print(twilio_server.argName(i));
    Serial.print(": ");
    Serial.println(twilio_server.arg(i));
#endif

    if (twilio_server.argName(i) == "From" and
        twilio_server.arg(i) == master_number) {
      authorized = true;
    } else if (twilio_server.argName(i) == "Body") {
      if (twilio_server.arg(i) == "?" or
          twilio_server.arg(i) == "0" or
          twilio_server.arg(i) == "1") {
        command = twilio_server.arg(i)[0];
      }
    }
  } // end for loop parsing Twilio's request

  // Logic to handle the incoming SMS
  String response = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
  if (command != '\0') {
    if (authorized) {
      switch (command) {
        case '0':
          digitalWrite(LED_BUILTIN, LOW);
          response += "<Response><Message>"
                      "Turning light off!"
                      "</Message></Response>";
          break;
        case '1':
          digitalWrite(LED_BUILTIN, HIGH);
          response += "<Response><Message>"
                      "Turning light on!"
                      "</Message></Response>";
          break;
        case '?':
        default:
          response += "<Response><Message>"
                      "0 - Light off, 1 - Light On, "
                      "? - Help\n"
                      "The light is currently: ";
          response += digitalRead(LED_BUILTIN);
          response += "</Message></Response>";
          break;
      }
    } else {
      response += "<Response><Message>"
                  "Unauthorized!"
                  "</Message></Response>";
    }

  } else {
    response += "<Response><Message>"
                "Look: a SMS response from an ESP8266!"
                "</Message></Response>";
  }

  twilio_server.send(200, "application/xml", response);
}

/*
   Setup function for ESP8266 Twilio Example.

   Here we connect to a friendly wireless network, instantiate our twilio
   object, optionally set up software serial, then send a SMS or MMS message.
*/
void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  twilio = new Twilio(account_sid, auth_token, fingerprint);

#if USE_SOFTWARE_SERIAL == 0
 // Serial.begin(115200);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to WiFi, IP address: ");
  Serial.println(WiFi.localIP());
#else
  while (WiFi.status() != WL_CONNECTED) delay(1000);
#endif

  // Response will be filled with connection info and Twilio API responses
  // from this initial SMS send.
  String response;
  bool success = twilio->send_message(
                   to_number,
                   from_number,
                   message_body,
                   response,
                   media_url
                 );

  // Set up a route to /message which will be the webhook url
  twilio_server.on("/message", handle_message);
  twilio_server.begin();

  // Use LED_BUILTIN to find the LED pin and set the GPIO to output
  pinMode(LED_BUILTIN, OUTPUT);

#if USE_SOFTWARE_SERIAL == 0
  Serial.println(response);
#endif
}


/*
    In our main loop, we listen for connections from Twilio in handleClient().
*/
void loop() {
  twilio_server.handleClient();
}
