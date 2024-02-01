#include <WiFi.h>
#include <Kalman.h>
using namespace BLA;

const char* ssid = "KOKO_CHANNEL";
const char* password = "12345678";
const char* host = "192.168.137.1";
const int potPin1 = 36;
const int potPin2 = 34;
const int potPin3 = 32;
const int numSamples = 30;  // Number of samples for multisampling

// Kalman filter parameters
#define Nstate 3 // length of the state vector
#define Nobs 3   // length of the measurement vector
KALMAN<Nstate, Nobs> K; // your Kalman filter
BLA::Matrix<Nobs> obs; // observation vector

void setup() {
    Serial.begin(115200);

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    // Kalman filter setup
    K.F = {1.0, 0.0, 0.0,
           0.0, 1.0, 0.0,
           0.0, 0.0, 1.0};
    K.H = {1.0, 0.0, 0.0,
           0.0, 1.0, 0.0,
           0.0, 0.0, 1.0};
    // Adjust the noise values based on your system
    K.R = {0.1, 0.0, 0.0,
           0.0, 0.1, 0.0,
           0.0, 0.0, 0.1};
    K.Q = {0.01, 0.0, 0.0,
           0.0, 0.01, 0.0,
           0.0, 0.0, 0.01};
}

void loop() {
    float total1 = 0;
    float total2 = 0;
    float total3 = 0;

    // Perform multisampling for each potentiometer
    for (int i = 0; i < numSamples; ++i) {
        total1 += analogRead(potPin1);
        total2 += analogRead(potPin2);
        total3 += analogRead(potPin3);
        // No delay here to avoid timing issues with plotting
    }

    float B = 9.25;
    float A = 0.0625;
    // Calculate the average for each potentiometer
//    float potValue1 = (total1 / numSamples);
//    float potValue2 = (total2 / numSamples);
//    float potValue3 = (total3 / numSamples);

    float potValue1 = ((total1 /numSamples) *A) + B;
    float potValue2 = (((total2 / numSamples) *A) + B)* 1.6667e-02;
    float potValue3 = ((total3 / numSamples) * A) + B;

    // Update Kalman filter observation vector
    obs(0) = potValue1;
    obs(1) = potValue2;
    obs(2) = potValue3;

    // Apply Kalman filter
    K.update(obs);

    // Access the filtered values from K.x
    float filteredPotValue1 = K.x(0);
    float filteredPotValue2 = K.x(1);
    float filteredPotValue3 = K.x(2);

    // Rest of the code remains unchanged

    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
        Serial.println("Connection to server failed");
        return;
    }
    // Send the HTTP GET request with the averaged sensor values
    String url = "/esp/connect.php?Filtered_Data=" + String(filteredPotValue1) +
                 "&pao=" + String(potValue3) +
                 "&v3=" + String(potValue2);

    client.print(String("GET ") + url + " HTTP/1.0\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
    delay(30);  // Adjust this delay if necessary
}
