/*
Copyright (c) 2015-19 Miranda NG team (https://miranda-ng.org)

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

class CreateSubscriptionsRequest : public HttpRequest
{
public:
	CreateSubscriptionsRequest(CSkypeProto *ppro) :
	  HttpRequest(REQUEST_POST, FORMAT, "%s/v1/users/ME/endpoints/SELF/subscriptions", ppro->m_szServer)
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", ppro->m_szToken)
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8");

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

		Body << VALUE(node.write().c_str());
	}
};

class CreateContactsSubscriptionRequest : public HttpRequest
{
public:
	CreateContactsSubscriptionRequest(const LIST<char> &skypenames, CSkypeProto *ppro) :
	  HttpRequest(REQUEST_POST, FORMAT, "%s/v1/users/ME/contacts", ppro->m_szServer)
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", ppro->m_szToken);


		JSONNode node;
		JSONNode contacts(JSON_ARRAY); contacts.set_name("contacts");

		for (auto &it : skypenames)
		{
			JSONNode contact;
			contact << JSONNode("id", CMStringA(::FORMAT, "8:%s", it));
			contacts << contact;
		}
		node << contacts;

		Body << VALUE(node.write().c_str());
	}
};

#endif //_SKYPE_REQUEST_SUBSCIPTIONS_H_
