/*
Copyright (c) 2015-22 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _SKYPE_REQUEST_SUBSCIPTIONS_H_
#define _SKYPE_REQUEST_SUBSCIPTIONS_H_

struct CreateSubscriptionsRequest : public AsyncHttpRequest
{
	CreateSubscriptionsRequest() :
		AsyncHttpRequest(REQUEST_POST, HOST_DEFAULT, "/users/ME/endpoints/SELF/subscriptions", &CSkypeProto::OnSubscriptionsCreated)
	{
		JSONNode interestedResources(JSON_ARRAY); interestedResources.set_name("interestedResources");
		interestedResources << CHAR_PARAM("", "/v1/users/ME/conversations/ALL/properties")
			<< CHAR_PARAM("", "/v1/users/ME/conversations/ALL/messages")
			<< CHAR_PARAM("", "/v1/users/ME/contacts/ALL")
			<< CHAR_PARAM("", "/v1/threads/ALL");

		JSONNode node;
		node << CHAR_PARAM("channelType", "httpLongPoll") << CHAR_PARAM("template", "raw") << interestedResources;
		m_szParam = node.write().c_str();
	}
};

struct CreateContactsSubscriptionRequest : public AsyncHttpRequest
{
	CreateContactsSubscriptionRequest(const LIST<char> &skypenames) :
		AsyncHttpRequest(REQUEST_POST, HOST_DEFAULT, "/users/ME/contacts")
	{
		JSONNode contacts(JSON_ARRAY); contacts.set_name("contacts");
		for (auto &it : skypenames) {
			JSONNode contact;
			contact << CHAR_PARAM("id", it);
			contacts << contact;
		}

		JSONNode node;
		node << contacts;
		m_szParam = node.write().c_str();
	}
};

#endif //_SKYPE_REQUEST_SUBSCIPTIONS_H_
