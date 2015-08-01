#include "utility.h"
#include <iostream>
using namespace std;

string GetBetween(string &sData,string strBeg,string strEnd)
{
	string::size_type stBeg = sData.find(strBeg);
	if(stBeg == string::npos)
	{
		//cout << "find " << strBeg << " failed!!!" << endl;
		//PrintNoResutl();
		//exit(-1);
		return "";
	}
	string::size_type stEnd = sData.find(strEnd,stBeg+strBeg.size() );
	
	if(stEnd == string::npos)
	{
		//cout << "find " << strEnd << " failed!!!" << endl;
		//PrintNoResutl();
		//exit(-1);
		return "";
	}
	return sData.substr(stBeg+strBeg.size(),stEnd-stBeg-strBeg.size() );
}



void PrintNoResutl()
{
	cout << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" << endl;
	cout << "<data>" << endl;
	cout << "<seconds>0</seconds>" << endl;
	cout << "<part>0</part>" << endl;
	cout << "<videoCacheUrls type=\"unknown\">" << endl;
	cout << "</videoCacheUrls>" << endl;
	cout << "</data>" << endl;
}



void ToAMPEntity(string &sUrl)
{
	string::size_type stBeg = 0,stEnd = 0;
	//modified by Louis 2012-09-27
	//todou has &amp; already
	if(sUrl.find("&amp;") != string::npos)
	{
		return;
	}
	while((stBeg=sUrl.find("&",stEnd)) != string::npos)
	{
		sUrl.replace(stBeg,1,"&amp;");
		stEnd = stBeg + 5;
	}
} 



/*===========================================================================================	
	GetStrBetweenTwoStr..strSrc.pos.....strPre.strSuf........strContent.
	....
		-1...
		>=0.......strSuf.....
=============================================================================================*/
int GetStrBetweenTwoStr(const string& strSrc ,string strPre, string  strSuf,string& strContent,int iPos)
{
	if (strSrc.empty())
	{
		return -1;
	}
	string::size_type iPos1=strSrc.find(strPre,iPos);
	if (iPos1 != string::npos)
	{
		iPos1 += strPre.length();
		string::size_type iPos2 = strSrc.find(strSuf,iPos1);
		if (iPos2 != string::npos)
		{
			strContent = strSrc.substr(iPos1,iPos2 - iPos1);
			return (iPos2 + strSuf.length());
		}
	}

	return -1;
}
