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
	state = STATE_RECIVING_DATA;
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

void ESP8266::waitMessage(){
	state = STATE_RECIVING_DATA;

	setResponseTrueKeywords(KEYWORD_PIPD);

	readResponse(30000, ReadMessage);
}

void ESP8266::ReadMessage(uint8_t serialResponseStatus) {
	//wifi.state = STATE_DATA_RECIVED;

	if (serialResponseStatus == SERIAL_RESPONSE_TRUE) {

		//uint32_t len = recvPkg(_messageBuffer,_buffer_size,NULL, 30000,NULL, KEYWORD_PIPD);

		/*if (len > 0) {

		}*/

	}
	else if (serialResponseStatus == SERIAL_RESPONSE_FALSE) {
		//DBG(F("\r\nESP8266 response msg error \r\n"));
	}
	else {
		//DBG(F("\r\nESP8266 response msg timeout \r\n"));
	}


	/*if (strcmp(wifi.requests[0].serverIP, currentServer) != 0) {
	wifi.closeConnection();
	}
	else {
	wifi.state = STATE_CONNECTED;
	}*/
}

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

bool ESP8266::isRecving(){
	return PIPDCound > 0;
}

void ESP8266::rx_empty(void) 
{
	while(m_puart->available() > 0) {
		m_puart->read();
	}
}

///
String ESP8266::recvString(String target, uint32_t timeout)
{
	String data;
	char a;
	unsigned long start = millis();
	while (millis() - start < timeout) {
		while(m_puart->available() > 0) {
			a = m_puart->read();
			if(a == '\0') continue;
			data += a;
		}

		if (data.indexOf(target) != -1) {
			break;
		}   
	}
	return data;
}

String ESP8266::recvString(String target1, String target2, uint32_t timeout)
{
	String data;
	char a;
	unsigned long start = millis();
	while (millis() - start < timeout) {
		while(m_puart->available() > 0) {
			a = m_puart->read();
			if(a == '\0') continue;
			data += a;
		}

		if (data.indexOf(target1) != -1) {
			break;
		} else if (data.indexOf(target2) != -1) {
			break;
		}
	}
	return data;
}

String ESP8266::recvString(String target1, String target2, String target3, uint32_t timeout)
{
	String data;
	char a;
	unsigned long start = millis();
	while (millis() - start < timeout) {
		while(m_puart->available() > 0) {
			a = m_puart->read();
			if(a == '\0') continue;
			data += a;
		}

		if (data.indexOf(target1) != -1) {
			break;
		} else if (data.indexOf(target2) != -1) {
			break;
		} else if (data.indexOf(target3) != -1) {
			break;
		}
	}
	return data;
}

///
bool ESP8266::recvFind(String target, uint32_t timeout)
{
	String data_tmp;
	data_tmp = recvString(target, timeout);
	if (data_tmp.indexOf(target) != -1) {
		return true;
	}
	return false;
}

///
bool ESP8266::recvFindAndFilter(String target, String begin, String end, String &data, uint32_t timeout)
{
	String data_tmp;
	data_tmp = recvString(target, timeout);
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

/// for joinAP
bool ESP8266::sATCWJAP(String ssid, String pwd)
{
	String data;
	rx_empty();
	m_puart->print("AT+CWJAP=\"");
	m_puart->print(ssid);
	m_puart->print("\",\"");
	m_puart->print(pwd);
	m_puart->println("\"");

	data = recvString("OK", "FAIL", 10000);
	if (data.indexOf("OK") != -1) {
		return true;
	}
	return false;
}


///for leaveAP
bool ESP8266::eATCWQAP(void)
{
	String data;
	rx_empty();
	m_puart->println("AT+CWQAP");
	return recvFind("OK");
}


///for getIPStatus
bool ESP8266::eATCIPSTATUS(String &list)
{
	String data;
	delay(100);
	rx_empty();
	m_puart->println("AT+CIPSTATUS");
	return recvFindAndFilter("OK", "\r\r\n", "\r\n\r\nOK", list);
}

///for createTCP
bool ESP8266::sATCIPSTARTSingle(String type, String addr, uint32_t port)
{
	String data;
	rx_empty();
	m_puart->print("AT+CIPSTART=\"");
	m_puart->print(type);
	m_puart->print("\",\"");
	m_puart->print(addr);
	m_puart->print("\",");
	m_puart->println(port);

	data = recvString("OK", "ERROR", "ALREADY CONNECT", 10000);
	if (data.indexOf("OK") != -1 || data.indexOf("ALREADY CONNECT") != -1) {
		return true;
	}
	return false;
}

///for send
bool ESP8266::sATCIPSENDSingle(const uint8_t *buffer, uint32_t len)
{
	rx_empty();
	m_puart->print("AT+CIPSEND=");
	m_puart->println(len);
	if (recvFind(">", 5000)) {
		rx_empty();
		for (uint32_t i = 0; i < len; i++) {
			m_puart->write(buffer[i]);
		}
		return recvFind("SEND OK", 10000);
	}
	return false;
}

///for releaseTCP
bool ESP8266::eATCIPCLOSESingle(void)
{
	rx_empty();
	m_puart->println("AT+CIPCLOSE");
	return recvFind("OK", 5000);
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
	case STATE_RECIVING_DATA:
		readResponse(serialResponseTimestamp, serialResponseHandler);

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

void ESP8266::readResponse(unsigned long timeout, void(*handler)(uint8_t serialResponseStatus)) {
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
	//	if ((currentTimestamp - serialResponseTimestamp) > serialResponseTimeout 
	//		|| currentTimestamp < serialResponseTimestamp 
	//		|| (bufferFind(responseTrueKeywords) 
	//		|| bufferFind(responseFalseKeywords) 
	//		|| bufferCursor == (SERIAL_RX_BUFFER_SIZE - 1))) {
	//		state = STATE_DATA_RECIVED;
	//		if (bufferFind(responseTrueKeywords) || bufferCursor == (SERIAL_RX_BUFFER_SIZE - 1)) {
	//			//DBG(F("serial true \r\n"));
	//			handler(SERIAL_RESPONSE_TRUE);
	//		}
	//		else if (bufferFind(responseFalseKeywords)) {
	//			//DBG(F("serial false \r\n"));
	//			handler(SERIAL_RESPONSE_FALSE);
	//		}
	//		else {
	//			//DBG(F("serial timeout \r\n"));
	//			handler(SERIAL_RESPONSE_TIMEOUT);
	//		}
	//	}
	//	else {
	//		while (_wifiSerial.available() > 0)
	//		{
	//			if (bufferCursor < (SERIAL_RX_BUFFER_SIZE-1)){
	//				buffer[bufferCursor] = _wifiSerial.read();
	//				bufferCursor++;
	//			}
	//			else {
	//				DBG(F("ESP8266 lib buffer overflow \r\n"));
	//				//empty the wifiSerial buffer'
	//				serialFlush();
	//				break;
	//			}
	//		}
	//		buffer[bufferCursor] = '\0';
	//	}
	//	break;
	//}

	//lastActivityTimestamp = currentTimestamp;
	////DBG(state);
}

