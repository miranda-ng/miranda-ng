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
		
		JSONNODE *node = json_new(5);
		JSONNODE *interestedResources = json_new(4);
		json_set_name(interestedResources, "interestedResources");

		json_push_back(node, json_new_a("channelType", "httpLongPoll"));
		json_push_back(node, json_new_a("template",	   "raw"		 ));

		json_push_back(interestedResources, json_new_a(NULL, "/v1/users/ME/conversations/ALL/properties"));
		json_push_back(interestedResources, json_new_a(NULL, "/v1/users/ME/conversations/ALL/messages"));
		json_push_back(interestedResources, json_new_a(NULL, "/v1/users/ME/contacts/ALL"));
		json_push_back(interestedResources, json_new_a(NULL, "/v1/threads/ALL"));
		json_push_back(node, interestedResources);

		ptrA data(mir_utf8encodeT(ptrT(json_write(node))));
		
		Body << VALUE(data);

		json_delete(node);
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
