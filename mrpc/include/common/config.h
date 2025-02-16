//
// Created by baitianyu on 7/14/24.
//

#ifndef RPCFRAME_CONFIG_H
#define RPCFRAME_CONFIG_H

#include <map>
#include <tinyxml/tinyxml.h>
#include <memory>


namespace mrpc {

    class Config {
    public:

        explicit Config(const char* xmlfile);

        ~Config();

    public:
        static std::unique_ptr<Config>& GetGlobalConfig();

        static void SetGlobalConfig(const char* xmlfile);

    public:
        std::string m_log_level;
        std::string m_log_file_name;
        std::string m_log_file_path;
        int m_log_max_file_size {0};
        int m_log_sync_interval {0};   // 日志同步间隔，ms
        int m_port {0};
        int m_io_threads {0};

        std::unique_ptr<TiXmlDocument> m_xml_document{nullptr};
    };


}

#endif //RPCFRAME_CONFIG_H
