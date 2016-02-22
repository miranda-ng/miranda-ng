#ifndef _LUA_METATABLE_H_
#define _LUA_METATABLE_H_

#include <functional>

#define LFUNC(T, L) std::function<void*(T*)>(L)

#define LUA_TINTEGER LUA_NUMTAGS + 1
#define LUA_TSTRINGA LUA_NUMTAGS + 2
#define LUA_TSTRINGW LUA_NUMTAGS + 3

template<typename T>
class MTField : public MZeroedObject
{
private:
	enum MTFieldGetter
	{
		MTFG_LAMBDA = 1,
		MTFG_OFFSET = 2
	};

	size_t offset;
	int type;
	int getType;
	std::function<void*(T*)> lambda;

public:
	MTField(size_t offset, int type)
		: offset(offset), type(type), getType(MTFG_OFFSET) { }

	MTField(std::function<void*(T*)> f, int type)
		: lambda(f), type(type), getType(MTFG_LAMBDA) { }

	int GetType() const { return type; }

	template<typename R>
	R GetValue(T *obj) const
	{
		if (getType == MTFG_LAMBDA) return (R)lambda(obj);
		else
		{
			R res = NULL;
			size_t size = sizeof(R);
			memcpy(&res, ((char*)obj) + offset, size);
			return res;
		}
	}
};

template<typename T>
class MT
{
private:
	lua_State *L;

	static const char *name;
	static std::map<std::string, MTField<T>*> fields;

	static void Init(lua_State *L, T **obj)
	{
		luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
		T *udata = (T*)lua_touserdata(L, 1);
		memcpy(*obj, udata, sizeof(T));
	}
	
	static void Free(T **obj)
	{
		*obj = NULL;
	}

	static int lua_new(lua_State *L)
	{
		T *udata = (T*)lua_newuserdata(L, sizeof(T));
		memset(udata, 0, sizeof(T));
		Init(L, &udata);
		if (udata == NULL)
		{
			lua_pushnil(L);
			return 1;
		}
		luaL_setmetatable(L, MT::name);

		return 1;
	}

	static int lua__index(lua_State *L)
	{
		T *obj = (T*)luaL_checkudata(L, 1, MT::name);
		const char *key = lua_tostring(L, 2);

		auto it = fields.find(key);
		if (it == fields.end())
		{
			lua_pushnil(L);
			return 1;
		}

		MTField<T> *field = it->second;
		
		switch (field->GetType())
		{
		case LUA_TBOOLEAN:
			lua_pushboolean(L, field->GetValue<BOOL>(obj));
			break;
		case LUA_TINTEGER:
			lua_pushinteger(L, field->GetValue<long long>(obj));
			break;
		case LUA_TNUMBER:
			lua_pushnumber(L, field->GetValue<intptr_t>(obj));
			break;
		case LUA_TSTRING:
			lua_pushstring(L, field->GetValue<char*>(obj));
			break;
		case LUA_TSTRINGA:
			lua_pushstring(L, ptrA(mir_utf8encode(field->GetValue<char*>(obj))));
			break;
		case LUA_TSTRINGW:
			lua_pushstring(L, T2Utf(field->GetValue<wchar_t*>(obj)));
			break;
		case LUA_TLIGHTUSERDATA:
			lua_pushlightuserdata(L, field->GetValue<void*>(obj));
			break;
		default:
			lua_pushnil(L);
		}

		return 1;
	}

	static int lua__gc(lua_State *L)
	{
		T *obj = (T*)luaL_checkudata(L, 1, MT::name);
		MT::Free(&obj);
		return 0;
	}

public:
	MT(lua_State *L, const char *tname) : L(L)
	{
		MT::name = tname;

		lua_register(L, MT::name, lua_new);

		luaL_newmetatable(L, MT::name);
		lua_pushcfunction(L, lua__index);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, lua__gc);
		lua_setfield(L, -2, "__gc");
	}

	template<typename R>
	MT& Field(R T::*M, const char *name, int type)
	{
		size_t offset = offsetof(T, *M);
		if (type != LUA_TNONE)
			fields[name] = new MTField<T>(offset, type);
		return *this;
	}

	template<typename L>
	MT& Field(const L &f, const char *name, int type)
	{
		if (type != LUA_TNONE)
			fields[name] = new MTField<T>(f, type);
		return *this;
	}

	MT& Method(lua_CFunction func, const char *name)
	{
		luaL_getmetatable(L, MT::name);
		lua_pushcfunction(L, func);
		lua_setfield(L, -2, name);
		lua_pop(L, 1);
		return *this;
	}

	static void Set(lua_State *L, T *obj)
	{
		if (obj == NULL)
		{
			lua_pushnil(L);
			return;
		}

		T *udata = (T*)lua_newuserdata(L, sizeof(T));
		memcpy(udata, obj, sizeof(T));
		luaL_setmetatable(L, MT::name);
	}
};

template<typename T>
const char *MT<T>::name;

template<typename T>
std::map<std::string, MTField<T>*> MT<T>::fields;

#endif //_LUA_METATABLE_H_
