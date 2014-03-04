#if !defined(HISTORYSTATS_GUARD_INOUT_H)
#define HISTORYSTATS_GUARD_INOUT_H

class InOut
{
public:
	int in;
	int out;

public:
	InOut()
		: in(0), out(0)
	{
	}

	InOut(int initIn, int initOut)
		: in(initIn), out(initOut)
	{
	}

	InOut& operator +=(const InOut& other)
	{
		in += other.in;
		out += other.out;

		return *this;
	}

	int total() const
	{
		return in + out;
	}

	int operator <(const InOut& other) const
	{
		return total() < other.total();
	}

	int operator >(const InOut& other) const
	{
		return total() > other.total();
	}

	int operator !=(const InOut& other) const
	{
		return total() != other.total();
	}
};

#endif // HISTORYSTATS_GUARD_INOUT_H
