#ifndef UTILITY_H_
#define UTILITY_H_

#include <string>
using namespace std;

void PrintNoResutl();
string GetBetween(string &sData,string strBeg,string strEnd);
void ToAMPEntity(string &sUrl);
int GetStrBetweenTwoStr(const string& strSrc ,string strPre, string  strSuf,string& strContent,int iPos);
#endif
