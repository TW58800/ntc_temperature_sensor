/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "c:/Users/timwh/Projects/ntc_temperature_sensor/src/ntc_temperature_sensor.ino"

#include "math.h"

void setup();
void loop();
#line 10 "c:/Users/timwh/Projects/ntc_temperature_sensor/src/ntc_temperature_sensor.ino"
SYSTEM_MODE(AUTOMATIC);
//uint32_t timer = millis();
TCPServer server = TCPServer(23);
TCPClient client;
int LED = D7;
int ntcPin[3] = {A0, A1, A2};
bool TaRising[3] = {false, false, false};
int Vo = 0;
int counter = 0;
float R1 = 10000.0f;
float logR2, R2, T, Tc, Tf;
float Ta[3] = {0.0f, 0.0f, 0.0f};
float TaLast[3] = {0.0f, 0.0f, 0.0f};
float c1 = 0.001125308852122f, c2 = 0.000234711863267f, c3 = 0.000000085663516f;

void setup() {

	// Connects to a network secured with WPA2 credentials.
	//WiFi.setCredentials("Humpty", "ttIImm11&&");
	//WiFi.setConfig(NetworkInterfaceConfig()
  		//.source(NetworkInterfaceConfigSource::STATIC)
  		//.address({192,168,0,7}, {255,255,255,0}));
  		//.gateway({192,168,0,1})
  		//.dns({192,168,0,1}));

	pinMode(LED, OUTPUT);   
	WiFi.connect();
	Serial.begin(9600);
	waitFor(Serial.isConnected, 15000);
	Serial.println("connected");
	Serial.printlnf("localIP=%s", WiFi.localIP().toString().c_str());
	Serial.printlnf("subnetMask=%s", WiFi.subnetMask().toString().c_str());
	Serial.printlnf("gatewayIP=%s", WiFi.gatewayIP().toString().c_str());
	server.begin();
}


void loop() {

	for (int i=0; i<3; i++) {
		Vo = analogRead(ntcPin[i]);  
  		R2 = R1 * (4095.0f / (float)Vo - 1.0f);
  		logR2 = log(R2);
		T = (1.0f / (c1 + (c2*logR2) + (c3*logR2*logR2*logR2)));
		Tc = T - 273.15f;
		Ta[i] = Ta[i] + Tc;
	}
	if (counter >= 100) {
		for (int i=0; i<3; i++) {
			Ta[i] = Ta[i] / (float)counter;
			Serial.printlnf("\nTemperature %02i: %3.1f°C", i, Ta[i]);
			float TaDelta = Ta[i] - TaLast[i];
			if (TaDelta > 0.0f) {
				if ((TaRising[i] == false) & (TaDelta < 0.2f)) {
					Ta[i] = TaLast[i];
				}
				else TaRising[i] = true;
			}
			else {
				if ((TaRising[i] == true) & (TaDelta > -0.2f)) {
					Ta[i] = TaLast[i];
				}
				else TaRising[i] = false;
			}
			TaLast[i] = Ta[i]; 
		}
  		digitalWrite(LED, HIGH); 
  		delay(200);              
  		digitalWrite(LED, LOW);  
  		if (client.status()) { 
    		Serial.println("\nTCP connected");
			while (client.available()) {
    			byte payload = client.read(); 
    			Serial.printlnf("TCP byte received: %i\n", payload);
			}
			byte* ta = reinterpret_cast<byte*>(&Ta);
			int bytes_sent = server.write(ta, 12, 5000);
			int err = server.getWriteError();
			if (err != 0) {
  				Serial.printlnf("TCPServer::write() failed (error = %d), number of bytes written: %d\n", err, bytes_sent);
			}
    	  	delay(8800);              
			}
		else {
			// if no client is yet connected, check for a new connection
			client = server.available();
		}
		for (int i=0; i<3; i++) Ta[i] = 0;
    	counter = 0;
	}   
	counter++;
	delay(10);
	Serial.print(".");
}