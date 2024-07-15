//
// Created by baitianyu on 7/14/24.
//

#ifndef RPCFRAME_CONFIG_H
#define RPCFRAME_CONFIG_H

#include <map>


namespace rocket {

    class Config {
    public:

        Config(const char* xmlfile);

    public:
        static Config* GetGlobalConfig();
        static void SetGlobalConfig(const char* xmlfile);

    public:
        std::string m_log_level;

    };


}

#endif //RPCFRAME_CONFIG_H
