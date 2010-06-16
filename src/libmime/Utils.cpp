#include "Utils.h"
#include <stdlib.h>
#include <stdio.h>

#include <botan/botan.h>
#include <botan/filters.h>
#include <boost/algorithm/string.hpp>
#include "Config.h"


// Caller supplied destination
char *substring (char *str, int start, int end) {
	char *s = (char*)malloc(end-start+1);
	int i, n = 0;
	for (i=start; i<end; i++) s[n++] = str[i];
	s[n++] = '\0';
	return s;
}

std::string
mime::Utils::sha1(std::string &request)
{
    Botan::Pipe pipe (new Botan::Hash_Filter("SHA-1"), new Botan::Hex_Encoder);
    pipe.process_msg(request);
    std::string res = pipe.read_all_as_string(0);
    return res;
}

std::string
mime::Utils::tidy(std::string &request, const TidyOptionId outOptId)
{
    if (request.empty()) return request;
    TidyBuffer output = {0};
    TidyBuffer errbuf = {0};
    int rc = -1;
    Bool ok;
    
    TidyDoc tdoc = tidyCreate();                     // Initialize "document"
    ok = tidyOptSetBool( tdoc, outOptId, yes );  // Convert to XHTML
    if (outOptId == TidyXmlOut)
        ok = tidyOptSetBool( tdoc, TidyXmlTags, yes );
    if ( ok )
        rc = tidySetErrorBuffer( tdoc, &errbuf );      // Capture diagnostics
    if (rc >= 0)
        rc = tidySetCharEncoding( tdoc, "utf8");
    if ( rc >= 0 )
        rc = tidyParseString( tdoc, request.c_str() );           // Parse the input
    if ( rc >= 0 )
        rc = tidyCleanAndRepair( tdoc );               // Tidy it up!
    if ( rc >= 0 )
        rc = tidyRunDiagnostics( tdoc );               // Kvetch
    if ( rc > 1 )                                    // If error, force output.
        rc = ( tidyOptSetBool(tdoc, TidyForceOutput, yes) ? rc : -1 );
    if ( rc >= 0 )
        rc = tidySaveBuffer( tdoc, &output );          // Pretty Print
    
    if ( rc >= 0 )
    {
        //if ( rc > 0 )
            //printf( "\nDiagnostics:\n\n%s", errbuf.bp );
        //printf( "\nAnd here is the result:\n\n%s", output.bp );
        std::string res(reinterpret_cast<const char *>(output.bp));
        return res;
    }else
        //printf( "A severe error (%d) occurred.\n", rc );
    
    tidyBufFree( &output );
    tidyBufFree( &errbuf );
    tidyRelease( tdoc );
    return request;
}

bool
mime::Utils::isSafeNode( TidyNode tnod )
{
    return !( tidyNodeIsHTML(tnod) || tidyNodeIsHEAD(tnod) || tidyNodeIsTITLE(tnod) || tidyNodeIsBASE(tnod) || tidyNodeIsMETA(tnod) || tidyNodeIsBODY(tnod) || tidyNodeIsFRAMESET(tnod) || tidyNodeIsFRAME(tnod) || tidyNodeIsIFRAME(tnod) || tidyNodeIsNOFRAMES(tnod) || tidyNodeIsLINK(tnod) || tidyNodeIsOPTION(tnod) || tidyNodeIsAREA(tnod) || tidyNodeIsNOBR(tnod) || tidyNodeIsSTYLE(tnod) || tidyNodeIsSCRIPT(tnod) || tidyNodeIsNOSCRIPT(tnod) || tidyNodeIsFORM(tnod) || tidyNodeIsTEXTAREA(tnod) || tidyNodeIsAPPLET(tnod) || tidyNodeIsOBJECT(tnod) || tidyNodeIsINPUT(tnod) || tidyNodeIsXMP(tnod) || tidyNodeIsSELECT(tnod) || tidyNodeIsEMBED(tnod) || tidyNodeIsMENU(tnod));
}

ctmbstr
mime::Utils::nodeName(TidyNode tnod)
{
    ctmbstr name;
    switch ( tidyNodeGetType(tnod) )
    {
        case TidyNode_Root:       name = "Root";                    break;
        case TidyNode_DocType:    name = "DOCTYPE";                 break;
        case TidyNode_Comment:    name = "Comment";                 break;
        case TidyNode_ProcIns:    name = "Processing Instruction";  break;
        case TidyNode_Text:       name = "Text";                    break;
        case TidyNode_CDATA:      name = "CDATA";                   break;
        case TidyNode_Section:    name = "XML Section";             break;
        case TidyNode_Asp:        name = "ASP";                     break;
        case TidyNode_Jste:       name = "JSTE";                    break;
        case TidyNode_Php:        name = "PHP";                     break;
        case TidyNode_XmlDecl:    name = "XML Declaration";         break;
        case TidyNode_Start:
        case TidyNode_End:
        case TidyNode_StartEnd:
        default:
            name = tidyNodeGetName( tnod );
            break;
    }
    return name;
}

std::string
mime::Utils::htmlNodeText( TidyDoc tdoc, TidyNode tnod )
{
    std::string response;
    TidyNode child;
    for ( child = tidyGetChild(tnod); child; child = tidyGetNext(child) )
    {
        ctmbstr name = mime::Utils::nodeName(child);
        //assert( name != NULL );
        if ( mime::Utils::isSafeNode(child) )
        {
            if (name=="Text")
            {
                TidyBuffer buf = {0};
                tidyNodeGetText (tdoc, child, &buf);
                std::string text(reinterpret_cast<const char *>(buf.bp));
                text.erase(std::remove(text.begin(), text.end(), '\n'), text.end());
                response += text;
                tidyBufFree( &buf );
            }
            response += mime::Utils::htmlNodeText( tdoc, child );
        }
    }
    return response;
}

std::string
mime::Utils::stripTags(std::string &req)
{
    if (req.empty()) return req;
    std::string html = mime::Utils::tidy(req);
    TidyDoc tdoc = tidyCreate();
    TidyBuffer errbuf = {0};
    tidySetErrorBuffer( tdoc, &errbuf );
    tidyBufFree( &errbuf );
    tidySetCharEncoding( tdoc, "utf8");
    tidyParseString( tdoc, html.c_str() );
    html = mime::Utils::htmlNodeText(tdoc, tidyGetBody(tdoc));
    boost::trim(html);
    return html;
}

std::vector<std::string> 
mime::Utils::segment(std::string &req, css::SegmenterManager* mgr)
{
    std::vector<std::string> response;
    int nRet = 0;
    nRet = mgr->init(mime::DICT_PATH.c_str());
    css::Segmenter* seg = mgr->getSegmenter();
    int length = req.length();
    char buffer[length];
    strcpy(buffer, req.c_str());
    buffer[length] = 0;
    //begin seg
    seg->setBuffer((u1*)buffer,length);
    u2 len = 0, symlen = 0;
    u2 kwlen = 0, kwsymlen = 0;
    //check 1st token.
    char txtHead[3] = {239,187,191};
    char* tok = (char*)seg->peekToken(len, symlen);
    seg->popToken(len);
    if(seg->isSentenceEnd())
    {
        do {
            char* kwtok = (char*)seg->peekToken(kwlen , kwsymlen,1);
            if(kwsymlen)
                printf("[kw]%*.*s/x ",kwsymlen,kwsymlen,kwtok);
        }while(kwsymlen);
    }
    
    if(len == 3 && memcmp(tok,txtHead,sizeof(char)*3) == 0)
    {
        //check is 0xFEFF
        //do nothing
    }else{
        //printf("%*.*s/x ",symlen,symlen,tok);
        char* substr;
        substr = substring(tok, 0, symlen); 
        if (*substr != ' ')
            response.push_back(std::string(substr));
        free(substr);
    }
    
    while(1)
    {
        len = 0;
        char* tok = (char*)seg->peekToken(len,symlen);
        if(!tok || !*tok || !len)
            break;
        seg->popToken(len);
        if(seg->isSentenceEnd())
        {
            do {
                char* kwtok = (char*)seg->peekToken(kwlen , kwsymlen,1);
                if(kwsymlen)
                    printf("[kw]%*.*s/x ",kwsymlen,kwsymlen,kwtok);
                }while(kwsymlen);
        }
        
        if(*tok == '\r')
            continue;
        if(*tok == '\n')
        {
            printf("\n");
            continue;
        }
        
        //printf("[%d]%*.*s/x ",len,len,len,tok);
        //printf("%*.*s/x ",symlen,symlen,tok);
        char* substr;
        substr = substring(tok, 0, symlen);
        if (*substr != ' ')
            response.push_back(std::string(substr));
        free(substr);
        //check thesaurus
        {
            const char* thesaurus_ptr = seg->thesaurus(tok, symlen);
            while(thesaurus_ptr && *thesaurus_ptr) {
                len = strlen(thesaurus_ptr);
                printf("%*.*s/s ",len,len,thesaurus_ptr);
                thesaurus_ptr += len + 1; //move next
            }
        }
        //printf("%s",tok);
    }
    return response;
}