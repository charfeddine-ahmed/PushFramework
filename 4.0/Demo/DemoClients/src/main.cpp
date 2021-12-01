// DemoClients.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DemoBot.h"


typedef std::vector<DemoBot*> botsVectT;
botsVectT botsVect;
int _tmain(int argc, _TCHAR* argv[])
{
	if (!readConfig())
	{
		cout << "Unable to read configuration file" << endl;
		return -1;
	}

	//Connect at once phase :
	for (int i = 0; i < options.nConnectAtOnce; i++)
	{
		botsVect.push_back(new DemoBot());
	}

	cout << "Put Q or q to quit" << endl;

	while (true)
	{
		char ch;

		cin >> ch;

		if (ch == 'q')
		{
			break;
		}
	}


	botsVectT::iterator it = botsVect.begin();
	while( it != botsVect.end())
	{
		DemoBot* pRobot = *it;
		pRobot->disconnect();
		//delete pRobot;
		it = botsVect.erase(it);
	}


	return 0;
}

