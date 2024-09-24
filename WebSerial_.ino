#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

// Replace with your network credentials
const char* ssid = "AhmedYasserLap";
const char* password = "123654789";

const int ledPin = 2;  // GPIO pin where the LED is connected

AsyncWebServer server(80);
String serialBuffer = "";  // Buffer to store serial messages

const char* htmlPage = R"rawliteral(
<!DOCTYPE HTML>
<html>
  <head>
    <title>Gloves Web Site</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      body { font-family: Arial, Helvetica, sans-serif; text-align: center; margin: 0; padding: 0; }
      .container { max-width: 600px; margin: 50px auto; padding: 20px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); border-radius: 10px; background-color: #f9f9f9; }
      .input-field { width: calc(100% - 22px); padding: 10px; margin-bottom: 10px; border: 1px solid #ddd; border-radius: 5px; }
      .send-button { padding: 10px 20px; border: none; border-radius: 5px; background-color: #007BFF; color: white; cursor: pointer; }
      .send-button:hover { background-color: #0056b3; }
      .output-field { width: calc(100% - 22px); padding: 10px; margin-top: 10px; border: 1px solid #ddd; border-radius: 5px; height: 200px; overflow-y: auto; }
    </style>
  </head>
  <body>
    <div class="container">
      <h2>Gloves Web Site</h2>
      <input type="text" id="inputField" class="input-field" placeholder="Type here">
      <button id="sendButton" class="send-button">Send</button>
      <div>
        <textarea id="outputField" class="output-field" readonly style="resize: vertical;"></textarea>
      </div>
    </div>
    <script>
      document.getElementById('sendButton').addEventListener('click', function() {
        sendData();
      });

      document.getElementById('inputField').addEventListener('keypress', function(event) {
        if (event.key === 'Enter') {
          event.preventDefault();
          sendData();
        }
      });

      function sendData() {
        var inputField = document.getElementById('inputField');
        var outputField = document.getElementById('outputField');
        var xhr = new XMLHttpRequest();
        xhr.open('GET', '/get?input=' + inputField.value, true);
        xhr.onload = function() {
          if (xhr.status == 200) {
            var newText = 'input Data : ' + inputField.value + '\n';
            outputField.value += newText;
            outputField.scrollTop = outputField.scrollHeight;
            inputField.value = '';
          }
        };
        xhr.send();
      }

      function fetchSerialData() {
        var xhr = new XMLHttpRequest();
        xhr.open('GET', '/serial', true);
        xhr.onload = function() {
          if (xhr.status == 200) {
            var outputField = document.getElementById('outputField');
            if (xhr.responseText) {
              outputField.value += xhr.responseText;
              outputField.scrollTop = outputField.scrollHeight;
            }
          }
        };
        xhr.send();
      }

      setInterval(fetchSerialData, 500);  // Fetch serial data every second
    </script>
  </body>
</html>
)rawliteral";

template<typename T>
void printToWeb(const T& message) {
  serialBuffer += String(message) + "\n";
}

void handleInput(String input) {
  if (input == "on" || input == "ON" || input == "1") {
    digitalWrite(ledPin, HIGH);  // Turn the LED on
  } else if (input == "off" || input == "OFF" || input == "0") {
    digitalWrite(ledPin, LOW);  // Turn the LED off
  }
}

void setup() {
  // Initialize the LED pin as an output
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Begin Serial for debugging
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Connected to WiFi");
  

  // Start the server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/html", htmlPage);
  });

  // Handle input from the user
  server.on("/get", HTTP_GET, [](AsyncWebServerRequest* request) {
    String inputMessage;
    if (request->hasParam("input")) {
      inputMessage = request->getParam("input")->value();
      handleInput(inputMessage);
      Serial.println("Data from Web : " + inputMessage);
    }
    request->send(200, "text/plain", "OK");
  });

  // Endpoint to fetch serial data
  server.on("/serial", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/plain", serialBuffer);
    serialBuffer = "";  // Clear buffer after sending
  });

  server.begin();
}
// unsigned long previousMillis = 0;
// const long interval = 1000;  // 1 ثانية
// int count = 0;

void loop() {
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    printToWeb("Data from Serial: " + line);
    handleInput(line);
    Serial.println("Data from Serial : " + line);
  }



  // unsigned long currentMillis = millis();
  // if (currentMillis - previousMillis >= interval) {
  //   previousMillis = currentMillis;
  //   count++;
  //   Serial.println(count);
  //   printToWeb("");
  //   printToWeb(count);
  //   printToWeb("");
  //   printToWeb("count : " + String(count));
  // }
}
