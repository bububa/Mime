#ifndef CONFIG_H_AEQAPF61
#define CONFIG_H_AEQAPF61

#include <string>

namespace mime
{
    typedef enum {
        DEBUG_OFF = 0,
        DEBUG_NORMAL,
        DEBUG_WARNING,
        DEBUG_ERROR,
        DEBUG_CRITICAL,
    } MimeDebugLevel;
    
    static const std::string MIME_VERSION("1.0");
    static const std::string DATA_PATH("/Users/syd/Documents/Works/codes/Mime/data/");
    static const std::string MONGODB_HOST("localhost:27017");
    static const std::string LOG_HOST("localhost:27017");
    static const std::string DICT_PATH("/Users/syd/Documents/Works/codes/Mime/data/dict/");
    static const int DEBUG_LEVEL = mime::DEBUG_NORMAL;
}

#endif /* end of include guard: CONFIG_H_AEQAPF61 */
