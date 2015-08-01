#ifndef VIDEOADDRESS_H_
#define VIDEOADDRESS_H_

#include <curl/curl.h>
#include <string>
#include <map>
#include <vector>

using namespace std;

#define DEFAULT_HTTP_USER_AGENT "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.4 (KHTML, like Gecko) Chrome/22.0.1229.2 Safari/537.4"

/*BASE*/
class videoaddressbase
{
public:
	videoaddressbase(){}
	virtual ~videoaddressbase(){}
	bool GetWebPage(string sUrl,string &sContent);
	virtual int GetDownloadAddr()=0;
	void PrintResultXML();
private:
	static int write_callback(void *ptr, size_t size, size_t nmemb, void *stream);
	static char errorBuffer[CURL_ERROR_SIZE];
	//string m_sUrl;
	//added
protected:		
	string m_sSeconds;
	unsigned int m_iParts;
	vector<string> m_v_sFlvUrls;		
	vector<string> m_v_sMp4Urls;
	vector<string> m_v_sHDUrls;
};



/*youku*/
class videoaddressyouku : public videoaddressbase
{
public:
	videoaddressyouku(string sUrl):m_sUrl(sUrl),videoaddressbase()
	{}
	~videoaddressyouku()
	{}
	int GetDownloadAddr();	
private:
	string m_sUrl;
	struct StreamInfo{
        string sid;
        vector<string> keylist;};

	int global_seed;
	
	void ParseJson(string& buffer, map<string,StreamInfo>& streaminfo,int &seed);
	int rad(){
        global_seed = (global_seed*211+30031)%65536;
        return global_seed;
	}
	
	string  ParseStreamID(const string sid,int seed);
	void GetStreamUrl(map<string,StreamInfo>& streaminfo,int seed);
	string ParseUrlId();
};


/*sohu*/
class videoaddresssohu : public videoaddressbase
{
public:
	videoaddresssohu(string sUrl):m_sUrl(sUrl),videoaddressbase()
	{}
	~videoaddresssohu()
	{}
	int GetDownloadAddr();
private:
	string m_sUrl;

	int GetPos(string &sData,string strBeg);
	void parsefirstjson(string& buffer, string &videoId, string &playLength);
	void parsesecondjson(string& buffer,unsigned int &iParts,  vector<string> &v_sUrls);
	
};
/*sohu boke*/
/*added by louis 5-17*/
class videoaddresssohuboke:public videoaddressbase
{
public:
	videoaddresssohuboke(string sUrl):m_sUrl(sUrl),videoaddressbase(){}
	~videoaddresssohuboke(){}
	int GetDownloadAddr();
private:
	string m_sUrl;

	void parsejson(string &buffer,vector<string> &clipsURLs,vector<string> &sus);
	string GetKey(string &sContent);/*3-4 | zhijian.Maybe change,then wrong.attention*/
	string GetVidFromUrl(string &sUrl);
};



	typedef struct MetaInfoA
	{
	        string file_name;               //fn in xml
	        string url;                             //url in xml
	        string id;                              //id in xml where sl is 1
	        string td;                              //td in xml
	        vector<string> cd_set;  //cd in xml
	        vector<string> idx_set; // idx in xml
	}MetaInfoA;
	typedef struct MetaInfoB
	{
	        string key;                             //key in xml
	        string level;                   //level in xml
	}MetaInfoB;

/*tencent*/
class videoaddresstencent:public videoaddressbase
{
public:
	videoaddresstencent(string sUrl):m_sUrl(sUrl),videoaddressbase(){}
	~videoaddresstencent(){}
	int GetDownloadAddr();
private:
	string m_sUrl;


	/*get real video address where we can download it*/
	vector<string> GetRealVideoAddress(const string &url);

	MetaInfoA GetVideoInfoFirstStep(const string &xml_content);
	MetaInfoB GetVideoInfoSecondStep(const string &xml_content);
	/*case1:can get vid from url directly*/
	string GetVidFromURL(const string &url);
	/*case2:vid is not in the url, but in the webpage*/
	string GetVidFromPage(const string &url);
	/*according to vid, generate an url*/
	string GenerateFirstURL(const string &vid);	
	/*generate an url to get key and level*/
	string GenerateSecondURL(const MetaInfoA &metainfo, const string &vid);
	/*get the real video url ,maybe there a more than one url if the video has been divided into severl parts*/
	vector<string> GenerateFinalURL(const MetaInfoA &metainfo_a, const MetaInfoB &metainfo_b );
	/*according to url, get vid from url directly or webpage content*/
	string GetVid(const string &url);
	
};

/*sina*/
class videoaddresssina:public videoaddressbase
{
public:
	videoaddresssina(string sUrl):m_sUrl(sUrl),videoaddressbase(){}
	~videoaddresssina(){}

	int GetDownloadAddr();
public:
	struct sinaVideoInfo
	{
		string m_sTimeLen;
		unsigned int m_iPartNum;
		vector<string> m_v_Urls;		
	};
private:
	string m_sUrl;
private:
	/*SingleVideo, not support videolist now*/
	void GetVidFromSingleVideoPage(string& html_content,string &sFlvVid, string& sHDVid);
	//flv or HD urls
	sinaVideoInfo ParseUrlsFromXML(string& xml_content);
	
};

/*tudou*/
class videoaddresstudou:public videoaddressbase
{
public:
	videoaddresstudou(string sUrl):m_sUrl(sUrl),videoaddressbase(){}
	~videoaddresstudou(){}

	int GetDownloadAddr();
public:
	struct tudouinfo
	{
		string st_sUrl;
		string st_timelen;	
	};
private:
	string m_sUrl;
private:
	string GetICODEFromUrl(string& sUrl);
	string GetIIDFromHtml(string& html_content,string& sIcode);
	tudouinfo GetOneDownloadurl(string& xml_content);

	string DigitStr(string& content, int ipos);
	string GetIIDPatch(string& html_content);
};

#endif
