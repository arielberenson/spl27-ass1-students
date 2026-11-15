#include <fstream>
#include <stdexcept>
#include "json.hpp"
#include "Server.h"
#include "SchedulingPolicy.h"
#include "Model.h"

using namespace std;
using json = nlohmann::json;

Server::Server(string configpath) : waitingRequests(), activeRequests(), nonSubmittedRequests(), currCycle(0) {
    Config config = readConfig(configpath);

    if (config.policy == "arrivalOrder")
        config = new ArrivalOrderPolicy();
    else if (config.policy == "shortestJob")
        config = new ShortestJobPolicy();
    else
        config = new LongestPrefixPolicy();

    for (const auto& gpuc : config.gpus){
        GPU gpu{
            gpuc.memoryBytes/config.blockSize,
            config.blockSize 
        };
        gpus.push_back(gpu);
    }
    for (const auto& modelc : config.models){
        Model* model = new Model{
            modelc.id, modelc.embeddingSize
        };
        models.push_back(model);
    }
    for (RequestConfig requestc : config.requests){
        Request* request = new Request{
            requestc.id, 
            //model,
            requestc.admissionCycle,
            requestc.prompt
        };
        nonSubmittedRequests.push_back(request);
    }
}

void Server::serve(){
    if (!step())
        //pring summary line
}

bool Server::step(){
    for (Request* request : nonSubmittedRequests){
        if (request->getAdmissionCycle() == currCycle)
            waitingRequests.push_back(request);
    }
    vector<const Request*> eligible(waitingRequests.begin(), waitingRequests.end());
    vector<int> order = policy->choose(eligible, gpus);
    for (int i=0; i<waitingRequests.size(); i++){
        bool admitted = admitRequest(waitingRequests[i]);
        
    }
    
}
bool Server::admitRequest(Request* request){
    
    std::vector<PrefixMatch> gpuPrefixes;
    for (size_t i=0; i<gpus.size();i++){
        //check if there are enough slots, need for new blocks and idle
        if (gpus[i].availableSlots() >= request->getBlocksForMemory(request->getNeededMemory(gpus[i])
        + gpus[i].)
        //if there are then push back the longest cache prefix for each gpu
            gpuPrefixes.push_back(gpus[i].findLongestCachedPrefix(
                request->getModel().getId()
                std::size_t embeddingSize,
                request->getCacheableTokens()
            ));
    }
    //sort the gpus by their longest cached prefix
    std::sort()
}
Config Server::readConfig(string configPath) const{
    std::ifstream input(configPath);
    json config;
    input >> config;
    // All the GPUs have the same block size.
    size_t blockSize = config["block_size"].get<size_t>();
    vector<GPUConfig> gpuConfigs;
    for(auto& gpu: config["gpus"]){
        size_t memoryBytes = gpu["memory_bytes"].get<size_t>();
        GPUConfig gpuConfig{memoryBytes};
        gpuConfigs.push_back(gpuConfig);
    }
    vector<ModelConfig> modelConfigs;
    for(auto& model: config["models"]){
        string id = model["id"].get<string>();
        size_t embeddingSize = model["embedding_size"].get<size_t>();
        ModelConfig modelConfig{id, embeddingSize};
        modelConfigs.push_back(modelConfig);
    }
    vector<RequestConfig> requestConfigs;
    for(auto& request: config["requests"]){
        size_t id = request["id"].get<size_t>();
        string typeStr = request["type"].get<string>();
        if(typeStr != "generative" && typeStr != "text_embedder"){
            throw invalid_argument("invalid request type");
        }
        RequestType type = typeStr=="generative"?RequestType::Generative:RequestType::TextEmbedder;
        string modelId = request["model"].get<string>();
        int admissionCycle = request["cycle"].get<int>();
        string prompt = request["prompt"].get<string>();
        string scriptedOutput = "";
        if (request.count("scripted_output") > 0) {
            scriptedOutput = request["scripted_output"].get<string>();
        }
        size_t maxOutputTokens = 0;
        if (request.count("max_output_tokens") > 0) {
            maxOutputTokens = request["max_output_tokens"].get<size_t>();
        }
        RequestConfig reqConfig{id, type, modelId, admissionCycle, prompt, scriptedOutput, maxOutputTokens};
        requestConfigs.push_back(reqConfig);
    }

    if (config.count("scheduling_policy") == 0) {
        throw invalid_argument("missing scheduling policy");
    }
    string policy = config["scheduling_policy"].get<string>();

    return Config{blockSize, gpuConfigs, modelConfigs, requestConfigs, policy};
}
