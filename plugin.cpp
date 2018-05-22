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
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>

struct ASTNodeLocation
{
    int line;
    int column;
    int offset;
};

struct ASTNode
{
    std::string tag;
    boost::optional<ASTNodeLocation> location;
    std::vector<ASTNode> children;
};

ASTNodeLocation extractLocation(CStackMap *map)
{
    ASTNodeLocation ret;
    ret.line = map->getInt("line");
    ret.column = map->getInt("column");
    ret.offset = map->getInt("offset");
    return ret;
}

ASTNode extractNode(CStackObject *obj)
{
    ASTNode ret;
    if(obj->getObjectType() == STACK_ARRAY)
    {
        // sequence of nodes
        ret.tag = "sequence";
        CStackArray *arr = static_cast<CStackArray*>(obj);
        const std::vector<CStackObject*> *objs = arr->getObjects();
        for(CStackObject *obj : *objs)
        {
            ret.children.push_back(extractNode(obj));
        }
    }
    else if(obj->getObjectType() == STACK_MAP)
    {
        // single node
        CStackMap *map = static_cast<CStackMap*>(obj);
        ret.tag = map->contains("tag", STACK_STRING) ? map->getString("tag") : "sequence";
        ret.location = extractLocation(map->getMap("location"));
        for(const auto &item : *map->getKeyValuePairsKInt())
        {
            ret.children.push_back(extractNode(item.second));
        }
    }
    else if(obj->getObjectType() == STACK_STRING)
    {
        // literal
        CStackString *str = static_cast<CStackString*>(obj);
        ret.tag = str->getValue();
    }
    else
    {
        throw std::runtime_error((boost::format("bad object type: %d (%s)") % obj->getObjectType() % static_cast<CStackString*>(obj)->getValue()).str());
    }
    return ret;
}

ASTNode parseLuaCode(std::string code)
{
    int scriptHandleOrType = sim_scripttype_sandboxscript;

    simInt stackHandle = simCreateStack();
    if(stackHandle == -1)
        throw std::runtime_error("failed to create a stack");

    std::string req = "parser=require 'luacheck.parser'@";
    simInt ret0 = simExecuteScriptString(scriptHandleOrType, req.c_str(), stackHandle);
    if(ret0 == -1)
        throw std::runtime_error("failed to load luacheck.parser");

    std::string delim = "========================================================";
    code = (boost::format("parser.parse[%s[%s]%s]@") % delim % code % delim).str();

    simInt ret = simExecuteScriptString(scriptHandleOrType, code.c_str(), stackHandle);
    if(ret == -1)
        throw std::runtime_error("exec script error");
    simInt size = simGetStackSize(stackHandle);
    if(size == 0)
        throw std::runtime_error("empty result in stack");

    CStackObject *obj = CStackObject::buildItemFromTopStackPosition(stackHandle);
    ASTNode node = extractNode(obj);

    std::string clean = "parser=nil@";
    simInt ret1 = simExecuteScriptString(scriptHandleOrType, clean.c_str(), stackHandle);

    simReleaseStack(stackHandle);

    return node;
}

void dumpSyntaxTree(std::ostream &os, ASTNode &n, bool showLocation = false, std::string indent = "", std::string indent_elem = "    ")
{
    std::string indent0 = indent;
    std::string indent1 = indent + indent_elem;
    os <<            n.tag;
    if((showLocation && n.location) || n.children.size()>0)
        os << " {" << std::endl;
    if(showLocation && n.location)
    {
        os << indent1 << "location.line="   << n.location->line << std::endl;
        os << indent1 << "location.column=" << n.location->column << std::endl;
        os << indent1 << "location.offset=" << n.location->offset << std::endl;
    }
    for(int i = 0; i < n.children.size(); i++)
    {
        os << indent1;
        dumpSyntaxTree(os, n.children[i], showLocation, indent1);
    }
    if((showLocation && n.location) || n.children.size()>0)
        os << indent0 << "}";
    os << std::endl;
}

void parse(SScriptCallBack *p, const char *cmd, parse_in *in, parse_out *out)
{
    ASTNode ast = parseLuaCode(in->code);
    std::stringstream ss;
    dumpSyntaxTree(ss, ast);
    out->result = ss.str();
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
