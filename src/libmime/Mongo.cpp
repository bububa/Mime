#include <json/json.h>
#include "Config.h"
#include "Mongo.h"
#include "Utils.h"

mime::Mongo::Mongo():host(mime::MONGODB_HOST)
{
    
}

mime::Mongo::~Mongo()
{
    
}

bool 
mime::Mongo::connect(mongo::DBClientConnection *c)
{
    return connect(c, host);
}

bool 
mime::Mongo::connect(mongo::DBClientConnection *c, const char *hostCStr)
{
    try{
        c->connect(hostCStr);
        return true;
    }catch( mongo::DBException &e ) {
        if (mime::DEBUG_LEVEL > mime::DEBUG_NORMAL)
        {
            std::cout << "Can't connect to mongodb!" << std::endl;
        }
    }
    return false;
}

bool 
mime::Mongo::connect(mongo::DBClientConnection *c, std::string &hostStr)
{
    return connect(c, hostStr.c_str());
}

void 
mime::Mongo::updateTF(mongo::DBClientConnection &c, std::string &ns, std::string &oid, std::map<std::string, int> &tokens)
{
    int total = tokens.size();
    if (total == 0) return;
    c.update(ns, BSON("_id"<<oid), BSON("$unset"<<BSON("tf"<<1)));
    mongo::BSONArrayBuilder arrb;
    for(std::map<std::string, int>::iterator it=tokens.begin();it!=tokens.end();++it) 
    {
        float tf = (float)it->second/total;
        arrb.append(BSON("w"<<it->first<<"f"<<tf));
    }
    c.update(ns, BSON("_id"<<oid), BSON("$set"<<BSON("tf"<<arrb.arr())));
}

void 
mime::Mongo::updateTF(mongo::DBClientConnection &c, std::string &ns, const mongo::OID &oid, std::map<std::string, int> &tokens)
{
    int total = tokens.size();
    if (total == 0) return;
    c.update(ns, BSON("_id"<<oid), BSON("$unset"<<BSON("tf"<<1)));
    mongo::BSONArrayBuilder arrb;
    for(std::map<std::string, int>::iterator it=tokens.begin();it!=tokens.end();++it) 
    {
        float tf = (float)it->second/total;
        arrb.append(BSON("w"<<it->first<<"f"<<tf));
    }
    c.update(ns, BSON("_id"<<oid), BSON("$set"<<BSON("tf"<<arrb.arr())));
}

void 
mime::Mongo::updateTFIDF(mongo::DBClientConnection &c, std::string &ns, std::string &oid, std::map<std::string, std::pair<float, float> > &tokens)
{
    c.update(ns, BSON("_id"<<oid), BSON("$unset"<<BSON("tf"<<1)));
    mongo::BSONArrayBuilder arrb;
    for(std::map<std::string, std::pair<float, float> >::iterator it=tokens.begin();it!=tokens.end();++it) 
    {
        std::pair<float, float> p = it->second;
        arrb.append(BSON("w"<<it->first<<"f"<<p.first<<"idf"<<p.second));
    }
    c.update(ns, BSON("_id"<<oid), BSON("$set"<<BSON("tf"<<arrb.arr())));
}

void 
mime::Mongo::updateTFIDF(mongo::DBClientConnection &c, std::string &ns, const mongo::OID &oid, std::map<std::string, std::pair<float, float> > &tokens)
{
    c.update(ns, BSON("_id"<<oid), BSON("$unset"<<BSON("tf"<<1)));
    mongo::BSONArrayBuilder arrb;
    for(std::map<std::string, std::pair<float, float> >::iterator it=tokens.begin();it!=tokens.end();++it) 
    {
        std::pair<float, float> p = it->second;
        arrb.append(BSON("w"<<it->first<<"f"<<p.first<<"idf"<<p.second));
    }
    c.update(ns, BSON("_id"<<oid), BSON("$set"<<BSON("tf"<<arrb.arr())));
}