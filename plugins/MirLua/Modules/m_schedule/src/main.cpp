#include "stdafx.h"

static mir_cs threadLock;
static HANDLE hScheduleEvent = NULL;
static HANDLE hScheduleThread = NULL;

#define STOP ((void *) -1)

#define SECOND	1
#define MINUTE	SECOND * 60
#define HOUR	MINUTE * 60
#define DAY		HOUR * 24
#define WEEK	DAY * 7

struct ScheduleTask
{
	time_t timestamp;
	time_t interval;
	bool repeat;

	lua_State *L;
	int threadRef;
	int callbackRef;

	ScheduleTask()
	{
		timestamp = time(NULL);
		interval = 0;
		repeat = false;
	}

	void Append(time_t value)
	{
		if (repeat)
			interval = value;
		timestamp += value;
	}
};

static LIST<ScheduleTask> tasks(1, HandleKeySortT);

void DestroyTask(ScheduleTask *task)
{
	luaL_unref(task->L, LUA_REGISTRYINDEX, task->callbackRef);
	luaL_unref(task->L, LUA_REGISTRYINDEX, task->threadRef);
	delete task;
}

void ExecuteTaskThread(void *arg)
{
	ScheduleTask *task = (ScheduleTask*)arg;

	lua_rawgeti(task->L, LUA_REGISTRYINDEX, task->callbackRef);
	lua_pcall(task->L, 0, 1, 0);

	void *res = lua_touserdata(task->L, -1);
	if (res == STOP || task->interval == 0)
	{
		DestroyTask(task);
		return;
	}

	{
		mir_cslock lock(threadLock);

		time_t now = time(NULL);
		if (task->timestamp + task->interval >= now)
			task->timestamp += task->interval;
		else
			task->timestamp += (((now - task->timestamp) / task->interval) + 1) * task->interval;
		tasks.insert(task);
	}

	SetEvent(hScheduleEvent);
}

void ScheduleThread(void*)
{
	time_t waitTime = INFINITE;

	while (true)
	{
wait:	WaitForSingleObject(hScheduleEvent, waitTime);

		while (ScheduleTask *task = tasks[0])
		{
			if (Miranda_IsTerminated())
				return;

			time_t now = time(NULL);
			if (task->timestamp > now)
			{
				waitTime = (task->timestamp - now) * 1000;
				goto wait;
			}

			{
				mir_cslock lock(threadLock);
				tasks.remove(task);
			}

			mir_forkthread(ExecuteTaskThread, task);
		}

		waitTime = INFINITE;
	}
}

void KillModuleScheduleTasks()
{
	mir_cslock lock(threadLock);

	while (ScheduleTask *task = tasks[0])
	{
		tasks.remove(task);
		DestroyTask(task);
	}
}

/***********************************************/

static time_t luaM_opttimestamp(lua_State *L, int idx, time_t def = 0)
{
	switch (lua_type(L, idx))
	{
	case LUA_TNUMBER:
		return luaL_optinteger(L, idx, def);

	case LUA_TSTRING:
	{
		const char *strtime = luaL_optstring(L, idx, "00:00:00");

		int hour = 0, min = 0, sec = 0;
		if (sscanf_s(strtime, "%02d:%02d:%02d", &hour, &min, &sec) >= 2)
		{
			struct tm *ti = localtime(&def);
			ti->tm_hour = hour;
			ti->tm_min = min;
			ti->tm_sec = sec;
			return mktime(ti);
		}
	}
	}
	return def;
}

static void luaM_change__index(lua_State *L, const luaL_Reg *funcs)
{
	int idx = lua_gettop(L);
	if (lua_getmetatable(L, idx))
	{
		lua_newtable(L);
		lua_pushvalue(L, idx);
		luaL_setfuncs(L, funcs, 1);
		lua_setfield(L, -2, "__index");
		lua_pop(L, 1);
	}
}

/***********************************************/

static int fluent_Do(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TFUNCTION);

	ScheduleTask *task = *(ScheduleTask**)lua_touserdata(L, lua_upvalueindex(1));

	time_t now = time(NULL);
	if (task->timestamp < now)
	{
		if (task->interval == 0)
			return 0;
		task->timestamp += (((now - task->timestamp) / task->interval) + 1) * task->interval;
	}

	lua_pushnil(L);
	lua_replace(L, lua_upvalueindex(1));

	task->L = lua_newthread(L);
	task->threadRef = luaL_ref(L, LUA_REGISTRYINDEX);
	lua_pushvalue(L, 1);
	task->callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
	{
		mir_cslock lock(threadLock);
		tasks.insert(task);
	}

	SetEvent(hScheduleEvent);

	return 0;
}

static int fluent_From(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TSTRING);

	ScheduleTask *task = *(ScheduleTask**)lua_touserdata(L, lua_upvalueindex(1));
	task->timestamp = luaM_opttimestamp(L, 1, 0);

	const luaL_Reg funcs[] =
	{
		{ "Do", fluent_Do },
		{ NULL, NULL }
	};

	lua_pushvalue(L, lua_upvalueindex(1));
	luaM_change__index(L, funcs);

	return 1;
}

static const luaL_Reg fluentApi[] =
{
	{ "From", fluent_From },
	{ "Do", fluent_Do },

	{ NULL, NULL }
};

static int fluent_Second(lua_State *L)
{
	ScheduleTask *task = *(ScheduleTask**)lua_touserdata(L, lua_upvalueindex(1));
	task->Append(SECOND);

	lua_pushvalue(L, lua_upvalueindex(1));
	luaM_change__index(L, fluentApi);

	return 1;
}

static int fluent_Seconds(lua_State *L)
{
	ScheduleTask *task = *(ScheduleTask**)lua_touserdata(L, lua_upvalueindex(1));

	time_t interval = luaL_optinteger(L, lua_upvalueindex(2), 1) * SECOND;
	task->Append(interval);

	lua_pushvalue(L, lua_upvalueindex(1));
	luaM_change__index(L, fluentApi);

	return 1;
}

static int fluent_Minute(lua_State *L)
{
	ScheduleTask *task = *(ScheduleTask**)lua_touserdata(L, lua_upvalueindex(1));
	task->Append(MINUTE);

	lua_pushvalue(L, lua_upvalueindex(1));
	luaM_change__index(L, fluentApi);

	return 1;
}

static int fluent_Minutes(lua_State *L)
{
	ScheduleTask *task = *(ScheduleTask**)lua_touserdata(L, lua_upvalueindex(1));
	time_t interval = luaL_optinteger(L, lua_upvalueindex(2), 1) * MINUTE;
	task->Append(interval);

	lua_pushvalue(L, lua_upvalueindex(1));
	luaM_change__index(L, fluentApi);

	return 1;
}

static int fluent_Hour(lua_State *L)
{
	ScheduleTask *task = *(ScheduleTask**)lua_touserdata(L, lua_upvalueindex(1));
	task->Append(HOUR);

	lua_pushvalue(L, lua_upvalueindex(1));
	luaM_change__index(L, fluentApi);

	return 1;
}

static int fluent_Hours(lua_State *L)
{
	ScheduleTask *task = *(ScheduleTask**)lua_touserdata(L, lua_upvalueindex(1));
	time_t interval = luaL_optinteger(L, lua_upvalueindex(2), 1) * HOUR;
	task->Append(interval);

	lua_pushvalue(L, lua_upvalueindex(1));
	luaM_change__index(L, fluentApi);

	return 1;
}

static int fluent_Day(lua_State *L)
{
	ScheduleTask *task = *(ScheduleTask**)lua_touserdata(L, lua_upvalueindex(1));
	task->Append(DAY);

	lua_pushvalue(L, lua_upvalueindex(1));
	luaM_change__index(L, fluentApi);

	return 1;
}

static int fluent_Days(lua_State *L)
{
	ScheduleTask *task = *(ScheduleTask**)lua_touserdata(L, lua_upvalueindex(1));
	time_t interval = luaL_optinteger(L, lua_upvalueindex(2), 1) * DAY;
	task->Append(interval);

	lua_pushvalue(L, lua_upvalueindex(1));
	luaM_change__index(L, fluentApi);

	return 1;
}

static int fluent_Week(lua_State *L)
{
	ScheduleTask *task = *(ScheduleTask**)lua_touserdata(L, lua_upvalueindex(1));
	task->Append(WEEK);

	lua_pushvalue(L, lua_upvalueindex(1));
	luaM_change__index(L, fluentApi);

	return 1;
}

static int fluent_Monday(lua_State *L)
{
	time_t timestamp = time(NULL);
	struct tm *ti = localtime(&timestamp);
	ti->tm_mday += abs(1 - ti->tm_wday);

	ScheduleTask *task = *(ScheduleTask**)lua_touserdata(L, lua_upvalueindex(1));
	task->timestamp = mktime(ti);
	task->interval = WEEK;

	lua_pushvalue(L, lua_upvalueindex(1));
	luaM_change__index(L, fluentApi);

	return 1;
}

static int fluent_Tuesday(lua_State *L)
{
	time_t timestamp = time(NULL);
	struct tm *ti = localtime(&timestamp);
	ti->tm_mday += abs(2 - ti->tm_wday);

	ScheduleTask *task = *(ScheduleTask**)lua_touserdata(L, lua_upvalueindex(1));
	task->timestamp = mktime(ti);
	task->interval = WEEK;

	lua_pushvalue(L, lua_upvalueindex(1));
	luaM_change__index(L, fluentApi);

	return 1;
}

static int fluent_Wednesday(lua_State *L)
{
	time_t timestamp = time(NULL);
	struct tm *ti = localtime(&timestamp);
	ti->tm_mday += abs(3 - ti->tm_wday);

	ScheduleTask *task = *(ScheduleTask**)lua_touserdata(L, lua_upvalueindex(1));
	task->timestamp = mktime(ti);
	task->interval = WEEK;

	lua_pushvalue(L, lua_upvalueindex(1));
	luaM_change__index(L, fluentApi);

	return 1;
}

static int fluent_Thursday(lua_State *L)
{
	time_t timestamp = time(NULL);
	struct tm *ti = localtime(&timestamp);
	ti->tm_mday += abs(4 - ti->tm_wday);

	ScheduleTask *task = *(ScheduleTask**)lua_touserdata(L, lua_upvalueindex(1));
	task->timestamp = mktime(ti);
	task->interval = WEEK;

	lua_pushvalue(L, lua_upvalueindex(1));
	luaM_change__index(L, fluentApi);

	return 1;
}

static int fluent_Friday(lua_State *L)
{
	time_t timestamp = time(NULL);
	struct tm *ti = localtime(&timestamp);
	ti->tm_mday += abs(5 - ti->tm_wday);

	ScheduleTask *task = *(ScheduleTask**)lua_touserdata(L, lua_upvalueindex(1));
	task->timestamp = mktime(ti);
	task->interval = WEEK;

	lua_pushvalue(L, lua_upvalueindex(1));
	luaM_change__index(L, fluentApi);

	return 1;
}

static int fluent_Saturday(lua_State *L)
{
	time_t timestamp = time(NULL);
	struct tm *ti = localtime(&timestamp);
	ti->tm_mday += abs(6 - ti->tm_wday);

	ScheduleTask *task = *(ScheduleTask**)lua_touserdata(L, lua_upvalueindex(1));
	task->timestamp = mktime(ti);
	task->interval = WEEK;

	lua_pushvalue(L, lua_upvalueindex(1));
	luaM_change__index(L, fluentApi);

	return 1;
}

static int fluent_Sunday(lua_State *L)
{
	time_t timestamp = time(NULL);
	struct tm *ti = localtime(&timestamp);
	ti->tm_mday += abs(-ti->tm_wday);

	ScheduleTask *task = *(ScheduleTask**)lua_touserdata(L, lua_upvalueindex(1));
	task->timestamp = mktime(ti);
	task->interval = WEEK;

	lua_pushvalue(L, lua_upvalueindex(1));
	luaM_change__index(L, fluentApi);

	return 1;
}

/***********************************************/

static const luaL_Reg scheduleAtApi[] =
{
	{ "Do", fluent_Do },

	{ NULL, NULL }
};

static int schedule_At(lua_State *L)
{
	ScheduleTask **task = (ScheduleTask**)lua_newuserdata(L, sizeof(ScheduleTask));
	*task = new ScheduleTask();
	(*task)->timestamp = luaM_opttimestamp(L, 1, time(NULL));

	lua_newtable(L);
	lua_newtable(L);
	lua_pushvalue(L, -3);
	luaL_setfuncs(L, scheduleAtApi, 1);
	lua_setfield(L, -2, "__index");
	lua_setmetatable(L, -2);

	return 1;
}

/***********************************************/

static const luaL_Reg scheduleEvery1Api[] =
{
	{ "Second", fluent_Second },
	{ "Minute", fluent_Minute },
	{ "Hour", fluent_Hour },
	{ "Day", fluent_Day },
	{ "Week", fluent_Week },
	{ "Monday", fluent_Monday },
	{ "Tuesday", fluent_Tuesday },
	{ "Wednesday", fluent_Wednesday },
	{ "Thursday", fluent_Thursday },
	{ "Friday", fluent_Friday },
	{ "Saturday", fluent_Saturday },
	{ "Sunday", fluent_Sunday },

	{ NULL, NULL }
};

static const luaL_Reg scheduleEvery2Api[] =
{
	{ "Seconds", fluent_Seconds },
	{ "Minutes", fluent_Minutes },
	{ "Hours", fluent_Hours },
	{ "Days", fluent_Days },

	{ NULL, NULL }
};

static int schedule_Every(lua_State *L)
{
	int count = luaL_optinteger(L, 1, 0);

	ScheduleTask **task = (ScheduleTask**)lua_newuserdata(L, sizeof(ScheduleTask*));
	*task = new ScheduleTask();
	(*task)->repeat = true;

	lua_newtable(L);
	lua_newtable(L);
	lua_pushvalue(L, -3);
	if (count)
	{
		lua_pushinteger(L, count);
		luaL_setfuncs(L, scheduleEvery2Api, 2);
	}
	else
		luaL_setfuncs(L, scheduleEvery1Api, 1);
	lua_setfield(L, -2, "__index");
	lua_setmetatable(L, -2);

	return 1;
}

/***********************************************/

static const luaL_Reg scheduleWait1Api[] =
{
	{ "Second", fluent_Second },
	{ "Minute", fluent_Minute },
	{ "Hour", fluent_Hour },
	{ "Day", fluent_Day },
	{ "Week", fluent_Week },

	{ NULL, NULL }
};

static const luaL_Reg scheduleWait2Api[] =
{
	{ "Seconds", fluent_Seconds },
	{ "Minutes", fluent_Minutes },
	{ "Hours", fluent_Hours },
	{ "Days", fluent_Days },

	{ NULL, NULL }
};

static int schedule_Wait(lua_State *L)
{
	int count = luaL_optinteger(L, 1, 0);

	ScheduleTask **task = (ScheduleTask**)lua_newuserdata(L, sizeof(ScheduleTask*));
	*task = new ScheduleTask();

	lua_newtable(L);
	lua_newtable(L);
	lua_pushvalue(L, -3);
	if (count)
	{
		lua_pushinteger(L, count);
		luaL_setfuncs(L, scheduleEvery2Api, 2);
	}
	else
		luaL_setfuncs(L, scheduleEvery1Api, 1);
	lua_setfield(L, -2, "__index");
	lua_setmetatable(L, -2);

	return 1;
}

static const luaL_Reg scheduleApi[] =
{
	{ "At", schedule_At },
	{ "Every", schedule_Every },
	{ "Wait", schedule_Wait },

	{ "STOP", NULL },

	{ NULL, NULL }
};

/***********************************************/

extern "C" LUAMOD_API int luaopen_m_schedule(lua_State *L)
{
	luaL_newlib(L, scheduleApi);
	lua_pushlightuserdata(L, STOP);
	lua_setfield(L, -2, "STOP");

	if (hScheduleEvent == NULL)
		hScheduleEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	if (hScheduleThread == NULL)
		hScheduleThread = mir_forkthread(ScheduleThread);

	return 1;
}
