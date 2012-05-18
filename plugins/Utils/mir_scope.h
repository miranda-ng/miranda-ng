#ifndef __PTR_H__
# define __PTR_H__


template<class T> 
class scope 
{
public:
	scope(T t) : p(t) {}
	~scope() { mir_free(); }

	void free()
	{
		if (p != NULL)
			mir_free(p);
		p = NULL;
	}

//	T operator->() const { return p; }
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
