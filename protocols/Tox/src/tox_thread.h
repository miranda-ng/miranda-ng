#ifndef _TOX_THREAD_H_
#define _TOX_THREAD_H_

class CToxThread
{
private:
	Tox *tox;
	ToxAV *toxAV;

public:
	CToxThread(Tox_Options *options, TOX_ERR_NEW *error = NULL)
		: tox(NULL), toxAV(NULL)
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

	Tox* Tox()
	{
		return tox;
	}

	ToxAV* ToxAV()
	{
		return toxAV;
	}
};

#endif //_TOX_THREAD_H_