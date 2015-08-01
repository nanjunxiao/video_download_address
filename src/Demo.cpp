#include<iostream>
#include"videofactory.h"
#include "videoaddress.h"
#include "utility.h"
#define __OUTPUT

int main(int argc, char *argv[])
{
	//cout << "Content-type:text/html" << endl << endl;
	cout << "Content-type:application/xml" << endl << endl;
	//if(argc != 2)
	//{
	//	//cout << "Usage: exe [url]" << endl;
	//	PrintNoResutl();
	//	return 1;
	//}	
	
	//modified by louis 5-22
	string sGetPara(getenv("QUERY_STRING"));
	//string sGetPara(argv[1]);
	if(sGetPara.empty() )
	{
		PrintNoResutl();
		return 1;
	}
	videofactory * pfactory = new videofactory(sGetPara);
	videoaddressbase *pvideo = pfactory->factory();
	if (pvideo == NULL)
	{
		//cout << "pvideo == null" << endl;
		PrintNoResutl();
		delete pfactory;
		return 1;
	}
	int iCode = pvideo->GetDownloadAddr();
	if (iCode < 0)
	{
		//cout << "sth wrong!!! code: " << iCode << endl;	
		PrintNoResutl();
	}
	delete pvideo;
	delete pfactory;
	return 0;
}
