//***********************************************************
//	Copyright © 2008 Valentin Pavlyuchenko
//
//	This file is part of Boltun.
//
//    Boltun is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 2 of the License, or
//    (at your option) any later version.
//
//    Boltun is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//	  along with Boltun. If not, see <http://www.gnu.org/licenses/>.
//
//***********************************************************

#ifndef StringChooserH
#define StringChooserH

template<class container = std::vector<std::tstring> >
class ValueChooser
{
private:
	int freeNumbers;
	bool *numbers;
	const container data;
	bool notifyOnReset;
public:
	ValueChooser(const container& vec, bool NotifyOnReset = false)
		:data(vec), notifyOnReset(NotifyOnReset)
	{
		//randomize();
		numbers = NULL;
		UpdateLength();
	}

	~ValueChooser()
	{
		delete numbers;
	}

	void UpdateLength()
	{
		delete numbers; //normal if numbers == NULL
		numbers = new bool[data.size()];
		Reset();
	}

	void Reset()
	{
		for (size_t i = 0; i < data.size(); i++)
			numbers[i] = false;
		freeNumbers = (int)data.size();
	}

	typename container::value_type GetString()
	{
		if (!freeNumbers)
		{
			Reset();
			if (notifyOnReset)
				return _T("");
		}
		int result;
		while (numbers[result = rand() % data.size()])
			;
		freeNumbers--;
		numbers[result] = true;
		return data[result];
	}

	container GetContainer() const;
};

template<class container>
container ValueChooser<container>::GetContainer() const
{
	return data;
}

#endif
