#include "parsexml.h"
#include "libxml/tree.h"
#include "libxml/parser.h"
#include "utility.h"

ParseXML::ParseXML()
{
}


void PrintNameContent(xmlDocPtr doc,xmlNodePtr cur)
{
    xmlChar *key;

    if (cur != NULL)
    {
        cur = cur->xmlChildrenNode;

        while (cur != NULL)
        {

            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if(key != NULL)
            {
                printf("%s:%s\n",cur->name,key);
                if ((!xmlStrcmp(cur->name, (const xmlChar *)"focus")))
                {
                    LogDebug("find it:%s", key);
                }

                xmlFree(key);
            }
            PrintNameContent(doc,cur);
            cur = cur->next;
        }
    }
    else
    {
        fprintf(stderr, "ERROR: Null Node!");
    }
}


int ParseXML::parse(string xml)
{
    int ret = -1;
    xmlDocPtr doc = NULL;
    xmlNodePtr node = NULL;

    doc = xmlParseMemory(xml.c_str(), xml.length());

    if(NULL  == doc)
    {
       LogError("ParseXML::%s() error", __func__);
       return ret;
    }

    LogDebug("ParseXML::%s() Ok", __func__);

    node = xmlDocGetRootElement(doc);
    if (!node)
    {
        xmlFreeDoc(doc);
        return ret;
    }


    PrintNameContent(doc, node);

    ret = 0;
    return ret;
}
