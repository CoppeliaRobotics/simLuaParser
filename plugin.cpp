#include "simPlusPlus/Plugin.h"
#include "config.h"
#include "plugin.h"
#include "stubs.h"
#include "simLib.h"
#include "stack/stackObject.h"
#include "stack/stackNull.h"
#include "stack/stackBool.h"
#include "stack/stackNumber.h"
#include "stack/stackString.h"
#include "stack/stackArray.h"
#include "stack/stackMap.h"
#include "tinyxml2.h"
#include "tixml2ex.h"
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <boost/format.hpp>

using namespace tinyxml2;

class Plugin : public sim::Plugin
{
public:
    void onStart()
    {
        if(!registerScriptStuff())
            throw std::runtime_error("failed to register script stuff");

        setExtVersion("Lua Parser Plugin");
        setBuildDate(BUILD_DATE);
    }

    void extractNode(XMLDocument *doc, XMLElement *parent, CStackObject *obj)
    {
        if(obj->getObjectType() == STACK_ARRAY)
        {
            // sequence of nodes
            CStackArray *arr = obj->asArray();
            const std::vector<CStackObject*> *objs = arr->getObjects();
            XMLElement *e = doc->NewElement("group");
            for(CStackObject *obj : *objs)
            {
                extractNode(doc, e, obj);
            }
            parent->InsertEndChild(e);
        }
        else if(obj->getObjectType() == STACK_MAP)
        {
            // single node
            CStackMap *oMap = obj->asMap();
            std::string tag = oMap->getString("tag");
            if(tag == "") tag = "group";
            XMLElement *e = doc->NewElement(tag.c_str());

            CStackMap *oLoc = oMap->getMap("location");
            XMLElement *loc = doc->NewElement("location");
            loc->SetAttribute("line", oLoc->getInt("line"));
            loc->SetAttribute("column", oLoc->getInt("column"));
            loc->SetAttribute("offset", oLoc->getInt("offset"));
            e->InsertEndChild(loc);

            CStackMap *oELoc = oMap->getMap("end_location");
            if(oELoc)
            {
                XMLElement *eloc = doc->NewElement("end-location");
                eloc->SetAttribute("line", oELoc->getInt("line"));
                eloc->SetAttribute("column", oELoc->getInt("column"));
                eloc->SetAttribute("offset", oELoc->getInt("offset"));
                e->InsertEndChild(eloc);
            }

            for(const auto &item : *oMap->getKeyValuePairsKInt())
            {
                extractNode(doc, e, item.second);
            }
            parent->InsertEndChild(e);
        }
        else if(obj->getObjectType() == STACK_STRING)
        {
            // literal
            CStackString *str = obj->asString();
            parent->SetAttribute("value", str->getValue().c_str());
        }
        else
        {
            throw sim::exception("bad object type: %d (%s)", obj->getObjectType(), obj->toString());
        }
    }

    XMLDocument * parse(std::string code)
    {
        int stackHandle = simCreateStack();
        if(stackHandle == -1)
            throw std::runtime_error("failed to create a stack");

        std::string req = "require 'luacheck.parser'@";
        int ret0 = simExecuteScriptString(sim_scripttype_sandboxscript, req.c_str(), stackHandle);
        if(ret0 == -1)
            throw std::runtime_error("failed to load luacheck.parser");

        std::string delim = "========================================================";
        code = (boost::format("package.loaded['luacheck.parser'].parse[%s[%s]%s]@") % delim % code % delim).str();

        int ret = simExecuteScriptString(sim_scripttype_sandboxscript, code.c_str(), stackHandle);
        if(ret == -1)
        {
            CStackObject *obj = CStackObject::buildItemFromTopStackPosition(stackHandle);
            throw sim::exception("error: %s", obj->toString());
        }
        int size = simGetStackSize(stackHandle);
        if(size == 0)
            throw std::runtime_error("empty result in stack");

        CStackObject *obj = CStackObject::buildItemFromTopStackPosition(stackHandle);
        XMLDocument *doc = new XMLDocument;
        XMLElement *root = doc->NewElement("ast");
        extractNode(doc, root, obj);
        doc->InsertFirstChild(root);

        simReleaseStack(stackHandle);

        return doc;
    }

    std::string parseXML(std::string code)
    {
        XMLDocument *doc = parse(code);
        XMLPrinter printer;
        doc->Print(&printer);
        std::string result = printer.CStr();
        delete doc;
        return result;
    }

    void parse(parse_in *in, parse_out *out)
    {
        out->result = parseXML(in->code);
    }

    /*!
     * this implements the same transformation as the xslt-tests/lua-ast.xslt stylesheet
     */
    XMLDocument * getFunctionDefs(std::string code)
    {
        XMLDocument *ret = new XMLDocument;
        XMLElement *root = ret->NewElement("function-defs");
        ret->InsertEndChild(root);

        XMLDocument *doc = parse(code);
        for(auto e : selection(*doc, "/ast/group/Set"))
        {
            std::vector<XMLElement*> groups;
            for(const auto c : selection(e, "group"))
                groups.push_back(c);
            XMLElement *id = tixml2ex::find_element(groups[0], "Id");
            std::string signature = id->Attribute("value");
            std::string args;
            for(const auto id : selection(groups[1], "Function/group/Id"))
                args += std::string(args == "" ? "" : ", ") + id->Attribute("value");
            signature += "(" + args + ")";
            XMLElement *func = tixml2ex::find_element(groups[1], "Function");
            XMLElement *loc = find_element(func, "location");
            XMLElement *eloc = find_element(func, "end-location");
            if(!func || !loc || !eloc) continue;
            XMLElement *f = ret->NewElement("function-def");
            f->InsertEndChild(loc->ShallowClone(ret));
            f->InsertEndChild(eloc->ShallowClone(ret));
            f->SetAttribute("name", signature.c_str());
            root->InsertEndChild(f);
        }
        delete doc;

        return ret;
    }

    std::string getFunctionDefsXML(std::string code)
    {
        XMLDocument *doc = getFunctionDefs(code);
        XMLPrinter printer;
        doc->Print(&printer);
        std::string result = printer.CStr();
        delete doc;
        return result;
    }

    void getFunctionDefs(getFunctionDefs_in *in, getFunctionDefs_out *out)
    {
        out->result = getFunctionDefsXML(in->code);
    }
};

SIM_PLUGIN(PLUGIN_NAME, PLUGIN_VERSION, Plugin)
#include "stubsPlusPlus.cpp"
