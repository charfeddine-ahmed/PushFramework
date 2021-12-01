#include "stdafx.h"
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

	path = strModuleDirectory + "\\DemoServer.ini";

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

	theDemoServerOptions.port = reader.GetInteger("server", "port", 9001);
	theDemoServerOptions.monitorPort = reader.GetInteger("server", "monitorPort", 9002);
	theDemoServerOptions.password = reader.Get("server", "password", "semsem");

	return true;
}

DemoServerOptions theDemoServerOptions;
