#ifndef MONGO_H_XX15100Z
#define MONGO_H_XX15100Z

#include <mongo/client/dbclient.h>
#include <string>

#define MONGO_DB_NAME "mime"
#define MONGO_ENTRY_COLLECTIONS "mime.entry"

namespace mime
{
    
    class Mongo
    {
        std::string host;
    public:
        Mongo();
        ~Mongo();
        
        bool connect(mongo::DBClientConnection *c);
        bool connect(mongo::DBClientConnection *c, const char *host);
        bool connect(mongo::DBClientConnection *c, std::string &host);
        void updateTF(mongo::DBClientConnection &c, std::string &ns, std::string &oid, std::map<std::string, int> &tokens);
        void updateTF(mongo::DBClientConnection &c, std::string &ns, const mongo::OID &oid, std::map<std::string, int> &tokens);
        void updateTFIDF(mongo::DBClientConnection &c, std::string &ns, std::string &oid, std::map<std::string, std::pair<float, float> > &tokens);
        void updateTFIDF(mongo::DBClientConnection &c, std::string &ns, const mongo::OID &oid, std::map<std::string, std::pair<float, float> > &tokens);
    };
}

#endif /* end of include guard: MONGO_H_XX15100Z */
