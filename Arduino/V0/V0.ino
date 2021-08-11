#include <SPI.h>
#include <WiFi101.h>


float chargingCurrent;
float loadCurrent;
float chargingVoltage;
float batteryVoltage;
int batteryPer;
float power;
float m = 0.0438460187666577;
float b = -22.569401590977478;


String parsed;

//char ssid[] = "TEST AP";
//char pass[] = "0123456789";    

char ssid[] = "DeEee_2.4";
char pass[] = "1tpFip$w4";    

int keyIndex = 0;            

int status = WL_IDLE_STATUS;


WiFiClient client;


char server[] = "18.117.252.6";
//IPAddress server(64,131,82,241);

unsigned long lastConnectionTime = 0;            // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 10L * 100L; // delay between updates, in milliseconds

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(9600); //Start Serial Monitor to display current read value on Serial monitor
  
//  while (!Serial) {
//    ; // wait for serial port to connect. Needed for native USB port only
//  }

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to WiFi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  // you're connected now, so print out the status:
  printWiFiStatus();
}

float measureChargingCurrent(){
  unsigned int x=0;
  float AcsValue=0.0,Samples=0.0,AvgAcs=0.0,AcsValueF=0.0,cc=0.0;
  for (int x = 0; x < 150; x++){ //Get 150 samples
    AcsValue = analogRead(A4);     //Read current sensor values   
    Samples = Samples + AcsValue;  //Add samples together
    delay (3); // let ADC settle before next sample 3ms
  }
  AvgAcs=Samples/150.0;//Taking Average of Samples

  cc = (m*AvgAcs)+b;
//  cc = AvgAcs*0.00075564;
  Serial.println("......................");
  Serial.println(String(cc)+"\t"+String(AvgAcs));
  Serial.println("......................");
  if(cc < 0.09){
    cc = 0.0;
  }
  return cc;
}

float measureLoadCurrent(){
  unsigned int x=0;
  float AcsValue=0.0,Samples=0.0,AvgAcs=0.0,AcsValueF=0.0,lc=0.0;
  for (int x = 0; x < 150; x++){ //Get 150 samples
    AcsValue = analogRead(A3);     //Read current sensor values   
    Samples = Samples + AcsValue;  //Add samples together
    delay (3); // let ADC settle before next sample 3ms
  }
  AvgAcs=Samples/150.0;//Taking Average of Samples

  lc = (m*AvgAcs)+b;
  if(lc < 0.09){
    lc = 0.0;
  }
  return chargingCurrent;
}

float measureChargingVoltage(){
  int val = analogRead(A2);
  float voltage = val*0.0162162;
  if(voltage < 1){
    voltage = 0.0;
  }
  return voltage;
}

float measureBatteryVoltage(){
  int val = analogRead(A1);
  float voltage = val*0.0162162;
  if(voltage < 1){
    voltage = 0.0;
  }
  return voltage;
}


// this method makes a HTTP connection to the server:
void httpRequest() {
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection:
  if (client.connect(server, 8000)) {
    Serial.println("connecting...");
    // send the HTTP PUT request:
//    client.println("GET / HTTP/1.1");
    parsed = "GET /sensor_data?cc=" + String(chargingCurrent) + "&lc=" + String(loadCurrent) + "&cv=" + String(chargingVoltage) + "&bp=" + String(batteryPer) + "&bv=" + String(batteryVoltage) + "&pw=" + String(power) + " HTTP/1.1";
    Serial.println(parsed);
    client.println(parsed);
    client.println("Host: 192.168.1.113");
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Connection: close");
    client.println();

    // note the time that the connection was made:
    lastConnectionTime = millis();
  }
  else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
    digitalWrite(LED_BUILTIN, LOW);
  }
}


void printWiFiStatus() {
  digitalWrite(LED_BUILTIN, HIGH);
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void loop() {
  chargingCurrent = measureChargingCurrent();
  loadCurrent = measureLoadCurrent();
  chargingVoltage = measureChargingVoltage();
  batteryVoltage = measureBatteryVoltage();
  batteryPer = map(int(batteryVoltage), 7,14,0,100);
  power = batteryVoltage*chargingCurrent;
  if(batteryPer < 0){
    batteryPer = 0;
  }
  if(batteryPer > 100){
    batteryPer = 100;
  }
  Serial.print("chargingCurrent: ");
  Serial.print(chargingCurrent);
  Serial.print("A\tloadCurrent: ");
  Serial.print(loadCurrent);
  Serial.print("A\tchargingVoltage: ");
  Serial.print(chargingVoltage);
  Serial.print("V\tbatteryPer: ");
  Serial.print(batteryPer);
  Serial.println("%");

  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  // if ten seconds have passed since your last connection,
  // then connect again and send data:
  if (millis() - lastConnectionTime > postingInterval) {
    httpRequest();
  }
}
