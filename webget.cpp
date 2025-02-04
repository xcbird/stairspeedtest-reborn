#include <iostream>
#include <unistd.h>

#include <curl/curl.h>

#include "webget.h"
#include "logger.h"

using namespace std;

const int times_to_ping = 3;

void draw_progress_sping(int progress, int values[3])
{
    cerr<<"\r[";
    for(int i = 0; i <= progress; i++)
    {
        cerr<<(values[i] == 0 ? "*" : "-");
    }
    if(progress == times_to_ping - 1)
    {
        cerr<<"]";
    }
    cerr<<" "<<progress + 1<<"/"<<times_to_ping<<" "<<values[progress]<<"ms";
}

string user_agent_str = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/74.0.3729.169 Safari/537.36";

static int writer(char *data, size_t size, size_t nmemb, string *writerData)
{
    if(writerData == NULL)
        return 0;

    writerData->append(data, size*nmemb);

    return size * nmemb;
}

static size_t writer_dummy(void *ptr, size_t size, size_t nmemb, void *data)
{
  /* we are not interested in the downloaded bytes itself,
     so we only return the size we would have saved ... */
  (void)ptr;  /* unused */
  (void)data; /* unused */
  return (size_t)(size * nmemb);
}

string httpGet(string host, string addr, string uri)
{
    string recvdata = "", strTmp = "";
    char bufRecv[BUF_SIZE];
    int retVal = 0, cur_len = 0;
    SOCKET sHost;

    sHost = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(INVALID_SOCKET == sHost)
        return string();
    startConnect(sHost, addr, 80);

    string content = "GET " + uri + " HTTP/1.1\r\n"
                   "Host: " + host + "\r\n"
                   "User-Agent: " + user_agent_str + "\r\n"
                   "Accept: */*\r\n\r\n";

    setTimeout(sHost, 1000);
    retVal = send_simple(sHost, content);
    if((unsigned)retVal != content.size())
    {
        closesocket(sHost);
        return string();
    }
    while(1)
    {
        ZeroMemory(bufRecv, BUF_SIZE);
        cur_len = Recv(sHost, bufRecv, BUF_SIZE, 0);
        if(cur_len < 0)
        {
            if(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
                continue;
            else
                break;
        }
        if(cur_len == 0)
            break;
        recvdata += bufRecv;
    }
    closesocket(sHost);
    return recvdata;

}

string httpsGet(string host, string addr, string uri)
{
    string recvdata;
    SSL_CTX *ctx;
    SSL *ssl;
    SOCKET sHost;

    SSL_load_error_strings();
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    ctx = SSL_CTX_new(TLS_client_method());
    ssl = SSL_new(ctx);
    if(ctx == NULL)
    {
        ERR_print_errors_fp(stdout);
        return string();
    }
    if((sHost = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cerr<<"socket error "<<errno<<endl;
        return string();
    }
    if(startConnect(sHost, addr, 443) != 0)
    {
        cerr<<"Connect err "<<errno<<endl;
        return string();
    }

    SSL_set_fd(ssl, sHost);
    if(SSL_connect(ssl) != 1)
    {
        ERR_print_errors_fp(stderr);
    }
    else
    {
        //cerr<<"connected with "<<SSL_get_cipher(ssl)<<" encryption."<<endl;
        string data = "GET " + uri + " HTTP/1.1\r\n"
                    "Host: " + host + "\r\n"
                    "User-Agent: " + user_agent_str + "\r\n"
                    "Accept: */*\r\n\r\n";
        cerr<<data<<endl;
        SSL_write(ssl, data.data(), data.size());
        int len;
        char tmpbuf[BUF_SIZE];
        while(true)
        {
            ZeroMemory(tmpbuf, BUF_SIZE);
            len = SSL_read(ssl, tmpbuf, BUF_SIZE-1);
            if(len <= 0)
                break;
            recvdata += tmpbuf;
        }
    }
    SSL_clear(ssl);
    closesocket(sHost);
    return recvdata;
}

string curlGet(string url, string proxy)
{
    CURL *curl_handle;
    string data;

    curl_global_init(CURL_GLOBAL_ALL);

    curl_handle = curl_easy_init();
    curl_easy_setopt(curl_handle, CURLOPT_URL, url.data());
    curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, user_agent_str.data());
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writer);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &data);
    if(proxy != "")
        curl_easy_setopt(curl_handle, CURLOPT_PROXY, proxy.data());

    curl_easy_perform(curl_handle);
    curl_easy_cleanup(curl_handle);

    return data;
}

long curlPost(string url, string data, string proxy)
{
    CURL *curl_handle;
    double retVal = 0.0;

    CURLcode res = curl_global_init(CURL_GLOBAL_ALL);

    curl_handle = curl_easy_init();

    curl_easy_setopt(curl_handle, CURLOPT_URL, url.data());
    curl_easy_setopt(curl_handle, CURLOPT_HEADER, 0L);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_POST, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, data.data());
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, data.size());
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
    if(proxy != "")
        curl_easy_setopt(curl_handle, CURLOPT_PROXY, proxy.data());

    res = curl_easy_perform(curl_handle);

    if(res == CURLE_OK)
    {
        res = curl_easy_getinfo(curl_handle, CURLINFO_SPEED_UPLOAD, &retVal);
    }

    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();
    return retVal;
}

string buildSocks5ProxyString(string addr, int port, string username, string password)
{
    string authstr = username != "" && password != "" ? username + ":" + password + "@" : "";
    string proxystr = "socks5://" + authstr + addr + ":" + to_string(port);
    return proxystr;
}

string buildSocks5ProxyString(socks5Proxy proxy)
{
    return buildSocks5ProxyString(proxy.address, proxy.port, proxy.username, proxy.password);
}

string webGet(string url, string proxy)
{
    return curlGet(url, proxy);
    /*
    string host,uri, addr;
    bool https = regmatch(url, "^https(.*)");

    url = regreplace(url, "^(http|https)://", "");
    host = url.substr(0, url.find("/"));
    uri = url.substr(url.find("/"));

    if(!regmatch(host, "\\d+.\\d+.\\d+.\\d")) addr = hostname2ipv4(host, 80); else addr = host;
    if(!addr.size()) return string();

    if(https) return httpsGet(host, addr, uri); else return httpGet(host, addr, uri);
    */
}

string webGet(string url, socks5Proxy proxy)
{
    return curlGet(url, buildSocks5ProxyString(proxy));
}

double getLoadPageTime(string url, long timeout, string proxy)
{
    CURL *curl_handle;
    CURLcode res;
    double time_total = 0.0;
    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    curl_easy_setopt(curl_handle, CURLOPT_URL, url.data());
    curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writer_dummy);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, user_agent_str.data());
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(curl_handle, CURLOPT_PROXY, proxy.data());
    res = curl_easy_perform(curl_handle);
    if(CURLE_OK == res) {
        double val;
        res = curl_easy_getinfo(curl_handle, CURLINFO_TOTAL_TIME, &val);
        if(val > 0.0)
            time_total = val;
    }
    else
        writeLog(LOG_TYPE_GPING, "Error while fetching '" + url + "' : " + string(curl_easy_strerror(res)));
    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();
    return time_total;
}

int websitePing(nodeInfo *node, string url, string local_addr, int local_port, string user, string pass)
{
    double time_total = 0.0, retval = 0.0;
    string proxystr = buildSocks5ProxyString(local_addr, local_port, user, pass);
    writeLog(LOG_TYPE_GPING, "Website ping started. Test with proxy '" + proxystr + "'.");
    int loop_times = 0, times_to_ping = 3, succeedcounter = 0, failcounter = 0;
    while(loop_times < times_to_ping)
    {
        retval = getLoadPageTime(url, 5L, proxystr);
        if(retval > 0)
        {
            succeedcounter++;
            time_total += retval * 1000.0;
            node->rawSitePing[loop_times] = retval * 1000.0;
            writeLog(LOG_TYPE_GPING, "Accessing '" + url + "' - Success - interval=" + to_string(node->rawSitePing[loop_times]) + "ms");
        }
        else
        {
            failcounter++;
            node->rawSitePing[loop_times] = 0;
            writeLog(LOG_TYPE_GPING, "Accessing '" + url + "' - Fail - interval=0ms");
        }
        loop_times++;
        draw_progress_sping(loop_times - 1, node->rawSitePing);
        sleep(200);
    }
    cerr<<endl;
    if(succeedcounter)
    {
        char strtmp[16] = {};
        snprintf(strtmp, sizeof(strtmp), "%0.2f", time_total / succeedcounter * 1.0);
        node->sitePing = strtmp;
    }
    writeLog(LOG_TYPE_GPING, "Website ping completed. Leaving.");
    return 0;
}
