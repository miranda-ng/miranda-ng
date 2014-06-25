/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007-09  Maxim Mluhov
Copyright (c) 2007-09  Victor Pavlychko
Copyright (c) 2012-14  Miranda NG project

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "jabber.h"
#include "jabber_caps.h"

#include "jabber_form2.h"


/////////////////////////////////////////////////////////////////////////////////////////
// FORM_TYPE Registry
namespace NSJabberRegistry
{
	// http://jabber.org/network/serverinfo
	static TJabberDataFormRegisry_Field form_type_serverinfo[] =
	{
		{ _T("abuse-addresses"),		JDFT_LIST_MULTI,		LPGENT("One or more addresses for communication related to abusive traffic") },
		{ _T("feedback-addresses"),		JDFT_LIST_MULTI,		LPGENT("One or more addresses for customer feedback") },
		{ _T("sales-addresses"),		JDFT_LIST_MULTI,		LPGENT("One or more addresses for communication related to sales and marketing") },
		{ _T("security-addresses"),		JDFT_LIST_MULTI,		LPGENT("One or more addresses for communication related to security concerns") },
		{ _T("support-addresses"),		JDFT_LIST_MULTI,		LPGENT("One or more addresses for customer support") },
	};

	// http://jabber.org/protocol/admin
	static TJabberDataFormRegisry_Field form_type_admin[] =
	{
		{ _T("accountjid"),				JDFT_JID_SINGLE,	LPGENT("The Jabber ID of a single entity to which an operation applies") },
		{ _T("accountjids"),			JDFT_JID_MULTI,		LPGENT("The Jabber ID of one or more entities to which an operation applies") },
		{ _T("activeuserjids"),			JDFT_JID_MULTI,		LPGENT("The Jabber IDs associated with active sessions") },
		{ _T("activeusersnum"),			JDFT_TEXT_SINGLE,	LPGENT("The number of online entities that are active") },
		{ _T("adminjids"),				JDFT_JID_MULTI,		LPGENT("A list of entities with administrative privileges") },
		{ _T("announcement"),			JDFT_TEXT_MULTI,	LPGENT("The text of an announcement to be sent to active users or all users") },
		{ _T("blacklistjids"),			JDFT_JID_MULTI,		LPGENT("A list of entities with whom communication is blocked") },
		{ _T("delay"),					JDFT_LIST_MULTI,	LPGENT("The number of seconds to delay before applying a change") },
		{ _T("disableduserjids"),		JDFT_JID_MULTI,		LPGENT("The Jabber IDs that have been disabled") },
		{ _T("disabledusersnum"),		JDFT_TEXT_SINGLE,	LPGENT("The number of disabled entities") },
		{ _T("email"),					JDFT_TEXT_SINGLE,	LPGENT("The email address for a user") },
		{ _T("given_name"),				JDFT_TEXT_SINGLE,	LPGENT("The given (first) name of a user") },
		{ _T("idleusersnum"),			JDFT_TEXT_SINGLE,	LPGENT("The number of online entities that are idle") },
		{ _T("ipaddresses"),			JDFT_LIST_MULTI,	LPGENT("The IP addresses of an account's online sessions") },
		{ _T("lastlogin"),				JDFT_TEXT_SINGLE,	LPGENT("The last login time (per XEP-0082) of a user") },
		{ _T("loginsperminute"),		JDFT_TEXT_SINGLE,	LPGENT("The number of logins per minute for an account") },
		{ _T("max_items"),				JDFT_LIST_SINGLE,	LPGENT("The maximum number of items associated with a search or list") },
		{ _T("motd"),					JDFT_TEXT_MULTI,	LPGENT("The text of a message of the day") },
		{ _T("onlineresources"),		JDFT_TEXT_SINGLE,	LPGENT("The names of an account's online sessions") },
		{ _T("onlineuserjids"),			JDFT_JID_MULTI,		LPGENT("The Jabber IDs associated with online users") },
		{ _T("onlineusersnum"),			JDFT_TEXT_SINGLE,	LPGENT("The number of online entities") },
		{ _T("password"),				JDFT_TEXT_PRIVATE,	LPGENT("The password for an account") },
		{ _T("password-verify"),		JDFT_TEXT_PRIVATE,	LPGENT("Password verification") },
		{ _T("registereduserjids"),		JDFT_JID_MULTI,		LPGENT("A list of registered entities") },
		{ _T("registeredusersnum"),		JDFT_TEXT_SINGLE,	LPGENT("The number of registered entities") },
		{ _T("rostersize"),				JDFT_TEXT_SINGLE,	LPGENT("Number of roster items for an account") },
		{ _T("stanzaspersecond"),		JDFT_TEXT_SINGLE,	LPGENT("The number of stanzas being sent per second by an account") },
		{ _T("surname"),				JDFT_TEXT_SINGLE,	LPGENT("The family (last) name of a user") },
		{ _T("welcome"),				JDFT_TEXT_MULTI,	LPGENT("The text of a welcome message") },
		{ _T("whitelistjids"),			JDFT_JID_MULTI,		LPGENT("A list of entities with whom communication is allowed") },
	};

	// http://jabber.org/protocol/muc#register
	static TJabberDataFormRegisry_Field form_type_muc_register[] =
	{
		{ _T("muc#register_first"),		JDFT_TEXT_SINGLE,	LPGENT("First Name") },
		{ _T("muc#register_last"),		JDFT_TEXT_SINGLE,	LPGENT("Last Name") },
		{ _T("muc#register_roomnick"),	JDFT_TEXT_SINGLE,	LPGENT("Desired Nickname") },
		{ _T("muc#register_url"),		JDFT_TEXT_SINGLE,	LPGENT("Your URL") },
		{ _T("muc#register_email"),		JDFT_TEXT_SINGLE,	LPGENT("Email Address") },
		{ _T("muc#register_faqentry"),	JDFT_TEXT_MULTI,	LPGENT("FAQ Entry") },
	};

	// http://jabber.org/protocol/muc#roomconfig
	static TJabberDataFormRegisry_Field form_type_muc_roomconfig[] =
	{
		{ _T("muc#roomconfig_allowinvites"),			JDFT_BOOLEAN,		LPGENT("Whether to allow occupants to invite others") },
		{ _T("muc#roomconfig_changesubject"),			JDFT_BOOLEAN,		LPGENT("Whether to allow occupants to change subject") },
		{ _T("muc#roomconfig_enablelogging"),			JDFT_BOOLEAN,		LPGENT("Whether to enable logging of room conversations") },
		{ _T("muc#roomconfig_lang"),					JDFT_TEXT_SINGLE,	LPGENT("Natural language for room discussions") },
		{ _T("muc#roomconfig_maxusers"),				JDFT_LIST_SINGLE,	LPGENT("Maximum number of room occupants") },
		{ _T("muc#roomconfig_membersonly"),				JDFT_BOOLEAN,		LPGENT("Whether to make room members-only") },
		{ _T("muc#roomconfig_moderatedroom"),			JDFT_BOOLEAN,		LPGENT("Whether to make room moderated") },
		{ _T("muc#roomconfig_passwordprotectedroom"),	JDFT_BOOLEAN,		LPGENT("Whether a password is required to enter") },
		{ _T("muc#roomconfig_persistentroom"),			JDFT_BOOLEAN,		LPGENT("Whether to make room persistent") },
		{ _T("muc#roomconfig_presencebroadcast"),		JDFT_LIST_MULTI,	LPGENT("Roles for which presence is broadcast") },
		{ _T("muc#roomconfig_publicroom"),				JDFT_BOOLEAN,		LPGENT("Whether to allow public searching for room") },
		{ _T("muc#roomconfig_roomadmins"),				JDFT_JID_MULTI,		LPGENT("Full list of room admins") },
		{ _T("muc#roomconfig_roomdesc"),				JDFT_TEXT_SINGLE,	LPGENT("Short description of room") },
		{ _T("muc#roomconfig_roomname"),				JDFT_TEXT_SINGLE,	LPGENT("Natural-language room name") },
		{ _T("muc#roomconfig_roomowners"),				JDFT_JID_MULTI,		LPGENT("Full list of room owners") },
		{ _T("muc#roomconfig_roomsecret"),				JDFT_TEXT_PRIVATE,	LPGENT("The room password") },
		{ _T("muc#roomconfig_whois"),					JDFT_LIST_SINGLE,	LPGENT("Affiliations that may discover real JIDs of occupants") },
	};

	// http://jabber.org/protocol/pubsub#publish-options
	static TJabberDataFormRegisry_Field form_type_publish_options[] =
	{
		{ _T("pubsub#access_model"),	JDFT_LIST_SINGLE,	LPGENT("Precondition: node configuration with the specified access model") },
	};

	// http://jabber.org/protocol/pubsub#subscribe_authorization
	static TJabberDataFormRegisry_Field form_type_subscribe_auth[] =
	{
		{ _T("pubsub#allow"),			JDFT_BOOLEAN,		LPGENT("Whether to allow the subscription") },
		{ _T("pubsub#subid"),			JDFT_TEXT_SINGLE,	LPGENT("The SubID of the subscription") },
		{ _T("pubsub#node"),			JDFT_TEXT_SINGLE,	LPGENT("The NodeID of the relevant node") },
		{ _T("pubsub#subscriber_jid"),	JDFT_JID_SINGLE,	LPGENT("The address (JID) of the subscriber") },
	};

	// http://jabber.org/protocol/pubsub#subscribe_options
	static TJabberDataFormRegisry_Field form_type_subscribe_options[] =
	{
		{ _T("pubsub#deliver"),				JDFT_BOOLEAN,		LPGENT("Whether an entity wants to receive or disable notifications") },
		{ _T("pubsub#digest"),				JDFT_BOOLEAN,		LPGENT("Whether an entity wants to receive digests (aggregations) of notifications or all notifications individually") },
		{ _T("pubsub#digest_frequency"),	JDFT_TEXT_SINGLE,	LPGENT("The minimum number of milliseconds between sending any two notification digests") },
		{ _T("pubsub#expire"),				JDFT_TEXT_SINGLE,	LPGENT("The date and time at which a leased subscription will end or has ended") },
		{ _T("pubsub#include_body"),		JDFT_BOOLEAN,		LPGENT("Whether an entity wants to receive an XMPP message body in addition to the payload format") },
		{ _T("pubsub#show-values"),			JDFT_LIST_MULTI,	LPGENT("The presence states for which an entity wants to receive notifications") },
		{ _T("pubsub#subscription_type"),	JDFT_LIST_SINGLE,	_T("") },
		{ _T("pubsub#subscription_depth"),	JDFT_LIST_SINGLE,	_T("") },
	};

	// http://jabber.org/protocol/pubsub#node_config
	static TJabberDataFormRegisry_Field form_type_node_config[] =
	{
		{ _T("pubsub#access_model"),					JDFT_LIST_SINGLE,	LPGENT("Who may subscribe and retrieve items") },
		{ _T("pubsub#body_xslt"),						JDFT_TEXT_SINGLE,	LPGENT("The URL of an XSL transformation which can be applied to payloads in order to generate an appropriate message body element.") },
		{ _T("pubsub#collection"),						JDFT_TEXT_SINGLE,	LPGENT("The collection with which a node is affiliated") },
		{ _T("pubsub#dataform_xslt"),					JDFT_TEXT_SINGLE,	LPGENT("The URL of an XSL transformation which can be applied to the payload format in order to generate a valid Data Forms result that the client could display using a generic Data Forms rendering engine") },
		{ _T("pubsub#deliver_payloads"),				JDFT_BOOLEAN,		LPGENT("Whether to deliver payloads with event notifications") },
		{ _T("pubsub#itemreply"),						JDFT_LIST_SINGLE,	LPGENT("Whether owners or publisher should receive replies to items") },
		{ _T("pubsub#children_association_policy"),		JDFT_LIST_SINGLE,	LPGENT("Who may associate leaf nodes with a collection") },
		{ _T("pubsub#children_association_whitelist"),	JDFT_JID_MULTI,		LPGENT("The list of JIDs that may associated leaf nodes with a collection") },
		{ _T("pubsub#children"),						JDFT_TEXT_MULTI,	LPGENT("The child nodes (leaf or collection) associated with a collection") },
		{ _T("pubsub#children_max"),					JDFT_TEXT_SINGLE,	LPGENT("The maximum number of child nodes that can be associated with a collection") },
		{ _T("pubsub#max_items"),						JDFT_TEXT_SINGLE,	LPGENT("The maximum number of items to persist") },
		{ _T("pubsub#max_payload_size"),				JDFT_TEXT_SINGLE,	LPGENT("The maximum payload size in bytes") },
		{ _T("pubsub#node_type"),						JDFT_LIST_SINGLE,	LPGENT("Whether the node is a leaf (default) or a collection") },
		{ _T("pubsub#notify_config"),					JDFT_BOOLEAN,		LPGENT("Whether to notify subscribers when the node configuration changes") },
		{ _T("pubsub#notify_delete"),					JDFT_BOOLEAN,		LPGENT("Whether to notify subscribers when the node is deleted") },
		{ _T("pubsub#notify_retract"),					JDFT_BOOLEAN,		LPGENT("Whether to notify subscribers when items are removed from the node") },
		{ _T("pubsub#persist_items"),					JDFT_BOOLEAN,		LPGENT("Whether to persist items to storage") },
		{ _T("pubsub#presence_based_delivery"),			JDFT_BOOLEAN,		LPGENT("Whether to deliver notifications to available users only") },
		{ _T("pubsub#publish_model"),					JDFT_LIST_SINGLE,	LPGENT("The publisher model") },
		{ _T("pubsub#replyroom"),						JDFT_JID_MULTI,		LPGENT("The specific multi-user chat rooms to specify for replyroom") },
		{ _T("pubsub#replyto"),							JDFT_JID_MULTI,		LPGENT("The specific JID(s) to specify for replyto") },
		{ _T("pubsub#roster_groups_allowed"),			JDFT_LIST_MULTI,	LPGENT("The roster group(s) allowed to subscribe and retrieve items") },
		{ _T("pubsub#send_item_subscribe"),				JDFT_BOOLEAN,		LPGENT("Whether to send items to new subscribers") },
		{ _T("pubsub#subscribe"),						JDFT_BOOLEAN,		LPGENT("Whether to allow subscriptions") },
		{ _T("pubsub#title"),							JDFT_TEXT_SINGLE,	LPGENT("A friendly name for the node") },
		{ _T("pubsub#type"),							JDFT_TEXT_SINGLE,	LPGENT("The type of node data, usually specified by the namespace of the payload (if any); MAY be list-single rather than text-single") },
	};

	// http://jabber.org/protocol/pubsub#meta-data
	static TJabberDataFormRegisry_Field form_type_metadata[] =
	{
		{ _T("pubsub#contact"),			JDFT_JID_MULTI,		LPGENT("The JIDs of those to contact with questions") },
		{ _T("pubsub#creation_date"),	JDFT_TEXT_SINGLE,	LPGENT("The date and time when the node was created") },
		{ _T("pubsub#creator"),			JDFT_JID_SINGLE,	LPGENT("The JID of the node creator") },
		{ _T("pubsub#description"),		JDFT_TEXT_SINGLE,	LPGENT("A description of the node") },
		{ _T("pubsub#language"),		JDFT_TEXT_SINGLE,	LPGENT("The default language of the node") },
		{ _T("pubsub#num_subscribers"),	JDFT_TEXT_SINGLE,	LPGENT("The number of subscribers to the node") },
		{ _T("pubsub#owner"),			JDFT_JID_MULTI,		LPGENT("The JIDs of those with an affiliation of owner") },
		{ _T("pubsub#publisher"),		JDFT_JID_MULTI,		LPGENT("The JIDs of those with an affiliation of publisher") },
		{ _T("pubsub#title"),			JDFT_TEXT_SINGLE,	LPGENT("The name of the node") },
		{ _T("pubsub#type"),			JDFT_TEXT_SINGLE,	LPGENT("Payload type") },
	};

	// http://jabber.org/protocol/rc
	static TJabberDataFormRegisry_Field form_type_rc[] =
	{
		{ _T("auto-auth"),			JDFT_BOOLEAN,		LPGENT("Whether to automatically authorize subscription requests") },
		{ _T("auto-files"),			JDFT_BOOLEAN,		LPGENT("Whether to automatically accept file transfers") },
		{ _T("auto-msg"),			JDFT_BOOLEAN,		LPGENT("Whether to automatically open new messages") },
		{ _T("auto-offline"),		JDFT_BOOLEAN,		LPGENT("Whether to automatically go offline when idle") },
		{ _T("sounds"),				JDFT_BOOLEAN,		LPGENT("Whether to play sounds") },
		{ _T("files"),				JDFT_LIST_MULTI,	LPGENT("A list of pending file transfers") },
		{ _T("groupchats"),			JDFT_LIST_MULTI,	LPGENT("A list of joined group chat rooms") },
		{ _T("status"),				JDFT_LIST_SINGLE,	LPGENT("A presence or availability status") },
		{ _T("status-message"),		JDFT_TEXT_MULTI,	LPGENT("The status message text") },
		{ _T("status-priority"),	JDFT_TEXT_SINGLE,	LPGENT("The new priority for the client") },
	};

	// jabber:iq:register
	static TJabberDataFormRegisry_Field form_type_register[] =
	{
		{ _T("username"),	JDFT_TEXT_SINGLE,	LPGENT("Account name associated with the user") },
		{ _T("nick"),		JDFT_TEXT_SINGLE,	LPGENT("Familiar name of the user") },
		{ _T("password"),	JDFT_TEXT_PRIVATE,	LPGENT("Password or secret for the user") },
		{ _T("name"),		JDFT_TEXT_SINGLE,	LPGENT("Full name of the user") },
		{ _T("first"),		JDFT_TEXT_SINGLE,	LPGENT("First name or given name of the user") },
		{ _T("last"),		JDFT_TEXT_SINGLE,	LPGENT("Last name, surname, or family name of the user") },
		{ _T("email"),		JDFT_TEXT_SINGLE,	LPGENT("Email address of the user") },
		{ _T("address"),	JDFT_TEXT_SINGLE,	LPGENT("Street portion of a physical or mailing address") },
		{ _T("city"),		JDFT_TEXT_SINGLE,	LPGENT("Locality portion of a physical or mailing address") },
		{ _T("state"),		JDFT_TEXT_SINGLE,	LPGENT("Region portion of a physical or mailing address") },
		{ _T("zip"),		JDFT_TEXT_SINGLE,	LPGENT("Postal code portion of a physical or mailing address") },
	};

	// jabber:iq:search
	static TJabberDataFormRegisry_Field form_type_search[] =
	{
		{ _T("first"),	JDFT_TEXT_SINGLE,	LPGENT("First Name") },
		{ _T("last"),	JDFT_TEXT_SINGLE,	LPGENT("Family Name") },
		{ _T("nick"),	JDFT_TEXT_SINGLE,	LPGENT("Nickname") },
		{ _T("email"),	JDFT_TEXT_SINGLE,	LPGENT("Email Address") },
	};

	// urn:xmpp:ssn
	static TJabberDataFormRegisry_Field form_type_ssn[] =
	{
		{ _T("accept"),									JDFT_BOOLEAN,		LPGENT("Whether to accept the invitation") },
		{ _T("continue"),								JDFT_TEXT_SINGLE,	LPGENT("Another resource with which to continue the session") },
		{ _T("disclosure"),								JDFT_LIST_SINGLE,	LPGENT("Disclosure of content, decryption keys or identities") },
		{ _T("http://jabber.org/protocol/chatstates"),	JDFT_LIST_SINGLE,	LPGENT("Whether may send Chat State Notifications per XEP-0085") },
		{ _T("http://jabber.org/protocol/xhtml-im"),	JDFT_LIST_SINGLE,	LPGENT("Whether allowed to use XHTML-IM formatting per XEP-0071") },
		{ _T("language"),								JDFT_LIST_SINGLE,	LPGENT("Primary written language of the chat (each value appears in order of preference and conforms to RFC 4646 and the IANA registry)") },
		{ _T("logging"),								JDFT_LIST_SINGLE,	LPGENT("Whether allowed to log messages (i.e., whether Off-The-Record mode is required)") },
		{ _T("renegotiate"),							JDFT_BOOLEAN,		LPGENT("Whether to renegotiate the session") },
		{ _T("security"),								JDFT_LIST_SINGLE,	LPGENT("Minimum security level") },
		{ _T("terminate"),								JDFT_BOOLEAN,		LPGENT("Whether to terminate the session") },
		{ _T("urn:xmpp:receipts"),						JDFT_BOOLEAN,		LPGENT("Whether to enable Message Receipts per XEP-0184") },
	};

	TJabberDataFormRegisry_Form form_types[] =
	{
	/*0157*/	{ _T("http://jabber.org/network/serverinfo"),						form_type_serverinfo,			SIZEOF(form_type_serverinfo) },
	/*0133*/	{ _T("http://jabber.org/protocol/admin"),							form_type_admin,				SIZEOF(form_type_admin) },
	/*0045*/	{ _T("http://jabber.org/protocol/muc#register"),					form_type_muc_register,			SIZEOF(form_type_muc_register) },
	/*0045*/	{ _T("http://jabber.org/protocol/muc#roomconfig"),					form_type_muc_roomconfig,		SIZEOF(form_type_muc_roomconfig) },
	/*0060*/	{ _T("http://jabber.org/protocol/pubsub#publish-options"),			form_type_publish_options,		SIZEOF(form_type_publish_options) },
	/*0060*/	{ _T("http://jabber.org/protocol/pubsub#subscribe_authorization"),	form_type_subscribe_auth,		SIZEOF(form_type_subscribe_auth) },
	/*0060*/	{ _T("http://jabber.org/protocol/pubsub#subscribe_options"),		form_type_subscribe_options,	SIZEOF(form_type_subscribe_options) },
	/*0060*/	{ _T("http://jabber.org/protocol/pubsub#node_config"),				form_type_node_config,			SIZEOF(form_type_node_config) },
	/*0060*/	{ _T("http://jabber.org/protocol/pubsub#meta-data"),				form_type_metadata,				SIZEOF(form_type_metadata) },
	/*0146*/	{ _T("http://jabber.org/protocol/rc"),								form_type_rc,					SIZEOF(form_type_rc) },
	/*0077*/	{ _T("jabber:iq:register"),											form_type_register,				SIZEOF(form_type_register) },
	/*0055*/	{ _T("jabber:iq:search"),											form_type_search,				SIZEOF(form_type_search) },
	/*0155*/	{ _T("urn:xmpp:ssn"),												form_type_ssn,					SIZEOF(form_type_ssn) },
	};
};

CJabberDataField::CJabberDataField(CJabberDataForm *form, XmlNode *node):
	m_node(node), m_options(5), m_values(1), m_descriptions(1)
{
	m_typeName = JabberXmlGetAttrValue(m_node, "type");
	m_var = JabberXmlGetAttrValue(m_node, "var");
	m_label = JabberXmlGetAttrValue(m_node, "label");
	m_required = JabberXmlGetChild(m_node, "required") ? true : false;

	if (m_typeName)
	{
		if (!lstrcmp(m_typeName, _T("text-private")))
			m_type = JDFT_TEXT_PRIVATE;
		else if (!lstrcmp(m_typeName, _T("text-multi")) || !lstrcmp(m_typeName, _T("jid-multi")))
			m_type = JDFT_TEXT_MULTI;
		else if (!lstrcmp(m_typeName, _T("boolean")))
			m_type = JDFT_BOOLEAN;
		else if (!lstrcmp(m_typeName, _T("list-single")))
			m_type = JDFT_LIST_SINGLE;
		else if (!lstrcmp(m_typeName, _T("list-multi")))
			m_type = JDFT_LIST_MULTI;
		else if (!lstrcmp(m_typeName, _T("fixed")))
			m_type = JDFT_FIXED;
		else if (!lstrcmp(m_typeName, _T("hidden")))
			m_type = JDFT_HIDDEN;
		else
			m_type = JDFT_TEXT_SINGLE;
	} else
	{
		m_typeName = _T("text-single");
		m_type = JDFT_TEXT_SINGLE;
	}

	for (int i = 0; i < m_node->numChild; ++i)
	{
		if (!lstrcmpA(m_node->child[i]->name, "value"))
		{
			m_values.insert(m_node->child[i]->text, m_values.getCount());
		} else
		if (!lstrcmpA(m_node->child[i]->name, "option"))
		{
			TOption *opt = new TOption;
			opt->label = JabberXmlGetAttrValue(m_node->child[i], "label");
			opt->value = NULL;
			if (XmlNode *p = JabberXmlGetChild(m_node->child[i], "value"))
				opt->value = p->text;
			m_options.insert(opt, m_options.getCount());
		} else
		if (!lstrcmpA(m_node->child[i]->name, "desc"))
		{
			m_descriptions.insert(m_node->child[i]->text, m_descriptions.getCount());
		}
	}
}

CJabberDataField::~CJabberDataField()
{
	m_values.destroy();
	m_descriptions.destroy();
}

CJabberDataFieldSet::CJabberDataFieldSet():
	m_fields(5)
{
}

CJabberDataField *CJabberDataFieldSet::GetField(TCHAR *var)
{
	for (int i = 0; i < m_fields.getCount(); ++i)
		if (!lstrcmp(m_fields[i].GetVar(), var))
			return &(m_fields[i]);
	return NULL;
}

CJabberDataForm::CJabberDataForm(XmlNode *node):
	m_node(node),
	m_form_type(0),
	m_form_type_info(0),
	m_title(0),
	m_instructions(1),
	m_items(1)
{
	m_typename = JabberXmlGetAttrValue(m_node, "type");

	for (int i = 0; i < m_node->numChild; ++i)
	{
		XmlNode *child = m_node->child[i];
		if (!lstrcmpA(child->name, "field"))
		{
			CJabberDataField *field = new CJabberDataField(this, child);
			m_fields.AddField(field);

			if ((field->GetType() == JDFT_HIDDEN) && !lstrcmp(field->GetVar(), _T("FORM_TYPE")))
			{
				using NSJabberRegistry::form_types;

				m_form_type = field->GetValue();
				for (int j = 0; j < SIZEOF(form_types); ++j)
					if (!lstrcmp(m_form_type, form_types[j].form_type))
					{
						m_form_type_info = form_types + j;
						break;
					}
			}
		} else
		if (!lstrcmpA(child->name, "title"))
		{
			m_title = child->text;
		} else
		if (!lstrcmpA(child->name, "instructions"))
		{
			m_instructions.insert(child->text, m_instructions.getCount());
		} else
		if (!lstrcmpA(child->name, "reported"))
		{
			if (m_reported.GetCount())
				continue;	// ignore second <reported/> -> error!!!!!!!!!!!

			for (int j = 0; j < child->numChild; ++j)
			{
				XmlNode *child2 = child->child[i];
				if (!lstrcmpA(child2->name, "field"))
				{
					CJabberDataField *field = new CJabberDataField(this, child2);
					m_reported.AddField(field);
				}
			}
		} else
		if (!lstrcmpA(child->name, "item"))
		{
			CJabberDataFieldSet *item = new CJabberDataFieldSet;
			m_items.insert(item);

			for (int j = 0; j < child->numChild; ++j)
			{
				XmlNode *child2 = child->child[i];
				if (!lstrcmpA(child2->name, "field"))
				{
					CJabberDataField *field = new CJabberDataField(this, child2);
					item->AddField(field);
				}
			}
		}
	}
}

CJabberDataForm::~CJabberDataForm()
{
}


/////////////////////////////////////////////////////////////////////////////////////////
// UI classes

#define FORM_CONTROL_MINWIDTH	100
#define FORM_CONTROL_HEIGHT		20

class CFormCtrlBase;

class CJabberDlgDataPage
{
public:
	CJabberDlgDataPage(HWND hwndParent);
	~CJabberDlgDataPage();

	void AddField(CJabberDataField *field);
	XmlNode *FetchData();

	HWND GetHwnd() { return m_hwnd; }
	void Layout();

	// internal usage
	int AddControl(CFormCtrlBase *ctrl)
	{
		m_controls.insert(ctrl, m_controls.getCount());
		return m_controls.getCount();
	}

private:
	HWND m_hwnd;
	OBJLIST<CFormCtrlBase> m_controls;
	int m_scrollPos, m_height, m_dataHeight;

	BOOL DlgProc(UINT msg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK DlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

class CFormCtrlBase
{
public:
	CFormCtrlBase(CJabberDlgDataPage *parent, CJabberDataField *field):
		m_field(field), m_parent(parent),
		m_hwnd(NULL), m_hwndLabel(NULL)
	{
	}

	HWND GetHwnd() { return m_hwnd; }
	void Init();

	int Layout(HDWP hdwp, int x, int y, int w);
	virtual XmlNode *FetchData() = 0;

protected:
	int m_id;
	HWND m_hwnd, m_hwndLabel;
	CJabberDataField *m_field;
	CJabberDlgDataPage *m_parent;

	virtual void CreateControl() = 0;
	virtual int GetHeight(int width) = 0;
	SIZE GetControlTextSize(HWND hwnd, int w);

	void CreateLabel();
	void SetupFont();
	XmlNode *CreateNode();
};

void CFormCtrlBase::Init()
{
	m_id = m_parent->AddControl(this);
	CreateControl();
	SetupFont();
}

SIZE CFormCtrlBase::GetControlTextSize(HWND hwnd, int w)
{
	int length = GetWindowTextLength(hwnd) + 1;
	TCHAR *text = (TCHAR *)mir_alloc(sizeof(TCHAR) * length);
	GetWindowText(hwnd, text, length);

	RECT rc;
	SetRect(&rc, 0, 0, w, 0);
	HDC hdc = GetDC(hwnd);
	HFONT hfntSave = (HFONT)SelectObject(hdc, (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0));
	DrawText(hdc, text, -1, &rc, DT_CALCRECT|DT_WORDBREAK);
	SelectObject(hdc, hfntSave);
	ReleaseDC(hwnd, hdc);

	mir_free(text);

	SIZE res = { rc.right, rc.bottom };
	return res;
}

int CFormCtrlBase::Layout(HDWP hdwp, int x, int y, int w)
{
	SIZE szLabel = {0}, szCtrl = {0};
	int h = 0;

	if (m_hwndLabel)
	{
		SIZE szLabel = GetControlTextSize(m_hwndLabel, w);

		szCtrl.cx = w - szLabel.cx - 5;
		szCtrl.cy = GetHeight(szCtrl.cx);
		if ((szCtrl.cx >= FORM_CONTROL_MINWIDTH) && (szCtrl.cy <= FORM_CONTROL_HEIGHT))
		{
			DeferWindowPos(hdwp, m_hwndLabel, NULL, x, y + (szCtrl.cy - szLabel.cy) / 2, szLabel.cx, szLabel.cy, SWP_NOZORDER|SWP_SHOWWINDOW);
			DeferWindowPos(hdwp, m_hwnd, NULL, x + szLabel.cx + 5, y, szCtrl.cx, szCtrl.cy, SWP_NOZORDER|SWP_SHOWWINDOW);

			h = szCtrl.cy;
		} else
		{
			szCtrl.cx = w - 10;
			szCtrl.cy = GetHeight(szCtrl.cx);

			DeferWindowPos(hdwp, m_hwndLabel, NULL, x, y, szLabel.cx, szLabel.cy, SWP_NOZORDER|SWP_SHOWWINDOW);
			DeferWindowPos(hdwp, m_hwnd, NULL, x + 10, y + szLabel.cy + 2, szCtrl.cx, szCtrl.cy, SWP_NOZORDER|SWP_SHOWWINDOW);

			h = szLabel.cy + 2 + szCtrl.cy;
		}

	} else
	{
		h = GetHeight(w);
		DeferWindowPos(hdwp, m_hwnd, NULL, x, y, w, h, SWP_NOZORDER|SWP_SHOWWINDOW);
	}

	return h;
}

void CFormCtrlBase::CreateLabel()
{
	if (m_field->GetLabel())
	{
		m_hwndLabel = CreateWindow(_T("static"), m_field->GetLabel(),
			WS_CHILD|WS_VISIBLE/*|SS_CENTERIMAGE*/,
			0, 0, 0, 0,
			m_parent->GetHwnd(), (HMENU)-1, hInst, NULL);
	}
}

void CFormCtrlBase::SetupFont()
{
	if (m_hwnd)
	{
		HFONT hFont = (HFONT)SendMessage(GetParent(m_hwnd), WM_GETFONT, 0, 0);
		if (m_hwnd) SendMessage(m_hwnd, WM_SETFONT, (WPARAM)hFont, 0);
		if (m_hwndLabel) SendMessage(m_hwndLabel, WM_SETFONT, (WPARAM) hFont, 0);
	}
}

XmlNode *CFormCtrlBase::CreateNode()
{
	XmlNode* field = new XmlNode("field");
	field->addAttr("var", m_field->GetVar());
	field->addAttr("type", m_field->GetTypeName());
	return field;
}

class CFormCtrlBoolean: public CFormCtrlBase
{
public:
	CFormCtrlBoolean(CJabberDlgDataPage *parent, CJabberDataField *field): CFormCtrlBase(parent, field) {}

	void CreateControl()
	{
		m_hwnd = CreateWindowEx(0, _T("button"), m_field->GetLabel(),
			WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_AUTOCHECKBOX|BS_MULTILINE,
			0, 0, 0, 0,
			m_parent->GetHwnd(), (HMENU)m_id, hInst, NULL);
		if (m_field->GetValue() && !_tcscmp(m_field->GetValue(), _T("1")))
			SendMessage(m_hwnd, BM_SETCHECK, 1, 0);
	}

	int GetHeight(int width)
	{
		return GetControlTextSize(m_hwnd, width - 20).cy;
	}

	XmlNode *FetchData()
	{
		XmlNode *field = CreateNode();
		field->addChild("value", (SendMessage(m_hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED) ? _T("1") : _T("0"));
		return field;
	}
};

class CFormCtrlFixed: public CFormCtrlBase
{
public:
	CFormCtrlFixed(CJabberDlgDataPage *parent, CJabberDataField *field): CFormCtrlBase(parent, field) {}

	void CreateControl()
	{
		CreateLabel();
		m_hwnd = CreateWindow(_T("edit"), m_field->GetValue(),
			WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_READONLY,
			0, 0, 0, 0,
			m_parent->GetHwnd(), (HMENU)-1, hInst, NULL);
	}

	int GetHeight(int width)
	{
		return GetControlTextSize(m_hwnd, width - 2).cy;
	}

	XmlNode *FetchData()
	{
		XmlNode *field = CreateNode();
		for (int i = 0; i < m_field->GetValueCount(); ++i)
			field->addChild("value", m_field->GetValue(i));
		return field;
	}
};

class CFormCtrlHidden: public CFormCtrlBase
{
public:
	CFormCtrlHidden(CJabberDlgDataPage *parent, CJabberDataField *field): CFormCtrlBase(parent, field) {}

	void CreateControl()
	{
	}

	int GetHeight(int width)
	{
		return 0;
	}

	XmlNode *FetchData()
	{
		XmlNode *field = CreateNode();
		for (int i = 0; i < m_field->GetValueCount(); ++i)
			field->addChild("value", m_field->GetValue(i));
		return field;
	}
};
/*
class CFormCtrlJidMulti: public CFormCtrlBase
{
public:
	CFormCtrlJidMulti(CJabberDlgDataPage *parent, CJabberDataField *field): CFormCtrlBase(parent, field) {}

	void CreateControl()
	{
	}

	int GetHeight(int width)
	{
		return 20;
	}

	XmlNode *FetchData()
	{
		return NULL;
	}
};

class CFormCtrlJidSingle: public CFormCtrlBase
{
public:
	CFormCtrlJidSingle(CJabberDlgDataPage *parent, CJabberDataField *field): CFormCtrlBase(parent, field) {}

	void CreateControl()
	{
	}

	int GetHeight(int width)
	{
		return 20;
	}

	XmlNode *FetchData()
	{
		return NULL;
	}
};
*/
class CFormCtrlListMulti: public CFormCtrlBase
{
public:
	CFormCtrlListMulti(CJabberDlgDataPage *parent, CJabberDataField *field): CFormCtrlBase(parent, field) {}

	void CreateControl()
	{
		CreateLabel();
		m_hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, _T("listbox"),
			NULL, WS_CHILD|WS_VISIBLE|WS_TABSTOP|LBS_MULTIPLESEL,
			0, 0, 0, 0,
			m_parent->GetHwnd(), (HMENU)m_id, hInst, NULL);

		for (int i = 0; i < m_field->GetOptionCount(); ++i)
		{
			DWORD dwIndex = SendMessage(m_hwnd, LB_ADDSTRING, 0, (LPARAM)m_field->GetOption(i)->label);
			SendMessage(m_hwnd, LB_SETITEMDATA, dwIndex, (LPARAM)m_field->GetOption(i)->value);
			for (int j = 0; j < m_field->GetValueCount(); ++j)
				if (!lstrcmp_null(m_field->GetValue(j), m_field->GetOption(i)->value))
				{
					SendMessage(m_hwnd, LB_SETSEL, TRUE, dwIndex);
					break;
				}
		}
	}

	int GetHeight(int width)
	{
		return 20 * 3;
	}

	XmlNode *FetchData()
	{
		XmlNode *field = CreateNode();
		int count = SendMessage(m_hwnd, LB_GETCOUNT, 0, 0);
		for (int i = 0; i < count; ++i)
			if (SendMessage(m_hwnd, LB_GETSEL, i, 0) > 0)
				field->addChild("value", (TCHAR *)SendMessage(m_hwnd, LB_GETITEMDATA, i, 0));
		return field;
	}
};

class CFormCtrlListSingle: public CFormCtrlBase
{
public:
	CFormCtrlListSingle(CJabberDlgDataPage *parent, CJabberDataField *field): CFormCtrlBase(parent, field) {}

	void CreateControl()
	{
		CreateLabel();
		m_hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, _T("combobox"), NULL,
			WS_CHILD|WS_VISIBLE|WS_TABSTOP|CBS_DROPDOWNLIST,
			0, 0, 0, 0,
			m_parent->GetHwnd(), (HMENU)m_id, hInst, NULL);

		for (int i = 0; i < m_field->GetOptionCount(); ++i)
		{
			DWORD dwIndex = SendMessage(m_hwnd, CB_ADDSTRING, 0, (LPARAM)m_field->GetOption(i)->label);
			SendMessage(m_hwnd, CB_SETITEMDATA, dwIndex, (LPARAM)m_field->GetOption(i)->value);
			if (!lstrcmp_null(m_field->GetValue(), m_field->GetOption(i)->value))
				SendMessage(m_hwnd, CB_SETCURSEL, dwIndex, 0);
		}
	}

	int GetHeight(int width)
	{
		return 20;
	}

	XmlNode *FetchData()
	{
		XmlNode *field = CreateNode();
		int sel = SendMessage(m_hwnd, CB_GETCURSEL, 0, 0);
		if (sel != CB_ERR)
			field->addChild("value", (TCHAR *)SendMessage(m_hwnd, CB_GETITEMDATA, sel, 0));
		return field;
	}
};

class CFormCtrlTextMulti: public CFormCtrlBase
{
public:
	CFormCtrlTextMulti(CJabberDlgDataPage *parent, CJabberDataField *field): CFormCtrlBase(parent, field) {}

	void CreateControl()
	{
		CreateLabel();
		int i, length = 1;
		for (i = 0; i < m_field->GetValueCount(); ++i)
			length += lstrlen(m_field->GetValue(i)) + 2;

		TCHAR *str = (TCHAR *)mir_alloc(sizeof(TCHAR) * length);
		*str = 0;
		for (i = 0; i < m_field->GetValueCount(); ++i)
		{
			if (i) lstrcat(str, _T("\r\n"));
			lstrcat(str, m_field->GetValue(i));
		}

		m_hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, _T("edit"), str,
			WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_VSCROLL|ES_LEFT|ES_MULTILINE|ES_AUTOVSCROLL|ES_WANTRETURN,
			0, 0, 0, 0,
			m_parent->GetHwnd(), (HMENU)m_id, hInst, NULL);
	//	WNDPROC oldWndProc = (WNDPROC)SetWindowLongPtr(item->hCtrl, GWL_WNDPROC, (LPARAM)JabberFormMultiLineWndProc);
	//	SetWindowLongPtr(item->hCtrl, GWL_USERDATA, (LONG) oldWndProc);

		mir_free(str);
	}

	int GetHeight(int width)
	{
		return 20 * 3;
	}

	XmlNode *FetchData()
	{
		XmlNode *field = CreateNode();
		int len = GetWindowTextLength(m_hwnd);
		TCHAR *str = (TCHAR *)mir_alloc(sizeof(TCHAR) * (len+1));
		GetWindowText(m_hwnd, str, len+1);
		TCHAR *p = str;
		while (p != NULL)
		{
			TCHAR *q = _tcsstr( p, _T("\r\n"));
			if (q) *q = '\0';
			field->addChild("value", p);
			p = q ? q+2 : NULL;
		}
		mir_free(str);
		return field;
	}
};

class CFormCtrlTextSingle: public CFormCtrlBase
{
public:
	CFormCtrlTextSingle(CJabberDlgDataPage *parent, CJabberDataField *field): CFormCtrlBase(parent, field) {}

	void CreateControl()
	{
		CreateLabel();
		m_hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, _T("edit"), m_field->GetValue(),
			WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_LEFT|ES_AUTOHSCROLL,
			0, 0, 0, 0,
			m_parent->GetHwnd(), (HMENU)m_id, hInst, NULL);
	}

	int GetHeight(int width)
	{
		return 20;
	}

	XmlNode *FetchData()
	{
		XmlNode *field = CreateNode();
		int len = GetWindowTextLength(m_hwnd);
		TCHAR *str = (TCHAR *)mir_alloc(sizeof(TCHAR) * (len+1));
		GetWindowText(m_hwnd, str, len+1);
		field->addChild("value", str);
		mir_free(str);
		return field;
	}
};

class CFormCtrlTextPrivate: public CFormCtrlBase
{
public:
	CFormCtrlTextPrivate(CJabberDlgDataPage *parent, CJabberDataField *field): CFormCtrlBase(parent, field) {}

	void CreateControl()
	{
		CreateLabel();
		m_hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, _T("edit"), m_field->GetValue(),
			WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_LEFT|ES_AUTOHSCROLL|ES_PASSWORD,
			0, 0, 0, 0,
			m_parent->GetHwnd(), (HMENU)m_id, hInst, NULL);
	}

	int GetHeight(int width)
	{
		return 20;
	}

	XmlNode *FetchData()
	{
		XmlNode *field = CreateNode();
		int len = GetWindowTextLength(m_hwnd);
		TCHAR *str = (TCHAR *)mir_alloc(sizeof(TCHAR) * (len+1));
		GetWindowText(m_hwnd, str, len+1);
		field->addChild("value", str);
		mir_free(str);
		return field;
	}
};

CJabberDlgDataPage::CJabberDlgDataPage(HWND hwndParent):
	m_controls(5)
{
	m_hwnd = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_DATAFORM_PAGE), hwndParent, DlgProc, (LPARAM)this);
	ShowWindow(m_hwnd, SW_SHOW);
}

CJabberDlgDataPage::~CJabberDlgDataPage()
{
	DestroyWindow(m_hwnd);
}

void CJabberDlgDataPage::AddField(CJabberDataField *field)
{
	CFormCtrlBase *ctrl = NULL;

	switch (field->GetType())
	{
		case JDFT_BOOLEAN:		ctrl = new CFormCtrlBoolean(this, field);		break;
		case JDFT_FIXED:		ctrl = new CFormCtrlFixed(this, field);			break;
		case JDFT_HIDDEN:		ctrl = new CFormCtrlHidden(this, field);		break;
		case JDFT_JID_MULTI:	ctrl = new CFormCtrlTextMulti(this, field);		break;
		case JDFT_JID_SINGLE:	ctrl = new CFormCtrlTextSingle(this, field);	break;
		case JDFT_LIST_MULTI:	ctrl = new CFormCtrlListMulti(this, field);		break;
		case JDFT_LIST_SINGLE:	ctrl = new CFormCtrlListSingle(this, field);	break;
		case JDFT_TEXT_MULTI:	ctrl = new CFormCtrlTextMulti(this, field);		break;
		case JDFT_TEXT_PRIVATE:	ctrl = new CFormCtrlTextPrivate(this, field);	break;
		case JDFT_TEXT_SINGLE:	ctrl = new CFormCtrlTextSingle(this, field);	break;
	}

	if (ctrl) ctrl->Init();
}

XmlNode *CJabberDlgDataPage::FetchData()
{
	XmlNode *result = new XmlNode("x");
	result->addAttr("xmlns", JABBER_FEAT_DATA_FORMS);
	result->addAttr("type", "submit");

	for (int i = 0; i < m_controls.getCount(); ++i)
		if (XmlNode *field = m_controls[i].FetchData())
			result->addChild(field);

	return result;
}

void CJabberDlgDataPage::Layout()
{
	RECT rc; GetClientRect(m_hwnd, &rc);
	int w = rc.right - 20;
	int x = 10;
	int y = 10;

	m_height = rc.bottom;
	m_scrollPos = GetScrollPos(m_hwnd, SB_VERT);

	HDWP hdwp = BeginDeferWindowPos(m_controls.getCount());
	for (int i = 0; i < m_controls.getCount(); ++i)
		if (int h = m_controls[i].Layout(hdwp, x, y - m_scrollPos, w))
			y += h + 5;
	EndDeferWindowPos(hdwp);

	m_dataHeight = y + 5;

	SCROLLINFO si = {0};
	si.cbSize = sizeof(si);
	si.fMask = SIF_DISABLENOSCROLL|SIF_PAGE|SIF_RANGE;
	si.nPage = m_height;
	si.nMin = 0;
	si.nMax = m_dataHeight;
	SetScrollInfo(m_hwnd, SB_VERT, &si, TRUE);
}

BOOL CJabberDlgDataPage::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
	{
		SCROLLINFO si = {0};
		si.cbSize = sizeof(si);
		si.fMask = SIF_DISABLENOSCROLL;
		SetScrollInfo(m_hwnd, SB_VERT, &si, TRUE);
		m_scrollPos = 0;

		break;
	}

	case WM_MOUSEWHEEL:
	{
		int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		if (zDelta)
		{
			int i, nScrollLines = 0;
			SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, (void*)&nScrollLines, 0);
			for (i = 0; i < (nScrollLines + 1) / 2; i++ )
				SendMessage(m_hwnd, WM_VSCROLL, (zDelta < 0) ? SB_LINEDOWN : SB_LINEUP, 0);
		}

		SetWindowLongPtr(m_hwnd, DWL_MSGRESULT, 0);
		return TRUE;
	}

	case WM_VSCROLL:
	{
		int pos = m_scrollPos;
		switch (LOWORD(wParam))
		{
		case SB_LINEDOWN:
			pos += 15;
			break;
		case SB_LINEUP:
			pos -= 15;
			break;
		case SB_PAGEDOWN:
			pos += m_height - 10;
			break;
		case SB_PAGEUP:
			pos -= m_height - 10;
			break;
		case SB_THUMBTRACK:
			pos = HIWORD(wParam);
			break;
		}

		if (pos > m_dataHeight - m_height) pos = m_dataHeight - m_height;
		if (pos < 0) pos = 0;

		if (m_scrollPos != pos)
		{
			ScrollWindow(m_hwnd, 0, m_scrollPos - pos, NULL, NULL);
			SetScrollPos(m_hwnd, SB_VERT, pos, TRUE);
			m_scrollPos = pos;
		}
		break;
	}

	case WM_SIZE:
		Layout();
		break;
	}

	return FALSE;
}

BOOL CJabberDlgDataPage::DlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CJabberDlgDataPage *pThis = NULL;

	if (msg == WM_INITDIALOG)
	{
		if (pThis = (CJabberDlgDataPage *)lParam)
			pThis->m_hwnd = hwnd;
		SetWindowLongPtr(hwnd, GWL_USERDATA, lParam);
	} else
	{
		pThis = (CJabberDlgDataPage *)GetWindowLongPtr(hwnd, GWL_USERDATA);
	}

	if (pThis)
	{
		BOOL result = pThis->DlgProc(msg, wParam, lParam);
		if (msg == WM_DESTROY)
		{
			pThis->m_hwnd = NULL;
			SetWindowLongPtr(hwnd, GWL_USERDATA, 0);
		}
		return result;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////
// Data form control -- Window class support
const TCHAR *CCtrlJabberForm::ClassName = _T("JabberDataFormControl");
bool CCtrlJabberForm::ClassRegistered = false;

bool CCtrlJabberForm::RegisterClass()
{
	if (ClassRegistered) return true;

	WNDCLASSEX wcx = {0};
	wcx.cbSize = sizeof(wcx);
	wcx.lpszClassName = ClassName;
	wcx.lpfnWndProc = DefWindowProc;
	wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcx.hbrBackground = 0;
	wcx.style = CS_GLOBALCLASS;

	if (::RegisterClassEx(&wcx))
		ClassRegistered = true;

	return ClassRegistered;
}

bool CCtrlJabberForm::UnregisterClass()
{
	if (!ClassRegistered) return true;

	if (::UnregisterClass(ClassName, hInst) == 0)
		ClassRegistered = false;

	return !ClassRegistered;
}

/////////////////////////////////////////////////////////////////////////////////
// Data form control
CCtrlJabberForm::CCtrlJabberForm(CDlgBase* dlg, int ctrlId):
	CCtrlBase(dlg, ctrlId), m_pForm(NULL), m_pDlgPage(NULL)
{
}

CCtrlJabberForm::~CCtrlJabberForm()
{
	if (m_pDlgPage) delete m_pDlgPage;
}

void CCtrlJabberForm::OnInit()
{
	CSuper::OnInit();
	Subclass();
	SetupForm();
}

void CCtrlJabberForm::SetDataForm(CJabberDataForm *pForm)
{
	if (m_pDlgPage)
	{
		delete m_pDlgPage;
		m_pDlgPage = NULL;
	}

	m_pForm = pForm;
	SetupForm();
}

XmlNode *CCtrlJabberForm::FetchData()
{
	return m_pDlgPage ? m_pDlgPage->FetchData() : NULL;
}

void CCtrlJabberForm::SetupForm()
{
	if (!m_pForm || !m_hwnd) return;

	m_pDlgPage = new CJabberDlgDataPage(m_hwnd);
	for (int i = 0; i < m_pForm->GetFields()->GetCount(); ++i)
		m_pDlgPage->AddField(m_pForm->GetFields()->GetField(i));

	Layout();
}

void CCtrlJabberForm::Layout()
{
	if (!m_pDlgPage) return;

	RECT rc;
	GetClientRect(m_hwnd, &rc);
	SetWindowPos(m_pDlgPage->GetHwnd(), NULL, 0, 0, rc.right, rc.bottom, SWP_NOZORDER);
}

LRESULT CCtrlJabberForm::CustomWndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_SIZE:
		{
			Layout();
			break;
		}
	}

	return CSuper::CustomWndProc(msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////
// testing
class CJabberDlgFormTest: public CDlgBase
{
public:
	CJabberDlgFormTest(CJabberDataForm *pForm):
	  CDlgBase(IDD_DATAFORM_TEST, NULL),
		m_frm(this, IDC_DATAFORM)
	{
		m_frm.SetDataForm(pForm);
	}

	int Resizer(UTILRESIZECONTROL *urc)
	{
		return RD_ANCHORX_WIDTH|RD_ANCHORY_HEIGHT;
	}

private:
	CCtrlJabberForm m_frm;
};

static VOID CALLBACK CreateDialogApcProc(void* param)
{
	XmlNode *node = (XmlNode *)param;

	CJabberDataForm form(node);

	CCtrlJabberForm::RegisterClass();
	CJabberDlgFormTest dlg(&form);
	dlg.DoModal();

	delete node;
}

void LaunchForm(XmlNode *node)
{
	node = JabberXmlCopyNode(node);
	CallFunctionAsync(CreateDialogApcProc, node);
}
