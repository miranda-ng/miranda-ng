/*
Copyright (c) 2015 Miranda NG project (http://miranda-ng.org)

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
	CreateSubscriptionsRequest(const char *regToken, const char *server = SKYPE_ENDPOINTS_HOST) :
		HttpRequest(REQUEST_POST, FORMAT, "%s/v1/users/ME/endpoints/SELF/subscriptions", server)
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", regToken)
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8");

		JSONNode interestedResources(JSON_ARRAY);
		interestedResources.set_name("interestedResources");
		interestedResources.push_back(JSONNode("", "/v1/users/ME/conversations/ALL/properties"));
		interestedResources.push_back(JSONNode("", "/v1/users/ME/conversations/ALL/messages"));
		interestedResources.push_back(JSONNode("", "/v1/users/ME/contacts/ALL"));
		interestedResources.push_back(JSONNode("", "/v1/threads/ALL"));

		JSONNode node(JSON_NODE);
		node.push_back(JSONNode("channelType", "httpLongPoll"));
		node.push_back(JSONNode("template", "raw"));
		node.push_back(interestedResources);

		Body << VALUE(node.write().c_str());
	}
};

class CreateContactsSubscriptionRequest : public HttpRequest
{
public:
	CreateContactsSubscriptionRequest(const char *regToken, const LIST<char> &skypenames, const char *server = SKYPE_ENDPOINTS_HOST) :
		HttpRequest(REQUEST_POST, FORMAT, "%s/v1/users/ME/contacts", server)
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", regToken);

		CMStringA data = "{\"contacts\":[";
		for (int i = 0; i < skypenames.getCount(); i++)
			data.AppendFormat("{\"id\":\"8:%s\"},", skypenames[i]);
		data.Truncate(data.GetLength() - 1);
		data.Append("]}");

		Body << VALUE(data);
	}
};

#endif //_SKYPE_REQUEST_SUBSCIPTIONS_H_
