/**
* @file ESP8266.h
* @brief The definition of class ESP8266. 
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
#ifndef __ESP8266_H__
#define __ESP8266_H__

#include "Arduino.h"


#define ESP8266_USE_SOFTWARE_SERIAL


#ifdef ESP8266_USE_SOFTWARE_SERIAL
#include "SoftwareSerial.h"
#endif

#define SERIAL_RX_BUFFER_SIZE 128


// parameter used for communication with handlers
#define SERIAL_RESPONSE_FALSE	0
#define SERIAL_RESPONSE_TRUE	1
#define SERIAL_RESPONSE_TIMEOUT	2


// library states (library works like a simple FSM
#define STATE_IDLE			0 //when server connected

#define STATE_CONNECTED_AP	1 //when connected AP but not yet connect to server;
#define STATE_NOT_CONNECTED_AP	2 //when start or ip lost

#define STATE_ERROR			3

#define STATE_RESETING		4

#define STATE_SENDING_DATA	5

#define STATE_RECIVING_DATA	6
#define STATE_DATA_RECIVED	7


#define KEYWORD_PIPD "+IPD,"

#define KEYWORD_OK "\nOK"
#define KEYWORD_SEND_OK "\nSEND OK"
#define KEYWORD_READY "\nready"
#define KEYWORD_ERROR "\nERROR"
#define KEYWORD_FAIL "\nFAIL"
#define KEYWORD_ALREAY_CONNECT "\nALREAY CONNECT"
#define KEYWORD_CURSOR ">"

/**
* Provide an easy-to-use way to manipulate ESP8266. 
*/
class ESP8266 {
public:

#ifdef ESP8266_USE_SOFTWARE_SERIAL
	/*
	* Constuctor. 
	*
	* @param uart - an reference of SoftwareSerial object. 
	* @param baud - the buad rate to communicate with ESP8266(default:9600). 
	*
	* @warning parameter baud depends on the AT firmware. 9600 is an common value. 
	*/
	ESP8266(SoftwareSerial &uart, uint32_t baud = 9600);
#else /* HardwareSerial */
	/*
	* Constuctor. 
	*
	* @param uart - an reference of HardwareSerial object. 
	* @param baud - the buad rate to communicate with ESP8266(default:9600). 
	*
	* @warning parameter baud depends on the AT firmware. 9600 is an common value. 
	*/
	ESP8266(HardwareSerial &uart, uint32_t baud = 9600);
#endif


	/**
	* Join in AP. 
	*
	* @param ssid - SSID of AP to join in. 
	* @param pwd - Password of AP to join in. 
	* @retval true - success.
	* @retval false - failure.
	* @note This method will take a couple of seconds. 
	*/
	bool joinAP(String ssid, String pwd);

	/**
	* Leave AP joined before. 
	*
	* @retval true - success.
	* @retval false - failure.
	*/
	bool leaveAP(void);


	/**
	* Get the current status of connection(UDP and TCP). 
	* 
	* @return the status. 
	*/
	String getIPStatus(void);



	/**
	* Create TCP connection in single mode. 
	* 
	* @param addr - the IP or domain name of the target host. 
	* @param port - the port number of the target host. 
	* @retval true - success.
	* @retval false - failure.
	*/
	bool createTCP(String addr, uint32_t port);

	/**
	* Release TCP connection in single mode. 
	* 
	* @retval true - success.
	* @retval false - failure.
	*/
	bool releaseTCP(void);


	/**
	* Send data based on TCP or UDP builded already in single mode. 
	* 
	* @param buffer - the buffer of data to send. 
	* @param len - the length of data to send. 
	* @retval true - success.
	* @retval false - failure.
	*/
	bool send(const uint8_t *buffer, uint32_t len);

	/**
	* Send data based on one of TCP or UDP builded already in multiple mode. 
	* 
	* @param mux_id - the identifier of this TCP(available value: 0 - 4). 
	* @param buffer - the buffer of data to send. 
	* @param len - the length of data to send. 
	* @retval true - success.
	* @retval false - failure.
	*/
	bool send(uint8_t mux_id, const uint8_t *buffer, uint32_t len);

	/**
	* Receive data from TCP or UDP builded already in single mode. 
	*
	* @param buffer - the buffer for storing data. 
	* @param buffer_size - the length of the buffer. 
	* @param timeout - the time waiting data. 
	* @return the length of data received actually. 
	*/
	//uint32_t recv(uint8_t *buffer, uint32_t buffer_size, uint32_t timeout = 1000);

	void waitMessage();


	uint32_t recvAsync(uint8_t *buffer, uint32_t buffer_size, uint32_t timeout);

	void stopRecving();
	bool isRecving();

	void update();

private:

	/* 
	* Empty the buffer or UART RX.
	*/
	void rx_empty(void);

	/* 
	* Recvive data from uart. Return all received data if target found or timeout. 
	*/
	String recvString(String target, uint32_t timeout = 1000);

	/* 
	* Recvive data from uart. Return all received data if one of target1 and target2 found or timeout. 
	*/
	String recvString(String target1, String target2, uint32_t timeout = 1000);

	/* 
	* Recvive data from uart. Return all received data if one of target1, target2 and target3 found or timeout. 
	*/
	String recvString(String target1, String target2, String target3, uint32_t timeout = 1000);

	/* 
	* Recvive data from uart and search first target. Return true if target found, false for timeout.
	*/
	bool recvFind(String target, uint32_t timeout = 1000);

	/* 
	* Recvive data from uart and search first target and cut out the substring between begin and end(excluding begin and end self). 
	* Return true if target found, false for timeout.
	*/
	bool recvFindAndFilter(String target, String begin, String end, String &data, uint32_t timeout = 1000);

	/*
	* Receive a package from uart. 
	*
	* @param buffer - the buffer storing data. 
	* @param buffer_size - guess what!
	* @param data_len - the length of data actually received(maybe more than buffer_size, the remained data will be abandoned).
	* @param timeout - the duration waitting data comming.
	* @param coming_mux_id - in single connection mode, should be NULL and not NULL in multiple. 
	*/
	//uint32_t recvPkg(uint8_t *buffer, uint32_t buffer_size, uint32_t *data_len, uint32_t timeout, uint8_t *coming_mux_id);
	uint32_t recvPkg(uint8_t *buffer, uint32_t buffer_size, uint32_t *data_len, uint32_t timeout, uint8_t *coming_mux_id,String data = "");


	bool sATCWJAP(String ssid, String pwd);///

	bool eATCWQAP(void);///

	bool eATCIPSTATUS(String &list);///
	bool sATCIPSTARTSingle(String type, String addr, uint32_t port);///

	bool sATCIPSENDSingle(const uint8_t *buffer, uint32_t len);///

	bool eATCIPCLOSESingle(void);///


	/*
	* +IPD,len:data
	* +IPD,id,len:data
	*/

	int8_t PIPDCound = 0;

	// various timestamps
	unsigned long currentTimestamp;
	unsigned long serialResponseTimeout;
	unsigned long serialResponseTimestamp;
	unsigned long lastActivityTimestamp;

	// library state
	uint8_t state;

	char buffer[SERIAL_RX_BUFFER_SIZE];
	uint8_t * _messageBuffer;
	uint32_t  _buffer_size;

	bool bufferFind(bool trueKeywords);

	// serial response keywords for current communication
	char responseTrueKeywords[1][16];
	char responseFalseKeywords[1][16];

	// non blocking serial reading
	void readResponse(unsigned long timeout);

	// serial keywords setters 
	void setResponseTrueKeywords(char w1[] = "", char w2[] = "");
	void setResponseFalseKeywords(char w1[] = "", char w2[] = "");


	void(*wifiConnectedHandler)();
	void(*wifiDisconnectedHandler)();

	void(*serverConnectedHandler)();
	void(*serverDisconnectedHandler)();

	void(*dataRecivedHandler)(char data[]);

	void(*serialResponseHandler)(uint8_t serialResponseStatus);


	static void ReadMessage(uint8_t serialResponseStatus);

#ifdef ESP8266_USE_SOFTWARE_SERIAL
	SoftwareSerial *m_puart; /* The UART to communicate with ESP8266 */
#else
	HardwareSerial *m_puart; /* The UART to communicate with ESP8266 */
#endif
};

#endif /* #ifndef __ESP8266_H__ */

