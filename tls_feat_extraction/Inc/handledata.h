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
    std::vector<SinglePacketInfo>* singlePacketInfoVector;
    std::vector<ProtocolInfo>* protocolInfoVector;
    std::vector<PacketsFeature>* packetsInfoVector;
};

#endif // HANDLEPACKETDATA_H
