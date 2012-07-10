#ifndef __PTR_H__
# define __PTR_H__


template<class T> 
class scope 
{
public:
	scope() : p(NULL) {}
	scope(T t) : p(t) {}
	~scope() { release(); }

	void release()
	{
		if (p != NULL)
			delete p;
		p = NULL;
	}

	T operator=(T t) { release(); p = t; return t; }
	T operator->() const { return p; }
	operator T() const { return p; }

	T detach() 
	{ 
		T ret = p;
		p = NULL;
		return ret;
	}

private:
	T p;
};



#endif // __PTR_H__
