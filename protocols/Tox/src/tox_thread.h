#ifndef _TOX_THREAD_H_
#define _TOX_THREAD_H_

class CToxThread
{
public:
	Tox *tox;
	ToxAv *toxAv;
	bool isConnected;
	bool isTerminated;

	mir_cs toxLock;

	CToxThread() : tox(NULL), toxAv(NULL),
		isConnected(false), isTerminated(false) { }

	void Do()
	{
		{
			mir_cslock lock(toxLock);
			tox_iterate(tox);
			if (toxAv)
				toxav_do(toxAv);
		}
		uint32_t interval = tox_iteration_interval(tox);
		Sleep(interval);
	}

	void Stop() { isTerminated = true; }
};

#endif //_TOX_THREAD_H_