#include <WiFi.h>
#include <SPI.h>

const char* ssid     = "Retea WIFi";
const char* password = "Parola retea WiFi";

const char* style_css = 
    "body { background-color: red; text-align: center; }";

int state = 0; 
char line[256] = {}; 
int idx = 0;  
 
WiFiServer server(80);

void setup()
{
  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.print("WiFi connected. IP address: ");
  Serial.println(WiFi.localIP());  
  server.begin();
}

void RespondToCSS(WiFiClient& client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/css"); 
  client.println("Connection: close");
  client.println();
  client.print(style_css);
}

void RespondToClient(WiFiClient& client, int currentState) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();

  client.println("<!DOCTYPE html><html><head><title>ESP32 Web Server Styled</title>");
  client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.println("<link rel=\"stylesheet\" href=\"/style.css\">"); // Aici se leaga CSS-ul!
  client.println("</head><body>");

  if (currentState == 1) {
    client.println("<h1 class=\"high\">HIGH State</h1>");
    client.println("<p>Starea curenta este <b class=\"high\">HIGH</b>.</p>");
    client.print("<a href=\"/L\">Schimba la LOW</a><br>");
  } else if (currentState == 2) {
    client.println("<h1 class=\"low\">LOW State</h1>");
    client.println("<p>Starea curenta este <b class=\"low\">LOW</b>.</p>");
    client.print("<a href=\"/H\">Schimba la HIGH</a><br>");
  } else {
    // Pagina de baza
    client.println("<h1>Welcome!</h1>");
    client.println("<p>Selecteaza o stare:</p>");
    client.print("<a href=\"/H\">Afiseaza HIGH</a><br>");
    client.print("<a href=\"/L\">Afiseaza LOW</a><br>");
  }
  
  client.println("</body></html>");
}

void loop() {
  WiFiClient client = server.available();  
  if (client) {                            
    Serial.println("New Client.");        
    memset(line, 0, 256);
    idx = 0;
    state = 0;
    bool currentLineIsBlank = true; 
    bool requestIsHandled = false; // Flag pentru a sti daca am raspuns

    while (client.connected()) {          
      if (client.available()) {            
        char c = client.read();
        
        if (c != '\r' && idx < 255) {
            line[idx++] = c;
        }

        if (c == '\n') {
          if (currentLineIsBlank) {
            Serial.print("Request: ");
            Serial.println(line); 

            // **LOGICA PENTRU CSS**
            if (strstr(line, "GET /style.css")) {
              RespondToCSS(client);
              requestIsHandled = true;
            } 
            // **LOGICA PENTRU HTML**
            else {
              if (strstr(line, "GET /H")) {
                state = 1;
              } else if (strstr(line, "GET /L")) {
                state = 2;
              } else {
                state = 0; 
              }
              RespondToClient(client, state);
              requestIsHandled = true;
            }

            break; 
          }
          memset(line, 0, 256);
          idx = 0;
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    
    if (requestIsHandled || !client.connected()) {
        client.stop();
        Serial.println("Client Disconnected.");
    }
  }
}