/*
 * loggerApi_test.cpp
 *
 *  Created on: Jun 10, 2013
 *      Author: brent
 */
#include "loggerApi_test.h"

#include "api.h"
#include "loggerApi.h"
#include "mock_serial.h"
#include "loggerConfig.h"
#include "jsmn.h"
#include "mod_string.h"
#include "modp_atonum.h"

#include <string>
#include <fstream>
#include <streambuf>

#define JSON_TOKENS 10000
#define FILE_PREFIX string("test/")

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( LoggerApiTest );

void LoggerApiTest::assertGenericResponse(char *buffer, const char * messageName, int responseCode){
	static jsmn_parser parser;
	static jsmntok_t json_tok[JSON_TOKENS];
	jsmn_init(&parser);
	int r = jsmn_parse(&parser, buffer, json_tok, JSON_TOKENS);
	if (r == JSMN_SUCCESS){
		if (json_tok->type == JSMN_OBJECT && json_tok->size == 2){
			jsmntok_t * tok_msgName = json_tok + 1;
			jsmn_trimData(tok_msgName);
			if (strcmp(tok_msgName->data, messageName) != 0){
				CPPUNIT_FAIL("assertGenericResponse: message name does not match");
			}
			jsmntok_t * tok_resultInfo = json_tok + 2;
			if (tok_resultInfo->type== JSMN_OBJECT && tok_resultInfo->size == 2){
				jsmntok_t *tok_rc = tok_resultInfo + 1;
				jsmntok_t *tok_rcVal = tok_resultInfo + 2;
				jsmn_trimData(tok_rc);
				jsmn_trimData(tok_rcVal);
				if (strcmp("rc", tok_rc->data) != 0){
					CPPUNIT_FAIL("assertGenericResponse: rc element name does not match");
				}
				CPPUNIT_ASSERT_EQUAL(responseCode, modp_atoi(tok_rcVal->data));
			}
			else{
				CPPUNIT_FAIL("assertGenericResponse: did not see expected result object");
			}
		}
		else{
			CPPUNIT_FAIL("assertGenericResponse: expected object at root level");
		}
	}
	else{
		CPPUNIT_FAIL("assertGenericResponse: failed to parse");
	}
}

int LoggerApiTest::findAndReplace(string & source, const string find, const string replace)
{
    int num=0;
    string::size_type fLen = find.size();
    string::size_type rLen = replace.size();
    for (string::size_type pos=0; (pos=source.find(find, pos))!=string::npos; pos+=rLen)
    {
        num++;
        source.replace(pos, fLen, replace);
    }
    return num;
}

string LoggerApiTest::readFile(string filename){
	ifstream t(filename.c_str());
	if (!t.is_open()){
		t.open(string(FILE_PREFIX + filename).c_str());

	}
	if (!t.is_open()){
		throw ("Can not find file " + filename);
	}
	string str;

	t.seekg(0, ios::end);
	int length = t.tellg();
	str.reserve(length);
	t.seekg(0, ios::beg);

	str.assign((istreambuf_iterator<char>(t)),
				istreambuf_iterator<char>());

	findAndReplace(str,"\r","");
	findAndReplace(str,string("\n"),string(""));
	str.append("\r\n");
	return str;
}

void LoggerApiTest::setUp()
{
	initApi();
	updateActiveLoggerConfig();
	setupMockSerial();
}


void LoggerApiTest::tearDown()
{
}

void LoggerApiTest::testAnalogConfigFile(string filename){

	Serial *serial = getMockSerial();
	string json = readFile(filename);
	mock_resetTxBuffer();
	process_api(getMockSerial(),(char *)json.c_str(), json.size());

	LoggerConfig *c = getWorkingLoggerConfig();

	ADCConfig *adcCfg = &c->ADCConfigs[0];

	CPPUNIT_ASSERT_EQUAL(string("analog1"),string(adcCfg->cfg.label));
	CPPUNIT_ASSERT_EQUAL(50, decodeSampleRate(adcCfg->cfg.sampleRate));
	CPPUNIT_ASSERT_EQUAL(string("Hz"),string(adcCfg->cfg.units));

	CPPUNIT_ASSERT_EQUAL(2, (int)adcCfg->scalingMode);
	CPPUNIT_ASSERT_EQUAL(3, adcCfg->loggingPrecision);
	CPPUNIT_ASSERT_EQUAL(1.234F, adcCfg->linearScaling);

	CPPUNIT_ASSERT_EQUAL(1, (int)adcCfg->scalingMap.rawValues[0]);
	CPPUNIT_ASSERT_EQUAL(2, (int)adcCfg->scalingMap.rawValues[1]);
	CPPUNIT_ASSERT_EQUAL(3, (int)adcCfg->scalingMap.rawValues[2]);
	CPPUNIT_ASSERT_EQUAL(4, (int)adcCfg->scalingMap.rawValues[3]);
	CPPUNIT_ASSERT_EQUAL(5, (int)adcCfg->scalingMap.rawValues[4]);

	CPPUNIT_ASSERT_EQUAL(1.1F, adcCfg->scalingMap.scaledValues[0]);
	CPPUNIT_ASSERT_EQUAL(1.2F, adcCfg->scalingMap.scaledValues[1]);
	CPPUNIT_ASSERT_EQUAL(1.3F, adcCfg->scalingMap.scaledValues[2]);
	CPPUNIT_ASSERT_EQUAL(1.4F, adcCfg->scalingMap.scaledValues[3]);
	CPPUNIT_ASSERT_EQUAL(1.5F, adcCfg->scalingMap.scaledValues[4]);

	char *txBuffer = mock_getTxBuffer();
	assertGenericResponse(txBuffer, "setAnalogCfg", 1);

}


void LoggerApiTest::testSetAnalogCfg()
{
	testAnalogConfigFile("setAnalogCfg1.json");
	testAnalogConfigFile("setAnalogCfg2.json");
	testAnalogConfigFile("setAnalogCfg3.json");
}

void LoggerApiTest::testAccelConfigFile(string filename){
	Serial *serial = getMockSerial();
	string json = readFile(filename);
	mock_resetTxBuffer();
	process_api(getMockSerial(),(char *)json.c_str(), json.size());

	LoggerConfig *c = getWorkingLoggerConfig();

	AccelConfig *accelCfg = &c->AccelConfigs[0];

	CPPUNIT_ASSERT_EQUAL(string("accel1"),string(accelCfg->cfg.label));
	CPPUNIT_ASSERT_EQUAL(50, decodeSampleRate(accelCfg->cfg.sampleRate));
	CPPUNIT_ASSERT_EQUAL(string("GG"),string(accelCfg->cfg.units));
	CPPUNIT_ASSERT_EQUAL(1, (int)accelCfg->mode);
	CPPUNIT_ASSERT_EQUAL(2, (int)accelCfg->accelChannel);
	CPPUNIT_ASSERT_EQUAL(1234, (int)accelCfg->zeroValue);

	char *txBuffer = mock_getTxBuffer();
	assertGenericResponse(txBuffer, "setAccelCfg", 1);

}

void LoggerApiTest::testSetAccelCfg(){
	testAccelConfigFile("setAccelCfg1.json");
}

void LoggerApiTest::testSetCellConfigFile(string filename){
	Serial *serial = getMockSerial();
	string json = readFile(filename);
	mock_resetTxBuffer();
	process_api(getMockSerial(),(char *)json.c_str(), json.size());

	LoggerConfig *c = getWorkingLoggerConfig();

	CellularConfig *cellCfg = &c->ConnectivityConfigs.cellularConfig;

	CPPUNIT_ASSERT_EQUAL(string("foo.xyz"), string(cellCfg->apnHost));
	CPPUNIT_ASSERT_EQUAL(string("blarg"), string(cellCfg->apnUser));
	CPPUNIT_ASSERT_EQUAL(string("blorg"), string(cellCfg->apnPass));

	char *txBuffer = mock_getTxBuffer();
	assertGenericResponse(txBuffer, "setCellCfg", 1);
}

void LoggerApiTest::testSetCellCfg()
{
	testSetCellConfigFile("setCellCfg1.json");
}

void LoggerApiTest::testSetBtConfigFile(string filename){
	Serial *serial = getMockSerial();
	string json = readFile(filename);
	mock_resetTxBuffer();
	process_api(getMockSerial(),(char *)json.c_str(), json.size());

	LoggerConfig *c = getWorkingLoggerConfig();

	BluetoothConfig *btCfg = &c->ConnectivityConfigs.bluetoothConfig;

	CPPUNIT_ASSERT_EQUAL(string("myRacecar"), string(btCfg->deviceName));
	CPPUNIT_ASSERT_EQUAL(string("3311"), string(btCfg->passcode));

	char *txBuffer = mock_getTxBuffer();
	assertGenericResponse(txBuffer, "setBtCfg", 1);
}

void LoggerApiTest::testSetBtCfg()
{
	testSetBtConfigFile("setBtCfg1.json");
}

void LoggerApiTest::testSetPwmConfigFile(string filename){
	Serial *serial = getMockSerial();
	string json = readFile(filename);
	mock_resetTxBuffer();
	process_api(getMockSerial(),(char *)json.c_str(), json.size());

	LoggerConfig *c = getWorkingLoggerConfig();
	PWMConfig *pwmCfg = &c->PWMConfigs[0];
	CPPUNIT_ASSERT_EQUAL(string("pwm1"),string(pwmCfg->cfg.label));
	CPPUNIT_ASSERT_EQUAL(100, decodeSampleRate(pwmCfg->cfg.sampleRate));
	CPPUNIT_ASSERT_EQUAL(string("ut1"),string(pwmCfg->cfg.units));
	CPPUNIT_ASSERT_EQUAL(2, (int)pwmCfg->loggingPrecision);
	CPPUNIT_ASSERT_EQUAL(1, (int)pwmCfg->outputMode);
	CPPUNIT_ASSERT_EQUAL(1, (int)pwmCfg->loggingMode);
	CPPUNIT_ASSERT_EQUAL(50, (int)pwmCfg->startupDutyCycle);
	CPPUNIT_ASSERT_EQUAL(110, (int)pwmCfg->startupPeriod);
	CPPUNIT_ASSERT_EQUAL(2.5F, (float)pwmCfg->voltageScaling);

	char *txBuffer = mock_getTxBuffer();
	assertGenericResponse(txBuffer, "setPwmCfg", 1);

}

void LoggerApiTest::testSetPwmCfg(){
	testSetPwmConfigFile("setPwmCfg1.json");
}

void LoggerApiTest::testSetGpioConfigFile(string filename){
	Serial *serial = getMockSerial();
	string json = readFile(filename);
	mock_resetTxBuffer();
	process_api(getMockSerial(),(char *)json.c_str(), json.size());

	LoggerConfig *c = getWorkingLoggerConfig();
	GPIOConfig *gpioCfg = &c->GPIOConfigs[0];

	CPPUNIT_ASSERT_EQUAL(string("gpio1"),string(gpioCfg->cfg.label));
	CPPUNIT_ASSERT_EQUAL(100, decodeSampleRate(gpioCfg->cfg.sampleRate));
	CPPUNIT_ASSERT_EQUAL(string("ut1"),string(gpioCfg->cfg.units));
	CPPUNIT_ASSERT_EQUAL(1, (int)gpioCfg->mode);

	char *txBuffer = mock_getTxBuffer();
	assertGenericResponse(txBuffer, "setGpioCfg", 1);

}

void LoggerApiTest::testSetGpioCfg(){
	testSetGpioConfigFile("setGpioCfg1.json");
}

void LoggerApiTest::testSetTimerConfigFile(string filename){
	Serial *serial = getMockSerial();
	string json = readFile(filename);
	mock_resetTxBuffer();
	process_api(getMockSerial(),(char *)json.c_str(), json.size());

	LoggerConfig *c = getWorkingLoggerConfig();
	TimerConfig *timerCfg = &c->TimerConfigs[0];

	CPPUNIT_ASSERT_EQUAL(string("timer1"),string(timerCfg->cfg.label));
	CPPUNIT_ASSERT_EQUAL(10, decodeSampleRate(timerCfg->cfg.sampleRate));
	CPPUNIT_ASSERT_EQUAL(string("ut1"),string(timerCfg->cfg.units));
	CPPUNIT_ASSERT_EQUAL(3, (int)timerCfg->loggingPrecision);
	CPPUNIT_ASSERT_EQUAL(1, (int)timerCfg->slowTimerEnabled);
	CPPUNIT_ASSERT_EQUAL(1, (int)timerCfg->mode);
	CPPUNIT_ASSERT_EQUAL(4, (int)timerCfg->pulsePerRevolution);
	CPPUNIT_ASSERT_EQUAL(2, (int)timerCfg->timerDivider);

	char *txBuffer = mock_getTxBuffer();
	assertGenericResponse(txBuffer, "setTimerCfg", 1);
}

void LoggerApiTest::testSetTimerCfg(){
	testSetTimerConfigFile("setTimerCfg1.json");
}

void LoggerApiTest::testSampleData(){
	testSampleDataFile("sampleData1.json", "sampleData_response1.json");
	testSampleDataFile("sampleData2.json", "sampleData_response2.json");
}

void LoggerApiTest::testSampleDataFile(string requestFilename, string responseFilename){

	string requestJson = readFile(requestFilename);
	string responseJson = readFile(responseFilename);

	mock_resetTxBuffer();
	process_api(getMockSerial(),(char *)requestJson.c_str(), requestJson.size());

	LoggerConfig *c = getWorkingLoggerConfig();
	char *txBuffer = mock_getTxBuffer();
}

void LoggerApiTest::testLogStartStopFile(string filename, int expectedResponse){

	string json = readFile(filename);
	mock_resetTxBuffer();
	process_api(getMockSerial(), (char *)json.c_str(), json.size());
	char *txBuffer = mock_getTxBuffer();
	assertGenericResponse(txBuffer,"log",expectedResponse);
}

void LoggerApiTest::testLogStartStop(){
	testLogStartStopFile("log1.json", 1);
	testLogStartStopFile("log2.json", 1);
}