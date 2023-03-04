/*
Lucky Robot
indicates status by waving depending on website input
mancas@lug-saar.de
works on D1 Wemos mini with data cable on D4 
*/

  #include <Arduino.h>
  #include <Servo.h>
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
  #include <ESPAsyncWebServer.h>


// CREDENTIALS
const char* ssid =     "xx";
const char* password = "xx";
String CodeToVerifyON =  "mumbleON";
//String CodeToVerifyOFF = "mumbleOFF";

float XPosTarget;
float YPosTarget;
int Action;
int pos;
int Do;

Servo myservo;  // create servo object to control a servo
 

const char* PARAM_INPUT_1 = "input1";
String globalinputMessage;  // Website input

AsyncWebServer server(80);

// HTML web page to handle input
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>lug-saar.de mumble</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <br>
  <br><br>
   <form action="/get">
  input1: <input type="text" name="input1">
  <input type="submit" value="Submit">
</form><br>
</body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}




void setup() {
  
  Serial.begin(115200);
  
  myservo.attach(2);  // attaches the servo on GIO2 to the servo object
  //Serial.println("Servo attached");
  //myservo.write(90);
 
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  delay(1200);
  
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }
  //Serial.println();
  
  //Serial.println("    Starte Webserver");

  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });
  
  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
   String inputMessage;
   String inputParam;
   
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      globalinputMessage = inputMessage; //gebe weiter an globale Variable
    }
    
  
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    
    //Serial.println(inputMessage);
    
    request->send(200, "text/html", "<br><a href=\"/\">You got it </a>"); 
    
    }

);

  server.onNotFound(notFound);
  server.begin();

Serial.println("IPv4 Address: ");   
Serial.println(WiFi.localIP());
}

void loop() {
pos=0;
Do=0;

//compare with your code
//Serial.print(CodeToVerifyON); Serial.print("  "); Serial.println(globalinputMessage);


if (globalinputMessage == CodeToVerifyON) { //only moving if requested
  Serial.println("  Move  ");

  for (Do = 1; Do <= 15; Do +=1){
  for (pos = 30; pos <= 130; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(25);                       // waits 15ms for the servo to reach the position
    //Serial.println(pos);
  }
   
  for (pos = 130; pos >=30; pos -= 1) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(25);                       // waits 15ms for the servo to reach the position
    //Serial.println(pos);
  }
  }
  
  globalinputMessage = "0";
  delay(3000);
}


 else {
  //Serial.println("No Code or  Wrong Code  ");
  delay(3000);
  }
  //globalinputMessage = "0";
}
