/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-17 Robert Pösel, 2017-18 Miranda NG team

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#pragma once

namespace List
{
	template< typename T > class Item
	{
	public:
		std::string key;
		T* data;
		Item< T >* prev;
		Item< T >* next;

		Item()
		{
			this->data = nullptr;
			this->prev = nullptr;
			this->next = nullptr;
		}

		~Item()
		{
			delete this->data;
		}
	};

	template< typename T > class List
	{
	private:
		Item< T >* first;
		Item< T >* last;
		unsigned int count;

	public:
		List()
		{
			this->first = this->last = nullptr;
			this->count = 0;
		}

		~List()
		{
			this->clear();
		}

		Item< T >* begin()
		{
			return first;
		}

		Item< T >* end()
		{
			return last;
		}

		unsigned int size()
		{
			return count;
		}

		bool empty()
		{
			return (this->first == nullptr);
		}

		void insert(Item< T >* item)
		{
			if (this->empty()) {
				this->first = this->last = item;
				this->count = 1;
			}
			else { // TODO: key sorting/comparation
				item->next = this->first;
				this->first->prev = item;
				this->first = item;
				this->count++;
			}
		}

		void insert(std::pair< std::string, T* > item)
		{
			Item<T>* ins = new Item<T>;
			ins->key = item.first;
			ins->data = item.second;
			this->insert(ins);
		}
		void erase(std::string key)
		{
			Item< T >* help = this->first;
			while (help != nullptr) {
				if (help->key.compare(key) != 0)
					help = help->next;
				else {
					if (help == this->first) {
						this->first = help->next;
						if (this->first != nullptr)
							this->first->prev = nullptr;
						else
							this->last = nullptr;
					}
					else if (help == this->last) {
						this->last = help->prev;
						if (this->last != nullptr)
							this->last->next = nullptr;
						else
							this->first = nullptr;
					}
					else {
						help->prev->next = help->next;
						help->next->prev = help->prev;
					}
					this->count--;
					delete help;
					break;
				}
			}
		}

		void erase(Item< T >* item)
		{
			if (item != nullptr)
				erase(item->key);
		}

		T* find(std::string key)
		{
			Item< T >* help = this->begin();
			while (help != nullptr) {
				if (help->key.compare(key) != 0)
					help = help->next;
				else
					return help->data;
			}
			return nullptr;
		}

		T* at(const unsigned int item)
		{
			if (item >= this->count)
				return nullptr;
			Item< T >* help = this->begin();
			for (unsigned int i = 0; i < item; i++)
				help = help->next;
			return help->item;
		}

		T* operator[](const unsigned int item)
		{
			return at(item);
		}

		void clear()
		{
			Item< T >* help;
			while (this->first != nullptr) {
				help = this->first;
				this->first = this->first->next;
				delete help;
			}
			this->last = nullptr;
			this->count = 0;
		}
	};
};
