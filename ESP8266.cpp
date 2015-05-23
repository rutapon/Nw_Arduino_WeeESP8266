/**
* @file ESP8266.cpp
* @brief The implementation of class ESP8266. 
* @author Wu Pengfei<pengfei.wu@itead.cc> 
* @date 2015.02
* 
* @par Copyright:
* Copyright (c) 2015 ITEAD Intelligent Systems Co., Ltd. \n\n
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License, or (at your option) any later version. \n\n
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/
#include "ESP8266.h"


#define LOG_OUTPUT_DEBUG            (1)
#define LOG_OUTPUT_DEBUG_PREFIX     (1)

#define logDebug(arg)\
	do {\
	if (LOG_OUTPUT_DEBUG)\
		{\
		if (LOG_OUTPUT_DEBUG_PREFIX)\
			{\
			Serial.print("[LOG Debug: ");\
			Serial.print((const char*)__FILE__);\
			Serial.print(",");\
			Serial.print((unsigned int)__LINE__);\
			Serial.print(",");\
			Serial.print((const char*)__FUNCTION__);\
			Serial.print("] ");\
			}\
			Serial.print(arg);\
		}\
	} while(0)

#ifdef ESP8266_USE_SOFTWARE_SERIAL
ESP8266::ESP8266(SoftwareSerial &uart, uint32_t baud): m_puart(&uart)
{
	m_puart->begin(baud);
	rx_empty();
}
#else
ESP8266::ESP8266(HardwareSerial &uart, uint32_t baud): m_puart(&uart)
{
	m_puart->begin(baud);
	rx_empty();
}
#endif


String ESP8266::getIPStatus(void)
{
	String list;
	eATCIPSTATUS(list);
	return list;
}


bool ESP8266::joinAP(String ssid, String pwd)
{
	return sATCWJAP(ssid, pwd);
}

bool ESP8266::leaveAP(void)
{
	return eATCWQAP();
}


bool ESP8266::createTCP(String addr, uint32_t port)
{
	return sATCIPSTARTSingle("TCP", addr, port);
}

bool ESP8266::releaseTCP(void)
{
	return eATCIPCLOSESingle();
}

bool ESP8266::send(const uint8_t *buffer, uint32_t len)
{
	return sATCIPSENDSingle(buffer, len);
}


//uint32_t ESP8266::recv(uint8_t *buffer, uint32_t buffer_size, uint32_t timeout)
//{
//
//	return  0;// recvPkg(buffer, buffer_size, NULL, timeout, NULL);
//}

//uint32_t ESP8266::recv(uint8_t mux_id, uint8_t *buffer, uint32_t buffer_size, uint32_t timeout)
//{
//	uint8_t id;
//	uint32_t ret;
//	ret = recvPkg(buffer, buffer_size, NULL, timeout, &id);
//	if (ret > 0 && id == mux_id) {
//		return ret;
//	}
//	return 0;
//}
//
//uint32_t ESP8266::recv(uint8_t *coming_mux_id, uint8_t *buffer, uint32_t buffer_size, uint32_t timeout)
//{
//	return recvPkg(buffer, buffer_size, NULL, timeout, coming_mux_id);
//}

/*----------------------------------------------------------------------------*/
/* +IPD,<id>,<len>:<data> */
/* +IPD,<len>:<data> */

//uint32_t ESP8266::recvPkg(uint8_t *buffer, uint32_t buffer_size, uint32_t *data_len, uint32_t timeout, uint8_t *coming_mux_id)
//{
//	String data;
//
//	return recvPkg(data,buffer,buffer_size,data_len,timeout,coming_mux_id);
//}


///
uint32_t ESP8266::recvPkg(uint8_t *buffer, uint32_t buffer_size, uint32_t *data_len, uint32_t timeout, uint8_t *coming_mux_id , String data)
{

	char a;
	int32_t index_PIPDcomma = -1;
	int32_t index_colon = -1; /* : */
	int32_t index_comma = -1; /* , */
	int32_t len = -1;
	int8_t id = -1;
	bool has_data = false;
	uint32_t ret;
	unsigned long start;
	uint32_t i;

	if (buffer == NULL) {
		return 0;
	}

	start = millis();

	while (millis() - start < timeout) {
		if(m_puart->available() > 0) {
			a = m_puart->read();
			data += a;
		}

		index_PIPDcomma = data.indexOf(KEYWORD_PIPD);
		if (index_PIPDcomma != -1) {
			index_colon = data.indexOf(':', index_PIPDcomma + 5);
			if (index_colon != -1) {
				index_comma = data.indexOf(',', index_PIPDcomma + 5);
				/* +IPD,id,len:data */
				if (index_comma != -1 && index_comma < index_colon) { 
					id = data.substring(index_PIPDcomma + 5, index_comma).toInt();
					if (id < 0 || id > 4) {
						return 0;
					}
					len = data.substring(index_comma + 1, index_colon).toInt();
					if (len <= 0) {
						return 0;
					}
				} else { /* +IPD,len:data */
					len = data.substring(index_PIPDcomma + 5, index_colon).toInt();
					if (len <= 0) {
						return 0;
					}
				}
				has_data = true;
				break;
			}
		}
	}

	if (has_data) {

		i = 0;
		ret = len > buffer_size ? buffer_size : len;
		start = millis();
		while (millis() - start < 3000) {
			while(m_puart->available() > 0 && i < ret) {
				a = m_puart->read();
				buffer[i++] = a;
			}
			if (i == ret) {
				rx_empty();
				if (data_len) {
					*data_len = len;    
				}
				if (index_comma != -1 && coming_mux_id) {
					*coming_mux_id = id;
				}

				return ret;
			}
		}


	}
	return 0;
}
//
uint32_t ESP8266::recvAsync(uint8_t *buffer, uint32_t buffer_size, uint32_t timeout){
	char PIPDcomma[] = KEYWORD_PIPD;
	if(m_puart->available() > 0) {
		char a = m_puart->read();

		if(PIPDcomma[PIPDCound] == a){
			PIPDCound++;

			if( PIPDCound == 5){
				//found pipd
				PIPDCound = 0;
				return recvPkg(
					buffer,
					buffer_size,
					NULL, 
					timeout,
					NULL, 
					KEYWORD_PIPD
					);
			}
		}else{
			PIPDCound = 0;
		}
	}
	return 0;
}

void ESP8266::stopRecving(){
	PIPDCound = 0;
}

//bool ESP8266::isRecving(){
//	return PIPDCound > 0;
//}

void ESP8266::rx_empty(void) 
{
	while(m_puart->available() > 0) {
		m_puart->read();
	}
}

///
void ESP8266::recvString(char target[], uint32_t timeout)
{
	bufferCursor = 0; 
	serialResponseTimestamp = millis();
	serialResponseTimeout = timeout;

	setResponseTrueKeywords(target);
	setResponseFalseKeywords();
}

void ESP8266::recvString(char target1[], char target2[], uint32_t timeout)
{
	bufferCursor = 0; 
	serialResponseTimestamp = millis();
	serialResponseTimeout = timeout;

	setResponseTrueKeywords(target1);
	setResponseFalseKeywords(target2);


}

void ESP8266::recvString(char target1[], char target2[], char target3[], uint32_t timeout)
{
	bufferCursor = 0; 
	serialResponseTimestamp = millis();
	serialResponseTimeout = timeout;

	setResponseTrueKeywords(target1,target3);
	setResponseFalseKeywords(target2);

}

String ESP8266::recvStringSync(String target, uint32_t timeout,String targetErr ){

	String data;
	char a;
	unsigned long start = millis();
	while (millis() - start < timeout) {
		while(m_puart->available() > 0) {
			a = m_puart->read();


			Serial.print(a);

			if(a == '\0') continue;
			data += a;
		}

		if (data.indexOf(target) != -1 
			|| (targetErr.length()> 0 && data.indexOf(targetErr) != -1 )) {

				break;
		}   
	}

	return data;
}

///
bool ESP8266::recvFind(String target, uint32_t timeout,String targetErr)
{
	String data_tmp;
	data_tmp = recvStringSync(target, timeout,targetErr);
	if (data_tmp.indexOf(target) != -1) {
		return true;
	}
	return false;
}

///
bool ESP8266::recvFindAndFilter(String target, String begin, String end, String &data, uint32_t timeout)
{
	String data_tmp;
	data_tmp = recvStringSync(target, timeout);

	if (data_tmp.indexOf(target) != -1) {
		int32_t index1 = data_tmp.indexOf(begin);
		int32_t index2 = data_tmp.indexOf(end);
		if (index1 != -1 && index2 != -1) {
			index1 += begin.length();
			data = data_tmp.substring(index1, index2);
			return true;
		}
	}
	data = "";
	return false;
}

/// for joinAP (async)
bool ESP8266::sATCWJAP(String ssid, String pwd)
{
	state = STATE_joinAP;

	String data;
	rx_empty();
	m_puart->print("AT+CWJAP=\"");
	m_puart->print(ssid);
	m_puart->print("\",\"");
	m_puart->print(pwd);
	m_puart->println("\"");

	//data = 
	//recvStringSync("OK", 10000);

	recvString("OK", "FAIL", 10000);


	/*if (data.indexOf("OK") != -1) {
	return true;
	}*/
	return false;
}


///for leaveAP (sync)
bool ESP8266::eATCWQAP(void)
{
	//state = STATE_leaveAP;

	String data;
	rx_empty();
	m_puart->println("AT+CWQAP");

	return recvFind("OK");
}


///for getIPStatus (sync)
bool ESP8266::eATCIPSTATUS(String &list)
{
	//state = STATE_getIPStatus;

	String data;
	delay(100);
	rx_empty();
	m_puart->println("AT+CIPSTATUS");
	return recvFindAndFilter("OK", "\r\r\n", "\r\n\r\nOK", list);
}

///for createTCP (async)
bool ESP8266::sATCIPSTARTSingle(String type, String addr, uint32_t port)
{
	state = STATE_createTCP;

	String data;
	rx_empty();
	m_puart->print("AT+CIPSTART=\"");
	m_puart->print(type);
	m_puart->print("\",\"");
	m_puart->print(addr);
	m_puart->print("\",");
	m_puart->println(port);

	//data = 
	recvString("OK", "ERROR", "ALREADY CONNECT", 10000);

	/*if (data.indexOf("OK") != -1 || data.indexOf("ALREADY CONNECT") != -1) {
	return true;
	}*/
	return false;

}

///for send (sync)
bool ESP8266::sATCIPSENDSingle(const uint8_t *buffer, uint32_t len)
{
	///state = STATE_send;

	rx_empty();
	m_puart->print("AT+CIPSEND=");
	m_puart->println(len);
	if (recvFind(KEYWORD_CURSOR, 5000, KEYWORD_SEND_ERROR)) {
		rx_empty();
		for (uint32_t i = 0; i < len; i++) {
			m_puart->write(buffer[i]);
		}
		return recvFind(KEYWORD_SEND_OK, 10000, KEYWORD_SEND_ERROR);
	}

	return false;
}

///for releaseTCP (sync)
bool ESP8266::eATCIPCLOSESingle(void)
{
	//state = STATE_releaseTCP;

	rx_empty();
	m_puart->println("AT+CIPCLOSE");
	return recvFind("OK", 5000);
}

// (async)
void ESP8266::waitMessage(){
	state = STATE_IDLE;
	rx_empty();
	recvString(KEYWORD_PIPD, 30000);
}

bool ESP8266::bufferFind(bool trueKeywords) {
	//char keywords[][16];
	char *keyword;

	for (int i = 0; i < 2; i++) {

		keyword = trueKeywords?responseTrueKeywords[i]: responseFalseKeywords[i];

		if (keyword != NULL && strlen(keyword) > 0) {
			if (strstr(buffer, keyword) != NULL) {
				return true;
			}
		}
	}
	return false;
}


void ESP8266::setResponseTrueKeywords(char w1[], char w2[]) {
	strncpy(responseTrueKeywords[0], w1, 16);
	strncpy(responseTrueKeywords[1], w2, 16);
}

void ESP8266::setResponseFalseKeywords(char w1[], char w2[]) {
	strncpy(responseFalseKeywords[0], w1, 16);
	strncpy(responseFalseKeywords[1], w2, 16);
}

void ESP8266::update()
{
	currentTimestamp = millis();

	switch (state) {
	case STATE_IDLE:
		ReadMessage();
		break;
	case STATE_joinAP:
	case STATE_createTCP:

		readResponse(serialResponseTimestamp);
		break;
		//case STATE_CONNECTED:
		//	if (requests[0].serverIP != NULL) {
		//		connectToServer();
		//	}
		//	break;
	}
	/*if (connected) {
	ipWatchdog();
	}*/
}

void ESP8266::readResponse(unsigned long timeout) {


	//switch (state) {
	//case STATE_IDLE:
	//case STATE_CONNECTED:
	//case STATE_SENDING_DATA:
	//case STATE_RESETING:
	//	state = STATE_RECIVING_DATA;
	//	serialResponseTimestamp = currentTimestamp;
	//	serialResponseTimeout = timeout;
	//	serialResponseHandler = handler;
	//	strcpy(buffer, "");
	//	bufferCursor = 0; 
	//	//DBG("started listening\r\n");
	//	break;

	//case STATE_RECIVING_DATA:
	if ((currentTimestamp - serialResponseTimestamp) > serialResponseTimeout 
		|| currentTimestamp < serialResponseTimestamp 
		|| (bufferFind(responseTrueKeywords) 
		|| bufferFind(responseFalseKeywords) 
		|| bufferCursor == (SERIAL_RX_BUFFER_SIZE - 1))) {
			//state = STATE_DATA_RECIVED;


			if (bufferFind(responseTrueKeywords) || bufferCursor == (SERIAL_RX_BUFFER_SIZE - 1)) {
				//DBG(F("serial true \r\n"));
				callSerialResponseMethod(SERIAL_RESPONSE_TRUE);
			}
			else if (bufferFind(responseFalseKeywords)) {
				//DBG(F("serial false \r\n"));
				callSerialResponseMethod(SERIAL_RESPONSE_FALSE);
			}
			else {
				//DBG(F("serial timeout \r\n"));
				callSerialResponseMethod(SERIAL_RESPONSE_TIMEOUT);
			}

	}
	else {

		/*Serial.println("readResponse");
		if(m_puart->available() > 0) {
		char a = m_puart->read();


		}*/

		/*String data;
		char a;

		while(m_puart->available() > 0) {
		a = m_puart->read();

		Serial.println(a);

		if(a == '\0') continue;
		data += a;
		}*/


		while (m_puart->available() > 0)
		{
			//Serial.println("available");
			if (bufferCursor < (SERIAL_RX_BUFFER_SIZE-1)){
				buffer[bufferCursor] =m_puart->read();
				Serial.print( buffer[bufferCursor]);
				bufferCursor++;
			}
			else {
				//DBG(F("ESP8266 lib buffer overflow \r\n"));
				//empty the wifiSerial buffer'
				rx_empty();
				break;
			}


		}
		buffer[bufferCursor] = '\0';
		//bufferCursor = 0;
	}
	/*break;
	}*/

	//lastActivityTimestamp = currentTimestamp;
	////DBG(state);
}


void ESP8266::ReadMessage() {
	//wifi.state = STATE_DATA_RECIVED;

	uint32_t len = recvAsync((uint8_t *)&_buffer, sizeof(DataPacket), 1000);

	if (len > 0) {
		processReceiveData();
	}
}


void   ESP8266::ProcessResponse_joinAP(uint8_t serialResponseStatus){
	if (serialResponseStatus == SERIAL_RESPONSE_TRUE) {
		if (onConnectAPEventHandle){
			onConnectAPEventHandle();
		}
	}
	else if (serialResponseStatus == SERIAL_RESPONSE_FALSE) {

	}
	else {

	}
}


void   ESP8266::ProcessResponse_createTCP(uint8_t serialResponseStatus){
	if (serialResponseStatus == SERIAL_RESPONSE_TRUE) {
		if (onConnectServerEventHandle){
			onConnectServerEventHandle();
		}
	}
	else if (serialResponseStatus == SERIAL_RESPONSE_FALSE) {

	}
	else {

	}
}


void ESP8266::callSerialResponseMethod(uint8_t serialResponseStatus){
	uint8_t lastState = state;

	state = STATE_IDLE;


	switch (lastState) {

	case STATE_joinAP: ProcessResponse_joinAP(serialResponseStatus);
		break;
	case STATE_createTCP: ProcessResponse_createTCP(serialResponseStatus);
		break;
	}

	strcpy(buffer, "");
	bufferCursor = 0; 	

	Serial.print("state ");
	Serial.println(state);

	if(state == STATE_IDLE){
		waitMessage();
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////


bool ESP8266::tryBegin(String  ssid, String pass,String hostName, uint32_t port){
	while(!begin(  ssid,  pass, hostName,  port)){
		//sl Serial.println("retry Begin");
	}
}
bool ESP8266::begin( String  ssid, String pass,String hostName, uint32_t port)
{
	_ssid = ssid;
	_pass = pass;
	_hostName = hostName;
	_port = port;

	//sl Serial.print("FW Version:");
	//Serial.println(_wifi->getVersion().c_str());

	//if (_wifi->setOprToStation()) {
	//	//sl Serial.print("station\r\n");

	//	//wifi.setSoftAPParam("softap","123456789");

	//} else {
	//	//sl Serial.print("to station err\r\n");

	//	return 0;
	//}

	////sl Serial.println(_wifi->getIPStatus().c_str());
	Serial.print("Join AP success\r\n");
	if (joinAP(_ssid, _pass)) {

		//sl Serial.print("IP:");
		//Serial.println(_wifi->getLocalIP().c_str());

	} else {
		//sl Serial.print("Join AP failure\r\n");
		return 0;
	}

	////sl Serial.println(_wifi->getIPStatus().c_str());

	//if (_wifi->disableMUX()) {
	//	//sl Serial.print("single ok\r\n");

	//} else {
	//	//sl Serial.print("single err\r\n");
	//	return 0;
	//}

	return 1;
}

bool ESP8266::connectTcp(String hostName, uint32_t port){
	_hostName = hostName;
	_port = port;

	//sl Serial.println(_wifi->getIPStatus().c_str());

	return createTCP(_hostName, _port);
}

//bool ESP8266::releaseTCP(){
//
//	//sl Serial.println(_wifi->getIPStatus().c_str());
//
//	return releaseTCP ();;
//}


bool ESP8266::tryConnectTcp(int numTry){

	/*while(numTry < 0 || numTry>0){
	numTry--;
	//sl Serial.println("retry connect tcp");

	if(_wifi->createTCP(_hostName, _port)){
	//sl Serial.println("connected tcp");
	return true;
	}
	}*/

	int i = 0;
	while(!createTCP(_hostName, _port)){
		i++;
		Serial.println("rc");
		releaseTCP();
		if(i > 10){
			Serial.println("rs");
			delay(100);
			resetFunc();
		}
	}
	//sl Serial.println("connected tcp");

	return true;
}

bool ESP8266::sendMsg (const uint8_t*  data ,uint32_t len ){

	//stopRecving();

	_lastSendTimeMillis  = millis();

	bool result = send(data, len);

	if(!result ){

		/*	for(int i =0;i<5;i++){
		result =  _wifi->send(data, len);
		if(result){
		return true;
		}
		}*/

		String status = getIPStatus();
		Serial.println(status);
		//if(!status.startsWith("STATUS:3")){
		//	////sl Serial.println(status.c_str());
		//	/*if (status.startsWith("STATUS:4")) 
		//	{
		//	//sl Serial.println("Disconnected");
		//	tryConnectTcp();
		//	}*/
		//	tryConnectTcp();
		//}


		/*releaseTCP();
		tryConnectTcp();*/

	}
	return result;
}


bool ESP8266::sendData (char * data, PacketType type, CmdType cmd ){

	//DataPacket packet =
	createPacket();
	_sendingPacket.type = type;
	_sendingPacket.cmd = cmd;
	strcpy(_sendingPacket.data,  data);

	//return sendData(packet);

	return sendData(_sendingPacket);
}

bool ESP8266::sendData (DataPacket &packet){
	return sendMsg((const uint8_t*)&packet,sizeof(DataPacket));
}

//bool NwEsp::sendDataQueue(DataPacket &packet){
//	_sendingPacket = packet;
//	//_packetQueue.push (packet);
//	return trySendQueueData();
//}

bool ESP8266::sendInterval(){

	return sendInterval("interval");	
}
bool ESP8266::sendInterval(char * msg){

	//sl Serial.print("sendInterval ");
	//sl Serial.println(_isSending);
	if(!_isSending){

		if ( millis() - _lastSendTimeMillis >= 1000) {

			return sendData(msg,itv);
		}
	}

	return false;
}

//bool ESP8266::trySendQueueData(){
//
//	if(!_isSending &&! isRecving()){
//
//		sendData(_sendingPacket);
//
//		if(_sendingPacket.type == rq || _sendingPacket.type == rrq){
//			_isSending = true;
//			startWaitRespond();
//		}
//		else if(_sendingPacket.type == rp){
//			_isSending = false;
//			//trySendQueueData();
//		}
//
//		return true;
//	}else{
//		return false;
//	}
//}

void ESP8266::startWaitRespond(){
	_startWaitingMillis = millis();
	_waitingInterval = 1000;
}

void ESP8266::dataRespontWaitAndRetry(){

	if(_isSending){
		unsigned long currentMillis = millis();

		if (currentMillis - _startWaitingMillis >= _waitingInterval) {

			//sl Serial.println("resend");
			sendData(_sendingPacket);

			_startWaitingMillis = currentMillis;
			_waitingInterval = 1000;
		}
	}else{
		startWaitRespond();
	}
}

uint32_t ESP8266::createPacket()
{
	_sendingPacket.pid = _pid;

	_pid++;

	return _pid;
}

//uint8_t  ESP8266::checkReceiveData() {
//
//	//String status = _wifi->getIPStatus();
//	//	
//	//	if(!status.startsWith("STATUS:3")){
//	//		 Serial.println(status);
//	//		////sl serial.println(status.c_str());
//	//		/*if (status.startswith("status:4")) 
//	//		{
//	//		//sl serial.println("disconnected");
//	//		tryconnecttcp();
//	//		}*/
//	//		//tryconnecttcp();
//	//	}
//
//	uint32_t len = recvAsync((uint8_t *)&_buffer, sizeof(DataPacket), 1000);
//
//	if (len > 0) {
//
//		/*	//sl Serial.print("Received:[");
//		uint32_t i = 0;
//		for (; i < len; i++) {
//		//sl Serial.print((char)buffer[i]);
//
//		bufferChar[i] = (char)buffer[i];
//		}
//		bufferChar[len] = 0;
//
//		//sl Serial.print("]\r\n");
//		*/
//		////sl Serial.println(bufferChar);
//
//		processReceiveData();
//
//		//delete root;
//
//		return len;
//	}else
//	{
//		return 0;
//	}
//} // end nRF_receive()

void  ESP8266::processReceiveData(){

	Serial.println(_buffer.data);

	if(_buffer.type ==rq||_buffer.type==rrq){
		if (onReceiveDataEventHandle){

			onReceiveDataEventHandle(_buffer);
			_buffer.type = rp;
			//delay(10);
			sendData(_buffer);
			//delay(1000);
		}

	} else if(_buffer.type == rp){

		if(_buffer.pid == _sendingPacket.pid){

			//sl Serial.print("maping ");
			//sl Serial.println(_buffer.pid);
			_isSending = false;
			//trySendQueueData();
		}
	} else {

	}
}

void ESP8266::setOnConnectAPEventHandle(void (*userFunc)()) {

	onConnectAPEventHandle = userFunc;
}

void ESP8266::setOnConnectServerEventHandle(void (*userFunc)()) {

	onConnectServerEventHandle = userFunc;
}

void ESP8266::setOnReceiveDataEventHandle(void (*userFunc)(DataPacket  &packet)) {
	onReceiveDataEventHandle = userFunc;
}
