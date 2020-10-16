#pragma once

#include <map>
#include <cstddef>
#include <functional>

#define LUA_TINTEGER LUA_NUMTAGS + 1
#define LUA_TSTRINGA LUA_NUMTAGS + 2
#define LUA_TSTRINGW LUA_NUMTAGS + 3

union MTValue
{
	void *userdata;
	int boolean;
	lua_Integer integer;
	lua_Number  number;
	const char *string;
	const char *stringA;
	const wchar_t *stringW;
	lua_CFunction function;
};

class CMTField
{
	ptrA name;
	int type;

public:
	CMTField(const char *name, int type) :
		name(mir_strdup(name)), type(type) {}

	virtual ~CMTField() {}

	const char* GetName() const { return name; }
	int GetType() const { return type; }

	static int Compare(const CMTField *p1, const CMTField *p2) {
		return mir_strcmp(p1->name, p2->name);
	}

	virtual MTValue GetValue(void *obj) = 0;
};

template <typename R>
class CMTFieldOffset : public CMTField
{
	ptrdiff_t offset;
	size_t size;

public:
	CMTFieldOffset(const char *name, int type, ptrdiff_t offset, size_t size)
		: CMTField(name, type), offset(offset), size(size) {}
	
	virtual MTValue GetValue(void *obj)
	{
		MTValue value = {};
		memcpy(&value, ((char*)obj + offset), sizeof(R));
		return value;
	}
};

class CMTFieldFunction : public CMTField
{
	lua_CFunction function;
public:

	CMTFieldFunction(const char *name, lua_CFunction function) :
		CMTField(name, LUA_TFUNCTION), function(function) {}

	virtual MTValue GetValue(void*)
	{
		MTValue value = {};
		value.function = function;
		return value;
	}
};

template <typename T>
class CMTFieldLambda : public CMTField
{
	int type;
	std::function<MTValue(T*)> lambda;
public:

	CMTFieldLambda(const char *name, int type, decltype(lambda) lambda)
		: CMTField(name, type), lambda(lambda), type(type) {}

	virtual MTValue GetValue(void *obj) {
		return lambda((Obj*)obj);
	}
};

template<typename T>
class MT
{
private:
	lua_State *L;

	static const char *Name;
	static const luaL_Reg Events[];
	static OBJLIST<CMTField> Fields;

	static T* Init(lua_State *L)
	{
		luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
		return (T*)lua_touserdata(L, 1);
	}

	static int Get(lua_State *L, T* /*obj*/)
	{
		lua_pushnil(L);
		return 1;
	}

	static bool Set(lua_State* /*L*/, T* /*obj*/)
	{
		return false;
	}

	static void Free(lua_State* /*L*/, T **obj)
	{
		*obj = nullptr;
	}

	static int lua__new(lua_State *L)
	{
		T **udata = (T**)lua_newuserdata(L, sizeof(T*));
		
		*udata = Init(L);
		if (*udata == nullptr) {
			lua_pushnil(L);
			return 1;
		}

		luaL_setmetatable(L, Name);

		return 1;
	}

	static int lua__gc(lua_State *L)
	{
		T **obj = (T**)luaL_checkudata(L, 1, Name);
		Free(L, obj);

		return 0;
	}

	static int lua__bnot(lua_State *L)
	{
		T *obj = *(T**)luaL_checkudata(L, 1, Name);
		lua_pushlightuserdata(L, obj);

		return 1;
	}

	static int lua__index(lua_State *L)
	{
		T *obj = *(T**)luaL_checkudata(L, 1, Name);
		const char *key = lua_tostring(L, 2);

		const void *tmp[2] = { nullptr, key };
		CMTField *field = Fields.find((CMTField*)&tmp);
		if (field == nullptr)
			return Get(L, obj);

		MTValue value = field->GetValue(obj);
		int type = field->GetType();
		switch (type) {
		case LUA_TBOOLEAN:
			lua_pushboolean(L, value.boolean);
			break;
		case LUA_TINTEGER:
			lua_pushinteger(L, value.integer);
			break;
		case LUA_TNUMBER:
			lua_pushnumber(L, value.number);
			break;
		case LUA_TSTRING:
			lua_pushstring(L, value.string);
			break;
		case LUA_TSTRINGA:
			lua_pushstring(L, ptrA(mir_utf8encode(value.stringA)));
			break;
		case LUA_TSTRINGW:
			lua_pushstring(L, T2Utf(value.stringW));
			break;
		case LUA_TLIGHTUSERDATA:
			lua_pushlightuserdata(L, value.userdata);
			break;
		case LUA_TFUNCTION:
			lua_pushcfunction(L, value.function);
			break;
		default:
			lua_pushnil(L);
		}

		return 1;
	}

	static int lua__newindex(lua_State *L)
	{
		T *obj = *(T**)luaL_checkudata(L, 1, Name);

		if (!Set(L, obj)) {
			const char *key = lua_tostring(L, 2);
			luaL_error(L, "attempt to index a %s value (%s is readonly)", Name, key);
		}

		return 0;
	}

	static int lua__tostring(lua_State *L)
	{
		T *obj = *(T**)luaL_checkudata(L, 1, Name);
		CMStringA data(Name);
		data += "(";

		for (auto &it : Fields) {
			data += it->GetName();
			data += "=";

			MTValue value = it->GetValue(obj);
			int type = it->GetType();
			switch (type) {
			case LUA_TNIL:
				data.Append("nil");
				break;
			case LUA_TBOOLEAN:
				data.Append(value.boolean == 0 ? "false" : "true");
				break;
			case LUA_TINTEGER:
				data.AppendFormat("%d", value.integer);
				break;
			case LUA_TNUMBER:
				data.AppendFormat("%f", value.number);
				break;
			case LUA_TSTRING:
				data.Append(value.string);
				break;
			case LUA_TSTRINGA:
				data.Append(ptrA(mir_utf8encode(value.stringA)));
				break;
			case LUA_TSTRINGW:
				data.Append(T2Utf(value.stringW));
				break;
			default:
				data.AppendFormat("%s(0x%p)", lua_typename(L, type), value.userdata);
				break;
			}
			data += ", ";
		}
		data += ")";
		lua_pushstring(L, data.c_str());
		return 1;
	}

	static int lua__call(lua_State *L)
	{
		luaL_checktype(L, 1, LUA_TTABLE);
		luaL_checkany(L, 2);

		int nres = lua_gettop(L) - 1;

		lua_getfield(L, 1, "new");
		for (int i = 1; i <= nres; i++)
			lua_pushvalue(L, i + 1);
		luaM_pcall(L, nres, 1);

		return 1;
	}

public:
	MT(lua_State *L, const char *name)
		: L(L)
	{
		Name = name;

		luaL_newmetatable(L, Name);
		luaL_setfuncs(L, Events, 0);
		lua_pop(L, 1);

		lua_createtable(L, 0, 1);
		lua_pushcfunction(L, lua__new);
		lua_setfield(L, -2, "new");

		lua_createtable(L, 0, 2);
		lua_pushcfunction(L, lua__call);
		lua_setfield(L, -2, "__call");
		lua_setmetatable(L, -2);
		lua_setglobal(L, Name);
	}

	template<typename R>
	MT& Field(R T:: *M, const char *name, int type, size_t size = 0)
	{
		if (size == 0)
			size = sizeof(M);
		size_t offset = (size_t)(&(((T*)0)->*M));
		if (type != LUA_TNONE)
			Fields.insert(new CMTFieldOffset<R>(name, type, offset, size));
		return *this;
	}

	template<typename L>
	MT& Field(const L &lambda, const char *name, int type)
	{
		if (type != LUA_TNONE)
			Fields.insert(new CMTFieldLambda<T>(name, type, lambda));
		return *this;
	}

	MT& Field(const lua_CFunction function, const char *name)
	{
		Fields.insert(new CMTFieldFunction(name, function));
		return *this;
	}

	static void Apply(lua_State *L, T *obj)
	{
		if (obj == nullptr) {
			lua_pushnil(L);
			return;
		}

		T **udata = (T**)lua_newuserdata(L, sizeof(T));
		*udata = obj;
		luaL_setmetatable(L, Name);
	}
};

template<typename T>
const char *MT<T>::Name;

template<typename T>
const luaL_Reg MT<T>::Events[] = {
	{ "__index", lua__index },
	{ "__newindex", lua__newindex },
	{ "__bnot", lua__bnot },
	{ "__tostring", lua__tostring },
	{ "__gc", lua__gc },

	{ NULL, NULL },
};

template<typename T>
OBJLIST<CMTField> MT<T>::Fields(5, &CMTField::Compare);
