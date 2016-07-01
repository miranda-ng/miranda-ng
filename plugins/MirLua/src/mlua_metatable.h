#ifndef _LUA_METATABLE_H_
#define _LUA_METATABLE_H_

#include <cstddef>
#include <functional>

#define LUA_TINTEGER LUA_NUMTAGS + 1
#define LUA_TSTRINGA LUA_NUMTAGS + 2
#define LUA_TSTRINGW LUA_NUMTAGS + 3

union MTFieldVal
{
	void *userdata;
	int boolean;
	lua_Integer integer;
	lua_Number  number;
	const char *string;
	const char *stringA;
	const wchar_t *stringW;
};

struct MTField
{
	int lua_type;
	MTFieldVal val;
};


class CMTField
{
public:
	virtual MTField GetValue(void *obj) = 0;
	virtual ~CMTField(){};
};

template <typename Obj, typename Ret>
class CMTFieldOffset : public CMTField
{
	int lua_type;
	ptrdiff_t offset;
	size_t size;
public:
	CMTFieldOffset(ptrdiff_t off, size_t s, int type) : offset(off), lua_type(type), size(s) {}
	virtual MTField GetValue(void *obj)
	{
		MTField fd = { lua_type };
		//fd.val = *(Ret*)((char*)obj + offset);
		memcpy(&fd.val, ((char*)obj + offset), sizeof(Ret));
		return fd;
	}
};

template <typename Obj>
class CMTFieldLambda : public CMTField
{
	int lua_type;
	std::function<MTFieldVal(Obj*)> lambda;
public:

	CMTFieldLambda(decltype(lambda) f, int type) : lambda(f), lua_type(type) {}

	virtual MTField GetValue(void *obj)
	{
		MTField tmp = { lua_type };
		tmp.val = lambda((Obj*)obj);
		return tmp;
	}
};


template<typename T>
class MT
{
private:
	lua_State *L;

	static const char *name;
	static std::map<std::string, CMTField*> fields;

	static void Init(lua_State *L, T **obj)
	{
		luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
		*obj = (T*)lua_touserdata(L, 1);
	}

	static int Index(lua_State *L, T* /*obj*/)
	{
		lua_pushnil(L);
		return 1;
	}

	static void Free(lua_State* /*L*/, T **obj)
	{
		*obj = NULL;
	}

	static int lua__new(lua_State *L)
	{
		T **udata = (T**)lua_newuserdata(L, sizeof(T*));
		Init(L, udata);
		if (*udata == NULL)
		{
			lua_pushnil(L);
			return 1;
		}
		luaL_setmetatable(L, MT::name);

		return 1;
	}

	static int lua__call(lua_State *L)
	{
		int nargs = lua_gettop(L);
		lua_pushcfunction(L, lua__new);
		for (int i = 2; i <= nargs; i++)
			lua_pushvalue(L, i);
		luaM_pcall(L, nargs - 1, 1);

		return 1;
	}

	static int lua__bnot(lua_State *L)
	{
		T *obj = *(T**)luaL_checkudata(L, 1, MT::name);
		lua_pushlightuserdata(L, obj);

		return 1;
	}

	static int lua__index(lua_State *L)
	{
		T *obj = *(T**)luaL_checkudata(L, 1, MT::name);
		const char *key = lua_tostring(L, 2);

		auto it = fields.find(key);
		if (it == fields.end())
		{
			return Index(L, obj);
		}

		MTField fieldVal = it->second->GetValue(obj);
		
		switch (fieldVal.lua_type)
		{
		case LUA_TBOOLEAN:
			lua_pushboolean(L, fieldVal.val.boolean);
			break;
		case LUA_TINTEGER:
			lua_pushinteger(L, fieldVal.val.integer);
			break;
		case LUA_TNUMBER:
			lua_pushnumber(L, fieldVal.val.number);
			break;
		case LUA_TSTRING:
			lua_pushstring(L, fieldVal.val.string);
			break;
		case LUA_TSTRINGA:
			lua_pushstring(L, ptrA(mir_utf8encode(fieldVal.val.stringA)));
			break;
		case LUA_TSTRINGW:
			lua_pushstring(L, T2Utf(fieldVal.val.stringW));
			break;
		case LUA_TLIGHTUSERDATA:
			lua_pushlightuserdata(L, fieldVal.val.userdata);
			break;
		default:
			lua_pushnil(L);
		}

		return 1;
	}
	static int lua__gc(lua_State *L)
	{
		T **obj = (T**)luaL_checkudata(L, 1, MT::name);
		MT::Free(L, obj);

		return 0;
	}

	static int lua__tostring(lua_State *L)
	{
		T *obj = *(T**)luaL_checkudata(L, 1, MT::name);
		CMStringA data(MT::name);
		data += "(";


		for (auto it = fields.begin(); it != fields.end(); ++it)
		{
			data += it->first.c_str();
			data += "=";
			MTField fieldVal = it->second->GetValue(obj);

			switch (fieldVal.lua_type)
			{
			case LUA_TBOOLEAN:
				data.Append(fieldVal.val.boolean == 0 ? "false" : "true");
				break;
			case LUA_TINTEGER:
				data.AppendFormat("%d", fieldVal.val.integer);
				break;
			case LUA_TNUMBER:
				data.AppendFormat("%f", fieldVal.val.number);
				break;
			case LUA_TSTRING:
				data.Append(fieldVal.val.string);
				break;
			case LUA_TSTRINGA:
				data.Append(ptrA(mir_utf8encode(fieldVal.val.stringA)));
				break;
			case LUA_TSTRINGW:
				data.Append(T2Utf(fieldVal.val.stringW));
				break;
			case LUA_TLIGHTUSERDATA:
				data.AppendFormat("(0x%p)", fieldVal.val.userdata);
				break;
			default:
				data.Append("nil");
			}
			data += ", ";
		}
		data += ")";
		lua_pushstring(L, data.c_str());
		return 1;
	}

public:
	MT(lua_State *L, const char *tname) : L(L)
	{
		MT::name = tname;

		luaL_newmetatable(L, MT::name);
		lua_pushcfunction(L, lua__call);
		lua_setfield(L, -2, "__call");
		lua_pushcfunction(L, lua__index);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, lua__bnot);
		lua_setfield(L, -2, "__bnot");
		lua_pushcfunction(L, lua__gc);
		lua_setfield(L, -2, "__gc");
		lua_pushcfunction(L, lua__tostring);
		lua_setfield(L, -2, "__tostring");
		lua_pop(L, 1);

		lua_createtable(L, 0, 1);
		lua_pushcfunction(L, lua__new);
		lua_setfield(L, -2, "new");
		lua_pushvalue(L, -1);
		lua_setglobal(L, MT::name);
		luaL_setmetatable(L, MT::name);
		lua_pop(L, 1);
	}

	template<typename R>
	MT& Field(R T:: *M, const char *name, int type, size_t size = 0)
	{
		if (size == 0)
			size = sizeof(M);
		size_t offset = (size_t)(&(((T*)0)->*M));
		if (type != LUA_TNONE)
			fields[name] = new CMTFieldOffset<T, R>(offset, size, type);
		return *this;
	}

	template<typename L>
	MT& Field(const L &f, const char *name, int type)
	{
		if (type != LUA_TNONE)
			fields[name] = new CMTFieldLambda<T>(f, type);
		return *this;
	}

	static void Set(lua_State *L, T *obj)
	{
		if (obj == NULL)
		{
			lua_pushnil(L);
			return;
		}

		T **udata = (T**)lua_newuserdata(L, sizeof(T));
		*udata = obj;
		luaL_setmetatable(L, MT::name);
	}
};

template<typename T>
const char *MT<T>::name;

template<typename T>
std::map<std::string, CMTField*> MT<T>::fields;

#endif //_LUA_METATABLE_H_
