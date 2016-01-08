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
	int threadRef;
	int callbackRef;
};

static int TaskCompare(const ScheduleTask *p1, const ScheduleTask *p2)
{
	return p1->startTime - p2->startTime;
}

static LIST<ScheduleTask> tasks(1, TaskCompare);

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
	luaM_pcall(task->L);

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

static int schedule_Second(lua_State *L)
{
	lua_pushvalue(L, lua_upvalueindex(1));

	lua_pushinteger(L, 1);
	lua_setfield(L, -2, "Interval");

	return 1;
}

static int schedule_Seconds(lua_State *L)
{
	lua_pushvalue(L, lua_upvalueindex(1));

	lua_getfield(L, -1, "Interval");
	int seconds = luaL_optinteger(L, -1, 1);
	lua_pop(L, 1);
	lua_pushinteger(L, seconds);
	lua_setfield(L, -2, "Interval");

	return 1;
}

static int schedule_Minute(lua_State *L)
{
	lua_pushvalue(L, lua_upvalueindex(1));

	lua_pushinteger(L, 60);
	lua_setfield(L, -2, "Interval");

	return 1;
}

static int schedule_Minutes(lua_State *L)
{
	lua_pushvalue(L, lua_upvalueindex(1));

	lua_getfield(L, -1, "Interval");
	int interval = luaL_optinteger(L, -1, 1);
	lua_pop(L, 1);
	lua_pushinteger(L, interval * 60);
	lua_setfield(L, -2, "Interval");

	return 1;
}

static int schedule_Hour(lua_State *L)
{
	lua_pushvalue(L, lua_upvalueindex(1));

	lua_pushinteger(L, 60 * 60);
	lua_setfield(L, -2, "Interval");

	return 1;
}

static int schedule_Hours(lua_State *L)
{
	lua_pushvalue(L, lua_upvalueindex(1));

	lua_getfield(L, -1, "Interval");
	int interval = luaL_optinteger(L, -1, 1);
	lua_pop(L, 1);
	lua_pushinteger(L, interval * 60 * 60);
	lua_setfield(L, -2, "Interval");

	return 1;
}

static int schedule_Day(lua_State *L)
{
	lua_pushvalue(L, lua_upvalueindex(1));

	lua_pushinteger(L, 60 * 60 * 24);
	lua_setfield(L, -2, "Interval");

	return 1;
}

static int schedule_Days(lua_State *L)
{
	lua_pushvalue(L, lua_upvalueindex(1));

	lua_getfield(L, -1, "Interval");
	int interval = luaL_optinteger(L, -1, 1);
	lua_pop(L, 1);
	lua_pushinteger(L, interval * 60 * 60 * 24);
	lua_setfield(L, -2, "Interval");

	return 1;
}

static int schedule_Week(lua_State *L)
{
	lua_pushvalue(L, lua_upvalueindex(1));

	lua_pushinteger(L, 60 * 60 * 24 * 7);
	lua_setfield(L, -2, "Interval");

	return 1;
}

static int schedule_Monday(lua_State *L)
{
	lua_pushvalue(L, lua_upvalueindex(1));

	time_t timestamp = time(NULL);
	struct tm *ti = localtime(&timestamp);
	ti->tm_mday += abs(1 - ti->tm_wday);
	lua_pushinteger(L, mktime(ti));
	lua_setfield(L, -2, "StartTime");
	lua_pushinteger(L, 60 * 60 * 24 * 7);
	lua_setfield(L, -2, "Interval");

	return 1;
}

static int schedule_Tuesday(lua_State *L)
{
	lua_pushvalue(L, lua_upvalueindex(1));

	time_t timestamp = time(NULL);
	struct tm *ti = localtime(&timestamp);
	ti->tm_mday += abs(2 - ti->tm_wday);
	lua_pushinteger(L, mktime(ti));
	lua_setfield(L, -2, "StartTime");
	lua_pushinteger(L, 60 * 60 * 24 * 7);
	lua_setfield(L, -2, "Interval");

	return 1;
}

static int schedule_Wednesday(lua_State *L)
{
	lua_pushvalue(L, lua_upvalueindex(1));

	time_t timestamp = time(NULL);
	struct tm *ti = localtime(&timestamp);
	ti->tm_mday += abs(3 - ti->tm_wday);
	lua_pushinteger(L, mktime(ti));
	lua_setfield(L, -2, "StartTime");
	lua_pushinteger(L, 60 * 60 * 24 * 7);
	lua_setfield(L, -2, "Interval");

	return 1;
}

static int schedule_Thursday(lua_State *L)
{
	lua_pushvalue(L, lua_upvalueindex(1));

	time_t timestamp = time(NULL);
	struct tm *ti = localtime(&timestamp);
	ti->tm_mday += abs(4 - ti->tm_wday);
	lua_pushinteger(L, mktime(ti));
	lua_setfield(L, -2, "StartTime");
	lua_pushinteger(L, 60 * 60 * 24 * 7);
	lua_setfield(L, -2, "Interval");

	return 1;
}

static int schedule_Friday(lua_State *L)
{
	lua_pushvalue(L, lua_upvalueindex(1));

	time_t timestamp = time(NULL);
	struct tm *ti = localtime(&timestamp);
	ti->tm_mday += abs(5 - ti->tm_wday);
	lua_pushinteger(L, mktime(ti));
	lua_setfield(L, -2, "StartTime");
	lua_pushinteger(L, 60 * 60 * 24 * 7);
	lua_setfield(L, -2, "Interval");

	return 1;
}

static int schedule_Saturday(lua_State *L)
{
	lua_pushvalue(L, lua_upvalueindex(1));

	time_t timestamp = time(NULL);
	struct tm *ti = localtime(&timestamp);
	ti->tm_mday += abs(6 - ti->tm_wday);
	lua_pushinteger(L, mktime(ti));
	lua_setfield(L, -2, "StartTime");
	lua_pushinteger(L, 60 * 60 * 24 * 7);
	lua_setfield(L, -2, "Interval");

	return 1;
}

static int schedule_Sunday(lua_State *L)
{
	lua_pushvalue(L, lua_upvalueindex(1));

	time_t timestamp = time(NULL);
	struct tm *ti = localtime(&timestamp);
	ti->tm_mday += abs(-ti->tm_wday);
	lua_pushinteger(L, mktime(ti));
	lua_setfield(L, -2, "StartTime");
	lua_pushinteger(L, 60 * 60 * 24 * 7);
	lua_setfield(L, -2, "Interval");

	return 1;
}

static int schedule_From(lua_State *L)
{
	time_t timestamp = time(NULL);
	time_t startTime = luaM_opttimestamp(L, 1, timestamp);

	lua_pushvalue(L, lua_upvalueindex(1));

	if (startTime < timestamp)
	{
		lua_getfield(L, -1, "Interval");
		int interval = luaL_optinteger(L, -1, 1);
		lua_pop(L, 1);
		if (interval > 0)
			startTime += (((timestamp - startTime) / interval) + 1) * interval;
	}

	lua_pushinteger(L, startTime);
	lua_setfield(L, -2, "StartTime");

	return 1;
}

static int schedule_To(lua_State *L)
{
	time_t endTime = luaM_opttimestamp(L, 1);

	lua_pushvalue(L, lua_upvalueindex(1));
	lua_pushinteger(L, endTime);
	lua_setfield(L, -2, "EndTime");

	return 1;
}

static int schedule_Do(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TFUNCTION);

	lua_getfield(L, lua_upvalueindex(1), "Interval");
	int interval = luaL_optinteger(L, -1, 0);
	lua_pop(L, 1);

	time_t timestamp = time(NULL);

	lua_getfield(L, lua_upvalueindex(1), "EndTime");
	time_t endTime = luaL_optinteger(L, -1, 0);
	lua_pop(L, 1);

	if (endTime > 0 && endTime < timestamp)
		return 0;

	lua_getfield(L, lua_upvalueindex(1), "StartTime");
	time_t startTime = luaL_optinteger(L, -1, timestamp);
	lua_pop(L, 1);

	if (startTime < timestamp)
	{
		if (interval == 0)
			return 0;
		startTime += (((timestamp - startTime) / interval) + 1) * interval;
	}

	lua_pushnil(L);
	lua_replace(L, lua_upvalueindex(1));

	ScheduleTask *task = new ScheduleTask();
	task->startTime = startTime;
	task->endTime = endTime;
	task->interval = interval;
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

static const luaL_Reg scheduleFluentApi[] =
{
	{ "Second", schedule_Second },
	{ "Seconds", schedule_Seconds },
	{ "Minute", schedule_Minute },
	{ "Minutes", schedule_Minutes },
	{ "Hour", schedule_Hour },
	{ "Hours", schedule_Hours },
	{ "Day", schedule_Day },
	{ "Days", schedule_Days },
	{ "Week", schedule_Week },
	{ "Monday", schedule_Monday },
	{ "Tuesday", schedule_Tuesday },
	{ "Wednesday", schedule_Wednesday },
	{ "Thursday", schedule_Thursday },
	{ "Friday", schedule_Friday },
	{ "Saturday", schedule_Saturday },
	{ "Sunday", schedule_Sunday },
	{ "From", schedule_From },
	{ "To", schedule_To },
	{ "Do", schedule_Do },

	{ NULL, NULL }
};

/***********************************************/

static int schedule_At(lua_State *L)
{
	time_t timestamp = time(NULL);
	time_t startTime = luaM_opttimestamp(L, 1, timestamp);

	lua_newtable(L);
	lua_pushinteger(L, startTime);
	lua_setfield(L, -2, "StartTime");

	lua_pushvalue(L, -1);
	lua_pushcclosure(L, schedule_To, 1);
	lua_setfield(L, -3, "To");
	lua_pushvalue(L, -1);
	lua_pushcclosure(L, schedule_Do, 1);
	lua_setfield(L, -3, "Do");

	return 1;
}

static int schedule_Every(lua_State *L)
{
	int interval = luaL_optinteger(L, 1, 0);

	lua_newtable(L);
	lua_pushvalue(L, -1);
	lua_pushinteger(L, interval);
	lua_setfield(L, -2, "Interval");
	luaL_setfuncs(L, scheduleFluentApi, 1);

	return 1;
}

static const luaL_Reg scheduleApi[] =
{
	{ "At", schedule_At },
	{ "Every", schedule_Every },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_schedule(lua_State *L)
{
	luaL_newlib(L, scheduleApi);

	if (hScheduleEvent == NULL)
		hScheduleEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	if (hScheduleThread == NULL)
		hScheduleThread = mir_forkthread(ScheduleThread);

	return 1;
}
