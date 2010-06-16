#ifndef UTILS_H_X2AED43U
#define UTILS_H_X2AED43U

#include <string>
#include <vector>
#include <map>
#include <time.h>
#include <tidy/tidy.h>
#include <tidy/buffio.h>
#include <boost/tuple/tuple.hpp>
#include <mmseg/UnigramCorpusReader.h>
#include <mmseg/UnigramDict.h>
#include <mmseg/SynonymsDict.h>
#include <mmseg/ThesaurusDict.h>
#include <mmseg/SegmenterManager.h>
#include <mmseg/Segmenter.h>

namespace mime
{
    class Utils
    {
    public:
        static std::string sha1(std::string &request);
        static std::string tidy(std::string &request, const TidyOptionId outOptId = TidyXhtmlOut);
        static bool childNodeIsText( TidyNode tnod );
        static bool isSafeNode( TidyNode tnod );
        static ctmbstr nodeName(TidyNode tnod);
        static std::string htmlNodeText( TidyDoc tdoc, TidyNode tnod );
        static std::string stripTags(std::string &req);
        static std::vector<std::string> segment(std::string &req, css::SegmenterManager* mgr);
    };
}

#endif /* end of include guard: UTILS_H_X2AED43U */
