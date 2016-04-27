#ifndef _TOX_THREAD_H_
#define _TOX_THREAD_H_

class CToxThread
{
private:
	Tox *tox;
	ToxAV *toxAV;
	bool isConnected;
	bool isTerminated;

	mir_cs toxLock;

	mir_cs lock;

public:
	CToxThread(Tox_Options *options, TOX_ERR_NEW *error = NULL) : tox(NULL), toxAV(NULL),
		isConnected(false), isTerminated(false)
	{
		tox = tox_new(options, error);
	}

	~CToxThread()
	{
		if (toxAV)
		{
			toxav_kill(toxAV);
			toxAV = NULL;
		}

		if (tox)
		{
			tox_kill(tox);
			tox = NULL;
		}
	}

	mir_cslock Lock()
	{
		return mir_cslock(lock);
	}

	Tox* Tox()
	{
		return tox;
	}

	ToxAV* ToxAV()
	{
		return toxAV;
	}

	bool IsConnected() const
	{
		return tox && isConnected;
	}

	void Connect()
	{
		Lock();

		isConnected = true;
	}

	void Disconnect()
	{
		Lock();

		isConnected = false;
	}

	void Iterate()
	{
		{
			mir_cslock lock(toxLock);
			tox_iterate(tox);
			//if (toxAV)
			//	toxav_iterate(toxAV);
		}
		uint32_t interval = tox_iteration_interval(tox);
		interval = interval ? interval : 50;
		Sleep(interval);
	}

	bool IsTerminated() const
	{
		return !tox || isTerminated;
	}

	void Stop() { isTerminated = true; }
};

#endif //_TOX_THREAD_H_