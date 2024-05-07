#include <map>
#include "flow.h"

struct HandlePacketData {
    //std::ofstream* outputFile;
    std::map<SessionKey, TLSFingerprint*>* stats;
    std::map<FlowKey, Flow*>* flows;
    std::vector<HttpRequest>* WebRequestVector;
    std::vector<HttpResponse>* WebResponseVector;
    std::vector<FlowFeature>* flowFeatureVector;
    std::vector<SinglePacketInfo>* singlePacketInfoVector;
    std::vector<ProtocolInfo>* protocolInfoVector;
    std::vector<PacketsFeature>* packetsFeatureVector;
    FlowsFeature* flowsFeature; // 流间特征
    VideoStreamMetrics* videoMetrics;// 视频类
    DownloadMetrics* downloadMetrics;// 下载类
};

extern HandlePacketData data;
// extern std::string app_label;
extern std::map<FlowKey, std::string> flow2app;