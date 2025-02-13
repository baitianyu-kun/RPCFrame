//
// Created by baitianyu on 7/14/24.
//
#include "common/config.h"

// 使用#把宏参数变为一个字符串，用##把两个宏参数贴合在一起。
#define READ_XML_NODE(name, parent) \
    TiXmlElement* name##_node = parent->FirstChildElement(#name); \
    if (!name##_node) { \
        printf("Start mrpc server error, failed to read node [%s]\n", #name); \
        exit(0); \
    } \

#define READ_STR_FROM_XML_NODE(name, parent) \
    TiXmlElement* name##_node = parent->FirstChildElement(#name); \
    if (!name##_node|| !name##_node->GetText()) { \
        printf("Start mrpc server error, failed to read config file %s\n", #name); \
        exit(0); \
    } \
    std::string name##_str = std::string(name##_node->GetText()); \

namespace mrpc {

    static std::unique_ptr<Config> g_config = nullptr;

    std::unique_ptr<Config> &Config::GetGlobalConfig() {
        return g_config;
    }

    void Config::SetGlobalConfig(const char *xmlfile) {
        if (g_config == nullptr) {
            g_config = std::make_unique<Config>(xmlfile);
        }
    }

    Config::Config(const char *xmlfile) {
        m_xml_document = std::make_unique<TiXmlDocument>();
        bool rt = m_xml_document->LoadFile(xmlfile);
        if (!rt) {
            printf("Start mrpc server error, failed to read config file %s, error info[%s] \n", xmlfile,
                   m_xml_document->ErrorDesc());
            exit(0);
        }
        READ_XML_NODE(root, m_xml_document);
        READ_XML_NODE(log, root_node);
        READ_XML_NODE(server, root_node);

        READ_STR_FROM_XML_NODE(log_level, log_node);
        READ_STR_FROM_XML_NODE(log_file_name, log_node);
        READ_STR_FROM_XML_NODE(log_file_path, log_node);
        READ_STR_FROM_XML_NODE(log_max_file_size, log_node);
        READ_STR_FROM_XML_NODE(log_sync_interval, log_node);

        m_log_level = log_level_str;
        m_log_file_name = log_file_name_str;
        m_log_file_path = log_file_path_str;
        // atoi()函数在转换时，如果遇到入参str不能转换或者str为空字符串时，返回值为0，不会抛出异常；
        // std::stoi()函数在转换时，如果入参str是字母或者空字符串而无法转换成数字时，
        // 会抛出std::invalid_argument异常，使用者必须手动处理异常，否则会造成程序crash；
        m_log_max_file_size = std::stoi(log_max_file_size_str);
        m_log_sync_interval = std::stoi(log_sync_interval_str);
        printf("LOG -- CONFIG LEVEL [%s], FILE_NAME [%s], FILE_PATH [%s], MAX_FILE_SIZE [%d Byte], SYNC_INTERVAL [%d ms]\n",
               m_log_level.c_str(), m_log_file_name.c_str(), m_log_file_path.c_str(), m_log_max_file_size,
               m_log_sync_interval);

        READ_STR_FROM_XML_NODE(port, server_node);
        READ_STR_FROM_XML_NODE(io_threads, server_node);

        m_port = std::stoi(port_str);
        m_io_threads = std::stoi(io_threads_str);
        printf("SERVER -- PORT [%d], IO THREADS [%d]\n", m_port, m_io_threads);

    }

    Config::~Config() {

    }

}