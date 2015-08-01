#include "videofactory.h"
#include <iostream>
using namespace std;

videoaddressbase * videofactory::factory()
{
	//added by Louis 2012-09-27
	if(m_sUrl.find("http://") == string::npos)
	{
		m_sUrl = "http://" + m_sUrl;
	}
	if(m_sUrl.find("youku") != string::npos)
	{
		return new videoaddressyouku(m_sUrl);
	}
	else if(m_sUrl.find("sohu") != string::npos)
	{
		if(m_sUrl.find("my.tv.sohu") != string::npos)
			return new videoaddresssohuboke(m_sUrl);
		else
			return new videoaddresssohu(m_sUrl);
	}
	else if(m_sUrl.find("v.qq") != string::npos)
	{
		return new videoaddresstencent(m_sUrl);
	}
	else if(m_sUrl.find("sina") != string::npos)
	{
		return new videoaddresssina(m_sUrl);
	}
	else if(m_sUrl.find("tudou") != string::npos)
	{
		return new videoaddresstudou(m_sUrl);
	}
	else 
	{
		cout << "unknown video site type!!!" << endl;	
		return NULL;
	}
}
