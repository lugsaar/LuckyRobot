/*
Lucky Robot
indicates status by waving depending on website input
mancas@lug-saar.de
works on D1 Wemos mini with data cable on D4 
this version sends email with its ip
*/

  #include <Arduino.h>
  #include <Servo.h>
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
  #include <ESPAsyncWebServer.h>
  #include <ESP_Mail_Client.h>
  #define SMTP_PORT 25
  SMTPSession smtp; // SMTP Session object used for Email sending
  void smtpCallback(SMTP_Status status); //Callback function to get the Email sending status
  


// CREDENTIALS
const char* ssid =     "xx";
const char* password = "xx";
//const char* ssid =     "xx";
//const char* password = "xx";

//send logfile per email
#define SMTP_HOST "xx"
#define AUTHOR_EMAIL "xx"
#define AUTHOR_PASSWORD "xx"

//my D1mini is http://esp-da6ec7/
//give him http://192.168.63.93/get?input1=mumbleON or
//give him http://esp-daec7/get?input1=mumbleON 
//http://esp-d6cab2/get?input1=mumbleON 

const byte led_eyeL = 4; // LED eyes for PIN D2
const byte led_eyeR = 5; // LED eyes for PIN D1

String CodeToVerifyON =  "mumbleON";
//String CodeToVerifyOFF = "mumbleOFF";

float XPosTarget;
float YPosTarget;
int Action;
int pos;
int Do;

char IP_file[20];
String HelpString;

Servo myservo;  // create servo object to control a servo
 

const char* PARAM_INPUT_1 = "input1";
String globalinputMessage;  // Website input

AsyncWebServer server(80);

// HTML web page to handle input
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Lucky Robot</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <br>
  <h2>Lucky Robot</h2>
  <br>
   <form action="/get">
  <input type="text" name="input1">
  <input type="submit" value="Go">
</form><br>
</body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}




void setup() {
  
  Serial.begin(115200);
  
  myservo.attach(2);  // attaches the servo on GIO2 to the servo object for PIN D4
  //Serial.println("Servo attached");
  //myservo.write(90);
 
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(led_eyeR, OUTPUT);
  pinMode(led_eyeL, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); //internal LED is is off, cause builtin LED is inverse
  digitalWrite(led_eyeR, LOW); 
  digitalWrite(led_eyeL, LOW); 
  
  Serial.println("LEDs are off ");
  
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
    
    request->send(200, "text/html", "<br><a href=\"/\"> Next Step </a>"); 
    
    }

);

  server.onNotFound(notFound);
  server.begin();   
      
Serial.println("HelpString  ");
Serial.println(HelpString);
Serial.println(".");
Serial.println("IP_file  ");
Serial.println(IP_file);
Serial.println("...");
  //IP_file =  WiFi.localIP().toString();

Serial.println("IPv4 Address: ");   
Serial.println(WiFi.localIP());

SendLogfile();

delay(2000);

digitalWrite(led_eyeL, HIGH); //Signal for got IPv4 Adress
digitalWrite(LED_BUILTIN, LOW); //light is on, cause builtin LED is inverse
delay(3000);
//digitalWrite(led_eyeL, LOW);
//digitalWrite(LED_BUILTIN, HIGH); 
}

void loop() {
pos=0;
Do=0;

digitalWrite(led_eyeL, LOW);
digitalWrite(led_eyeR, LOW);
digitalWrite(LED_BUILTIN, HIGH); //light is off, cause builtin LED is inverse

//compare with your code
//Serial.print(CodeToVerifyON); Serial.print("  "); Serial.println(globalinputMessage);


if (globalinputMessage == CodeToVerifyON) { //only moving if requested
  Serial.println("  I am waving 15 times  ");

  for (Do = 1; Do <= 15; Do +=1){
    
    digitalWrite(led_eyeR, HIGH);
    digitalWrite(led_eyeL, HIGH);
    digitalWrite(LED_BUILTIN, LOW); //light is on, cause builtin LED is inverse
    
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


//---------------------

void SendLogfile()
{
  //smtp.debug(0);

delay(50);
      HelpString = ("ip= " + WiFi.localIP().toString()+" ");
      byte i = 20; // IP_file is an arry of 15 char
      while ( i-- ) {
        IP_file [i] =  HelpString [i] ;
        delay(50); 
      }
      delay(50);


  //set the callback function to get the sending results
  smtp.callback(smtpCallback);

  //declare the session config data
  ESP_Mail_Session session;

  //set the session config
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
  session.login.user_domain = "22x22.de"; // "mydomain.net";

  //declare the message class
  SMTP_Message message;

  //set the message headers
  message.sender.name = "Lucky Robot";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = "my ip adress";
  message.addRecipient ("webcam", "webcam@22x22.de"); //("Someone", "####@#####_dot_com");
  message.text.content = (IP_file);
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;
  message.addHeader("Message-ID: <abcde.fghij@gmail.com>");
  
  // connect to the server
  if (!smtp.connect(&session))
    return;

  //start sending Email and close the session
  if (!MailClient.sendMail(&smtp, &message))
    Serial.println("Error sending Email, " + smtp.errorReason());

}

//callback function to get the Email sending status
void smtpCallback(SMTP_Status status)
{
  //print the current status
  Serial.println(status.info());

  //print the sending result
  if (status.success())
  {
    Serial.println("----------------");
    Serial.printf("Message sent success: %d\n", status.completedCount());
    Serial.printf("Message sent failled: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      //get the result item
      SMTP_Result result = smtp.sendingResult.getItem(i);
      localtime_r(&result.timesstamp, &dt);

      Serial.printf("Message No: %d\n", i + 1);
      Serial.printf("Status: %s\n", result.completed ? "success" : "failed");
      Serial.printf("Date/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
      Serial.printf("Recipient: %s\n", result.recipients);
      Serial.printf("Subject: %s\n", result.subject);
    }
    Serial.println("----------------\n");
  }
}
