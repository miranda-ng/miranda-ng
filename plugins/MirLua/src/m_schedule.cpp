#include "stdafx.h"

static mir_cs threadLock;
static HANDLE hScheduleEvent = NULL;
static HANDLE hScheduleThread = NULL;

struct ScheduleTask
{
	time_t startTime;
	time_t endTime;
	time_t interval;

	lua_State *L;
	//lua_State *T;
	//int threadRef;
	int callbackRef;
};

static int TaskCompare(const ScheduleTask *p1, const ScheduleTask *p2)
{
	return p1->startTime - p2->startTime;
}

static LIST<ScheduleTask> tasks(1, TaskCompare);

void DestroyTask(ScheduleTask *task)
{
	//luaL_unref(task->L, LUA_REGISTRYINDEX, task->threadRef);
	delete task;
}

void ExecuteTaskThread(void *arg)
{
	ScheduleTask *task = (ScheduleTask*)arg;

	lua_State *L = lua_newthread(task->L);
	lua_rawgeti(L, LUA_REGISTRYINDEX, task->callbackRef);
	luaM_pcall(L, 0, 2);

	if (task->interval == 0)
	{
		DestroyTask(task);
		return;
	}

	{
		mir_cslock lock(threadLock);

		time_t timestamp = time(NULL);
		if(task->startTime + task->interval >= timestamp)
			task->startTime += task->interval;
		else
			task->startTime = timestamp + task->interval;
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
			if (Miranda_Terminated())
				return;

			mir_cslock lock(threadLock);

			time_t timestamp = time(NULL);
			if (task->startTime > timestamp)
			{
				waitTime = (task->startTime - timestamp - 1) * 1000;
				goto wait;
			}

			tasks.remove(task);

			if (task->endTime > 0 && task->endTime < timestamp)
			{
				DestroyTask(task);
				continue;
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
		sscanf_s(strtime, "%02d:%02d:%02d", &hour, &min, &sec);
		struct tm *ti = localtime(&def);
		ti->tm_hour = hour;
		ti->tm_min = min;
		ti->tm_sec = sec;
		return mktime(ti);
	}
	}
	return def;
}

/***********************************************/

enum DayOfWeek
{
	None = -1,
	Sunday = 0,
	Monday = 1,
	Tuesday = 2,
	Wednesday = 3,
	Thursday = 4,
	Friday = 5,
	Saturday = 6
};

static int lua__Second(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);

	lua_pushinteger(L, 1);
	lua_setfield(L, -2, "Interval");
	lua_pushinteger(L, time(NULL));
	lua_setfield(L, -2, "Timestamp");
	lua_pushinteger(L, DayOfWeek::None);
	lua_setfield(L, -2, "DayOfWeek");

	return 1;
}

static int lua__Seconds(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);

	lua_pushliteral(L, "Interval");
	lua_gettable(L, -2);
	int seconds = luaL_optinteger(L, -1, 1);
	lua_pop(L, 1);
	lua_pushinteger(L, seconds);
	lua_setfield(L, -2, "Interval");
	lua_pushinteger(L, DayOfWeek::None);
	lua_setfield(L, -2, "DayOfWeek");

	return 1;
}

static int lua__Minute(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);

	lua_pushinteger(L, 60);
	lua_setfield(L, -2, "Interval");
	lua_pushinteger(L, DayOfWeek::None);
	lua_setfield(L, -2, "DayOfWeek");

	return 1;
}

static int lua__Minutes(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);

	lua_pushliteral(L, "Interval");
	lua_gettable(L, -2);
	int interval = luaL_optinteger(L, -1, 1);
	lua_pop(L, 1);
	lua_pushinteger(L, interval * 60);
	lua_setfield(L, -2, "Interval");
	lua_pushinteger(L, DayOfWeek::None);
	lua_setfield(L, -2, "DayOfWeek");

	return 1;
}

static int lua__Hour(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);

	lua_pushinteger(L, 60 * 60);
	lua_setfield(L, -2, "Interval");
	lua_pushinteger(L, DayOfWeek::None);
	lua_setfield(L, -2, "DayOfWeek");

	return 1;
}

static int lua__Hours(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);

	lua_pushliteral(L, "Interval");
	lua_gettable(L, -2);
	int interval = luaL_optinteger(L, -1, 1);
	lua_pop(L, 1);
	lua_pushinteger(L, interval * 60 * 60);
	lua_setfield(L, -2, "Interval");
	lua_pushinteger(L, DayOfWeek::None);
	lua_setfield(L, -2, "DayOfWeek");

	return 1;
}

static int lua__Day(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);

	lua_pushinteger(L, 60 * 60 * 24);
	lua_setfield(L, -2, "Interval");
	lua_pushinteger(L, DayOfWeek::None);
	lua_setfield(L, -2, "DayOfWeek");

	return 1;
}

static int lua__Days(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);

	lua_pushliteral(L, "Interval");
	lua_gettable(L, -2);
	int interval = luaL_optinteger(L, -1, 1);
	lua_pop(L, 1);
	lua_pushinteger(L, interval * 60 * 60 * 24);
	lua_setfield(L, -2, "Interval");
	lua_pushinteger(L, DayOfWeek::None);
	lua_setfield(L, -2, "DayOfWeek");

	return 1;
}

static int lua__Week(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);

	lua_pushinteger(L, 60 * 60 * 24 * 7);
	lua_setfield(L, -2, "Interval");
	lua_pushinteger(L, DayOfWeek::None);
	lua_setfield(L, -2, "DayOfWeek");

	return 1;
}

static int lua__Weeks(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);

	lua_pushliteral(L, "Interval");
	lua_gettable(L, -2);
	int interval = luaL_optinteger(L, -1, 1);
	lua_pop(L, 1);
	lua_pushinteger(L, interval * 60 * 60 * 24 * 7);
	lua_setfield(L, -2, "Interval");
	lua_pushinteger(L, DayOfWeek::None);
	lua_setfield(L, -2, "DayOfWeek");

	return 1;
}

static int lua__Monday(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);

	lua_pushinteger(L, 60 * 60 * 24 * 7);
	lua_setfield(L, -2, "Interval");
	lua_pushinteger(L, Monday);
	lua_setfield(L, -2, "DayOfWeek");

	return 1;
}

static int lua__Tuesday(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);

	lua_pushinteger(L, 60 * 60 * 24 * 7);
	lua_setfield(L, -2, "Interval");
	lua_pushinteger(L, Tuesday);
	lua_setfield(L, -2, "DayOfWeek");

	return 1;
}

static int lua__Wednesday(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);

	lua_pushinteger(L, 60 * 60 * 24 * 7);
	lua_setfield(L, -2, "Interval");
	lua_pushinteger(L, Wednesday);
	lua_setfield(L, -2, "DayOfWeek");

	return 1;
}

static int lua__Thursday(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);

	lua_pushinteger(L, 60 * 60 * 24 * 7);
	lua_setfield(L, -2, "Interval");
	lua_pushinteger(L, Thursday);
	lua_setfield(L, -2, "DayOfWeek");

	return 1;
}

static int lua__Friday(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);

	lua_pushinteger(L, 60 * 60 * 24 * 7);
	lua_setfield(L, -2, "Interval");
	lua_pushinteger(L, Friday);
	lua_setfield(L, -2, "DayOfWeek");

	return 1;
}

static int lua__Saturday(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);

	lua_pushinteger(L, 60 * 60 * 24 * 7);
	lua_setfield(L, -2, "Interval");
	lua_pushinteger(L, Saturday);
	lua_setfield(L, -2, "DayOfWeek");

	return 1;
}

static int lua__Sunday(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);

	lua_pushinteger(L, 60 * 60 * 24 * 7);
	lua_setfield(L, -2, "Interval");
	lua_pushinteger(L, Sunday);
	lua_setfield(L, -2, "DayOfWeek");

	return 1;
}

static int lua__From(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);

	time_t timestamp = time(NULL);
	time_t startTime = luaM_opttimestamp(L, 2, timestamp);

	if (startTime < timestamp)
	{
		lua_pushliteral(L, "Interval");
		lua_gettable(L, 1);
		int interval = luaL_optinteger(L, -1, 1);
		lua_pop(L, 1);
		if (interval > 0)
			startTime += (((timestamp - startTime) / interval) + 1) * interval;
	}

	lua_pushvalue(L, 1);
	lua_pushinteger(L, startTime);
	lua_setfield(L, -2, "StartTime");

	return 1;
}

static int lua__Until(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);

	time_t endTime = luaM_opttimestamp(L, 2);

	lua_pushvalue(L, 1);
	lua_pushinteger(L, endTime);
	lua_setfield(L, -2, "EndTime");

	return 1;
}

static int lua__Do(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);
	luaL_checktype(L, 2, LUA_TFUNCTION);

	lua_pushliteral(L, "Interval");
	lua_gettable(L, 1);
	int interval = luaL_optinteger(L, -1, 0);
	lua_pop(L, 1);

	time_t timestamp = time(NULL);

	lua_pushliteral(L, "StartTime");
	lua_gettable(L, 1);
	time_t startTime = luaL_optinteger(L, -1, timestamp);
	lua_pop(L, 1);

	if (startTime < timestamp && interval == 0)
		return 0;

	lua_pushliteral(L, "DayOfWeek");
	lua_gettable(L, 1);
	DayOfWeek dayOfWeek = (DayOfWeek)luaL_optinteger(L, -1, DayOfWeek::None);
	lua_pop(L, 1);

	if (dayOfWeek > DayOfWeek::None && dayOfWeek <= DayOfWeek::Saturday)
	{
		struct tm *ti = localtime(&startTime);
		ti->tm_mday += abs((int)dayOfWeek - ti->tm_wday);
		startTime = mktime(ti);
	}

	lua_pushliteral(L, "EndTime");
	lua_gettable(L, 1);
	time_t endTime = luaL_optinteger(L, -1, 0);
	lua_pop(L, 1);

	if (endTime > 0 && endTime < timestamp)
		return 0;

	ScheduleTask *task = new ScheduleTask();
	task->startTime = startTime;
	task->endTime = endTime;
	task->interval = interval;
	task->L = L;
	lua_pushvalue(L, 2);
	task->callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
	//task->T = lua_newthread(L);
	//task->threadRef = luaL_ref(L, LUA_REGISTRYINDEX);
	{
		mir_cslock lock(threadLock);
		tasks.insert(task);
	}
	SetEvent(hScheduleEvent);

	return 0;
}

static int lua__index(lua_State *L)
{
	int t1 = lua_type(L, 1);
	int t2 = lua_type(L, 2);

	//lua_pushvalue(L, 1);
	lua_getmetatable(L, 1);

	return 1;
}

static const luaL_Reg scheduleMeta[] =
{
	{ "Second", lua__Second },
	{ "Seconds", lua__Seconds },
	{ "Minute", lua__Minute },
	{ "Minutes", lua__Minutes },
	{ "Hour", lua__Hour },
	{ "Hours", lua__Hours },
	{ "Day", lua__Day },
	{ "Days", lua__Days },
	{ "Week", lua__Week },
	{ "Weeks", lua__Weeks },
	{ "Monday", lua__Week },
	{ "Tuesday", lua__Tuesday },
	{ "Wednesday", lua__Wednesday },
	{ "Thursday", lua__Thursday },
	{ "Friday", lua__Friday },
	{ "Saturday", lua__Saturday },
	{ "Sunday", lua__Sunday },
	{ "From", lua__From },
	{ "Until", lua__Until },
	{ "Do", lua__Do },

	//{ "__index", lua__index },

	{ NULL, NULL }
};

/***********************************************/

#define MT_SCHEDULETASK "SCHEDULETASK"

static int lua__At(lua_State *L)
{
	time_t timestamp = time(NULL);
	time_t startTime = luaM_opttimestamp(L, 1, timestamp);

	lua_newtable(L);
	lua_pushcclosure(L, lua__Until, 0);
	lua_setfield(L, -2, "Until");
	lua_pushcclosure(L, lua__Do, 0);
	lua_setfield(L, -2, "Do");
	lua_pushinteger(L, startTime);
	lua_setfield(L, -2, "StartTime");

	return 1;
}

static int lua__Every(lua_State *L)
{
	int interval = luaL_optinteger(L, 1, 0);

	int top = lua_gettop(L);
	luaL_newlib(L, scheduleMeta);
	//lua_newtable(L);
	lua_pushinteger(L, interval);
	lua_setfield(L, -2, "Interval");
	top = lua_gettop(L);

	lua_createtable(L, 0, 1);
	//lua_pushcfunction(L, lua__index);
	lua_pushvalue(L, -2);
	lua_setfield(L, -2, "__index");
	lua_setmetatable(L, -2);
	top = lua_gettop(L);

	//luaL_setmetatable(L, MT_SCHEDULETASK);

	return 1;
}

static const luaL_Reg scheduleApi[] =
{
	{ "At", lua__At },
	{ "Every", lua__Every },
	{ "Do", lua__Do },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_schedule(lua_State *L)
{
	luaL_newlib(L, scheduleApi);

	//luaL_newmetatable(L, MT_SCHEDULETASK);
	/*lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");*/
	//luaL_setfuncs(L, scheduleMeta, 0);
	//lua_pop(L, 1);

	if (hScheduleEvent == NULL)
		hScheduleEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	if (hScheduleThread == NULL)
		hScheduleThread = mir_forkthread(ScheduleThread);

	return 1;
}
