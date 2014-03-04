#if !defined(HISTORYSTATS_GUARD_UTILS_PATTERN_H)
#define HISTORYSTATS_GUARD_UTILS_PATTERN_H

namespace pattern
{
	template<typename T_>
	class NotCopyable
	{
	private:
		NotCopyable(const NotCopyable&);
		const NotCopyable& operator =(const NotCopyable&);

	protected:
		NotCopyable()
		{
		}

		~NotCopyable()
		{
		}
	};

	template<typename T_>
	class NotInstantiable
	{
	private:
		NotInstantiable(const NotInstantiable&);
		const NotInstantiable& operator =(const NotInstantiable&);

	protected:
		NotInstantiable();

		~NotInstantiable()
		{
		}
	};
}

#endif // HISTORYSTATS_GUARD_UTILS_PATTERN_H