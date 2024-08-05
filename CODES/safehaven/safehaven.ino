#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>

// Initialize the SoftwareSerial for the SIM module
SoftwareSerial simserial(2, 4);

// Initialize the LCD with I2C address 0x27 and size 20x4
LiquidCrystal_I2C lcd(0x27, 20, 4);

const int sensorpin = A0;
const int threshold = 400; // Set the threshold value
int smokevalue = 0; // Declare smokevalue as a global variable
bool messageInProgress = false; // Flag to indicate if a message is being sent

void setup() {
  pinMode(sensorpin, INPUT);
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Cephas Smoke");
  lcd.setCursor(2, 1);
  lcd.print("Detector");
  delay(4000);
  lcd.clear();
  simserial.begin(9600);
  Serial.println("Initializing...");
  delay(1000);

  // Send AT commands to initialize the SIM module
  if (sendATCommand("AT")) {
    Serial.println("SIM module communication OK");
  } else {
    Serial.println("Failed to communicate with SIM module");
  }

  if (sendATCommand("AT+CSQ")) {
    Serial.println("Signal quality checked");
  } else {
    Serial.println("Failed to check signal quality");
  }

  if (sendATCommand("AT+CCID")) {
    Serial.println("SIM card detected");
  } else {
    Serial.println("SIM card not detected");
  }

  if (sendATCommand("AT+CREG?")) {
    Serial.println("Network registration checked");
  } else {
    Serial.println("Failed to check network registration");
  }
}

void loop() {
  smokevalue = analogRead(sensorpin);
  Serial.print("Smoke value= ");
  Serial.println(smokevalue); // Use println to move to a new line
  lcd.setCursor(0, 0);
  lcd.print("Smoke value");
  lcd.setCursor(3, 1);
  lcd.print(smokevalue);
  delay(1000);
  
  if (smokevalue > threshold && !messageInProgress) {
    Serial.println("Smoke detected");
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Smoke");
    lcd.setCursor(1, 1);
    lcd.print("Detected");
    delay(4000);
    sendMessage();
  }
}

bool sendATCommand(const char* command) {
  simserial.println(command);
  delay(1000); // Wait for the response
  if (simserial.available()) {
    String response = simserial.readString();
    Serial.println("Response: " + response);
    return response.indexOf("OK") != -1;
  }
  return false;
}

void sendMessage() {
  messageInProgress = true; // Set the flag to indicate a message is being sent
  Serial.println("Sending SMS..."); 
  if (sendATCommand("AT+CMGF=1")){
    simserial.print("AT+CMGS=\"+233551560593\"\r");  // Enter phone number
    delay(500);
    simserial.print("Smoke Detected at your apartment. Value: "); 
    simserial.print(smokevalue);  // Send message
    simserial.print("\r\n");
    delay(500);
    simserial.write(26);  // Send Ctrl+Z
    delay(5000);
    Serial.println("Message sent");
  } else {
    Serial.println("Failed to set SMS mode");
  }
  messageInProgress = false; // Reset the flag after the message is sent
}

void updateSerial() {
  delay(500);
  while (Serial.available()) {
    simserial.write(Serial.read());
  }
  while (simserial.available()) {
    Serial.write(simserial.read());
  }
}
