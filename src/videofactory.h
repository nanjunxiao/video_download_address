#ifndef VIDEOFACTORY_H_
#define VIDEOFACTORY_H_

#include"videoaddress.h"
class videofactory
{
public:
	videofactory(string sUrl):m_sUrl(sUrl){}
	~videofactory(){}
	videoaddressbase * factory();

private:
	string m_sUrl;		
};
#endif
