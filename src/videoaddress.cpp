#include"videoaddress.h"
#include "json/json.h"
#include "urlcode.h"
#include "utility.h"
#include <iostream>
#include <cassert>

char videoaddressbase::errorBuffer[CURL_ERROR_SIZE];

bool videoaddressbase::GetWebPage(string sUrl,string &sContent)
{
	if (sUrl.empty() )
	{
		sContent = "";
		return false;
	}
	if (sUrl.find("http://") == string::npos)
	{
		sUrl = "http://" + sUrl;
	}
	CURL *curl;
	curl = curl_easy_init();
	if (!curl)
	{
		sContent = "";
		return false;
	}
	sContent = "";

	curl_easy_setopt(curl, CURLOPT_URL, sUrl.c_str() );
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &sContent);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 150);

	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
	//curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1);                // 以▒~K3个为▒~G~M▒~Z▒~P~Q设置
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10);

	curl_easy_setopt(curl, CURLOPT_USERAGENT, DEFAULT_HTTP_USER_AGENT);

	CURLcode res;
	res = curl_easy_perform(curl);
	if (res != CURLE_OK)
	{
		//cout << sUrl << " gather error code: " << res << endl;
		//cout << "Error: [" << res<< "] - " << errorBuffer << endl;
		PrintNoResutl();
		exit(-1);

		sContent = "";
		curl_easy_cleanup(curl);
		return false;
	}
	curl_easy_cleanup(curl);
	return true;	
}

int videoaddressbase::write_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
	static_cast<string *>(stream)->append(static_cast<const char *>(ptr), size * nmemb);
	return size * nmemb;
}

void videoaddressbase::PrintResultXML()
{
	cout << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" << endl;
	cout << "<data>" << endl;
	cout << "<seconds>" << m_sSeconds << "</seconds>" << endl;;
	cout << "<part>" << m_iParts << "</part>" << endl;;

	if(!m_v_sFlvUrls.empty() )
	{
		cout << "<videoCacheUrls type=\"flv\">" << endl;
		for(int i=0;i<m_v_sFlvUrls.size();++i)
		{
			//cout << m_v_sFlvUrls[i] << endl;
			ToAMPEntity(m_v_sFlvUrls[i]);
			cout << "<videoCacheUrl>" <<  m_v_sFlvUrls[i]<< "</videoCacheUrl>" ;
		}
		cout << endl;
		cout << "</videoCacheUrls>" << endl;
	}
	
	if(!m_v_sMp4Urls.empty() )
	{
		cout << "<videoCacheUrls type=\"mp4\">" << endl;
		for(int i=0;i<m_v_sMp4Urls.size();++i)
		{
			ToAMPEntity(m_v_sMp4Urls[i]);
			cout << "<videoCacheUrl>" << m_v_sMp4Urls[i] << "</videoCacheUrl>" ;
		}
		cout << endl;
		cout << "</videoCacheUrls>" << endl;
	}

	if(!m_v_sHDUrls.empty() )
	 {
		cout << "<videoCacheUrls type=\"hd\">" << endl;
		for(int i=0;i<m_v_sHDUrls.size();++i)
		{
			ToAMPEntity(m_v_sHDUrls[i]);
			cout << "<videoCacheUrl>" <<  m_v_sHDUrls[i]<< "</videoCacheUrl>" ;
		}
		cout << endl;
		cout << "</videoCacheUrls>" << endl;
	}
	cout << "</data>" << endl;
}


/*youku*/
int videoaddressyouku::GetDownloadAddr()
{
	string sId = ParseUrlId();
	if (sId.empty() )
	{
		return -2;
	}
	string sContent;
	string sUrl = "v.youku.com/player/getPlayList/VideoIDS/";
	sUrl += sId;
	sUrl += "/timezone/+08/version/5/source/video?n=3&ran=4132&password=";
	bool bCode = GetWebPage(sUrl,sContent);
	if (bCode == false)
	{
		//cout << "videoaddressyouku::GetWebPage failed!!!"  << endl;
		PrintNoResutl();
		exit(-1);

		return -1;
	}
	map<string,StreamInfo> streaminfo;
    	int seed;
    	ParseJson(sContent,streaminfo,seed);
 //   cout << streaminfo.size() <<endl;
    	GetStreamUrl(streaminfo,seed);	
	PrintResultXML();			
	return 0;
}


string videoaddressyouku::ParseUrlId()
{
	string sId="";
	string::size_type stBeg = m_sUrl.find("id_");
	if(stBeg == string::npos)
	{
		//cout << m_sUrl << " ParseUrlId find id_ failed!!!" << endl;
		PrintNoResutl();
		exit(-1);

		return "";
	}
	string::size_type stEnd = m_sUrl.find(".html");
	if(stEnd == string::npos)
	{
		
		//cout << m_sUrl << " ParseUrlId find .html failed!!!" << endl;
		PrintNoResutl();
		exit(-1);

		return "";
	}
	sId = m_sUrl.substr(stBeg+3,stEnd-stBeg-3);
	return sId;
}

void videoaddressyouku::ParseJson(string& buffer, map<string,StreamInfo>& streaminfo,int &seed)
{
	Json::Reader reader;
        Json::Value root;

        if (!reader.parse(buffer, root, false))
        {
                return ;
        }
                StreamInfo flvinfo;
                StreamInfo mp4info;
                StreamInfo hd2info;

        try{
                const Json::Value arrayObj = root["data"];
                if( arrayObj.size() !=1)
                        return;
                for (int i = 0; i < arrayObj.size(); i++)
                {
                        seed = arrayObj[i]["seed"].asInt();
                        string seconds;
                        seconds = arrayObj[i]["seconds"].asString();
			//modified by louis 5-7
                        //cout << seconds << endl;
			m_sSeconds = seconds;
                        //         const Json::Value streamfileidsObj = arrayObj[i]["streamfileids"];
                        //         cout << streamfileidsObj.size() << endl;
                        flvinfo.sid = arrayObj[i]["streamfileids"]["flv"].asString();
                        mp4info.sid = arrayObj[i]["streamfileids"]["mp4"].asString();
                        hd2info.sid = arrayObj[i]["streamfileids"]["hd2"].asString();
//                      cout << arrayObj[i]["streamfileids"]["flv"].asString()<< endl;
//                      cout << arrayObj[i]["streamfileids"]["hd2"].asString()<< endl;
//                      cout << arrayObj[i]["streamfileids"]["mp4"].asString()<< endl;
                        /*         for(int j=0;j< streamfileidsObj.size();j++){
                                   string hd2_sid =  streamfileidsObj[i].asString();
                                   string mp4_sid =  streamfileidsObj[i].asString();
                        //              string flv_sid =  streamfileidsObj[i]["flv"].asString();
                        cout << hd2_sid << endl;
                        cout << mp4_sid << endl;
                        //              cout << flv_sid << endl;
                        }*/
                        const Json::Value segs_hd2Obj = arrayObj[i]["segs"]["hd2"];
                        const Json::Value segs_flvObj = arrayObj[i]["segs"]["flv"];
                        const Json::Value segs_mp4Obj = arrayObj[i]["segs"]["mp4"];
			//added by louis 5-7
			//m_iParts = segs_flvObj.size();
			for(int j=0;j< segs_hd2Obj.size();j++){
                                string key = segs_hd2Obj[j]["k"].asString();
				//added by louis 5-8
				if ("-1" == key)
				{
					continue;
				}
                                hd2info.keylist.push_back(key);
//                              cout << segs_hd2Obj[j]["no"] << endl;
//                              cout << segs_hd2Obj[j]["k"] << endl;
                        }
                        for(int j=0;j< segs_flvObj.size();j++){
                                string key = segs_flvObj[j]["k"].asString();
				//added by louis 5-8
				if ("-1" == key)
				{
					continue;
				}
                                flvinfo.keylist.push_back(key);
//                              cout << segs_flvObj[j]["no"] << endl;
//                              cout << segs_flvObj[j]["k"] << endl;
                        }
                        for(int j=0;j< segs_mp4Obj.size();j++){
                                string key = segs_mp4Obj[j]["k"].asString();
				//added by louis 5-8
				if ("-1" == key)
				{
					continue;
				}
                                mp4info.keylist.push_back(key);
//                              cout << segs_mp4Obj[j]["no"] << endl;
//                              cout << segs_mp4Obj[j]["k"] << endl;
                        }
			//added by louis 5-8
			m_iParts = flvinfo.keylist.size();

                }
        }
        catch ( const std::exception &e )
        {
                //printf( "Unhandled exception:\n%s\n", e.what() );
		PrintNoResutl();
		exit(-1);
                // exitCode = 1;
        }
        if(flvinfo.sid.size()!=0  &&flvinfo.keylist.size()!=0){
                streaminfo["flv"]=flvinfo;
        }
        if(mp4info.sid.size()!=0  &&mp4info.keylist.size()!=0){
                streaminfo["mp4"]=mp4info;
        }
        if(hd2info.sid.size()!=0  &&hd2info.keylist.size()!=0){
                streaminfo["hd2"]=hd2info;
        }
}	


string videoaddressyouku::ParseStreamID(const string sid,int seed){
        string tran_sid;
        global_seed = seed;
        string abc ="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ/\\:._-1234567890";
        string cba;
        int k = abc.size();
        for(int i=0;i<k;i++){
                int index = (rad()*abc.size()/65535);
                char ex_c = abc[index];
                abc.replace(index,1,"");
         //       cout << abc.size() <<endl;
                cba+=ex_c;
        }
        char* sidbuf = new char[sid.size()+1];
        memset(sidbuf,0,sid.size()+1);
        memcpy(sidbuf,sid.c_str(),sid.size());
//      sidbuf[sid.size()]=0;
        char* pch = strtok(sidbuf, "*");
        while (pch != NULL){
//              printf ("%s\n",pch);
                tran_sid += cba[atoi(pch)];
                pch = strtok (NULL, "*");
        }
        delete[] sidbuf;
        return tran_sid;
}


void videoaddressyouku::GetStreamUrl(map<string,StreamInfo>& streaminfo,int seed){
//      global_seed = seed;
        map<string,StreamInfo>::const_iterator it = streaminfo.begin();
	//cout << "Content-type:text/html" << endl << endl;
        for(;it!=streaminfo.end();it++){
                //cout << it->first  <<endl;
                string config_id = "133576593670375839133";
                string sid = ParseStreamID(it->second.sid,seed);
                char stream_url[1024];
                char num_buf[4];
                int type;

                if(it->first == "mp4"){
                        type=0;
                }
                else if(it->first == "flv"){
                        type=1;
                }
                else if(it->first == "hd2"){
                        type=2;
                }else{
                        //cout <<it->first << "  type error" << endl;
			PrintNoResutl();
			exit(-1);

                        continue;
                }
                string num_str;
                for(int i = 0;i< it->second.keylist.size();i++){
                        memset(stream_url,0,1024);
                        memset(num_buf,0,4);
                        sprintf(num_buf,"%02X",i);
//                      num_str = num_buf;
                        sid.replace(8,2,num_buf);
                        if(type ==0 ){ //mp4
                                sprintf(stream_url,"http://f.youku.com/player/getFlvPath/sid/%s/st/%s/fileid/%s?K=%s",config_id.c_str(),"mp4",sid.c_str(),it->second.keylist[i].c_str());
                               // cout << i << ":\t" << stream_url << endl;
				m_v_sMp4Urls.push_back(stream_url);
                        }else if(type==1){//flv
                                sprintf(stream_url,"http://f.youku.com/player/getFlvPath/sid/%s/st/%s/fileid/%s?K=%s",config_id.c_str(),"flv",sid.c_str(),it->second.keylist[i].c_str());
                                //cout << i << ":\t" << stream_url << endl;
				m_v_sFlvUrls.push_back(stream_url);
                        }else{//hd2
                                sprintf(stream_url,"http://f.youku.com/player/getFlvPath/sid/%s/st/%s/fileid/%s?K=%s",config_id.c_str(),"flv",sid.c_str(),it->second.keylist[i].c_str());
                                //cout << i << ":\t" << stream_url << endl;
				m_v_sHDUrls.push_back(stream_url);
				}

                }
        }
}


/*sohu*/
int videoaddresssohu::GetDownloadAddr()
{
	//hard code
	string sUrlPage;
	bool bCode = GetWebPage(m_sUrl,sUrlPage);
	if(bCode == false)
	{
		//cout << "videoaddresssohu::GetDownloadAddr GetWebPage failed!!!" << endl;
		PrintNoResutl();
		exit(-1);

		return -1;
	}
	string sBeg = "PLAYLIST_ID=\"";
	string sEnd = "\"";

	string playlist_id = GetBetween(sUrlPage,sBeg,sEnd);
	string sjsonurl="";
	if(playlist_id.empty() )
	{
		//return -2;
		string vidbeg = "var vid=\"";
		string vidend = "\"";
		string vid = GetBetween(sUrlPage,vidbeg,vidend);
		//cout << "vid: " << vid << endl;
		if(vid.empty())
		{
			return -2;
		}
		string pidbeg = "var pid =\"";
		string pidend = "\"";
		string pid = GetBetween(sUrlPage,pidbeg,pidend);
		//cout << "pid: " << pid << endl;
		if(pid.empty() )
		{
			return -2;
		}
		sjsonurl = "http://hot.vrs.sohu.com/vrs_videolist.action?vid=";
		sjsonurl += vid;
		sjsonurl += "&pid=";
		sjsonurl += pid;
	}
	else
	{
		sjsonurl = "http://hot.vrs.sohu.com/vrs_videolist.action?playlist_id=";
		sjsonurl += playlist_id;
		sjsonurl += "&preload=1";
	}
	string jsondata;
	bCode = GetWebPage(sjsonurl,jsondata);
	if(bCode == false)
	{
		//cout << "videoaddresssohu::GetDownloadAddr GetWebPage failed!!!" << endl;
		PrintNoResutl();
		exit(-1);

		return -1;
	}
	int ijsonpos = GetPos(jsondata,"var vrsvideolist = ");
	if(ijsonpos == -1)
		return -3;	
	jsondata = jsondata.substr(ijsonpos);
	//parsejson
	string svideoId; 
	parsefirstjson(jsondata,svideoId,m_sSeconds);
	
	string anotherurl = "http://hot.vrs.sohu.com/vrs_flash.action?vid=";
	anotherurl += svideoId;
	anotherurl += "&af=1&bw=1230&g=8&t=0.4669713797047734";
	string anotherjson;
	bCode = GetWebPage(anotherurl,anotherjson);
	if(bCode == false)
        {
                //cout << "videoaddresssohu::GetDownloadAddr GetWebPage failed!!!" << endl;
		PrintNoResutl();
		exit(-1);

                return -1;
        }
	//added by louis 5-22
	vector<string> v_sUrl;
	parsesecondjson(anotherjson, m_iParts,v_sUrl);
	if(!v_sUrl.empty() )
	{
		string sFirstUrl = v_sUrl[0];
		if(sFirstUrl.find(".mp4") != string::npos)
			m_v_sMp4Urls = v_sUrl;
		else if(sFirstUrl.find(".flv") != string::npos)
			m_v_sFlvUrls = v_sUrl;
		else
			m_v_sHDUrls = v_sUrl;
	}
	//parsesecondjson(anotherjson, m_iParts,m_v_sMp4Urls);
	
	//print 
	PrintResultXML();
	return 0;
}


int videoaddresssohu::GetPos(string &sData,string strBeg)
{
	int ipos = sData.find(strBeg);
	if (ipos == -1)
	{
		//cout << "find first " << strBeg << " failed!!!" << endl;
		PrintNoResutl();
		exit(-1);

		return -1;
	}
	return ipos + strBeg.size();
}




void videoaddresssohu::parsefirstjson(string& buffer, string &videoId, string &playLength)
{
	Json::Reader reader;
        Json::Value root;

        if (!reader.parse(buffer, root, false))
        {
                return ;
        }
        try{
                const Json::Value arrayObj = root["videolist"];
                //if( arrayObj.size() !=1)
                //        return;
                //for (int i = 0; i < arrayObj.size(); i++)
                //{
			videoId = arrayObj[0]["videoId"].asString();	
			playLength = arrayObj[0]["playLength"].asString();
		//} 
        }
        catch ( const std::exception &e )
        {
                //printf( "Unhandled exception:\n%s\n", e.what() );
		PrintNoResutl();
		exit(-1);

                // exitCode = 1;
        }
	catch (...)
	{
		//cout << "Unknown error !!!" << endl;
		PrintNoResutl();
		exit(-1);
	}
}	



void videoaddresssohu::parsesecondjson(string& buffer,unsigned int &iParts,  vector<string> &v_sUrls)
{
	Json::Reader reader;
        Json::Value root;

        if (!reader.parse(buffer, root, false))
        {
                return ;
        }
        try{
                const Json::Value arrayObj = root["data"];
		iParts = -1;
		iParts = arrayObj["totalBlocks"].asInt();
		if(-1 == iParts)
		{
			cerr << "iParts == -1" << endl;
			return ;
		}	
                for (int i = 0; i < iParts; i++)
                {
			string one = arrayObj["clipsURL"][i].asString();	
			string two = arrayObj["su"][i].asString();
			string urlmerge = one + "?new=" + two;
			v_sUrls.push_back(urlmerge);
		} 
        }
        catch ( const std::exception &e )
        {
                //printf( "Unhandled exception:\n%s\n", e.what() );
		PrintNoResutl();
		exit(-1);
                // exitCode = 1;
        }
	catch (...)
	{
		//cout << "Unknown error !!!" << endl;
		PrintNoResutl();
		exit(-1);
	}
}	


/*sohuboke*/
/*********************************************/
string videoaddresssohuboke::GetVidFromUrl(string &sUrl)
{
        string::size_type stEnd = sUrl.rfind("/");
        if (stEnd == string::npos)
        {
		PrintNoResutl();
		exit(-1);
                return "";
        }
        return sUrl.substr(stEnd+1);
}

void videoaddresssohuboke::parsejson(string &buffer,vector<string> &clipsURLs, vector<string> &sus)
{
        Json::Reader reader;
        Json::Value root;

        if (!reader.parse(buffer, root, false))
        {
		PrintNoResutl();
		exit(-1);
                return ;
        }
        try{
                const Json::Value dataObj = root["data"];
                const Json::Value arrayObj = dataObj["clipsURL"];
                for(int i=0;i<arrayObj.size();++i)
                {
                        clipsURLs.push_back(arrayObj[i].asString() );
                }
                const Json::Value suObj = dataObj["su"];
                for(int i=0;i<suObj.size();++i)
                {
                        sus.push_back(suObj[i].asString() );
                }
                m_sSeconds = dataObj["totalDuration"].asString();
                if (clipsURLs.size() != sus.size() )
		{
			PrintNoResutl();
			exit(-1);
                        cerr << "++++++++ clipsurl.size != su.size ++++++++++" << endl;
		}
                m_iParts = clipsURLs.size();
        }
        catch ( const std::exception &e )
        {
		PrintNoResutl();
		exit(-1);
                printf( "Unhandled exception:\n%s\n", e.what() );
                // exitCode = 1;
        }
        catch (...)
        {
		PrintNoResutl();
		exit(-1);
                cout << "Unknown error !!!" << endl;
        }
}


string videoaddresssohuboke::GetKey(string &sContent)
{
        string::size_type stBeg = sContent.find("|");
        if(stBeg == string::npos)
        {
		PrintNoResutl();
		exit(-1);
                cerr << "find first | failed!!!" << endl;
                return "";
        }
        stBeg = sContent.find("|",stBeg+1);
        if(stBeg == string::npos)
        {
		PrintNoResutl();
		exit(-1);
                cerr << "find second | failed!!!" << endl;
                return "";
        }
        stBeg = sContent.find("|",stBeg+1);
        if(stBeg == string::npos)
        {
		PrintNoResutl();
		exit(-1);
                cerr << "find third | failed!!!" << endl;
                return "";
        }
        string::size_type stEnd = sContent.find("|",stBeg+1);
        if(stEnd == string::npos)
        {
		PrintNoResutl();
		exit(-1);
                cerr << "find forth | failed!!!" << endl;
                return "";
        }
        return sContent.substr(stBeg+1,stEnd-stBeg-1);
}


int videoaddresssohuboke::GetDownloadAddr()
{
        string vid = GetVidFromUrl(m_sUrl);
        string firsturl = "http://my.tv.sohu.com/videinfo.jhtml?m=viewnew&vid=";
        firsturl+=vid;
        string sContent;
        bool bCode = GetWebPage(firsturl,sContent);
        if(bCode == false)
        {
		PrintNoResutl();
		exit(-1);
                cerr << "sohuboke::getwebpage failed!!!" << endl;
                return -1;
        }
        vector<string> v_clipsUrls, v_sus;
        parsejson(sContent,v_clipsUrls,v_sus);
        for(int i=0;i<v_clipsUrls.size();++i)
        {
                string secondurl = "http://data.vod.itc.cn/?prot=2&file=";
                secondurl += v_clipsUrls[i]+"&new="+v_sus[i]+"&t=0.34299587132409215";
                sContent.clear();
                bCode = GetWebPage(secondurl,sContent);
                if(bCode == false)
                {
			PrintNoResutl();
			exit(-1);
                        cerr << "sohuboke::getwebpage failed!!!" << endl;
                        return -1;
                }
                string key = GetKey(sContent);
                if(key.empty() )
                {
			PrintNoResutl();
			exit(-1);
                        cerr << "sohuboke GetKey failed!!!" << endl;
                        return -2;
                }
                string smergeurl = "http://sohu.vodnew.lxdns.com";
                smergeurl += v_sus[i]+ "?key=" + key;
                //m_v_sMp4Urls.push_back(smergeurl);
		//modified by louis 5-22
		if(smergeurl.find(".mp4") != string::npos)
			m_v_sMp4Urls.push_back(smergeurl);
		else if(smergeurl.find(".flv") != string::npos)
			m_v_sFlvUrls.push_back(smergeurl);
		else
			m_v_sHDUrls.push_back(smergeurl);
        }
        PrintResultXML();
        return 0;
}
/*********************************************/

/*tencent*/
MetaInfoA videoaddresstencent::GetVideoInfoFirstStep(const string &xml_content)
{
        MetaInfoA metainfo;

        //get file name
        int fn_pos_start = xml_content.find("<fn>");
        int fn_pos_end = xml_content.find("</fn>");
        if(fn_pos_start != -1 && fn_pos_end != -1)
        {
                metainfo.file_name = xml_content.substr(fn_pos_start+4, fn_pos_end-fn_pos_start-4);
        }

        //get url
        /*int url_pos_start = xml_content.find("<url>");
        int url_pos_end = xml_content.find("</url>");
        if(url_pos_start != -1 && url_pos_end != -1)
        {
                metainfo.url = xml_content.substr(url_pos_start+5, url_pos_end-url_pos_start-5);
        }*/
	//modified by Louis 2012-08-27,the last url,first url not run ok.
        int url_pos_start = xml_content.rfind("<url>");
        int url_pos_end = xml_content.rfind("</url>");
        if(url_pos_start != -1 && url_pos_end != -1)
        {
                metainfo.url = xml_content.substr(url_pos_start+5, url_pos_end-url_pos_start-5);
        }

        //get id
        int sl_pos = xml_content.find("<sl>1</sl>");
        if(sl_pos != -1)
        {
                string part = xml_content.substr(0, sl_pos);
                int id_pos_start = part.rfind("<id>");
                int id_pos_end = part.rfind("</id>");
                metainfo.id = part.substr(id_pos_start+4, id_pos_end-id_pos_start-4);
        }

        //get td
        int td_pos_start = xml_content.find("<td>");
        int td_pos_end = xml_content.find("</td>");
        if(td_pos_start != -1 && td_pos_end != -1)
        {
                metainfo.td = xml_content.substr(td_pos_start+4, td_pos_end-td_pos_start-4);
        }

        //get cd and idx
        string part_content(xml_content);
        while(1)
        {
                int cd_pos_start = part_content.find("<cd>");
                int cd_pos_end = part_content.find("</cd>");
                int idx_pos_start = part_content.find("<idx>");
		 int idx_pos_end = part_content.find("</idx>");
                if(cd_pos_start != -1 && cd_pos_end != -1 && idx_pos_start != -1 && idx_pos_end != -1)
                {

                        metainfo.cd_set.push_back(part_content.substr(cd_pos_start+4, cd_pos_end-cd_pos_start-4));
                        metainfo.idx_set.push_back(part_content.substr(idx_pos_start+5, idx_pos_end-idx_pos_start-5));
                        part_content = part_content.substr(idx_pos_end+8);
                }
                else
                {
                        break;
                }
        }

        return metainfo;
}


MetaInfoB videoaddresstencent::GetVideoInfoSecondStep(const string &xml_content)
{
        MetaInfoB metainfo;

        //get key
        int key_pos_start = xml_content.find("<key>");
        int key_pos_end = xml_content.find("</key>");
        if(key_pos_start != -1 && key_pos_end != -1)
        {
                metainfo.key = xml_content.substr(key_pos_start+5, key_pos_end-key_pos_start-5);
        }

        //get level
        int level_pos_start = xml_content.find("<level>");
        int level_pos_end = xml_content.find("</level>");
        if(level_pos_start != -1 && level_pos_end != -1)
        {
                metainfo.level = xml_content.substr(level_pos_start+7, level_pos_end-level_pos_start-7);
        }

        return metainfo;
}


/*case1:can get vid from url directly*/
string videoaddresstencent::GetVidFromURL(const string &url)
{
        string vid("");
        int pos = url.find("vid=");
        vid = url.substr(pos+4);

        return vid;
}


/*case2:vid is not in the url, but in the webpage*/
string videoaddresstencent::GetVidFromPage(const string &url)
{
        string vid("");
	//modified by louis 5-7
        //string page_content = CurlURL(url);
	string page_content;
	bool bCode = GetWebPage(url,page_content); 
	if(bCode == false)
	{
		//cout << "videoaddresstencent::GetVidFromPage GetWebPage failed!!!" << endl;
		PrintNoResutl();
		exit(-1);

		return "";
	}
        int pos = page_content.find("vid:\"");
        if(pos != -1)
        {
                string part = page_content.substr(pos+5);
                vid = part.substr(0, part.find("\""));
        }
	//added by louis 5-16
	if(vid.empty() )	
	{
		int inewpos = page_content.find("vid = vid || \"");		
		if(inewpos != -1)
		{
			string part = page_content.substr(inewpos+14);
			vid = part.substr(0,part.find("\""));
		}
	}
        return vid;
}


/*according to vid, generate an url*/
string videoaddresstencent::GenerateFirstURL(const string &vid)
{
        const string url = "http://vv.video.qq.com/getinfo?ran=34534&platform=1&charge=0&vids="+vid+"&speed=711&otype=xml";

        return url;
}



/*generate an url to get key and level*/
string videoaddresstencent::GenerateSecondURL(const MetaInfoA &metainfo, const string &vid)
{
        string url = "http://vv.video.qq.com/getkey?charge=0&format="+metainfo.id+"&otype=xml&ran=34543&platform=1&vid="+vid+"&filename="+UrlEncode(metainfo.file_name);

        return url;
}


/*get the real video url ,maybe there a more than one url if the video has been divided into severl parts*/
vector<string> videoaddresstencent::GenerateFinalURL(const MetaInfoA &metainfo_a, const MetaInfoB &metainfo_b )
{
        int pos = metainfo_a.file_name.rfind('.');
        string video_type = metainfo_a.file_name.substr(pos+1);
        string video_name, url;
        vector<string> all_url;

        if(metainfo_a.idx_set.empty())
        {
                video_name = metainfo_a.file_name;
                url = metainfo_a.url+video_name+"?sdtfrom=v2&type="+video_type+"&vkey="+metainfo_b.key+"&level="+metainfo_b.level;
                all_url.push_back(url);
        }
        else
        {
                for(int i = 0; i < metainfo_a.idx_set.size(); ++i)
                {
                        video_name = metainfo_a.file_name.substr(0, pos+1)+metainfo_a.idx_set[i]+"."+video_type;
                        url = metainfo_a.url+video_name+"?sdtfrom=v2&type="+video_type+"&vkey="+metainfo_b.key+"&level="+metainfo_b.level;
                        all_url.push_back(url);
                }
        }

        return all_url;
}



/*according to url, get vid from url directly or webpage content*/
string videoaddresstencent::GetVid(const string &url)
{
        if(url.find("vid=") != -1)
        {
                return GetVidFromURL(url);
        }
        else
        {
		 //qq mv <meta http-equiv="refresh" content="0; url=http://www.3Gabc.com">
                //modified by louis 2012-6-10
                string sVid = GetVidFromPage(url);
                if(sVid.empty() )
                {
                        string sContent;
                        bool bCode = GetWebPage(url,sContent);
                        if(bCode == false)
                        {
				//PrintNoResutl();
				//exit(-1);
                                //cerr << "qq GetVid download page failed" << endl;
                                return "";
                        }
                        string::size_type stRefresh = sContent.find("meta http-equiv=\"refresh\"");
                        if(stRefresh == string::npos)
                        {
				//PrintNoResutl();
				//exit(-1);
                                //cout << "find meta http-equiv=\"refresh\" failed!!!" << endl;
                                return "";
                        }
                        string::size_type stUrl = sContent.find("url=",stRefresh+25);
                        if(stUrl == string::npos)
                        {
				//PrintNoResutl();
				//exit(-1);
                                //cout << "find url= failed!!! " << endl;
                                return "";
                        }
                        string::size_type stTag = sContent.find("\"",stUrl+4);
                        if(stTag == string::npos)
                        {
				//PrintNoResutl();
				//exit(-1);
                                //cout << "find \" failed!!!" << endl;
                                return "";
                        }
                        string sNewUrl = sContent.substr(stUrl+4,stTag-stUrl-4);
                        //cout << sNewUrl << endl;
                        //vid in url
                        return GetVidFromURL(sNewUrl);
                }
                return sVid;
                //return GetVidFromPage(url);
        }
}



/*get real video address where we can download it*/
vector<string> videoaddresstencent::GetRealVideoAddress(const string &url)
{
    const string vid = GetVid(url);

        string __url = GenerateFirstURL(vid);
	//modified by louis 5-7
        //string content = CurlURL(__url);
	string content;
	bool bCode = GetWebPage(__url,content);
	if(bCode == false)
	{
		//cout << "videoaddresstencent::GetRealVideoAddress GetWebPage failed!!!" << endl;
		PrintNoResutl();
		exit(-1);

		vector<string> v_sTmp;
		return v_sTmp;
	}
        const MetaInfoA info_a = GetVideoInfoFirstStep(content);
	//added by louis 5-7
	m_sSeconds = info_a.td;
	//modified by louis 5-16
	//m_iParts = info_a.cd_set.size();

        __url = GenerateSecondURL(info_a, vid);

	//modified by louis 5-7
        //content = CurlURL(__url);
	content.clear();
	bCode = GetWebPage(__url,content);
	if(bCode == false)
	{
		//cout << "videoaddresstencent::GetRealVideoAddress GetWebPage failed!!!" << endl;
		PrintNoResutl();
		exit(-1);

		vector<string> v_sTmp;
		return v_sTmp;
	}
	
        const MetaInfoB info_b = GetVideoInfoSecondStep(content);
        vector<string> all_url = GenerateFinalURL(info_a, info_b);
	//modified by louis 5-16
	m_iParts = all_url.size();
        return all_url;
}


int videoaddresstencent::GetDownloadAddr()
{
	//added by louis 5-22
	//affect effecient
	vector<string> v_sUrls= GetRealVideoAddress(m_sUrl);
	if(!v_sUrls.empty() )
	{
		string sFirstUrl = v_sUrls[0];
		if(sFirstUrl.find(".mp4") != string::npos)
			m_v_sMp4Urls = v_sUrls;	
		else if(sFirstUrl.find(".flv") != string::npos)
			m_v_sFlvUrls = v_sUrls;
		else//left to HD,maybe...
			m_v_sHDUrls = v_sUrls;
	}
	//m_v_sMp4Urls = GetRealVideoAddress(m_sUrl);
	PrintResultXML();
	return 0;
}

/***********************************************/
/*sina*/
void videoaddresssina::GetVidFromSingleVideoPage(string& html_content,string &sFlvVid, string& sHDVid)
{
	//struct 
	string strBeg = "video:{";
	string strEnd = "}";
	string videobetween = GetBetween(html_content,strBeg,strEnd);	
	//cout << "vibt:\t" << videobetween << endl;
	if(videobetween.empty() )
	{
		//may be bugs, strBeg not only.
		//TODO
		strBeg = "video : {";
		videobetween = GetBetween(html_content,strBeg,strEnd);
		if(videobetween.empty() )
		{
			PrintNoResutl();
			exit(-10);
		}
	}
	//cout << "vibt2:\t" << videobetween << endl;
	string vidbetween = GetBetween(videobetween,"vid:\'","\'");
	if(vidbetween.empty() )
	{
		vidbetween = GetBetween(videobetween,"vid :\'","\'");
		if(vidbetween.empty() )
		{
			PrintNoResutl();
			exit(-10);
		}
	}
	string::size_type stflag = vidbetween.find("|");
	if(stflag == string::npos)
	{
		sFlvVid = vidbetween;
	}
	else
	{
		sFlvVid = vidbetween.substr(0,stflag);
		sHDVid = vidbetween.substr(stflag+1);
	}
	//cout << "vid:\t" << sFlvVid <<  endl;
}

videoaddresssina::sinaVideoInfo videoaddresssina::ParseUrlsFromXML(string& xml_content)
{
	sinaVideoInfo infotmp;
	infotmp.m_sTimeLen = GetBetween(xml_content,"<timelength>","</timelength>");	
	infotmp.m_iPartNum = 0;
	//string::size_type stbegin = 0;
	int stbegin = 0;
	string soneurl;
	while(stbegin != -1)
	{
		stbegin = GetStrBetweenTwoStr(xml_content,"http://","]",soneurl,stbegin);
		if(stbegin != -1)
		{
			infotmp.m_v_Urls.push_back("http://"+soneurl);
			//cout << soneurl << endl;
			infotmp.m_iPartNum++;
		}	
	}		
	return infotmp;
}


int videoaddresssina::GetDownloadAddr()
{
	if(m_sUrl.find(".htm") == string::npos)
	{
		//TODO videolist
		PrintNoResutl();
		exit(-2);
	}
	//cout << m_sUrl << endl;
	string shtml_content;
	bool bcode = GetWebPage(m_sUrl,shtml_content);	
	if(false == bcode)
	{
		cout << "gather failed!!!" << endl;
		PrintNoResutl();
		exit(-1);
	}

	string sFlvVid,sHDVid;
	GetVidFromSingleVideoPage(shtml_content,sFlvVid,sHDVid);	
	//
	//cout << "sFlvVid:\t" << sFlvVid << "sHDVid:\t" << sHDVid << endl;
	if(sFlvVid.empty() && sHDVid.empty() )
	{
		PrintNoResutl();
		exit(-10);
		return -10;
	}
	if(!sFlvVid.empty() )
	{
		string sxmlurl = "http://v.iask.com/v_play.php?vid=" + sFlvVid +  "&uid=0&referrer=http://video.sina.com.cn&ran=0.23728&r=video.sina.com.c";
		string sxml_content;
		bcode = GetWebPage(sxmlurl,sxml_content);
		if(false == bcode)
		{
			PrintNoResutl();
			exit(-10);
		}
		sinaVideoInfo sinavinfo = ParseUrlsFromXML(sxml_content);
		m_sSeconds = sinavinfo.m_sTimeLen;
		m_iParts = sinavinfo.m_iPartNum;
		m_v_sFlvUrls = sinavinfo.m_v_Urls;	
	}
	if(!sHDVid.empty() )	
	{
		string sxmlurl = "http://v.iask.com/v_play.php?vid=" + sHDVid +  "&uid=0&referrer=http://video.sina.com.cn&ran=0.23728&r=video.sina.com.c";
		string sxml_content;
		bcode = GetWebPage(sxmlurl,sxml_content);
		if(false == bcode)
		{
			PrintNoResutl();
			exit(-10);
		}
		sinaVideoInfo sinavinfo = ParseUrlsFromXML(sxml_content);
		m_sSeconds = sinavinfo.m_sTimeLen;
		m_iParts = sinavinfo.m_iPartNum;
		m_v_sHDUrls = sinavinfo.m_v_Urls;	
	}
	PrintResultXML();
	return 0;
}


/*********************************************/
/*tudou*/
string videoaddresstudou::GetICODEFromUrl(string& sUrl)
{
	string::size_type sthtml = sUrl.find(".htm");
	if(sthtml == string::npos)
	{
		string::size_type stslash = sUrl.rfind("/");
		if(stslash == string::npos)
		{
			PrintNoResutl();
			exit(-10);
		}
		return sUrl.substr(stslash+1);
	}
	else
	{
		string::size_type stslash = sUrl.rfind("/",sthtml);	
		if(stslash == string::npos)
		{
			PrintNoResutl();
			exit(-10);
		}
		return sUrl.substr(stslash+1,sthtml-stslash-1);
	}
}

string videoaddresstudou::GetIIDFromHtml(string& html_content,string& sIcode)
{
	int icodetovaluelen = 5;
	string::size_type sticode = 0,stvalue = 0;
	stvalue = html_content.find(sIcode);
	if(stvalue == string::npos)
	{
		return "";
	}
	sticode = html_content.rfind("icode",stvalue);
	if(sticode == string::npos || stvalue-sticode-5>=icodetovaluelen)
	{
		//PrintNoResutl();
		//exit(-11);
		return "";
	}
	//rfind iid
	string::size_type stiid = html_content.rfind("iid",sticode);
	if(stiid == string::npos)
	{
		//PrintNoResutl();
		//exit(-11);
		return "";
	}
	return DigitStr( html_content, stiid+3); 
}

string videoaddresstudou::DigitStr(string& content, int ipos)
{
	string sDigit;
	if(ipos >= content.size() )
	{
		PrintNoResutl();
		exit(-11);
	}
	int ibeg = ipos;
	while((int)content[ibeg]<0 || !isdigit(content[ibeg]) )
	{
		ibeg++;
	}
	while((int)content[ibeg]>0 && isdigit(content[ibeg]) )
	{
		sDigit.push_back(content[ibeg]);
		ibeg++;
	}
	return sDigit;
}

string videoaddresstudou::GetIIDPatch(string& html_content)
{
	//TODO
	//may be bug,has iid = condition
	string::size_type stiid = html_content.find("iid:");
	if(stiid == string::npos)
	{
		PrintNoResutl();
		exit(-10);
	}
	return DigitStr(html_content,stiid+4);
}

videoaddresstudou::tudouinfo videoaddresstudou::GetOneDownloadurl(string& xml_content)
{
	tudouinfo tinfo;
	tinfo.st_timelen = GetBetween(xml_content,"tm=\"","\"");

	tinfo.st_sUrl = GetBetween(xml_content,"http://","</f>");
	if(tinfo.st_timelen.empty() || tinfo.st_sUrl.empty() )
	{
		PrintNoResutl();
		exit(-10);
	}
	tinfo.st_sUrl = "http://"+tinfo.st_sUrl;
	return tinfo;
}

int videoaddresstudou::GetDownloadAddr()
{
	string sICODE = GetICODEFromUrl(m_sUrl);
	//cout << "icode:\t" << sICODE << endl;
	string html_content; 
	bool bcode = GetWebPage(m_sUrl,html_content);
	if(false == bcode)
	{
		PrintNoResutl();
		exit(-1);
	}
	string sIID = GetIIDFromHtml(html_content,sICODE);	
	//cout << "iid 1:\t" << sIID << endl;
	if(sIID.empty() )
	{
		sIID = GetIIDPatch(html_content);
		if(sIID.empty() )
		{
			PrintNoResutl();
			exit(-10);
		}
	}
	//cout << "iid 2:\t" << sIID << endl;
	string sxmlurl = "http://v2.tudou.com/v?vn=02&st=1%2C2&it=" + sIID;
	string xml_content;
	bcode = GetWebPage(sxmlurl,xml_content);
	if(false == bcode)
	{
		PrintNoResutl();
		exit(-1);
	}
	tudouinfo tinfo = GetOneDownloadurl(xml_content);
	m_iParts = 1;
	m_v_sFlvUrls.clear();
	m_v_sFlvUrls.push_back(tinfo.st_sUrl); 
	m_sSeconds = tinfo.st_timelen;


	PrintResultXML();
	return 0;
}
