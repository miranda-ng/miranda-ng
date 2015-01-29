/*
 * WAConnection.h
 *
 *  Created on: 26/06/2012
 *      Author: Antonio
 */




#ifndef WACONNECTION_H_
#define WACONNECTION_H_

#include <string>
#include <time.h>
#include <map>
#include "WAException.h"
#include "FMessage.h"
#include "WALogin.h"
#include "utilities.h"
#include "BinTreeNodeReader.h"
#include "BinTreeNodeWriter.h"

#pragma warning(disable : 4290)

class WALogin;
class WASocketConnection;
class KeyStream;
class BinTreeNodeReader;

class WAListener {
public:
	virtual void onMessageForMe(FMessage* paramFMessage, bool paramBoolean) throw (WAException)=0;
	virtual void onMessageStatusUpdate(FMessage* paramFMessage)=0;
	virtual void onMessageError(FMessage* message, int paramInt)=0;
	virtual void onPing(const std::string& paramString) throw (WAException)=0;
	virtual void onPingResponseReceived()=0;
	virtual void onAvailable(const std::string& paramString, bool paramBoolean)=0;
	virtual void onClientConfigReceived(const std::string& paramString)=0;
	virtual void onLastSeen(const std::string& paramString1, int paramInt, const string &paramString2) = 0;
	virtual void onIsTyping(const std::string& paramString, bool paramBoolean)=0;
	virtual void onAccountChange(int paramInt, time_t paramLong)=0;
	virtual void onPrivacyBlockListAdd(const std::string& paramString)=0;
	virtual void onPrivacyBlockListClear()=0;
	virtual void onDirty(const std::map<string,string>& paramHashtable)=0;
	virtual void onDirtyResponse(int paramHashtable)=0;
	virtual void onRelayRequest(const std::string& paramString1, int paramInt, const std::string& paramString2)=0;
	virtual void onSendGetPicture(const std::string& jid, const std::vector<unsigned char>& data, const std::string& id)=0;
	virtual void onPictureChanged(const std::string& from, const std::string& author, bool set)=0;
	virtual void onDeleteAccount(bool result)=0;
};

class WAGroupListener {
public:
	virtual void onGroupAddUser(const std::string& paramString1, const std::string& paramString2)=0;
	virtual void onGroupRemoveUser(const std::string& paramString1, const std::string& paramString2)=0;
	virtual void onGroupNewSubject(const std::string& from, const std::string& author, const std::string& newSubject, int paramInt)=0;
	virtual void onServerProperties(std::map<std::string, std::string>* nameValueMap)=0;
	virtual void onGroupCreated(const std::string& paramString1, const std::string& paramString2)=0;
	virtual void onGroupInfo(const std::string& paramString1, const std::string& paramString2, const std::string& paramString3, const std::string& paramString4, int paramInt1, int paramInt2)=0;
	virtual void onGroupInfoFromList(const std::string& paramString1, const std::string& paramString2, const std::string& paramString3, const std::string& paramString4, int paramInt1, int paramInt2)=0;
	virtual void onOwningGroups(const std::vector<string>& paramVector)=0;
	virtual void onSetSubject(const std::string& paramString)=0;
	virtual void onAddGroupParticipants(const std::string& paramString, const std::vector<string>& paramVector, int paramHashtable)=0;
	virtual void onRemoveGroupParticipants(const std::string& paramString, const std::vector<string>& paramVector, int paramHashtable)=0;
	virtual void onGetParticipants(const std::string& gjid, const std::vector<string>& participants)=0;
	virtual void onParticipatingGroups(const std::vector<string>& paramVector)=0;
	virtual void onLeaveGroup(const std::string& paramString)=0;
};

class GroupSetting {
public:
	std::string jid;
	bool enabled;
	time_t muteExpiry;

	GroupSetting() {
		enabled = true;
		jid = "";
		muteExpiry = 0;
	}
};

class WAConnection
{
	class IqResultHandler {
	protected:
		WAConnection* con;
	public:
		IqResultHandler(WAConnection* con) {this->con = con;}
		virtual void parse(ProtocolTreeNode* paramProtocolTreeNode, const std::string& paramString) throw (WAException)=0;
		void error(ProtocolTreeNode* node, int code) {
			con->logData("WAConnection: error node %s: code = %d", node->getAttributeValue("id").c_str(), code);
		}
		void error(ProtocolTreeNode* node) throw (WAException) {
			std::vector<ProtocolTreeNode*> nodes(node->getAllChildren("error"));
			for (size_t i = 0; i < nodes.size(); i++) {
				ProtocolTreeNode* errorNode = nodes[i];
				if (errorNode != NULL) {
					const string &errorCodeString = errorNode->getAttributeValue("code");
					if (!errorCodeString.empty()) {
						int errorCode = atoi(errorCodeString.c_str());
						error(node, errorCode);
					}
				}
			}
		}

		virtual ~IqResultHandler() {}
	};

	class IqResultPingHandler: public IqResultHandler {
	public:
		IqResultPingHandler(WAConnection* con):IqResultHandler(con) {}
		virtual void parse(ProtocolTreeNode* node, const std::string& from) throw (WAException) {
			if (this->con->m_pEventHandler != NULL)
				this->con->m_pEventHandler->onPingResponseReceived();
		}

		void error(ProtocolTreeNode* node) throw (WAException) {
			if (this->con->m_pEventHandler != NULL)
				this->con->m_pEventHandler->onPingResponseReceived();
		}
	};

	class IqResultGetGroupsHandler: public IqResultHandler {
	private:
		std::string type;
	public:
		IqResultGetGroupsHandler(WAConnection* con, const std::string& type ):IqResultHandler(con) {this->type = type;}
		virtual void parse(ProtocolTreeNode* node, const std::string& from) throw (WAException) {
			std::vector<std::string> groups;
			this->con->readGroupList(node, groups);
			if (this->con->m_pGroupEventHandler != NULL) {
				if (this->type.compare("participating") == 0)
					this->con->m_pGroupEventHandler->onParticipatingGroups(groups);
				else if (this->type.compare("owning") == 0)
					this->con->m_pGroupEventHandler->onOwningGroups(groups);
			}
		}
	};

	class IqResultServerPropertiesHandler: public IqResultHandler {
	public:
		IqResultServerPropertiesHandler(WAConnection* con):IqResultHandler(con) {}
		virtual void parse(ProtocolTreeNode* node, const std::string& from) throw (WAException) {
			std::vector<ProtocolTreeNode*> nodes(node->getAllChildren("prop"));
			std::map<std::string,std::string> nameValueMap;
			for (size_t i = 0; i < nodes.size();i++) {
				ProtocolTreeNode* propNode = nodes[i];
				const string &nameAttr = propNode->getAttributeValue("name");
				const string &valueAttr = propNode->getAttributeValue("value");
				nameValueMap[nameAttr] = valueAttr;
			}

			if (this->con->m_pGroupEventHandler != NULL)
				this->con->m_pGroupEventHandler->onServerProperties(&nameValueMap);
		}
	};

	class IqResultPrivayListHandler: public IqResultHandler {
	public:
		IqResultPrivayListHandler(WAConnection* con):IqResultHandler(con) {}
		virtual void parse(ProtocolTreeNode* node, const std::string& from) throw (WAException) {
			ProtocolTreeNode* queryNode = node->getChild(0);
			ProtocolTreeNode::require(queryNode, "query");
			ProtocolTreeNode* listNode = queryNode->getChild(0);
			ProtocolTreeNode::require(listNode, "list");
			if (this->con->m_pEventHandler != NULL)
				this->con->m_pEventHandler->onPrivacyBlockListClear();
			if (listNode->children != NULL) {
				for (size_t i = 0; i < listNode->children->size(); i++) {
					ProtocolTreeNode* itemNode = (*listNode->children)[i];
					ProtocolTreeNode::require(itemNode, "item");
					if (itemNode->getAttributeValue("type").compare("jid") == 0) {
						const string &jid = itemNode->getAttributeValue("value");
						if (!jid.empty() && this->con->m_pEventHandler != NULL)
							this->con->m_pEventHandler->onPrivacyBlockListAdd(jid);
					}
				}
			}
		}
	};

	class IqResultGetGroupInfoHandler: public IqResultHandler {
	public:
		IqResultGetGroupInfoHandler(WAConnection* con):IqResultHandler(con) {}
		virtual void parse(ProtocolTreeNode* node, const std::string& from) throw (WAException) {
			ProtocolTreeNode* groupNode =  node->getChild(0);
			ProtocolTreeNode::require(groupNode, "group");
			const string &owner = groupNode->getAttributeValue("owner");
			const string &subject = groupNode->getAttributeValue("subject");
			const string &subject_t = groupNode->getAttributeValue("s_t");
			const string &subject_owner = groupNode->getAttributeValue("s_o");
			const string &creation = groupNode->getAttributeValue("creation");
			if (this->con->m_pGroupEventHandler != NULL)
				this->con->m_pGroupEventHandler->onGroupInfo(from, owner, subject, subject_owner, atoi(subject_t.c_str()), atoi(creation.c_str()));
		}
	};

	class IqResultGetGroupParticipantsHandler: public IqResultHandler {
	public:
		IqResultGetGroupParticipantsHandler(WAConnection* con):IqResultHandler(con) {}
		virtual void parse(ProtocolTreeNode* node, const std::string& from) throw (WAException) {
			std::vector<std::string> participants;
			this->con->readAttributeList(node, participants, "participant", "jid");
			if (this->con->m_pGroupEventHandler != NULL)
				this->con->m_pGroupEventHandler->onGetParticipants(from, participants);
		}
	};

	class IqResultCreateGroupChatHandler: public IqResultHandler {
	public:
		IqResultCreateGroupChatHandler(WAConnection* con):IqResultHandler(con) {}
		virtual void parse(ProtocolTreeNode* node, const std::string& from) throw (WAException) {
			ProtocolTreeNode* groupNode = node->getChild(0);
			ProtocolTreeNode::require(groupNode, "group");
			const string &groupId = groupNode->getAttributeValue("id");
			if (!groupId.empty() && con->m_pGroupEventHandler != NULL)
				this->con->m_pGroupEventHandler->onGroupCreated(from, groupId);
		}
	};

	class IqResultQueryLastOnlineHandler: public IqResultHandler {
	public:
		IqResultQueryLastOnlineHandler(WAConnection* con):IqResultHandler(con) {}
		virtual void parse(ProtocolTreeNode* node, const std::string& from) throw (WAException) {
			ProtocolTreeNode* firstChild = node->getChild(0);
			ProtocolTreeNode::require(firstChild, "query");
			const string &seconds = firstChild->getAttributeValue("seconds");
			const string &status = firstChild->getDataAsString();
			if (!seconds.empty() && !from.empty())
				if (this->con->m_pEventHandler != NULL)
					this->con->m_pEventHandler->onLastSeen(from, atoi(seconds.c_str()), status);
		}
	};

	class IqResultGetPhotoHandler: public IqResultHandler {
	private:
		std::string jid;
		std::string oldId;
		std::string newId;
	public:
		IqResultGetPhotoHandler(WAConnection* con, const std::string& jid):IqResultHandler(con) {
			this->jid = jid;
		}
		virtual void parse(ProtocolTreeNode* node, const std::string& from) throw (WAException) {
			const string &attributeValue = node->getAttributeValue("type");

			if (!attributeValue.empty() && attributeValue == "result" && this->con->m_pEventHandler != NULL) {
				std::vector<ProtocolTreeNode*> children(node->getAllChildren("picture"));
				for (size_t i = 0; i < children.size(); i++) {
					ProtocolTreeNode* current = children[i];
					const string &id = current->getAttributeValue("id");
					if (!id.empty() && current->data != NULL && current->data->size() > 0) {
						if (current->data != NULL)
							this->con->m_pEventHandler->onSendGetPicture(this->jid, *current->data, id);
						break;
					}
				}
			}
		}
		void error(ProtocolTreeNode* node) throw (WAException) {
			if (this->con->m_pEventHandler != NULL) {
				std::vector<unsigned char> v;
				this->con->m_pEventHandler->onSendGetPicture("error", v, "");
			}
		}
	};

	class IqResultSetPhotoHandler: public IqResultHandler {
	private:
		std::string jid;
	public:
		IqResultSetPhotoHandler(WAConnection* con, const std::string& jid):IqResultHandler(con) {this->jid = jid;}
		virtual void parse(ProtocolTreeNode* node, const std::string& from) throw (WAException) {
			if (this->con->m_pEventHandler != NULL) {
				ProtocolTreeNode* child = node->getChild("picture");
				if (child != NULL)
					this->con->m_pEventHandler->onPictureChanged(this->jid, "", true);
				else
					this->con->m_pEventHandler->onPictureChanged(this->jid, "", false);
			}
		}
	};

	class IqResultSendDeleteAccount: public IqResultHandler {
	public:
		IqResultSendDeleteAccount(WAConnection* con):IqResultHandler(con) {}
		virtual void parse(ProtocolTreeNode* node, const std::string& from) throw (WAException) {
			if (this->con->m_pEventHandler != NULL)
				this->con->m_pEventHandler->onDeleteAccount(true);
		}

		void error(ProtocolTreeNode* node) throw (WAException) {
			if (this->con->m_pEventHandler != NULL)
				this->con->m_pEventHandler->onDeleteAccount(false);
		}
	};

	class IqResultClearDirtyHandler: public IqResultHandler {
	public:
		IqResultClearDirtyHandler(WAConnection* con):IqResultHandler(con) {}
		virtual void parse(ProtocolTreeNode* node, const std::string& from) throw (WAException) {
		}
	};

	class IqSendClientConfigHandler: public IqResultHandler {
	public:
		IqSendClientConfigHandler(WAConnection* con):IqResultHandler(con) {}
		virtual void parse(ProtocolTreeNode* node, const std::string& from) throw (WAException) {
			con->logData("Clientconfig response %s", node->toString().c_str());
		}

		void error(ProtocolTreeNode* node) throw (WAException) {
			con->logData("Clientconfig response error %s", node->toString().c_str());
		}
	};

	friend class WALogin;

private:
	ISocketConnection *rawConn;
	BinTreeNodeReader in;
	BinTreeNodeWriter out;
	WAListener *m_pEventHandler;
	WAGroupListener *m_pGroupEventHandler;
	bool verbose;
	int iqid;
	std::map<string, IqResultHandler*> pending_server_requests;
	IMutex *m_pMutex;

	void parseAck(ProtocolTreeNode *node) throw (WAException);
	void parseChatStates(ProtocolTreeNode *node) throw (WAException);
	void parseIq(ProtocolTreeNode *node) throw(WAException);
	void parseMessage(ProtocolTreeNode* node) throw(WAException);
	void parsePresense(ProtocolTreeNode*) throw(WAException);
	void parseReceipt(ProtocolTreeNode *node) throw (WAException);
	std::map<string, string> parseCategories(ProtocolTreeNode* node) throw(WAException);

	void sendMessageWithMedia(FMessage* message) throw(WAException);
	void sendMessageWithBody(FMessage* message) throw(WAException);
	ProtocolTreeNode* getReceiptAck(const std::string& to, const std::string& id, const std::string& receiptType) throw(WAException);
	std::string makeId(const std::string& prefix);
	void sendGetGroups(const std::string& id, const std::string& type) throw (WAException);
	void readGroupList(ProtocolTreeNode* node, std::vector<std::string>& groups) throw (WAException);
	std::string gidToGjid(const std::string& gid);
	void readAttributeList(ProtocolTreeNode* node, std::vector<std::string>& vector, const std::string& tag, const std::string& attribute) throw (WAException);
	void sendVerbParticipants(const std::string& gjid, const std::vector<std::string>& participants, const std::string& id, const std::string& inner_tag) throw (WAException);
	bool supportsReceiptAcks();
	static ProtocolTreeNode* getMessageNode(FMessage* message, ProtocolTreeNode* node);
	std::vector<ProtocolTreeNode*>* processGroupSettings(const std::vector<GroupSetting>& gruops);

public:
	WAConnection(const std::string& user, const std::string& resource, IMutex* mutex, IMutex *write_mutex, ISocketConnection *conn, WAListener* m_pEventHandler, WAGroupListener* m_pGroupEventHandler);
	virtual ~WAConnection();

	std::string user;
	std::string domain;
	std::string resource;
	std::string jid;
	std::string nick;

	KeyStream inputKey, outputKey;

	bool retry;
	bool supports_receipt_acks;
	time_t expire_date;
	int account_kind;
	time_t lastTreeRead;

	static void globalInit(void);
	static int  tokenLookup(const std::string&);

	void logData(const char *format, ...);

	static std::string removeResourceFromJid(const std::string& jid);

	void setLogin(WALogin *login);
	void setVerboseId(bool b);
	void sendMessage(FMessage* message) throw(WAException);
	void sendAvailableForChat() throw(WAException);
	
	bool read() throw(WAException);
	
	void sendPing() throw(WAException);
	void sendQueryLastOnline(const std::string& jid) throw (WAException);
	void sendPong(const std::string& id) throw(WAException);
	void sendComposing(const std::string& to) throw(WAException);
	void sendActive() throw(WAException);
	void sendInactive() throw(WAException);
	void sendPaused(const std::string& to) throw(WAException);
	void sendSubjectReceived(const std::string& to, const std::string& id) throw(WAException);
	void sendMessageReceived(FMessage* message) throw(WAException);
	void sendPresenceSubscriptionRequest(const std::string& to) throw (WAException);
	void sendClientConfig(const std::string& sound,  const std::string& pushID, bool preview, const std::string& platform) throw(WAException);
	void sendClientConfig(const std::string& pushID, bool preview, const std::string& platform, bool defaultSettings, bool groupSettings, const std::vector<GroupSetting>& groups) throw(WAException);
	void sendClose() throw (WAException);
   void sendAvailable() throw (WAException); // U.H.
	void sendGetPrivacyList() throw (WAException);
	void sendGetServerProperties() throw (WAException);
	void sendGetGroups() throw (WAException);
	void sendGetOwningGroups() throw (WAException);
	void sendCreateGroupChat(const std::string& subject) throw (WAException);
	void sendEndGroupChat(const std::string& gjid) throw (WAException);
	void sendGetGroupInfo(const std::string& gjid) throw (WAException);
	void sendGetParticipants(const std::string& gjid) throw (WAException);
	void sendClearDirty(const std::string& category) throw (WAException);
	void sendLeaveGroup(const std::string& gjid) throw (WAException);
	void sendAddParticipants(const std::string& gjid, const std::vector<std::string>& participants) throw (WAException);
	void sendRemoveParticipants(const std::string& gjid, const std::vector<std::string>& participants) throw (WAException);
	void sendSetNewSubject(const std::string& gjid, const std::string& subject) throw (WAException);
	void sendStatusUpdate(std::string& status) throw (WAException);
	void sendGetPicture(const std::string& jid, const std::string& type) throw (WAException);
	void sendSetPicture(const std::string& jid, std::vector<unsigned char>* data) throw (WAException);
	void sendNotificationReceived(const std::string& from, const std::string& id) throw(WAException);
	void sendDeleteAccount() throw(WAException);
};

#endif /* WACONNECTION_H_ */
