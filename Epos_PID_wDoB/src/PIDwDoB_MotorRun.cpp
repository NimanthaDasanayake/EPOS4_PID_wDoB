//Edited by Nimantha Dasanayake to control a BLDC motor using PID with DoB

#define _USE_MATH_DEFINES

#include <iostream>
#include <fstream>
#include "Definitions.h"
#include <string.h>
#include <sstream>
#include "getopt.h"
#include <stdlib.h>
#include <stdio.h>
#include <list>
#include <math.h>
#include <sys/types.h>
#include <time.h>
#include <cmath>
#include <chrono>


typedef void* HANDLE;
typedef int BOOL;

enum EAppMode
{
	AM_UNKNOWN,
	AM_RUN,
	AM_INTERFACE_LIST,
	AM_PROTOCOL_LIST,
	AM_VERSION_INFO
};

using namespace std;

void* g_pKeyHandle = 0;
void* g_pKeyHandle2 = 0;
unsigned short g_usNodeId = 1;
unsigned short g_usNodeId2 = 2;
string g_deviceName;
string g_protocolStackName;
string g_interfaceName;
string g_portName;
string g_portName2;
int g_baudrate = 0;
EAppMode g_eAppMode = AM_RUN;

const string g_programName = "New2";

#ifndef MMC_SUCCESS
#define MMC_SUCCESS 0
#endif

#ifndef MMC_FAILED
#define MMC_FAILED 1
#endif

#ifndef MMC_MAX_LOG_MSG_SIZE
#define MMC_MAX_LOG_MSG_SIZE 512
#endif

void  LogError(string functionName, int p_lResult, unsigned int p_ulErrorCode);
void  LogInfo(string message);
void  PrintUsage();
void  PrintHeader();
void  PrintSettings();
int   OpenDevice(DWORD* p_pErrorCode);
int   CloseDevice(DWORD* p_pErrorCode);
void  SetDefaultParameters();
int   ParseArguments(int argc, char** argv);
int   MotorRun(unsigned int* p_pErrorCode);
int   PrepareRun(DWORD* p_pErrorCode);
int   PrintAvailableInterfaces();
int	  PrintAvailablePorts(char* p_pInterfaceNameSel);
int	  PrintAvailableProtocols();
int   PrintDeviceVersion();



void PrintUsage()
{
	cout << "Usage: HelloEposCmd" << endl;
	cout << "\t-h : this help" << endl;
	cout << "\t-n : node id (default 1)" << endl;
	cout << "\t-d   : device name (EPOS2, EPOS4, default - EPOS4)" << endl;
	cout << "\t-s   : protocol stack name (MAXON_RS232, CANopen, MAXON SERIAL V2, default - MAXON SERIAL V2)" << endl;
	cout << "\t-i   : interface name (RS232, USB, CAN_ixx_usb 0, CAN_kvaser_usb 0,... default - USB)" << endl;
	cout << "\t-p   : port name (COM1, USB0, CAN0,... default - USB0)" << endl;
	cout << "\t-b   : baudrate (115200, 1000000,... default - 1000000)" << endl;
	cout << "\t-l   : list available interfaces (valid device name and protocol stack required)" << endl;
	cout << "\t-r   : list supported protocols (valid device name required)" << endl;
	cout << "\t-v   : display device version" << endl;
}

void LogError(string functionName, int p_lResult, unsigned int p_ulErrorCode)
{
	cerr << g_programName << ": " << functionName << " failed (result=" << p_lResult << ", errorCode=0x" << std::hex << p_ulErrorCode << ")" << endl;
}

void LogInfo(string message)
{
	cout << message << endl;
}

void SeparatorLine()
{
	const int lineLength = 65;
	for (int i = 0; i < lineLength; i++)
	{
		cout << "-";
	}
	cout << endl;
}

void PrintSettings()
{
	stringstream msg;

	msg << "default settings:" << endl;
	msg << "node id             = " << g_usNodeId << endl;
	msg << "device name         = '" << g_deviceName << "'" << endl;
	msg << "protocal stack name = '" << g_protocolStackName << "'" << endl;
	msg << "interface name      = '" << g_interfaceName << "'" << endl;
	msg << "port name           = '" << g_portName << "'" << endl;
	msg << "baudrate            = " << g_baudrate;

	LogInfo(msg.str());

	SeparatorLine();
}

void SetDefaultParameters()
{
	//USB
	g_usNodeId = 1;
	g_usNodeId2 = 2;
	g_deviceName = "EPOS4";
	g_protocolStackName = "MAXON SERIAL V2";
	g_interfaceName = "USB";
	g_portName = "USB0";//USB port connection for motor
	g_baudrate = 1000000;
}

int OpenDevice(DWORD* p_pErrorCode)
{
	int lResult = MMC_FAILED;

	char* pDeviceName = new char[255];
	char* pProtocolStackName = new char[255];
	char* pInterfaceName = new char[255];
	char* pPortName = new char[255];
	char* pPortName2 = new char[255];

	strcpy(pDeviceName, g_deviceName.c_str());
	strcpy(pProtocolStackName, g_protocolStackName.c_str());
	strcpy(pInterfaceName, g_interfaceName.c_str());
	strcpy(pPortName, g_portName.c_str());
	strcpy(pPortName2, g_portName2.c_str());

	LogInfo("Open device...");

	g_pKeyHandle = VCS_OpenDevice(pDeviceName, pProtocolStackName, pInterfaceName, pPortName, p_pErrorCode);
	
	
	if (g_pKeyHandle != 0 && g_pKeyHandle2 != 0 && *p_pErrorCode == 0)
	{
		DWORD lBaudrate = 0;
		DWORD lTimeout = 0;

		if ( (VCS_GetProtocolStackSettings(g_pKeyHandle, &lBaudrate, &lTimeout, p_pErrorCode) != 0) )
		{
			if (  (VCS_SetProtocolStackSettings(g_pKeyHandle, g_baudrate, lTimeout, p_pErrorCode) != 0) )
			{
				if ( (VCS_GetProtocolStackSettings(g_pKeyHandle, &lBaudrate, &lTimeout, p_pErrorCode) != 0)  )
				{
					if (g_baudrate == (int)lBaudrate)
					{
						lResult = MMC_SUCCESS;
					}
				}
			}
		}
	}
	else
	{
		g_pKeyHandle = 0;
		g_pKeyHandle2 = 0;
	}
	

	delete[]pDeviceName;
	delete[]pProtocolStackName;
	delete[]pInterfaceName;
	delete[]pPortName;
	delete[]pPortName2;

	return lResult;

}

int CloseDevice(DWORD* p_pErrorCode)
{
	int lResult = MMC_FAILED;

	*p_pErrorCode = 0;

	LogInfo("Close device");

	if (VCS_CloseDevice(g_pKeyHandle, p_pErrorCode) != 0 && *p_pErrorCode == 0)
	{
		lResult = MMC_SUCCESS;
	}

	return lResult;
}

int ParseArguments(int argc, char** argv)
{
	int lOption;
	int lResult = MMC_SUCCESS;

	opterr = 0;

	while ((lOption = getopt(argc, argv, "hlrvd:s:i:p:p2:b:n:n2")) != -1)
	{
		switch (lOption)
		{
		case 'h':
			PrintUsage();
			lResult = 1;
			break;
		case 'd':
			g_deviceName = optarg;
			break;
		case 's':
			g_protocolStackName = optarg;
			break;
		case 'i':
			g_interfaceName = optarg;
			break;
		case 'p':
			g_portName = optarg;
			break;
		case 'p2':
			g_portName2 = optarg;
			break;
		case 'b':
			g_baudrate = atoi(optarg);
			break;
		case 'n':
			g_usNodeId = (unsigned short)atoi(optarg);
			break;
		case 'n2':
			g_usNodeId2 = (unsigned short)atoi(optarg);
			break;
		case 'l':
			g_eAppMode = AM_INTERFACE_LIST;
			break;
		case 'r':
			g_eAppMode = AM_PROTOCOL_LIST;
			break;
		case 'v':
			g_eAppMode = AM_VERSION_INFO;
			break;
		case '?':  // unknown option...
			stringstream msg;
			msg << "Unknown option: '" << char(optopt) << "'!";
			LogInfo(msg.str());
			PrintUsage();
			lResult = MMC_FAILED;
			break;
		}
	}

	return lResult;
}

int CurrentMode(HANDLE p_DeviceHandle, unsigned short p_usNodeId, unsigned short p_usNodeId2, DWORD& p_rlErrorCode)
{
	short targetCurrent = 0;
	short CurrentReading = 0;
	long Ini_Position;
	long Position = 0;
	long Pre_Position = 0;
	long Des_Position = 0;
	float Err_Position = 0;
	float Int_Err_Position = 0;
	long Velocity = 0;
	float Ang_Speed = 0;
	float Ang_Speed_sq = 0;
	float Pre_Ang_Speed = 0;
	float Des_Ang_Speed = 0;
	float Err_Ang_Speed = 0;
	float Int_Err_Ang_Speed = 0;
	float Err_Ang_Speed_Pre = 0;
	float Der_Err_Ang_Speed = 0;
	float Ang_Acc = 0;
	long P_add = 0;
	float epsilon = 0.000000000001;

	float time = 0 ;
	struct timespec gettime_now;
	struct timespec start, stop;
	float delT = 0;

	int lResult = MMC_SUCCESS;
	stringstream msg;

	//Motor parameters
	float J = 0.000317; //Motor rotor inertia
	float K_t = 0.0619; //Motor torque constant (value in datasheet = 0.0712) 
	float K_f = 0.000214;//Motor friction constant
	float K_b = 0.0712;//new value from SINDY with gearbox = 0.075 //value in datasheet (w/o gearbox)= 0.0712 //Motor speed constant
	float L = 0.000232;//Armature inductance
	float R = 0.216;//Armature resistance
	float T_d = 0; //Disturbance torque
	float Pre_T_d = 0;//Disturbance torque in previous time step
	float T_d_fil = 0;//Disturbance torque after filtering
	float I_d = 0;//Disturbance current
	float Pre_I_d = 0;//Disturbance current in previous time step
	float I_d_rate = 0;//Rate of change of diturbance current
	float V_cmd = 0;//Commanded voltage
	float V_comp = 0;//disturbance compensation voltage
	float omega_c = 314.16; //Q-filter cut-off frequency (for Disturbance Observer)
	float Ki = 4.66751;//Integral gain
	float Kp = 0.28254;//Proportional gain
	float Kd = 0.0;//derivative gain
	float alpha_2 = 0.000353;//Viscous friction with gearbox = 0.001310
	float alpha_0 = 0.00746;//with gearbox = 0.0118;//Coulomb friction

	//Opening data file to store motor states: position, velocity, current---------------
	ofstream datafile;
	datafile.open("data.txt");


	//Reading the initial position----------------------------
	VCS_GetPositionIs(p_DeviceHandle, p_usNodeId, &Ini_Position, &p_rlErrorCode);
	cout << "initial Position = " << Ini_Position << endl;
	//-------------------------------------------

	if ( (VCS_SetOperationMode(p_DeviceHandle, p_usNodeId, -3, &p_rlErrorCode) == 0) )
	{
		LogError("VCS_SetOperationMode", lResult, p_rlErrorCode);
		lResult = MMC_FAILED;
	}

	else
	{
		if ( (VCS_ActivateCurrentMode(p_DeviceHandle, p_usNodeId, &p_rlErrorCode) == 0) )
		{
			LogError("VCS_ActivateCurrentMode", lResult, p_rlErrorCode);
			lResult = MMC_FAILED;
		}


		else
		{
			//Starting the clock to measure the loop time
			auto start = chrono::steady_clock::now();

			//Motor current measurement
			if (VCS_GetCurrentIs(p_DeviceHandle, p_usNodeId, &CurrentReading, &p_rlErrorCode) == 0)
			{
				LogError("VCS_GetCurrentIs", lResult, p_rlErrorCode);
				lResult = MMC_FAILED;
			}

			//Setting the iniitlal target current
			V_cmd = 0;
			targetCurrent = V_cmd + CurrentReading;

			Sleep(1000);//1 second delay

			//Running the motors in loop for 5000 cycles; change this as necessary
			for (int i = 1; i <= 5000; i++)
			{	

				//Test step signal sequence for commanded velocity (Only works when PID controller is OFF)
				if (i < 25) {
					V_cmd = 0;
				}
				else if (i < 50) {
					V_cmd = 0.35;
				}
				else if (i < 250) {
					V_cmd = 0;
				}
				else if (i < 275) {
					V_cmd = 0.4;
				}
				else if (i < 475) {
					V_cmd = 0;
				}
				else if (i < 500) {
					V_cmd = 0.45;
				}
				else if (i < 700) {
					V_cmd = 0;
				}
				else if (i < 725) {
					V_cmd = 0.5;
				}
				else if (i < 925) {
					V_cmd = 0;
				}
				else if (i < 950) {
					V_cmd = 0.55;
				}
				else if (i < 1150) {
					V_cmd = 0;
				}
				else if (i < 1175) {
					V_cmd = 0.6;
				}
				else if (i < 1375) {
					V_cmd = 0;
				}
				else if (i < 1400) {
					V_cmd = 0.65;
				}
				else if (i < 1600) {
					V_cmd = 0;
				}
				else if (i < 1625) {
					V_cmd = 0.7;
				}
				else if (i < 1825) {
					V_cmd = 0;
				}
				else if (i < 1850) {
					V_cmd = 0.75;
				}
				else if (i < 2050) {
					V_cmd = 0;
				}
				else if (i < 2075) {
					V_cmd = 0.8;
				}
				else if (i < 2275) {
					V_cmd = 0;
				}
				else if (i < 2300) {
					V_cmd = 0.85;
				}
				else if (i < 2500) {
					V_cmd = 0;
				}
				else if (i < 2525) {
					V_cmd = 0.9;
				}
				else if (i < 2725) {
					V_cmd = 0;
				}
				else if (i < 2750) {
					V_cmd = 0.95;
				}
				else if (i < 2950) {
					V_cmd = 0;
				}
				else if (i < 2975) {
					V_cmd = 1;
				}
				else {
					V_cmd = 0;
				}

			
				//Setting safety limits for motor current	
				if (abs(targetCurrent) > 20000) {
					targetCurrent = 20000* targetCurrent / abs(targetCurrent);
				}

						
				//Setting the target current
				if ( (VCS_SetCurrentMust(p_DeviceHandle, p_usNodeId, targetCurrent, &p_rlErrorCode) == 0) )
				{
					LogError("VCS_SetCurrentMust", lResult, p_rlErrorCode);
					lResult = MMC_FAILED;
					break;
				}
						
				//Reading the motor currents
				if (VCS_GetCurrentIs(p_DeviceHandle, p_usNodeId, &CurrentReading, &p_rlErrorCode) == 0)
				{
					LogError("VCS_GetCurrentIs", lResult, p_rlErrorCode);
					lResult = MMC_FAILED;
					break;
				}

		
				//Reading the motor velocity
				if (VCS_GetVelocityIs(p_DeviceHandle, p_usNodeId, &Velocity, &p_rlErrorCode) == 0 )
				{
					LogError("VCS_GetPositionIs", lResult, p_rlErrorCode);
					lResult = MMC_FAILED;
					break;
				}
				
				//Reading the motor position
				if (VCS_GetPositionIs(p_DeviceHandle, p_usNodeId, &Position, &p_rlErrorCode) == 0 )
				{
					LogError("VCS_GetPositionIs", lResult, p_rlErrorCode);
					lResult = MMC_FAILED;
					break;
				}


				//Calculating the angular speed
				Ang_Speed = float(Velocity) * (2 * 3.14) / 66;//66 hall sensor steps corresponds to on revolution (Maxon EC 90 Flat BLDC Motor)
				Pre_Ang_Speed = Ang_Speed;
				Ang_Acc = (Ang_Speed - Pre_Ang_Speed) / delT; //calculatin the angular acceleration

				Des_Ang_Speed = 100;

								
				//////PID Controller//////(comment this section to turn OFF PID)
				Err_Ang_Speed = Des_Ang_Speed - Ang_Speed;//calculating the angular speed error
				Err_Ang_Speed_Pre = Err_Ang_Speed;
				Int_Err_Ang_Speed += Err_Ang_Speed * delT;//calculating the intergral of angular speed error
				Der_Err_Ang_Speed = (Err_Ang_Speed - Int_Err_Ang_Speed) / delT; //calculating the derivative of angular speed error
				
				V_cmd = Kp*Err_Ang_Speed + Ki*Int_Err_Ang_Speed + Kd*Der_Err_Ang_Speed; //comment to turn of PID

				//////////////////////////////////////////////////////////////
				
				//////Disturbance Observer//////

				T_d = CurrentReading * K_t / 1000 - J * Ang_Acc - K_f * Ang_Speed; //calculating the disturbance torque

				T_d_fil = 0.04321 * T_d_fil + 0.6954 * T_d + 0.2613 * Pre_T_d;//omega_c = 314.16 rad/s Ts = 10 ms : disturbance filter (Qfilter) with cut-off freq omega_c

				Pre_T_d = T_d; //saving the current T_d for next time step


				//limiting the commanded voltage
				if (abs(V_cmd) < 0.35) {
					V_cmd = 0.35 * abs(V_cmd) / (V_cmd + epsilon);//epsilon, a very small value, used to avoid division by zero

				}
	
				//calculating the disturbance current
				I_d = (T_d_fil + J * Ang_Acc + 1.0 * alpha_2 * Ang_Speed  + alpha_0 * Ang_Speed / abs(Ang_Speed + epsilon)) / K_t;

				I_d_rate = (I_d - Pre_I_d) / delT; //calculating the first time derivative of the disturbance current
				Pre_I_d = I_d;

				V_comp = (I_d_rate * L + R * I_d + K_b * Ang_Speed); //calculating disurbance compensation voltage

				V_cmd = 1000 * V_cmd + 1000 * V_comp;//commanded voltage in milivolts

				targetCurrent = V_cmd + CurrentReading;//the PID gains of the inbuild PID loop of epos controller are set to P=1,D=0,I=0; therefore targetcurrent in mA = commanded voltage in mV

				//Writing motor states to a data file
				datafile << delT << " " << CurrentReading << " " << Position << " " << Ang_Speed << " " << endl;
				
				//Calculating the step time and total elapsed time
				auto end = chrono::steady_clock::now();
				delT = chrono::duration_cast<chrono::milliseconds>(end - start).count();
				delT = delT / 1000;
				time = time + delT;
				start = chrono::steady_clock::now();

				}

			//Setting motor currents to zero at the end of the run
			if ( (VCS_SetCurrentMust(p_DeviceHandle, p_usNodeId, 0, &p_rlErrorCode) == 0) )
			{
				LogError("VCS_SetCurrentMust", lResult, p_rlErrorCode);
				lResult = MMC_FAILED;
			}
		}
			

			msg << "stopping current = " << ", node = " << p_usNodeId << ", position = " << Position << endl;
			LogInfo(msg.str());

			if (VCS_SetCurrentMust(p_DeviceHandle, p_usNodeId, 0, &p_rlErrorCode) == 0)
			{
				LogError("VCS_SetCurrentMust", lResult, p_rlErrorCode);
				lResult = MMC_FAILED;

			}
			if (VCS_GetCurrentIs(p_DeviceHandle, p_usNodeId, &CurrentReading, &p_rlErrorCode) == 0)
			{
				LogError("VCS_GetCurrentIs", lResult, p_rlErrorCode);
				lResult = MMC_FAILED;

			}


		}
	

	if (VCS_SetCurrentMust(p_DeviceHandle, p_usNodeId, 0, &p_rlErrorCode) == 0)
	{
		LogError("VCS_SetCurrentMust", lResult, p_rlErrorCode);
		lResult = MMC_FAILED;
	}

	datafile.close();
	return lResult;
}


int PrepareRun(DWORD* p_pErrorCode)
{
	//Preparing to run the motors: checking the fault states and enable states
	int lResult = MMC_SUCCESS;
	BOOL oIsFault = 0;

	if ( (VCS_GetFaultState(g_pKeyHandle, g_usNodeId, &oIsFault, p_pErrorCode) == 0) )
	{
		LogError("VCS_GetFaultState", lResult, *p_pErrorCode);
		lResult = MMC_FAILED;
	}

	if (lResult == 0)
	{
		if (oIsFault)
		{
			stringstream msg;
			msg << "clear fault, node = '" << g_usNodeId << "'";
			LogInfo(msg.str());

			if ( (VCS_ClearFault(g_pKeyHandle, g_usNodeId, p_pErrorCode) == 0) )
			{
				LogError("VCS_ClearFault", lResult, *p_pErrorCode);
				lResult = MMC_FAILED;
			}
		}

		if (lResult == 0)
		{
			BOOL oIsEnabled = 0;

			if ( (VCS_GetEnableState(g_pKeyHandle, g_usNodeId, &oIsEnabled, p_pErrorCode) == 0) )
			{
				LogError("VCS_GetEnableState", lResult, *p_pErrorCode);
				lResult = MMC_FAILED;
			}

			if (lResult == 0)
			{
				if (!oIsEnabled)
				{
					if (VCS_SetEnableState(g_pKeyHandle, g_usNodeId, p_pErrorCode) == 0)
					{
						LogError("VCS_SetEnableState", lResult, *p_pErrorCode);
						lResult = MMC_FAILED;
					}

				}
			}
		}
	}
	return lResult;





}


int MotorRun(DWORD* p_pErrorCode)
{
	//Running the motors with error handling

	long Position_In;
	int lResult = MMC_SUCCESS;
	int lResult2 = MMC_SUCCESS;
	DWORD lErrorCode = 0;

	if (lResult != MMC_SUCCESS)
	{
		LogError("PositionMode", lResult, lErrorCode);
	}
	else
	{
		//Running the motors in current mode: see CurrentMode function definition
		lResult = CurrentMode(g_pKeyHandle, g_usNodeId, g_usNodeId2, lErrorCode);

		if ((lResult != MMC_SUCCESS) && (lResult2 != MMC_SUCCESS))
		{
			LogError("CurrentMode", lResult, lErrorCode);
			LogError("CurrentMode", lResult2, lErrorCode);
		}
		else
		{
			if ((VCS_SetDisableState(g_pKeyHandle, g_usNodeId, &lErrorCode) == 0) )
			{
				LogError("VCS_SetDisableState", lResult, lErrorCode);
				lResult = MMC_FAILED;
				lResult2 = MMC_FAILED;
			}

		}

	}

	return lResult;
	return lResult2;
}

void PrintHeader()
{
	SeparatorLine();

	LogInfo("Epos Command Library Example Program, (c) maxonmotor ag 2014-2018");

	SeparatorLine();
}

int PrintAvailablePorts(char* p_pInterfaceNameSel)
{
	int lResult = MMC_FAILED;
	int lStartOfSelection = 1;
	int lMaxStrSize = 255;
	char* pPortNameSel = new char[lMaxStrSize];
	int lEndOfSelection = 0;
	DWORD ulErrorCode = 0;

	do
	{
		if (!VCS_GetPortNameSelection((char*)g_deviceName.c_str(), (char*)g_protocolStackName.c_str(), p_pInterfaceNameSel, lStartOfSelection, pPortNameSel, lMaxStrSize, &lEndOfSelection, &ulErrorCode))
		{
			lResult = MMC_FAILED;
			LogError("GetPortNameSelection", lResult, ulErrorCode);
			break;
		}
		else
		{
			lResult = MMC_SUCCESS;
			printf("            port = %s\n", pPortNameSel);
		}

		lStartOfSelection = 0;
	} while (lEndOfSelection == 0);

	return lResult;
}

int PrintAvailableInterfaces()
{
	int lResult = MMC_FAILED;
	int lStartOfSelection = 1;
	int lMaxStrSize = 255;
	char* pInterfaceNameSel = new char[lMaxStrSize];
	int lEndOfSelection = 0;
	DWORD ulErrorCode = 0;

	do
	{
		if (!VCS_GetInterfaceNameSelection((char*)g_deviceName.c_str(), (char*)g_protocolStackName.c_str(), lStartOfSelection, pInterfaceNameSel, lMaxStrSize, &lEndOfSelection, &ulErrorCode))
		{
			lResult = MMC_FAILED;
			LogError("GetInterfaceNameSelection", lResult, ulErrorCode);
			break;
		}
		else
		{
			lResult = MMC_SUCCESS;

			printf("interface = %s\n", pInterfaceNameSel);

			PrintAvailablePorts(pInterfaceNameSel);
		}

		lStartOfSelection = 0;
	} while (lEndOfSelection == 0);

	SeparatorLine();

	delete[] pInterfaceNameSel;

	return lResult;
}

int PrintDeviceVersion()
{
	int lResult = MMC_FAILED;
	unsigned short usHardwareVersion = 0;
	unsigned short usSoftwareVersion = 0;
	unsigned short usApplicationNumber = 0;
	unsigned short usApplicationVersion = 0;
	DWORD ulErrorCode = 0;

	if (VCS_GetVersion(g_pKeyHandle, g_usNodeId, &usHardwareVersion, &usSoftwareVersion, &usApplicationNumber, &usApplicationVersion, &ulErrorCode))
	{
		printf("%s Hardware Version    = 0x%04x\n      Software Version    = 0x%04x\n      Application Number  = 0x%04x\n      Application Version = 0x%04x\n",
			g_deviceName.c_str(), usHardwareVersion, usSoftwareVersion, usApplicationNumber, usApplicationVersion);
		lResult = MMC_SUCCESS;
	}

	return lResult;
}

int PrintAvailableProtocols()
{
	int lResult = MMC_FAILED;
	int lStartOfSelection = 1;
	int lMaxStrSize = 255;
	char* pProtocolNameSel = new char[lMaxStrSize];
	int lEndOfSelection = 0;
	DWORD ulErrorCode = 0;

	do
	{
		if (!VCS_GetProtocolStackNameSelection((char*)g_deviceName.c_str(), lStartOfSelection, pProtocolNameSel, lMaxStrSize, &lEndOfSelection, &ulErrorCode))
		{
			lResult = MMC_FAILED;
			LogError("GetProtocolStackNameSelection", lResult, ulErrorCode);
			break;
		}
		else
		{
			lResult = MMC_SUCCESS;

			printf("protocol stack name = %s\n", pProtocolNameSel);
		}

		lStartOfSelection = 0;
	} while (lEndOfSelection == 0);

	SeparatorLine();

	delete[] pProtocolNameSel;

	return lResult;
}





int main(int argc, char** argv)
{

	int lResult = MMC_FAILED;
	DWORD ulErrorCode = 0;

	PrintHeader();

	SetDefaultParameters();

	PrintSettings();

	if ((lResult = OpenDevice(&ulErrorCode)) != MMC_SUCCESS)
	{
		LogError("OpenDevice", lResult, ulErrorCode);
		return lResult;
	}


	switch (g_eAppMode)
	{
	case AM_RUN:
	{
		if ((lResult = OpenDevice(&ulErrorCode)) != MMC_SUCCESS)
		{
			LogError("OpenDevice", lResult, ulErrorCode);
			return lResult;
		}

		if ((lResult = PrepareRun(&ulErrorCode)) != MMC_SUCCESS)
		{
			LogError("PrepareRun", lResult, ulErrorCode);
			return lResult;
		}

		if ((lResult = MotorRun(&ulErrorCode)) != MMC_SUCCESS)
		{
			LogError("MotorRun", lResult, ulErrorCode);
			return lResult;
		}

		if ((lResult = CloseDevice(&ulErrorCode)) != MMC_SUCCESS)
		{
			LogError("CloseDevice", lResult, ulErrorCode);
			return lResult;
		}
	} break;
	case AM_INTERFACE_LIST:
		PrintAvailableInterfaces();
		break;
	case AM_PROTOCOL_LIST:
		PrintAvailableProtocols();
		break;
	case AM_VERSION_INFO:
	{
		if ((lResult = OpenDevice(&ulErrorCode)) != MMC_SUCCESS)
		{
			LogError("OpenDevice", lResult, ulErrorCode);
			return lResult;
		}

		if ((lResult = PrintDeviceVersion()) != MMC_SUCCESS)
		{
			LogError("PrintDeviceVersion", lResult, ulErrorCode);
			return lResult;
		}

		if ((lResult = CloseDevice(&ulErrorCode)) != MMC_SUCCESS)
		{
			LogError("CloseDevice", lResult, ulErrorCode);
			return lResult;
		}
	} break;
	case AM_UNKNOWN:
		printf("unknown option\n");
		break;
	}

	return lResult;
	getchar();

}

