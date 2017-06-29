#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <NeoPixelPainter.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

//#define SERIAL_OUT

#define PIN            2
#define NUMPIXELS      5
const char *ssid = "Christian";
const char *password = "nasenasenase"; //needs to be longer than 8 letters
const char *gateway = "192.168.43.200";
const int g_sleepInMinutes = 1;

Adafruit_NeoPixel neopixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

NeoPixelPainterCanvas pixelcanvas = NeoPixelPainterCanvas(&neopixels); //create canvas, linked to the neopixels (must be created before the brush)
NeoPixelPainterBrush pixelbrush = NeoPixelPainterBrush(&pixelcanvas); //crete brush, linked to the canvas to paint to

ESP8266WebServer server(8080);

int g_iCurrentId = 0;
bool g_bBreakRequest = false;

void setup() {
  pinMode(2, OUTPUT);
  
#ifdef SERIAL_OUT
  Serial.begin(9600);
  Serial.println("Setup begin");
#endif
  neopixels.begin(); // This initializes the NeoPixel library.
  neopixels.show(); // Initialize all pixels to 'off'

  delay(1000);
  
  if (!tryToConnect())
  {
    delay(200);
    //sleep and try again
    ESP.deepSleep(g_sleepInMinutes * 60 * 1000 * 1000);
    delay(1000);  
  }
  
#ifdef SERIAL_OUT
  Serial.println("Setup end");
#endif
}

void setErrorLevel(int level)
{
  if (level == 0){
    for (int i = 0; i < NUMPIXELS; i++)
      neopixels.setPixelColor(i, 0, 100, 0); //color in RGB: dark blue
    delay(1000);
    turnAllOff();    
  }
  else  
    for (int i = 0; i < level; i++)
      neopixels.setPixelColor(i, 100, 0, 0); //color in RGB: dark blue
}

bool tryToConnect(){
#ifdef SERIAL_OUT
  Serial.println("let's try to connect");
#endif
  WiFi.begin(ssid, password);
  const int retryCnt = 10;
  for (int i = 0; i < retryCnt; i++) // retry a few times, wifi is still wifi....
  {
    if (WiFi.status() == WL_CONNECTED)
      break;
    delay(500);
#ifdef SERIAL_OUT
    Serial.print(".");
#endif
    if (retryCnt-1 == i) // last round -> connection failed
      return false;
  }
  IPAddress myIP = WiFi.localIP();
  
#ifdef SERIAL_OUT
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(myIP);
#endif

  registerToService(gateway, 8080);

  server.on("/setStyle", handleRoot);
  server.begin();

  return true;  
}

void registerToService(const char* host, int port){
  WiFiClient client;
  for (int i = 0; i < 10; i++)
  {
    if (client.connect(host, port))
      break;
    if (9==i) 
    {
#ifdef SERIAL_OUT
  Serial.println("connection failed");
#endif
      setErrorLevel(1);
      return;
    }
    delay(50);
  }
       
//   This will send the request to the server
  client.print(String("GET ") + "/register" + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
#ifdef SERIAL_OUT
  Serial.println(">>> Client Timeout !");
#endif
      setErrorLevel(2);
      client.stop();
      return;
    }
  } 
  
      setErrorLevel(0);
  while(client.available()) {
    String line = client.readStringUntil('\r');
#ifdef SERIAL_OUT
  Serial.print(line);
#endif
  }   
}


void handleRoot() {
#ifdef SERIAL_OUT
  Serial.println(server.arg("id"));
#endif
  int id = server.arg("id").toInt();
  if (id != g_iCurrentId)
  {
    g_iCurrentId = id;
    g_bBreakRequest = true;
  }
  server.send(200, "text/html");
}

void turnAllOff()
{
    neopixels.clear();
    neopixels.show();  
}
void twinklyStars()
{

  unsigned int duration = 5000;
  unsigned int pausePerCyle = 1;
  unsigned int newBornRate = 1000;
  
  pixelbrush.setSpeed(1);
  pixelbrush.setFadein(true); //fade in 
  pixelbrush.setFadeout(true); //and fade out
  for(unsigned int loopcounter = 0; loopcounter<duration; loopcounter++) 
  {
    if (g_bBreakRequest)
      break;
    HSV brushcolor;

    if (rand() % newBornRate == 0) //at a random interval, move the brush to paint a new pixel (brush only paints a new pixel once)
    {
      brushcolor.h = rand();
      brushcolor.s = random(40); //set low saturation, almost white
      brushcolor.v = random(70); //set random brightness
      pixelbrush.setColor(brushcolor);
      pixelbrush.moveTo(random(NUMPIXELS)); //move the brush to a new, random pixel
      pixelbrush.setFadeSpeed(random(10) + 5); //set random fade speed, minimum of 5
    }

    //add a background color by setting all pixels to a color (instead of clearing all pixels):
    for (int i = 0; i < NUMPIXELS; i++)
    {
      neopixels.setPixelColor(i, 1, 0, 6); //color in RGB: dark blue
    }


    pixelbrush.paint(); //paint the brush to the canvas 
    pixelcanvas.transfer(); //transfer (add) the canvas to the neopixels

    neopixels.show();
    delay(pausePerCyle);
  }
}

void colorRandomizer()
{
  unsigned int duration = 5000;
  unsigned int pausePerCyle = 1;
  unsigned int newBornRate = 500;
  pixelbrush.setSpeed(1);
  pixelbrush.setFadein(true); //fade in 
  pixelbrush.setFadeout(true); //and fade out
  
  pixelbrush.setFadeHueNear(true); //fade using the near path on the colorcircle
  pixelbrush.setFadeHueFar(false); //fade using the far path on the colorcircle (if both are set, this path is chosen)
  for(unsigned int loopcounter = 0; loopcounter < duration; loopcounter++) 
  {
    if (g_bBreakRequest)
      break;
    HSV brushcolor;
    if (loopcounter % newBornRate == 0 )
    {
      brushcolor.h = random(255);
      brushcolor.s = 255; 
      brushcolor.v = random(100) + 20; //set random brightness
      pixelbrush.setColor(brushcolor);
      pixelbrush.moveTo(random(NUMPIXELS)); //move the brush to a new, random pixel
      pixelbrush.setFadeSpeed(random(10) + 5); //set random fade speed, minimum of 5
    }
    
    neopixels.clear();
    pixelbrush.paint(); //paint the brush to the canvas 
    pixelcanvas.transfer(); //transfer (add) the canvas to the neopixels

    neopixels.show();
    delay(pausePerCyle);
  }
  
}

void loop() {
  server.handleClient();
  g_bBreakRequest = false;
  switch (g_iCurrentId) //todo: add here a "change needed"?
  {
    case 0:
      turnAllOff();
      break;
    case 1:
      twinklyStars();
      break;
    case 2:
      colorRandomizer();
      break;
    default:
      turnAllOff();
      break;  
  }
  delay(10);
  
}
