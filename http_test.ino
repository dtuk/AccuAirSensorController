#include <SoftwareSerial.h>
#define RX 10
#define TX 11

#define co2AnalogPin A1
#define co2PWMPin 9
#define co2RX 6
#define co2TX 5


const long samplePeriod = 1000L;

//String AP = "Dialog 4G 105";       // CHANGE ME
//String PASS = "11464984";
//String HOST = "192.168.8.156";
//String PORT = "8000";

//String AP = "kithmi";       // CHANGE ME
//String PASS = "11464984";
//String HOST = "accuair.cf";
//String PORT = "80";

String AP = "kithmi";       // CHANGE ME
String PASS = "11464984";
String HOST = "192.168.43.85";
String PORT = "8000";

boolean isAT = false;
boolean isCWMODE = false;

boolean isConnected = false;

//int mux_timeout = 10;

SoftwareSerial esp8266(RX,TX);

SoftwareSerial sensor(co2RX, co2TX);

const byte requestReading[] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
byte result[9];
long lastSampleTime = 0;

void setup() {
  Serial.begin(9600);
  sensor.begin(9600);
  esp8266.begin(115200);
  pinMode(co2PWMPin, INPUT_PULLUP);
  isConnected = connectToWifi();
}

void loop() {
   
  
  if(isConnected){

//    delay(60000);
    String getData = "POST /api/read?di=1&ac=MQ135&"+String(getLoc())+"&co="+String(getCoReading())+"&co2="+String(getCo2Data())+"&tem="+String(getSensorData())+"&hm="+String(getSensorData());
     if(sendCommand("AT+CIPMUX=1",5, (char *) "OK")){
        sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15, (char *) "OK");
        delay(3000);
        sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4, (char *) ">");
        delay(3000);
        Serial.println(getData);
        esp8266.println(getData);
        esp8266.println();
        esp8266.println();
        esp8266.println();
        esp8266.println();
        delay(3000);
        sendCommand("AT+CIPCLOSE=0",5, (char *) "OK");
        

      
  }
     else{
//      isConnected = connectToWifi();
      delay(3000);
//      mux_timeout--;
//      if(mux_timeout <= 0){
//        isConnected = connectToWifi();
//        mux_timeout = 10;
//      }
     }
  }
  else{
    delay(3000);
    isConnected = connectToWifi();
  }
  
}


int getSensorData(){
  return random(40, 60);
//  return random(1000); // Replace with 
}

String getLoc(){
  String lat = "6."+String(random(1000, 9999));
  String lng = "79."+String(random(1000, 9999));
  return "lat="+lat+"&lng="+lng;
}

int getCo2Data(){
   /* CO2 */
//      long now = millis();
//      if (now > lastSampleTime + samplePeriod) {
//        lastSampleTime = now;
//        int ppmV = readPPMV();
//        int ppmS = readPPMSerial();
//        int ppmPWM = readPPMPWM();
//        Serial.print(ppmV); 
//        Serial.print("\t"); 
//        Serial.print(ppmPWM); 
//        Serial.print("\t");
//        Serial.println(ppmS);
//      }

//  int ppmV = readPPMV();
//        int ppmS = readPPMSerial();
//        int ppmPWM = readPPMPWM();
//        Serial.print(ppmV); 
//        Serial.print("\t"); 
//        Serial.print(ppmPWM); 
//        Serial.print("\t");
//        Serial.println(ppmS);

//        return ppmV;
  int ar = analogRead(co2AnalogPin);
  Serial.println("CO2 AR = "+String(ar));
  float v = ar * 5.0 / 1023.0;
  Serial.println("CO2 V = "+String(v));
  int ppm = int((v - 0.4) * 3125.0);
  Serial.println("CO2 ppm = "+String(ppm));
  return ppm;

}

/* CO2 Sensor readings */

int readPPMV() {
  float v = analogRead(co2AnalogPin) * 5.0 / 1023.0;
  int ppm = int((v - 0.4) * 3125.0);
  return ppm;
}

int readPPMSerial() {
  for (int i = 0; i < 9; i++) {
    sensor.write(requestReading[i]); 
  }
  //Serial.println("sent request");
  while (sensor.available() < 9) {}; // wait for response
  for (int i = 0; i < 9; i++) {
    result[i] = sensor.read(); 
  }
  int high = result[2];
  int low = result[3];
    //Serial.print(high); Serial.print(" ");Serial.println(low);
  return high * 256 + low;
}

int readPPMPWM() {
  while (digitalRead(co2PWMPin) == LOW) {}; // wait for pulse to go high
  long t0 = millis();
  while (digitalRead(co2PWMPin) == HIGH) {}; // wait for pulse to go low
  long t1 = millis();
  while (digitalRead(co2PWMPin) == LOW) {}; // wait for pulse to go high again
  long t2 = millis();
  long th = t1-t0;
  long tl = t2-t1;
  long ppm = 5000L * (th - 2) / (th + tl - 4);
  while (digitalRead(co2PWMPin) == HIGH) {}; // wait for pulse to go low
  delay(10); // allow output to settle.
  return int(ppm);
}

/* End of CO2 Sensor readings */




int getCoReading(){
  int co = analogRead(0);
//  Serial.print("\n\nAir Quality = ");
//  Serial.print(co, DEC);
//  Serial.print(" PPM \n\n");
  Serial.print("\n\nAir Quality (st) = "+String(co)+" PPM \n\n");
  return co;
}

boolean sendCommand(String command, int maxTime, char readReplay[]){

  boolean res = false;
  String esp_res = "";
  
  Serial.println("Sending Command: "+command);
  esp8266.println(command);
//  delay(3000);
  
  if(esp8266.available() > 0){
      Serial.println("\n\n=====================ESP res: ");
      esp_res = esp8266.readString();
      Serial.println(esp_res);
      Serial.println("End of ESP res=====================\n\n ");
      Serial.println("==**==**==**==**==\nfor command: "+command+"\n==**==**==**==**==\n\n");
    }

    if( esp_res.indexOf(readReplay) > 0)//ok
    {
      return true;
    }
    else{
      return false;
    }
}

 boolean connectToWifi(){

  if(!isAT){
    delay(3000);
    isAT = sendCommand("AT",5, (char *) "OK");
    return connectToWifi();
  }
  else if(!isCWMODE){
    delay(3000);
    isCWMODE = sendCommand("AT+CWMODE=1",5, (char *) "OK");
    return connectToWifi();
  }
  else {
      
      delay(3000);
      return sendCommand("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"",20, (char *) "OK");
  }
}
