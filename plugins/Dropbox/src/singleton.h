#ifndef _SINGLETON_H_
#define _SINGLETON_H_

template<typename T>
class Singleton
{
public:
	static T *GetInstance()
	{
		if (!instance)
			instance = new T;
		return instance;
	}
	
private:
	static T* instance;

	Singleton();
	Singleton(Singleton const&);

	~Singleton();

	Singleton &operator=(Singleton const&);
};

template <typename T> T *Singleton<T>::instance = 0;


#endif //_SINGLETON_H_