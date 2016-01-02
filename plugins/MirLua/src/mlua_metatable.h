#ifndef _LUA_METATABLE_H_
#define _LUA_METATABLE_H_

#define LUA_TINTEGER LUA_NUMTAGS + 1
#define LUA_TSTRINGA LUA_NUMTAGS + 2
#define LUA_TSTRINGW LUA_NUMTAGS + 3

struct MTField
{
	size_t offset;
	size_t size;
	int type;

	MTField(size_t offset, size_t size, int type)
		: offset(offset), size(size), type(type) { }
};

template<typename T>
class MT
{
private:
	lua_State *L;

	static const char *name;
	static std::map<std::string, MTField*> fields;

	template<typename R>
	static R GetValue(const T *obj, size_t offset, size_t size)
	{
		R res = NULL;
		memcpy(&res, ((char*)obj) + offset, size);
		return res;
	}

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

	static int lua__new(lua_State *L)
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

		MTField *field = it->second;
		size_t offset = field->offset;
		size_t size = field->size;
		
		switch (field->type)
		{
		case LUA_TBOOLEAN:
			lua_pushboolean(L, GetValue<bool>(obj, offset, size));
			break;
		case LUA_TINTEGER:
			lua_pushinteger(L, GetValue<long long>(obj, offset, size));
			break;
		case LUA_TNUMBER:
			lua_pushnumber(L, GetValue<double>(obj, offset, size));
			break;
		case LUA_TSTRING:
			lua_pushstring(L, GetValue<char*>(obj, offset, size));
			break;
		case LUA_TSTRINGA:
			lua_pushstring(L, ptrA(mir_utf8encode(GetValue<char*>(obj, offset, size))));
			break;
		case LUA_TSTRINGW:
			lua_pushstring(L, ptrA(mir_utf8encodeW(GetValue<wchar_t*>(obj, offset, size))));
			break;
		case LUA_TLIGHTUSERDATA:
			lua_pushlightuserdata(L, GetValue<void*>(obj, offset, size));
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

		lua_pushcfunction(L, lua__new);
		lua_setglobal(L, MT::name);

		luaL_newmetatable(L, MT::name);
		lua_pushcfunction(L, lua__index);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, lua__gc);
		lua_setfield(L, -2, "__gc");
	}

	template<typename R>
	MT& Field(R T::*M, const char *name, int type, size_t size = sizeof(R))
	{
		size_t offset = reinterpret_cast<size_t>(&(((T*)0)->*M));
		if (type != LUA_TNONE)
			fields[name] = new MTField(offset, size, type);
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
std::map<std::string, MTField*> MT<T>::fields;

#endif //_LUA_METATABLE_H_
