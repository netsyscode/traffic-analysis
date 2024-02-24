// handlepacketdata.h
#ifndef HANDLEPACKETDATA_H
#define HANDLEPACKETDATA_H

#include <fstream>
#include <map>
#include "flow.h"
// 其他需要的包含

struct HandlePacketData {
    std::ofstream* outputFile;
    std::map<SessionKey, TLSFingerprint*>* stats;
    std::map<FlowKey, Flow*>* flows;
    std::vector<HttpRequest>* WebRequest;
    std::vector<HttpResponse>* WebResponse;
    std::vector<PacketInfo>* packetInfoVector;
    std::vector<ProtocolInfo>* protocolInfoVector;
};

#endif // HANDLEPACKETDATA_H
