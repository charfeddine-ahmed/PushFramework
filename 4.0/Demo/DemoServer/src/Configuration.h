#pragma once



struct DemoServerOptions
{
	int port;
	int monitorPort;
	string password;
} ;

bool getConfigFile(std::string& path);
bool readConfig();

extern DemoServerOptions theDemoServerOptions;