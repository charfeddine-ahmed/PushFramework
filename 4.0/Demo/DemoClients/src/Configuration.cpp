#include "StdAfx.h"
#include "Configuration.h"
#include "INIReader.h"


bool getConfigFile( std::string& path )
{
	char szPath[MAX_PATH];
	if( !GetModuleFileNameA( NULL, szPath, MAX_PATH ) )
		return 1;
	std::string strPath(szPath);
	std::string::size_type pos1 = strPath.rfind('\\');
	std::string::size_type pos2 = strPath.rfind('/');
	std::string::size_type pos = (pos1 == -1) ? pos2 : (pos2==-1) ? pos1 : (max(pos1, pos2));
	szPath[pos] = '\0';
	std::string strModuleDirectory(szPath);

	path = strModuleDirectory + "\\DemoClients.ini";

	return true;
}

bool readConfig()
{
	std::string strIniFile;
	if (!getConfigFile(strIniFile))
	{
		return false;
	}
	
	INIReader reader(strIniFile.c_str());

	if (reader.ParseError() < 0)
	{
		std::cout << "Could not load ini file " << strIniFile << endl;
		return false;
	}


	options.serverIP = reader.Get("server", "hostIp", "127.0.0.1");
	options.port = reader.GetInteger("server", "port", 10010);

	options.stayDuration = reader.GetInteger("connection", "stayDuration", 40);
	options.nConnectAtOnce = reader.GetInteger("connection", "connectAtOnce", 100);
	
	options.echo1Freq = reader.GetInteger("behavior", "echo1Freq", 18);
	options.echo2Freq = reader.GetInteger("behavior", "echo2Freq", 10);

	options.disableActivity = reader.GetInteger("trace", "disableActivity", 0) == 1;

	return true;
}

DemoClientsOptions options;
