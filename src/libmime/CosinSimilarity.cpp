#include "CosinSimilarity.h"
#include "Utils.h"
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/threadpool.hpp>
#include <boost/utility/result_of.hpp>
#include <boost/shared_ptr.hpp>


template<typename Thp, typename Func>
boost::shared_future< typename boost::result_of<Func()>::type >
submit_job(Thp& thp, Func f)
{
  typedef typename boost::result_of<Func()>::type result;
  typedef boost::packaged_task<result> packaged_task;
  typedef boost::shared_ptr<boost::packaged_task<result> > packaged_task_ptr;
 
  packaged_task_ptr task(new packaged_task(f));
  boost::shared_future<result> res(task->get_future());
  boost::threadpool::schedule(thp, boost::bind(&packaged_task::operator(), task));
  return res;
}

mime::CosinSimilarity::CosinSimilarity():maxThreads(COSINSIMILARITY_MAX_THREADS)
{
    mongoDB = mime::Mongo();
}

mime::CosinSimilarity::~CosinSimilarity()
{
    
}


void 
mime::CosinSimilarity::TF(const char *ns, const char *key)
{
    std::string tns(ns);
    std::string tkey(key);
    TF(tns, tkey);
}

void 
mime::CosinSimilarity::TF(std::string &ns, std::string &key)
{
    mongo::BSONObj emptyObj;
    mongo::DBClientConnection mongoConnection;
    mongoDB.connect(&mongoConnection);
    std::auto_ptr<mongo::DBClientCursor> cursor = mongoConnection.query(ns, emptyObj );
    unsigned int workerCount = 0;
    boost::threadpool::pool tp(maxThreads);
    boost::shared_future < std::map<std::string, int> > res[maxThreads];
    std::vector<mongo::BSONElement> oids;
    while( cursor->more() )
    {
        mongo::BSONObj b = cursor->next();
        if (!b.hasField(key.c_str())) continue;
        std::string rawContent = b.getStringField(key.c_str());
        if (rawContent.empty()) continue;
        std::string content = mime::Utils::stripTags(rawContent);
        if (content.empty()) continue;
        mongo::BSONElement e;
        b.getObjectID(e);
        oids.push_back(e);
        res[workerCount] = submit_job(tp, boost::bind(&mime::CosinSimilarity::CalculateTF, this, ns, content));
        workerCount ++;
        if (workerCount > maxThreads)
        {
            workerCount = 0;
            tp.wait();
            unsigned int i = 0;
            for(std::vector<mongo::BSONElement>::iterator it=oids.begin();it!=oids.end();++it)
            {
                std::map<std::string, int> tokens = res[i].get();
                i ++;
                if (tokens.empty()) continue;
                mongo::BSONElement e = *it;
                if (e.type() == mongo::jstOID)
                {
                    mongoDB.updateTF(mongoConnection, ns, e.__oid(), tokens);
                }else{
                    std::string oid=e.str();
                    mongoDB.updateTF(mongoConnection, ns, oid, tokens);
                }

            }
            oids.clear();
        }
    }
    if (!oids.empty())
    {
        unsigned int i = 0;
        for(std::vector<mongo::BSONElement>::iterator it=oids.begin();it!=oids.end();++it)
        {
            std::map<std::string, int> tokens = res[i].get();
            i ++;
            if (tokens.empty()) continue;
            mongo::BSONElement e = *it;
            if (e.type() == mongo::jstOID)
            {
                mongoDB.updateTF(mongoConnection, ns, e.__oid(), tokens);
            }else{
                std::string oid=e.str();
                mongoDB.updateTF(mongoConnection, ns, oid, tokens);
            }

        }
    }
}

std::map<std::string, int> 
mime::CosinSimilarity::CalculateTF(std::string &ns, std::string &content)
{
    std::map<std::string, int> tokens;
    std::vector<std::string> res = mime::Utils::segment(content, mgr);
    if (res.empty()) return tokens;
    for(std::vector<std::string>::iterator it=res.begin();it!=res.end();++it)
    {
        std::string tok = *it;
        tokens[tok] += 1;
    }
    return tokens;
}

void 
mime::CosinSimilarity::TF(mongo::DBClientConnection &c, const char *ns, const char *content, const char *oid)
{
    std::string tns(ns);
    std::string tcontent(content);
    std::string toid(oid);
    TF(c, tns, tcontent, toid);
}

void 
mime::CosinSimilarity::TF(mongo::DBClientConnection &c, std::string &ns, std::string &content, std::string &oid)
{
    std::map<std::string, int> tokens = CalculateTF(ns, content);
    if (tokens.empty()) return;
    mongoDB.updateTF(c, ns, oid, tokens);
}

void 
mime::CosinSimilarity::TF(mongo::DBClientConnection &c, const char *ns, const char *content, const mongo::OID &oid)
{
    std::string tns(ns);
    std::string tcontent(content);
    TF(c, tns, tcontent, oid);
}

void 
mime::CosinSimilarity::TF(mongo::DBClientConnection &c, std::string &ns, std::string &content, const mongo::OID &oid)
{
    std::map<std::string, int> tokens = CalculateTF(ns, content);
    if (tokens.empty()) return;
    mongoDB.updateTF(c, ns, oid, tokens);
}

void 
mime::CosinSimilarity::IDF(const char *ns, const char *output)
{
    std::string tns(ns);
    std::string toutput(output);
    IDF(tns, toutput);
}

void 
mime::CosinSimilarity::IDF(std::string &ns, std::string &output)
{
    mongo::DBClientConnection mongoConnection;
    mongoDB.connect(&mongoConnection);
    long long int D = mongoConnection.count(ns);
    std::string mapFuc = "function(){ if(!this.tf) return; this.tf.forEach( function(t){ emit(t.w, {count:1}); } ); }";
    std::string reduceFuc = "function(key , values) { var total = 0; for ( var i=0; i<values.length; i++ ){ total += values[i].count; } var idf=Math.log(" + boost::lexical_cast<std::string>(D) + "/total)/Math.log(10); return {count:total, idf:idf}; }";
    mongoConnection.mapreduce(ns, mapFuc, reduceFuc, mongo::BSONObj(), output);
}

void 
mime::CosinSimilarity::TFIDF(const char *ns, const char *tokenNs)
{
    std::string tns(ns);
    std::string tTokenNs(tokenNs);
    TFIDF(tns, tTokenNs);
}

void 
mime::CosinSimilarity::TFIDF(std::string &ns, std::string &tokenNs)
{
    mongo::BSONObj emptyObj;
    mongo::DBClientConnection mongoConnection;
    mongoDB.connect(&mongoConnection);
    std::auto_ptr<mongo::DBClientCursor> cursor = mongoConnection.query(ns, emptyObj );
    while( cursor->more() )
    {
        mongo::BSONObj b = cursor->next();
        if (!b.hasField("tf")) continue;
        mongo::BSONElement e;
        b.getObjectID(e);
        mongo::BSONObjIterator it (b["tf"].embeddedObject());
        mongo::BSONArrayBuilder arrb;
        std::map<std::string, std::pair<float, float> > tokens;
        while(it.more())
        {
            mongo::BSONElement e = it.next();
            mongo::BSONObj tf = e.embeddedObject();
            std::string word = tf.getStringField("w");
            float freq = tf["f"].number();
            std::pair<float, float> value(freq, 0.0);
            tokens.insert(std::pair<std::string, std::pair<float, float> >(word, value));
            arrb << word;
        }
        std::auto_ptr<mongo::DBClientCursor> cursor_k = mongoConnection.query(tokenNs, BSON("_id" << BSON("$in" << arrb.arr() ) ));
        while(cursor_k->more())
        {
            mongo::BSONObj b = cursor_k->next();
            std::string word = b.getStringField("_id");
            float idf = b.getFieldDotted("value.idf").number();
            float tf = tokens[word].first;
            tokens[word] = std::pair<float, float>(tf, tf*idf);
        }
        if (e.type() == mongo::jstOID)
        {
            mongoDB.updateTFIDF(mongoConnection, ns, e.__oid(), tokens);
        }else{
            std::string oid=e.str();
            mongoDB.updateTFIDF(mongoConnection, ns, oid, tokens);
        }
    }
}

void 
mime::CosinSimilarity::slopeOne(const char *ns, const char *output)
{
    std::string tns(ns);
    std::string toutput(output);
    slopeOne(tns, toutput);
}

void 
mime::CosinSimilarity::slopeOne(std::string &ns, std::string &output)
{
    std::string mapFuc("function(){ if(!this.tf) return; var tokens=this.tf; tokens.forEach(function(a){ tokens.forEach(function(b){ if (a.w==b.w) return; emit(a.w+'|||'+b.w, {times:1, rating:a.idf-b.idf}) }); }); }");
    std::string reduceFuc("function(key , values){ var total = 0; var rating=0; for ( var i=0; i<values.length; i++ ){ total+=values[i].times; rating+=values[i].rating; } var keys=key.split('|||');return {a:keys[0], b:keys[1], times:total, rating:rating} }");
    mongo::DBClientConnection mongoConnection;
    mongoDB.connect(&mongoConnection);
    mongoConnection.mapreduce(ns, mapFuc, reduceFuc, mongo::BSONObj(), output);
}

std::map<std::string, float> 
mime::CosinSimilarity::getRecommendById(const char *ns, const char *id, unsigned int limit, const char *output)
{
    std::string tns(ns);
    std::string tid(id);
    std::string toutput(output);
    return getRecommendById(tns, tid, limit, toutput);
}

std::map<std::string, float> 
mime::CosinSimilarity::getRecommendById(std::string &ns, std::string &id, unsigned int limit, const std::string &output)
{
    std::string mapFuc("function(){ emit(this.value.b, this.value.rating/this.value.times); }");
    std::string reduceFuc("function(key, values){ var total=0; for ( var i=0; i<values.length; i++ ){ total+=values[i]; } return total; }");
    mongo::DBClientConnection mongoConnection;
    mongoDB.connect(&mongoConnection);
    mongo::BSONObj retval = mongoConnection.mapreduce(ns, mapFuc, reduceFuc, BSON("value.a"<<id), output);
    std::string tmp_db = mongoConnection.nsGetDB(ns) + "." + retval.getStringField("result");
    std::auto_ptr<mongo::DBClientCursor> cursor = mongoConnection.query(tmp_db, mongo::Query().sort(BSON("value"<<1)), limit);
    std::map<std::string, float> tokens;
    while( cursor->more() )
    {
        mongo::BSONObj b = cursor->next();
        tokens.insert(std::pair<std::string, float> (b.getStringField("_id"), b["value"].number()));
    }
    return tokens;
}

std::map<std::string, float> 
mime::CosinSimilarity::getRecommendByEntry(const char *ns, const char *slopeonens, const char *entry, unsigned int limit, const char *output)
{
    std::string tns(ns);
    std::string tslopeonens(slopeonens);
    std::string tentry(entry);
    std::string toutput(output);
    return getRecommendByEntry(tns, tslopeonens, tentry, limit, toutput);
}

std::map<std::string, float> 
mime::CosinSimilarity::getRecommendByEntry(std::string &ns, std::string &slopeonens, std::string &entry, unsigned int limit, const std::string &output)
{
    mongo::DBClientConnection mongoConnection;
    mongoDB.connect(&mongoConnection);
    mongo::BSONArrayBuilder arrb;
    std::auto_ptr<mongo::DBClientCursor> cursor_ns = mongoConnection.query(ns, BSON("_id"<<entry));
    while( cursor_ns->more() )
    {
        mongo::BSONObj b = cursor_ns->next();
        if (!b.hasField("tf")) continue;
        mongo::BSONObj words = b.getObjectField("tf");
        mongo::BSONObjIterator it (words);
        while(it.more())
        {
            mongo::BSONObj e = it.next().embeddedObject();
            arrb << e.getStringField("w");
        }
    }
    std::string mapFuc = "function(){ var w=this.value.b; var entry=db." + ns + ".findOne({_id:'" + entry + "', 'tf.w':w}); if(!entry) continue; var rating=0; entry.tf.forEach(function(e){ if (e.w==w) rating=e.idf; });emit(this.value.b, {rating:rating*this.value.times-this.value.rating, times:this.value.times});  }";
    std::string reduceFuc("(key, values){ var rating=0; var times=0; for ( var i=0; i<values.length; i++ ){ rating+=values[i].rating; times+=value[i].times;} return {rating:rating, times:times, rank:(float)rating/times}; }");
    mongo::BSONObj retval = mongoConnection.mapreduce(slopeonens, mapFuc, reduceFuc, BSON("_id"<<BSON("$in" <<arrb.arr())), output);
    std::string tmp_db = mongoConnection.nsGetDB(ns) + "." + retval.getStringField("result");
    std::auto_ptr<mongo::DBClientCursor> cursor = mongoConnection.query(tmp_db, mongo::Query().sort(BSON("value.rank"<<-1)), limit);
    std::map<std::string, float> tokens;
    while( cursor->more() )
    {
        mongo::BSONObj b = cursor->next();
        tokens.insert(std::pair<std::string, float> (b.getStringField("_id"), b["value.rank"].number()));
    }
    return tokens;
}

void 
mime::CosinSimilarity::set_mgr(css::SegmenterManager* manager)
{
    mgr = manager;
}

void 
mime::CosinSimilarity::set_mongodb(mime::Mongo &mongodb)
{
    mongoDB = mongodb;
}

void 
mime::CosinSimilarity::set_max_threads(int max_threads)
{
    maxThreads = max_threads;
}