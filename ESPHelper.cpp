/*    
	ESPHelper.cpp
	Copyright (c) 2017 ItKindaWorks Inc All right reserved.
	github.com/ItKindaWorks

	This file is part of ESPHelper

	ESPHelper is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	ESPHelper is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with ESPHelper.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "ESPHelper.h"
#include <WiFiClientSecure.h>
//empy initializer 
ESPHelper::ESPHelper(){	}

//initializer with single netInfo network
ESPHelper::ESPHelper(netInfo *startingNet){	

	//diconnect from and previous wifi networks
	WiFi.softAPdisconnect();
	WiFi.disconnect();	

	//setup current network information
	_currentNet = *startingNet;

	//validate various bits of network/MQTT info

	//network pass
	if(_currentNet.pass[0] == '\0'){_passSet = false;}
	else{_passSet = true;}

	//ssid
	if(_currentNet.ssid[0] == '\0'){_ssidSet = false;}
	else{_ssidSet = true;}	

	//mqtt host
	if(_currentNet.mqttHost[0] == '\0'){_mqttSet = false;}
	else{_mqttSet = true;}

	//mqtt port
	if(_currentNet.mqttPort == 0){_currentNet.mqttPort = 1883;}
  
	//mqtt username
	if(_currentNet.mqttUser[0] == '\0'){_mqttUserSet = false;}
	else{_mqttUserSet = true;}

	//mqtt password
	if(_currentNet.mqttPass[0] == '\0'){_mqttPassSet = false;}
	else{_mqttPassSet = true;}

	//disable hopping on single network
	_hoppingAllowed = false;

	//disable ota by default
	_useOTA = false;
}


//initializer with netInfo array and index
ESPHelper::ESPHelper(netInfo *netList[], uint8_t netCount, uint8_t startIndex){	

	//diconnect from and previous wifi networks
	WiFi.softAPdisconnect();
	WiFi.disconnect();	
	_netList = netList;
	_netCount = netCount;
	_currentIndex = startIndex;

	//enable hopping since we have an array of networks to use
	_hoppingAllowed = true;

	//disable ota by default
	_useOTA = false;

	//select the network from the array that the user specfied
	_currentNet = *netList[constrain(_currentIndex, 0, _netCount)];

	//validate various bits of network/MQTT info

	//network pass
	if(_currentNet.pass[0] == '\0'){_passSet = false;}
	else{_passSet = true;}

	//ssid
	if(_currentNet.ssid[0] == '\0'){_ssidSet = false;}
	else{_ssidSet = true;}	

	//mqtt host
	if(_currentNet.mqttHost[0] == '\0'){_mqttSet = false;}
	else{_mqttSet = true;}

	//mqtt port
	if(_currentNet.mqttPort == 0){_currentNet.mqttPort = 1883;}
  
	//mqtt username
	if(_currentNet.mqttUser[0] == '\0'){_mqttUserSet = false;}
	else{_mqttUserSet = true;}

	//mqtt password
	if(_currentNet.mqttPass[0] == '\0'){_mqttPassSet = false;}
	else{_mqttPassSet = true;}

}

//initializer with single network information and MQTT broker
ESPHelper::ESPHelper(const char *ssid, const char *pass, const char *mqttIP){	

	//diconnect from and previous wifi networks
	WiFi.softAPdisconnect();
	WiFi.disconnect();
	_currentNet.ssid = ssid;
	_currentNet.pass = pass;
	_currentNet.mqttHost= mqttIP;
	_currentNet.mqttPort = 1883;

	//disable hopping on single network
	_hoppingAllowed = false;

	_useOTA = false;

	_mqttPassSet = false;
	_mqttUserSet = false;

	//validate various bits of network/MQTT info

	//network pass
	if(_currentNet.pass[0] == '\0'){_passSet = false;}
	else{_passSet = true;}

	//ssid
	if(_currentNet.ssid[0] == '\0'){_ssidSet = false;}
	else{_ssidSet = true;}	

	//mqtt host
	if(_currentNet.mqttHost[0] == '\0'){_mqttSet = false;}
	else{_mqttSet = true;}
}

//initializer with single network information (MQTT user/pass)
ESPHelper::ESPHelper(const char *ssid, const char *pass, const char *mqttIP, const char *mqttUser, const char *mqttPass, const int mqttPort){

	//diconnect from and previous wifi networks
	WiFi.softAPdisconnect();
	WiFi.disconnect();

	_currentNet.ssid = ssid;
	_currentNet.pass = pass;
	_currentNet.mqttHost= mqttIP;
	_currentNet.mqttUser = mqttUser;
	_currentNet.mqttPass = mqttPass;
	_currentNet.mqttPort = mqttPort;

	//disable hopping on single network
	_hoppingAllowed = false;

	_useOTA = false;

	//validate various bits of network/MQTT info

	//network pass
	if(_currentNet.pass[0] == '\0'){_passSet = false;}
	else{_passSet = true;}

	//ssid
	if(_currentNet.ssid[0] == '\0'){_ssidSet = false;}
	else{_ssidSet = true;}	

	//mqtt host
	if(_currentNet.mqttHost[0] == '\0'){_mqttSet = false;}
	else{_mqttSet = true;}
  
	//mqtt username
	if(_currentNet.mqttUser[0] == '\0'){_mqttUserSet = false;}
	else{_mqttUserSet = true;}

	//mqtt password
	if(_currentNet.mqttPass[0] == '\0'){_mqttPassSet = false;}
	else{_mqttPassSet = true;}
}

//initializer with single network information (no MQTT)
ESPHelper::ESPHelper(const char *ssid, const char *pass){

	//diconnect from and previous wifi networks
	WiFi.softAPdisconnect();
	WiFi.disconnect();
	_currentNet.ssid = ssid;
	_currentNet.pass = pass;
	_currentNet.mqttHost= '\0';
	_currentNet.mqttPort = 1883;

	//disable hopping on single network
	_hoppingAllowed = false;

	_useOTA = false;

	_mqttSet = false;
	_mqttUserSet = false;
	_mqttPassSet = false;

	//validate various bits of network info

	//network pass
	if(_currentNet.pass[0] == '\0'){_passSet = false;}
	else{_passSet = true;}

	//ssid
	if(_currentNet.ssid[0] == '\0'){_ssidSet = false;}
	else{_ssidSet = true;}	
}



//start the wifi & mqtt systems and attempt connection (currently blocking)
	//true on: parameter check validated
	//false on: parameter check failed
bool ESPHelper::begin(){	
	if(_ssidSet){
		// Generate client name based on MAC address and last 8 bits of microsecond counter
		_clientName += "esp8266-";
		uint8_t mac[6];
		WiFi.macAddress(mac);
		_clientName += macToStr(mac);

		//set the wifi mode to station and begin the wifi (connect using either ssid or ssid/pass)
		WiFi.mode(WIFI_STA);
		if(_passSet){WiFi.begin(_currentNet.ssid, _currentNet.pass);}
		else{WiFi.begin(_currentNet.ssid);}

		//as long as an mqtt ip has been set create an instance of PubSub for client
		if(_mqttSet){
			//make mqtt client use either the secure or non-secure wifi client depending on the setting
			if(_useSecureClient){client = PubSubClient(_currentNet.mqttHost, _currentNet.mqttPort, wifiClientSecure);}
			else{client = PubSubClient(_currentNet.mqttHost, _currentNet.mqttPort, wifiClient);}
			
			//set the mqtt message callback if needed
			if(_mqttCallbackSet){
				client.setCallback(_mqttCallback);
			}
		}

		//define a dummy instance of mqtt so that it is instantiated if no mqtt ip is set
		else{
			//make mqtt client use either the secure or non-secure wifi client depending on the setting
			//(this shouldnt be needed if making a dummy connection since the idea would be that there wont be mqtt in this case)
			if(_useSecureClient){client = PubSubClient("192.0.2.0", _currentNet.mqttPort, wifiClientSecure);}
			else{client = PubSubClient("192.0.2.0", _currentNet.mqttPort, wifiClient);}
			
		}

		
		//ota event handlers
		ArduinoOTA.onStart([]() {/* ota start code */});
		ArduinoOTA.onEnd([]() {
			//on ota end we disconnect from wifi cleanly before restarting.
			WiFi.softAPdisconnect();
			WiFi.disconnect();
			int timeout = 0;
			//max timeout of 2seconds before just dropping out and restarting
			while(WiFi.status() != WL_DISCONNECTED && timeout < 200){
				delay(10);
				timeout++;
			}
		});
		ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {/* ota progress code */});
		ArduinoOTA.onError([](ota_error_t error) {/* ota error code */});

		//initially attempt to connect to wifi when we begin (but only block for 2 seconds before timing out)
		int timeout = 0;	//counter for begin connection attempts
		while (((!client.connected() && _mqttSet) || WiFi.status() != WL_CONNECTED) && timeout < 200 ) {	//max 2 sec before timeout
			reconnect();
			delay(10);
			timeout++;
		}

		//attempt to start ota if needed
		OTA_begin();
		
		//mark the system as started and return
		_hasBegun = true;
		return true;
	}

	//if no ssid was set even then dont try to begin and return false
	return false;
}

//end the instance of ESPHelper (shutdown wifi, ota, mqtt)
void ESPHelper::end(){
	OTA_disable();
	WiFi.softAPdisconnect();
	WiFi.disconnect();

	int timeout = 0;
	while(WiFi.status() != WL_DISCONNECTED && timeout < 200){
		delay(10);
		timeout++;
	}
}

//enables the use of a secure (SSL) connection to an MQTT broker. 
//(Make sure your mqtt port is set to one expecting a secure connection)
void ESPHelper::useSecureClient(const char* fingerprint){
	_fingerprint = fingerprint;

	//fall back to wifi only connection if it was previously at full connection 
	//(because we just changed how the device is going to connect to the mqtt broker)
	if(setConnectionStatus() == FULL_CONNECTION){
		_connectionStatus = WIFI_ONLY;
	}

	//if use of secure connection is set retroactivly (after begin), then re-instantiate client
	if(_hasBegun){client = PubSubClient(_currentNet.mqttHost, _currentNet.mqttPort, wifiClientSecure);}
	
	//flag use of secure client
	_useSecureClient = true;
}

//enables and sets up broadcast mode rather than station mode. This allows users to create a network from the ESP
//and upload using OTA even if there is no network already present. This disables all MQTT connections
void ESPHelper::broadcastMode(const char* ssid, const char* password, const IPAddress ip){
	//disconnect from any previous wifi networks (max timeout of 2 seconds)
	WiFi.softAPdisconnect();
	WiFi.disconnect();
	int timeout = 0;
	while(WiFi.status() != WL_DISCONNECTED && timeout < 200){
		delay(10);
		timeout++;
	}
	//set the mode for access point
	WiFi.mode(WIFI_AP);
	//config the AP
	WiFi.softAPConfig(ip, ip, IPAddress(255, 255, 255, 0));
	//set the ssid and password
	WiFi.softAP(ssid, password);

	//update the connection status
	_connectionStatus = BROADCAST;
}

//disable broadcast mode and reset to station mode (causes a call to begin - may want to change this in the future...)
void ESPHelper::disableBroadcast(){
	//disconnect from any previous wifi networks (max timeout of 2 seconds)
	WiFi.softAPdisconnect();
	WiFi.disconnect();
	int timeout = 0;
	while(WiFi.status() != WL_DISCONNECTED && timeout < 200){
		delay(10);
		timeout++;
	}
	_connectionStatus = NO_CONNECTION;
	begin();
}

//main loop - should be called as often as possible - handles wifi/mqtt connection and mqtt handler
	//true on: network/server connected
	//false on: network or server disconnected
int ESPHelper::loop(){	
	if(_ssidSet){

		//check for good connections and attempt a reconnect if needed
		if (((_mqttSet && !client.connected()) || setConnectionStatus() < WIFI_ONLY) && _connectionStatus != BROADCAST) {
			reconnect();
		}


		//run the wifi loop as long as the connection status is at a minimum of BROADCAST
		if(_connectionStatus >= BROADCAST){
			
			//run the MQTT loop if we have a full connection
			if(_connectionStatus == FULL_CONNECTION) {
				client.loop();
				ESP.wdtFeed();
			}
			
			//run the heartbeat
			heartbeat();

			//check for whether we want to use OTA and whether the system is running
			if(_useOTA && _OTArunning) {ArduinoOTA.handle();}

			//if we want to use OTA but its not running yet, start it up.
			else if(_useOTA && !_OTArunning){
				ESP.wdtFeed();
				OTA_begin();
				ArduinoOTA.handle();
			}


			return _connectionStatus;
		}
	}

	//return -1 for no connection because of bad network info
	return -1;
}

//subscribe to a speicifc topic (does not add to topic list)
	//true on: subscription success
	//false on: subscription failed (either from PubSub lib or network is disconnected)
bool ESPHelper::subscribe(const char* topic, int qos){		
	if(_connectionStatus == FULL_CONNECTION){
		//set the return value to the output of subscribe
		bool returnVal = client.subscribe(topic, qos);

		//loop mqtt client
		client.loop();
		return returnVal;
	}

	//if not fully connected return false
	else{return false;}
}

//add a topic to the list of subscriptions and attempt to subscribe to the topic on the spot
	//true on: subscription added to list (does not guarantee that the topic was subscribed to, only that it was added to the list)
	//false on: subscription not added to list
bool ESPHelper::addSubscription(const char* topic){	
	//default return value is false
	bool subscribed = false;

	//loop throough finding the next available slot for a subscription and add it
	for(int i = 0; i < MAX_SUBSCRIPTIONS; i++){
		if(_subscriptions[i].isUsed == false){
			_subscriptions[i].topic = topic;
			_subscriptions[i].isUsed = true;
			subscribed = true;
			break;
		}
	}

	//if added to the list, subscibe to the topic
	if(subscribed){subscribe(topic, _qos);}
	
	return subscribed;
}

//loops through list of subscriptions and attempts to subscribe to all topics
void ESPHelper::resubscribe(){	
	for(int i = 0; i < MAX_SUBSCRIPTIONS; i++){
		if(_subscriptions[i].isUsed){
			subscribe(_subscriptions[i].topic, _qos);
			yield();
		}
	}
}

//attempts to remove a topic from the topic list
	//true on: subscription removed from list (does not guarantee that the topic was unsubscribed from, only that it was removed from the list)
	//false on: topic was not found in list and therefore cannot be removed
bool ESPHelper::removeSubscription(const char* topic){	
	bool returnVal = false;
	String topicStr = topic;

	//loop through all subscriptions
	for(int i = 0; i < MAX_SUBSCRIPTIONS; i++){
		//if an element is used, check for it being the one we want to remove
		if(_subscriptions[i].isUsed){
			String subStr = _subscriptions[i].topic;
			if(subStr.equals(topicStr)){
				//reset the used flag to false
				_subscriptions[i].isUsed = false;

				//unsubscribe
				client.unsubscribe(_subscriptions[i].topic);
				returnVal = true;
				break;
			}
		}
	}

	return returnVal;
}

//manually unsubscribes from a topic (This is basically just a wrapper for the pubsubclient function)
bool ESPHelper::unsubscribe(const char* topic){
	return client.unsubscribe(topic);
}

//publish to a specified topic
void ESPHelper::publish(const char* topic, const char* payload){		
	publish(topic, payload, false);
}

//publish to a specified topic with a given retain level
void ESPHelper::publish(const char* topic, const char* payload, bool retain){		
	client.publish(topic, payload, retain);
}

//set the callback function for MQTT
void ESPHelper::setMQTTCallback(MQTT_CALLBACK_SIGNATURE){
	_mqttCallback = callback;

	//only set the callback if using mqtt AND the system has already been started. Otherwise just save it for later	
	if(_hasBegun && _mqttSet) {
		client.setCallback(_mqttCallback);
	}
	_mqttCallbackSet = true;
}

//legacy funtion - here for compatibility. Sets the callback function for MQTT (see function above)
bool ESPHelper::setCallback(MQTT_CALLBACK_SIGNATURE){
	setMQTTCallback(callback);
	return true;
}




//sets a custom function to run when connection to wifi is established
void ESPHelper::setWifiCallback(void (*callback)()){
	_wifiCallback = callback;
	_wifiCallbackSet = true;
}

//attempts to connect to wifi & mqtt server if not connected
void ESPHelper::reconnect() {		
	static int tryCount = 0;

	if(reconnectMetro.check() && _connectionStatus != BROADCAST && setConnectionStatus() != FULL_CONNECTION){
		debugPrintln("Attempting WiFi Connection...");
		//attempt to connect to the wifi if connection is lost
		if(WiFi.status() != WL_CONNECTED){
			_connectionStatus = NO_CONNECTION;

			//increment try count each time it cannot connect (this is used to determine when to hop to a new network)
			tryCount++;
			if(tryCount == 20){
				//change networks (if possible) when we have tried to connnect 20 times unsucessfully
				changeNetwork();
				tryCount = 0;
				return;
			}
		}

		// make sure we are connected to WIFI before attemping to reconnect to MQTT
		//----note---- maybe want to reset tryCount whenever we succeed at getting wifi connection?
		if(WiFi.status() == WL_CONNECTED){
			//if the wifi previously wasnt connected but now is, run the callback
			if(_connectionStatus < WIFI_ONLY && _wifiCallbackSet){	
				_wifiCallback();
			}


			debugPrintln("\n---WIFI Connected!---");
			_connectionStatus = WIFI_ONLY;
			

			//attempt to connect to mqtt when we finally get connected to WiFi
			if(_mqttSet){

				static int timeout = 0;	//allow a max of 5 mqtt connection attempts before timing out
				if (!client.connected() && timeout < 5) {
					debugPrint("Attemping MQTT connection");

					
					int connected = 0;

					//connect to mqtt with user/pass
					if (_mqttUserSet) {
						connected = client.connect((char*) _clientName.c_str(), _currentNet.mqttUser, _currentNet.mqttPass);
					}

					//connect to mqtt without credentials
					else{
						connected = client.connect((char*) _clientName.c_str());
					}

					//if connected, subscribe to the topic(s) we want to be notified about
					if (connected) {
						debugPrintln(" -- Connected");

						//if using https, verify the fingerprint of the server before setting full connection (return on fail)
						if(_useSecureClient){
							if (wifiClientSecure.verify(_fingerprint, _currentNet.mqttHost)) {
								debugPrintln("Certificate Matches - SUCESS");
							} else {
								debugPrintln("Certificate Doesn't Match - FAIL");
								return;
							}
						}

						_connectionStatus = FULL_CONNECTION;
						resubscribe();
						timeout = 0;
					}
					else{
						debugPrintln(" -- Failed");
					}
					timeout++;

				}

				//if we still cant connect to mqtt after 10 attempts increment the try count
				if(timeout >= 5 && !client.connected()){	
					timeout = 0;
					tryCount++;
					if(tryCount == 20){
						changeNetwork();
						tryCount = 0;
						return;
					}
				}
			}


		}

		//reset the reconnect metro
		reconnectMetro.reset();
	}
}

int ESPHelper::setConnectionStatus(){

	//assume no connection
	int returnVal = NO_CONNECTION;
	
	//make sure were not in broadcast mode
	if(_connectionStatus != BROADCAST){

		//if connected to wifi set the mode to wifi only and run the callback if needed
		if(WiFi.status() == WL_CONNECTED){
			if(_connectionStatus < WIFI_ONLY && _wifiCallbackSet){	//if the wifi previously wasnt connected but now is, run the callback
				_wifiCallback();
			}
			returnVal = WIFI_ONLY;

			//if mqtt is connected as well then set the status to full connection
			if(client.connected()){
				returnVal = FULL_CONNECTION;
			}
		}
	}


	else{
		returnVal = BROADCAST;
	}

	//set the connection status and return
	_connectionStatus = returnVal;
	return returnVal;
}

//changes the current network settings to the next listed network if network hopping is allowed
void ESPHelper::changeNetwork(){	

	//only attempt to change networks if hopping is allowed
	if(_hoppingAllowed){
		//change the index/reset to 0 if we've hit the last network setting
		_currentIndex++;
		if(_currentIndex >= _netCount){_currentIndex = 0;}

		//set the current netlist to the new network
		_currentNet = *_netList[_currentIndex];

		//verify various bits of network info

		//network password
		if(_currentNet.pass[0] == '\0'){_passSet = false;}
		else{_passSet = true;}

		//ssid
		if(_currentNet.ssid[0] == '\0'){_ssidSet = false;}
		else{_ssidSet = true;}	

		//mqtt host
		if(_currentNet.mqttHost[0] == '\0'){_mqttSet = false;}
		else{_mqttSet = true;}

		//mqtt username
		if(_currentNet.mqttUser[0] == '\0'){_mqttUserSet = false;}
		else{_mqttUserSet = true;}

		//mqtt password
		if(_currentNet.mqttPass[0] == '\0'){_mqttPassSet = false;}
		else{_mqttPassSet = true;}

		debugPrint("Trying next network: ");
		debugPrintln(_currentNet.ssid);

		//update the network connection
		updateNetwork();
	}
}

void ESPHelper::updateNetwork(){
	debugPrintln("\tDisconnecting from WiFi");
	WiFi.disconnect();
	debugPrintln("\tAttempting to begin on new network");

	//set the wifi mode
	WiFi.mode(WIFI_STA);

	//connect to the network
	if(_passSet && _ssidSet){WiFi.begin(_currentNet.ssid, _currentNet.pass);}
	else if(_ssidSet){WiFi.begin(_currentNet.ssid);}
	else{WiFi.begin("NO_SSID_SET");}

	debugPrintln("\tSetting new MQTT server");
	//setup the mqtt broker info
	if(_mqttSet){client.setServer(_currentNet.mqttHost, _currentNet.mqttPort);}
	else{client.setServer("192.0.2.0", 1883);}
	
	debugPrintln("\tDone - Ready for next reconnect attempt");
}

//generate unique MQTT name from MAC addr
String ESPHelper::macToStr(const uint8_t* mac){ 

  String result;

  for (int i = 0; i < 6; ++i) {
	result += String(mac[i], 16);

	if (i < 5){
	  result += ':';
	}
  }

  return result;
}

//change the current network info to a new netInfo - does not automatically disconnect from current network if already connected
void ESPHelper::setNetInfo(netInfo newNetwork){	
	_currentNet = newNetwork;
	_ssidSet = true;
	_passSet = true;
	_mqttSet = true;
	_mqttUserSet = true;
}

//change the current network info to a new *netInfo - does not automatically disconnect from current network if already connected
void ESPHelper::setNetInfo(netInfo *newNetwork){ 	
	_currentNet = *newNetwork;
	_ssidSet = true;
	_passSet = true;
	_mqttSet = true;
	_mqttUserSet = true;
}

//return the current netInfo state
netInfo* ESPHelper::getNetInfo(){	
	return &_currentNet;
}

//return the current SSID
const char* ESPHelper::getSSID(){			
	if(_ssidSet){return _currentNet.ssid;}
	return "SSID NOT SET";
}
//set a new SSID - does not automatically disconnect from current network if already connected
void ESPHelper::setSSID(const char* ssid){		
	_currentNet.ssid = ssid;
	_ssidSet = true;
}

//return the current network password
const char* ESPHelper::getPASS(){			
	if(_passSet){return _currentNet.pass;}
	return "PASS NOT SET";
}
//set a new network password - does not automatically disconnect from current network if already connected
void ESPHelper::setPASS(const char* pass){ 	
	_currentNet.pass = pass;
	_passSet = true;
}

//return the current MQTT server IP
const char* ESPHelper::getMQTTIP(){		
	if(_mqttSet){return _currentNet.mqttHost;}
	return "MQTT IP NOT SET";
}
//set a new MQTT server IP - does not automatically disconnect from current network/server if already connected
void ESPHelper::setMQTTIP(const char* mqttIP){ 
	_currentNet.mqttHost= mqttIP;
	_mqttSet = true;
}

//set a new MQTT server IP - does not automatically disconnect from current network/server if already connected
void ESPHelper::setMQTTIP(const char* mqttIP, const char* mqttUser, const char* mqttPass){
	_currentNet.mqttHost = mqttIP;
	_currentNet.mqttUser = mqttUser;
	_currentNet.mqttPass = mqttPass;
	_mqttSet = true;
	_mqttUserSet = true;
}

//return the QOS level for mqtt
int ESPHelper::getMQTTQOS(){
	return _qos;

}

//set the QOS level for mqtt 
void ESPHelper::setMQTTQOS(int qos){
	_qos = qos;
}

//return the local IP address of the ESP as a string
String ESPHelper::getIP(){
	return WiFi.localIP().toString();
}

//return the local IP address of the ESP
IPAddress ESPHelper::getIPAddress(){
	return WiFi.localIP();
}

//get the current connection status of ESPHelper
int ESPHelper::getStatus(){
	return _connectionStatus;
}

//enable or disable hopping - generally set automatically by initializer
void ESPHelper::setHopping(bool canHop){	
	_hoppingAllowed = canHop;
}

//DEBUG ONLY - print the subscribed topics list to the serial line
void ESPHelper::listSubscriptions(){
	for(int i = 0; i < MAX_SUBSCRIPTIONS; i++){
		if(_subscriptions[i].isUsed){
			debugPrintln(_subscriptions[i].topic);
		}
	}
}



//enable the connection heartbeat on a given pin
void ESPHelper::enableHeartbeat(int16_t pin){	
	#ifdef DEBUG
		if(pin == 1){_heartbeatEnabled = false;}
		else{
			_heartbeatEnabled = true;
			_ledPin = pin;
			pinMode(_ledPin, OUTPUT);
			digitalWrite(_ledPin, HIGH);
		}
	#else	
		_heartbeatEnabled = true;
		_ledPin = pin;
		pinMode(_ledPin, OUTPUT);
		digitalWrite(_ledPin, HIGH);
	#endif
}

//disable the connection heartbeat
void ESPHelper::disableHeartbeat(){			
	_heartbeatEnabled = false;
}

//heartbeat to indicate network connection
void ESPHelper::heartbeat(){				
	static Metro heartbeatMetro = Metro(10);
	static int counter = 0;

	static bool ledState = true;

	if(heartbeatMetro.check() && _heartbeatEnabled){
		if(counter == 1){
			digitalWrite(_ledPin, ledState);
			heartbeatMetro.interval(10);
			ledState = !ledState;
		}
		else if(counter == 2){
			digitalWrite(_ledPin, ledState);
			heartbeatMetro.interval(300);
			ledState = !ledState;
		}
		else if(counter == 3){
			digitalWrite(_ledPin, ledState);
			heartbeatMetro.interval(10);
			ledState = !ledState;
		}
		else{
			digitalWrite(_ledPin, ledState);
			heartbeatMetro.interval(1000);
			ledState = !ledState;
			counter = 0;
		}
		counter++;
	}
}

//enable use of OTA updates
void ESPHelper::OTA_enable(){
	_useOTA = true;
	OTA_begin();
}

//begin the OTA subsystem but with a check for connectivity and enabled use of OTA
void ESPHelper::OTA_begin(){
	Serial.print("OTA SHOULD BEGIN: ");

	Serial.println(_connectionStatus);
	Serial.print("Wifi status: ");
	Serial.println(WiFi.status());
	Serial.println(WiFi.localIP());

	if(_connectionStatus >= BROADCAST && _useOTA){
		Serial.println("OTA BEGIN");
		ArduinoOTA.begin();
		_OTArunning = true;
	}

}

//disable use of OTA updates
void ESPHelper::OTA_disable(){
	_useOTA = false;
	_OTArunning = false;
}

//set a password for OTA updates
void ESPHelper::OTA_setPassword(const char* pass){
	ArduinoOTA.setPassword(pass);
}


//set the hostname of the ESP for OTA uploads
void ESPHelper::OTA_setHostname(const char* hostname){
	strcpy(_hostname, hostname);
	ArduinoOTA.setHostname(_hostname);
}

//set the hostname of the ESP for OTA uploads and append the ESPHelper version number
void ESPHelper::OTA_setHostnameWithVersion(const char* hostname){
	strcpy(_hostname, hostname);
	strcat(_hostname, "----");
	strcat(_hostname, VERSION);

	ArduinoOTA.setHostname(_hostname);
}


Metro::Metro()
{

	this->interval_millis = 1000;

}


Metro::Metro(unsigned long interval_millis)
{

	this->interval_millis = interval_millis;

}


void Metro::interval(unsigned long interval_millis)
{
	this->interval_millis = interval_millis;
}

uint8_t Metro::check()
{

	unsigned long now = millis();

	if (interval_millis == 0) {
		previous_millis = now;
		return 1;
	}

	if ((now - previous_millis) >= interval_millis) {
#ifdef NOCATCH-UP
		previous_millis = now;
#else
		previous_millis += interval_millis;
#endif
		return 1;
	}

	return 0;

}

void Metro::reset()
{

	this->previous_millis = millis();

}

