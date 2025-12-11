#include <WiFi.h>
#include <SPI.h>

// Credențiale WiFi (trebuie completate)
const char* ssid     = "Retea WIFi"; 
const char* password = "Parola retea WiFi";

// Variabile de stare și server
int state = 0; // 1 pentru HIGH, 2 pentru LOW
char line[256] = {};
int idx = 0;  
WiFiServer server(80);

void setup()
{
  Serial.begin(115200);
  Serial.print("Conectare la ");
  Serial.println(ssid);
  
  // 1. Conectare la WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\n");
  Serial.println("=========================================");
  Serial.println("WiFi conectat. Serverul a pornit.");
  Serial.print("Adresa IP: ");
  Serial.println(WiFi.localIP());  
  Serial.println("=========================================");
  
  server.begin();
  
}


void RespondToClient(WiFiClient& client, int currentState){
  
  String htmlContent = "";
  
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close"); // Închide conexiunea după răspuns
  client.println();
  
  if (currentState == 1) {
    // Suntem în starea HIGH (URL: /H)
    htmlContent += "<h1>Stare Curenta: HIGH</h1>";
    htmlContent += "<p>Aici este continutul paginii pentru HIGH.</p>";
    htmlContent += "Apasa <a href=\"/L\">aici</a> pentru a schimba in LOW.<br>";
  } else if (currentState == 2) {
    // Suntem în starea LOW (URL: /L)
    htmlContent += "<h1>Stare Curenta: LOW</h1>";
    htmlContent += "<p>Aici este continutul paginii pentru LOW.</p>";
    htmlContent += "Apasa <a href=\"/H\">aici</a> pentru a schimba in HIGH.<br>";
  } else {
    // Starea inițială sau URL-ul rădăcină (/)
    htmlContent += "<h1>Server Web Simplu</h1>";
    htmlContent += "<p>Alegeti o optiune:</p>";
    htmlContent += "Apasa <a href=\"/H\">aici</a> pentru a afisa HIGH.<br>";
    htmlContentContent += "Apasa <a href=\"/L\">aici</a> pentru a afisa LOW.<br>";
  }
  
  client.println(htmlContent);
}


void loop(){
  WiFiClient client = server.available();  
  if (client) {                            
    Serial.println("\nClient Nou Conectat.");        
    memset(line, 0, 256); // Curăță bufferul
    idx = 0;
    
    // Logica de citire a cererii HTTP
    while (client.connected()) {          
      if (client.available()) {            
        char c = client.read();       
        
        if (c == '\n') { // S-a terminat o linie
          
          if (idx == 0) {
            RespondToClient(client, state);
            break; // Oprește citirea și deconectează clientul
            
          } else {
            
            // Verifică URL-ul accesat
            if (strstr(line, "GET /H")) {
              state = 1; // Stare HIGH
              Serial.print("URL Accesat: ");
              Serial.println(line);
            }
            else if (strstr(line, "GET /L")) {
              state = 2; // Stare LOW
              Serial.print("URL Accesat: ");
              Serial.println(line);
            }
            else if (strstr(line, "GET / ")) {
              state = 0; // Stare Initială (pentru pagina principală)
              Serial.print("URL Accesat: ");
              Serial.println(line);
            }
            memset(line, 0, 256); 
            idx = 0;
          }
        }
        else {
          if (c != '\r' && idx < 255)
            line[idx++] = c;
        }
      }
    }
    client.stop();
    Serial.println("Client Deconectat.");
  }
}