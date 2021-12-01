#pragma once



struct DemoClientsOptions
{
	std::string serverIP;
	int port;
	
	int nConnectAtOnce;
	int stayDuration;	

	int echo1Freq;
	int echo2Freq;
	//
	bool disableActivity;
	

} ;

bool getConfigFile(std::string& path);
bool readConfig();

extern DemoClientsOptions options;