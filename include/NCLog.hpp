#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

enum nc_runtime_alert_severity{
    ncras_Point_Of_Note,            //Something happened that may be of note to debug
    ncras_Badly_Formed_Function,    //A trigger in a function when it wasn't used correctly
    ncras_Severe_Error              //Everything has gone horribly, horribly wrong
};
const std::string alertToStr[] = {"  Point of Note  ", "Badly formed func", "  Severe Error   "};

class NC_Runtime_Log{
public:

    void addLog(nc_runtime_alert_severity severity, unsigned lineNum, const std::string& message){
        if(listeningTo.count(severity) > 0)
            log.emplace_back(severity, lineNum, message);
    }

    void printLog(){
        printf(">Node Call Runtime Log>\n");
        for(auto& l : log)
            l.out();
        log.clear();
    }

private:
    struct Log_Message{
        std::string message;
        nc_runtime_alert_severity severity;
        unsigned lineNumber;

        void out(){
            std::string slineNum = std::to_string(lineNumber);
            while(slineNum.size() < 4)
                slineNum = slineNum + " ";
            printf("\t| On line %s | %s |> %s\n", slineNum.c_str(), alertToStr[severity].c_str(), message.c_str());
        }

        Log_Message(nc_runtime_alert_severity severity, unsigned lineNum, const std::string& message):
            severity(severity),
            lineNumber(lineNum),
            message(message)
        {}
    };

    std::unordered_set<nc_runtime_alert_severity> listeningTo;
    std::vector<Log_Message> log;
};
