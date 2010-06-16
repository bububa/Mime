#include <botan/botan.h>
#include <iostream>
#include <string>
#include <vector>
#include <mmseg/SegmenterManager.h>
#include "mime/Config.h"
#include "mime/Utils.h"
#include "mime/CosinSimilarity.h"

int main()
{
    Botan::LibraryInitializer init;
    css::SegmenterManager* mgr = new css::SegmenterManager();
    int nRet = 0;
    nRet = mgr->init(mime::DICT_PATH.c_str());
    mime::CosinSimilarity cs;
    cs.set_mgr(mgr);
    //cs.TF("airport.feedentry", "desc");
    //cs.IDF("airport.feedentry", "feedentry_tokens");
    //cs.TFIDF("airport.feedentry", "airport.feedentry_tokens");
    //cs.slopeOne("airport.feedentry", "feedentry_slopeone");
    std::map<std::string, float> rec = cs.getRecommendById("airport.feedentry_slopeone", "电子书", 20);
    for(std::map<std::string, float>::iterator it=rec.begin();it!=rec.end(); ++it)
    {
        std::cout << it->first << ": "<<it->second << std::endl;
    }
    delete mgr;
}