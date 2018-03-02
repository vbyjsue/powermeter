#include "ESPHelper.h"

#define PUB_BLINK "pwrm/blink"
#define PUB_MIN "pwrm/min"
#define PUB_10S "pwrm/10s"
#define PUB_LOG "log/powwa"
//#define SUB_PUMP "cmnd/esplant/pump"

#define LDR_PIN 4

#define INTERVAL (1000 * 60)


netInfo homeNet = { homeNet.mqttHost = "192.168.0.114", homeNet.mqttUser = "", homeNet.mqttPass = "",homeNet.mqttPort = 1883,homeNet.ssid = "Lovas-internet",homeNet.pass = "" };

ESPHelper esp(&homeNet);

void blink();
void log(const char* msg);
void callback(char* topic, uint8_t* payload, unsigned int length);

void setupInterrupt();
void loopInterrupt();

void setupPolling();
void loopPolling();

int connectionStatus;
unsigned long previousMillis = 0;
unsigned long currentMillis;

unsigned long blinkMillis = 0;
unsigned long lastBlinkMillis = 0;

int count = 0;
unsigned long last10s;
unsigned long next10s;

int countMin = 0;
unsigned long lastMin;
unsigned long nextMin;

int num10s = 0;

char num[20];

String data;

int	lastState = -1;
int state;
unsigned long debounceMillis = 0;


void setup()
{
	Serial.begin(115200);
	Serial.println("Starting up power meter");

	pinMode(LED_BUILTIN, OUTPUT);


	//setupInterrupt();
	setupPolling();


	esp.setMQTTCallback(callback);
	//esp.addSubscription(SUB_PUMP);
	esp.OTA_enable();
	esp.OTA_setHostname("powwa");
	esp.begin();
	previousMillis = millis() + 100000;
}

void loop()
{
	connectionStatus = esp.loop();
	currentMillis = millis();
	if (connectionStatus == 3)
	{
		loopPolling();
		//loopInterrupt();
	}

	yield();
}

void setupPolling() {
	pinMode(LDR_PIN, INPUT_PULLUP);
}


void loopPolling() {

	state = digitalRead(LDR_PIN);

	//same state return...
	if (state == lastState) { return; }


	//debounce
	if (currentMillis < debounceMillis) { return; }

	lastState = state;
	debounceMillis = currentMillis + 100;
	digitalWrite(LED_BUILTIN, state);

	if (state != LOW) { return; } // only act on blinks

	//BLINK
	countMin++;

	if (lastBlinkMillis == 0) {

		log("First blink with wifi, set as start point");
		lastBlinkMillis = currentMillis;
		return;
	}


	ltoa(currentMillis - lastBlinkMillis, num, 10);
	esp.publish(PUB_BLINK, num);
	Serial.println(num);


	// Min reporting...
	if (currentMillis > nextMin) {
		data = "";
		data += countMin;
		data += ";";
		data += currentMillis - lastMin;

		esp.publish(PUB_MIN, data.c_str());

		countMin = 0;
		lastMin = currentMillis;
		nextMin = currentMillis + 60000;
	}

	lastBlinkMillis = currentMillis;

}

void setupInterrupt() {
	pinMode(LDR_PIN, INPUT_PULLUP);
	attachInterrupt(LDR_PIN, blink, FALLING);
}

void loopInterrupt() {
	if (currentMillis > previousMillis) {

		num10s++;

		if (next10s == 0) {
			next10s = blinkMillis + 10000;
			last10s = blinkMillis;
			num10s = 0;
		}

		if (blinkMillis > next10s) {
			data = "";
			data += num10s;
			data += ";";
			data += blinkMillis - last10s;

			esp.publish(PUB_10S, data.c_str());

			num10s = 0;
			last10s = blinkMillis;
			next10s = blinkMillis + 10000;
		}


		if (lastBlinkMillis != 0) {
			ltoa(blinkMillis - lastBlinkMillis, num, 10);
			esp.publish(PUB_BLINK, num);
			Serial.println(num);
		}

		lastBlinkMillis = blinkMillis;



		//delay(10);
		digitalWrite(LED_BUILTIN, HIGH);
		//esp.publish(PUB_POWWA, "powwa!!");
		Serial.println("powwa in loop");
		previousMillis = currentMillis + 100000;
	}
}


void blink() {

	if (connectionStatus == 3)
	{
		digitalWrite(LED_BUILTIN, LOW);
		previousMillis = millis() + 100;
		blinkMillis = millis();
	}

	//esp.loop();
	//yield();
}

void callback(char* topic, uint8_t* payload, unsigned int length) {

	//Serial.println("Got callback");
	Serial.println(topic);
}

void log(const char* msg) {

	esp.publish(PUB_LOG, msg);

}
