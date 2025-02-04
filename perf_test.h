#ifndef PERF_TEST_H_INCLUDED
#define PERF_TEST_H_INCLUDED
#include "socket.h"
#include "misc.h"

struct nodePerfInfo
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
    string pkLoss = "100.00%";
    int rawPing[6] = {};
    string avgPing = "0.00";
    int rawSitePing[3] = {};
    string sitePing = "0.00";
    int rawTelegramPing[3] = {};
    string telegramPing = "0.00";
    int rawCloudflarePing[3] = {};
    string cloudflarePing = "0.00";
    geoIPInfo inboundGeoIP;
    geoIPInfo outboundGeoIP;
};

void testTelegram(string localaddr, int localport, nodePerfInfo *node);
void testCloudflare(string localaddr, int localport, nodePerfInfo *node);

#endif // PERF_TEST_H_INCLUDED
