/*
Copyright (c) 2015-20 Miranda NG team (https://miranda-ng.org)

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
	CreateSubscriptionsRequest(CSkypeProto *ppro) :
		AsyncHttpRequest(REQUEST_POST, "/users/ME/endpoints/SELF/subscriptions", &CSkypeProto::OnSubscriptionsCreated)
	{
		AddHeader("Accept", "application/json, text/javascript");
		AddHeader("Content-Type", "application/json; charset=UTF-8");
		AddRegistrationToken(ppro);

		JSONNode interestedResources(JSON_ARRAY); interestedResources.set_name("interestedResources");
		interestedResources
			<< JSONNode("", "/v1/users/ME/conversations/ALL/properties")
			<< JSONNode("", "/v1/users/ME/conversations/ALL/messages")
			<< JSONNode("", "/v1/users/ME/contacts/ALL")
			<< JSONNode("", "/v1/threads/ALL");

		JSONNode node;
		node
			<< JSONNode("channelType", "httpLongPoll")
			<< JSONNode("template", "raw")
			<< interestedResources;

		m_szParam = node.write().c_str();
	}
};

struct CreateContactsSubscriptionRequest : public AsyncHttpRequest
{
	CreateContactsSubscriptionRequest(const LIST<char> &skypenames, CSkypeProto *ppro) :
		AsyncHttpRequest(REQUEST_POST, "/users/ME/contacts")
	{
		AddHeader("Accept", "application/json, text/javascript");
		AddHeader("Content-Type", "application/json; charset=UTF-8");
		AddRegistrationToken(ppro);

		JSONNode node;
		JSONNode contacts(JSON_ARRAY); contacts.set_name("contacts");

		for (auto &it : skypenames) {
			JSONNode contact;
			contact << JSONNode("id", CMStringA(::FORMAT, "8:%s", it));
			contacts << contact;
		}
		node << contacts;

		m_szParam = node.write().c_str();
	}
};

#endif //_SKYPE_REQUEST_SUBSCIPTIONS_H_
