#ifndef MISC_H_INCLUDED
#define MISC_H_INCLUDED

#include <string>
#include <vector>
#include <sstream>

#ifdef _WIN32
#define PATH_SLASH "\\"
#else
#define PATH_SLASH "//"
#endif // _WIN32

#include "geoip.h"

using namespace std;

struct nodeInfo
{
    int linkType = -1;
    int id = 0;
    int groupID = 0;
    bool online = false;
    string group;
    string remarks;
    string server;
    int port = 0;
    string proxyStr;
    long long rawSpeed[20] = {};
    long long totalRecvBytes = 0;
    int duration = 0;
    string avgSpeed = "N/A";
    string maxSpeed = "N/A";
    string ulSpeed = "N/A";
    string pkLoss = "100.00%";
    int rawPing[6] = {};
    string avgPing = "0.00";
    int rawSitePing[10] = {};
    string sitePing = "0.00";
    string traffic;
    geoIPInfo inboundGeoIP;
    geoIPInfo outboundGeoIP;
    string testFile;
    string ulTarget;
};

static const string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

string UrlDecode(const string& str);
string base64_decode(string encoded_string);
string base64_encode(string string_to_encode);

vector<string> split(const string &s, const string &seperator);
string getUrlArg(string url, string request);
string replace_all_distinct(string str, string old_value, string new_value);
string urlsafe_base64_reverse(string encoded_string);
string urlsafe_base64_decode(string encoded_string);
string UTF8ToGBK(string str_src);
string GBKToUTF8(string str_src);
string trim(const string& str);
string getSystemProxy();
string rand_str(const int len);

void sleep(int interval);
bool regFind(string src, string target);
string regReplace(string src, string match, string rep);
bool regMatch(string src, string match);
string speedCalc(double speed);
string grabContent(string raw);
string getMD5(string data);
bool isIPv4(string address);
bool isIPv6(string address);
void urlParse(string url, string &host, string &path, int &port, bool &isTLS);

string fileGet(string path);
int fileWrite(string path, string content, bool overwrite);
bool fileExist(string path);
bool fileCopy(string source,string dest);
string fileToBase64(string filepath);
string fileGetMD5(string filepath);

static inline bool strFind(string str, string target)
{
    return str.find(target) != str.npos;
}

template <typename T> static inline void eraseElements(T *target)
{
    T().swap(*target);
}

#ifdef _WIN32
void StringToWstring(std::wstring& szDst, std::string str);
#endif // _WIN32

#endif // MISC_H_INCLUDED
