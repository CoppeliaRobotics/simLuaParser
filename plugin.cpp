#include "v_repPlusPlus/Plugin.h"
#include "plugin.h"
#include "stubs.h"
#include "v_repLib.h"
#include "stack/stackObject.h"
#include "stack/stackNull.h"
#include "stack/stackBool.h"
#include "stack/stackNumber.h"
#include "stack/stackString.h"
#include "stack/stackArray.h"
#include "stack/stackMap.h"
#include "tinyxml2.h"
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>

using namespace tinyxml2;

void extractNode(XMLDocument *doc, XMLElement *parent, CStackObject *obj)
{
    if(obj->getObjectType() == STACK_ARRAY)
    {
        // sequence of nodes
        CStackArray *arr = static_cast<CStackArray*>(obj);
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
        CStackMap *oMap = static_cast<CStackMap*>(obj);
        XMLElement *e = doc->NewElement(oMap->getString("tag").c_str());

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
        CStackString *str = static_cast<CStackString*>(obj);
        parent->SetAttribute("value", str->getValue().c_str());
    }
    else
    {
        throw std::runtime_error((boost::format("bad object type: %d (%s)") % obj->getObjectType() % static_cast<CStackString*>(obj)->getValue()).str());
    }
}

void parse(SScriptCallBack *p, const char *cmd, parse_in *in, parse_out *out)
{
    simInt stackHandle = simCreateStack();
    if(stackHandle == -1)
        throw std::runtime_error("failed to create a stack");

    std::string req = "require 'luacheck.parser'@";
    simInt ret0 = simExecuteScriptString(sim_scripttype_sandboxscript, req.c_str(), stackHandle);
    if(ret0 == -1)
        throw std::runtime_error("failed to load luacheck.parser");

    std::string delim = "========================================================";
    std::string code = (boost::format("package.loaded['luacheck.parser'].parse[%s[%s]%s]@") % delim % in->code % delim).str();

    simInt ret = simExecuteScriptString(sim_scripttype_sandboxscript, code.c_str(), stackHandle);
    if(ret == -1)
    {
        CStackObject *obj = CStackObject::buildItemFromTopStackPosition(stackHandle);
        throw std::runtime_error((boost::format("error: %s") % obj->toString()).str());
    }
    simInt size = simGetStackSize(stackHandle);
    if(size == 0)
        throw std::runtime_error("empty result in stack");

    CStackObject *obj = CStackObject::buildItemFromTopStackPosition(stackHandle);
    XMLDocument *doc = new XMLDocument;
    XMLElement *root = doc->NewElement("ast");
    extractNode(doc, root, obj);
    doc->InsertFirstChild(root);

    XMLPrinter printer;
    doc->Print(&printer);
    out->result = printer.CStr();
    delete doc;

    simReleaseStack(stackHandle);
}

class Plugin : public vrep::Plugin
{
public:
    void onStart()
    {
        if(!registerScriptStuff())
            throw std::runtime_error("failed to register script stuff");
    }
};

VREP_PLUGIN(PLUGIN_NAME, PLUGIN_VERSION, Plugin)
