#ifndef COSINSIMILARITY_H_QKNTOVN6
#define COSINSIMILARITY_H_QKNTOVN6

#include <string>
#include <vector>
#include <map>
#include <mongo/client/dbclient.h>
#include <mmseg/SegmenterManager.h>
#include "Mongo.h"

#define COSINSIMILARITY_MAX_THREADS 10

namespace mime
{
    
    class CosinSimilarity
    {
        css::SegmenterManager* mgr;
        mime::Mongo mongoDB;
        int maxThreads;
        std::map<std::string, int> CalculateTF(std::string &ns, std::string &content);
    public:
        CosinSimilarity();
        ~CosinSimilarity();
        
        void TF(const char *ns, const char *key);
        void TF(std::string &ns, std::string &key);
        void TF(mongo::DBClientConnection &c, const char *ns, const char *content, const char *oid);
        void TF(mongo::DBClientConnection &c, std::string &ns, std::string &content, std::string &oid);
        void TF(mongo::DBClientConnection &c, const char *ns, const char *content, const mongo::OID &oid);
        void TF(mongo::DBClientConnection &c, std::string &ns, std::string &content, const mongo::OID &oid);
        void IDF(const char *ns, const char *output);
        void IDF(std::string &ns, std::string &output);
        void TFIDF(const char *ns, const char *tokenNs);
        void TFIDF(std::string &ns, std::string &tokenNs);
        void slopeOne(const char *ns, const char *output);
        void slopeOne(std::string &ns, std::string &output);
        std::map<std::string, float> getRecommendById(const char *ns, const char *id, unsigned int limit, const char *output="");
        std::map<std::string, float> getRecommendById(std::string &ns, std::string &id, unsigned int limit, const std::string &output="");
        std::map<std::string, float> getRecommendByEntry(const char *ns, const char *slopeonens, const char *entry, unsigned int limit, const char *output="");
        std::map<std::string, float> getRecommendByEntry(std::string &ns, std::string &slopeonens, std::string &entry, unsigned int limit, const std::string &output="");
        void set_mgr(css::SegmenterManager* mgr);
        void set_mongodb(mime::Mongo &mongodb);
        void set_max_threads(int max_threads);
    };
}

#endif /* end of include guard: COSINSIMILARITY_H_QKNTOVN6 */
