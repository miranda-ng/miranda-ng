/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-12 Robert Pösel

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

#include "common.h"
#include "JSON_CAJUN/reader.h"
#include "JSON_CAJUN/writer.h"
#include "JSON_CAJUN/elements.h"

int facebook_json_parser::parse_buddy_list( void* data, List::List< facebook_user >* buddy_list )
{
	using namespace json;

	try
	{
		facebook_user* current = NULL;
		std::string buddyData = static_cast< std::string* >( data )->substr( 9 );
		std::istringstream sDocument( buddyData );
		Object objDocument;
		Reader::Read(objDocument, sDocument);

		const Object& objRoot = objDocument;
/*		const Array& wasAvailableIDs = objRoot["payload"]["buddy_list"]["wasAvailableIDs"];

		for ( Array::const_iterator itWasAvailable( wasAvailableIDs.Begin() );
			itWasAvailable != wasAvailableIDs.End(); ++itWasAvailable)
		{
			const Number& member = *itWasAvailable;
			char was_id[32];
			lltoa( member.Value(), was_id, 10 );

			current = buddy_list->find( std::string( was_id ) );
			if ( current != NULL )
				current->status_id = ID_STATUS_OFFLINE;
		}*/ // Facebook removed support for "wasAvailableIDs"

		// Set all contacts in map to offline
		for ( List::Item< facebook_user >* i = buddy_list->begin( ); i != NULL; i = i->next ) {
			i->data->status_id = ID_STATUS_OFFLINE;
		}

		// Find mobile friends
		if (DBGetContactSettingByte(NULL,proto->m_szModuleName,FACEBOOK_KEY_LOAD_MOBILE, DEFAULT_LOAD_MOBILE)) {
			const Array& mobileFriends = objRoot["payload"]["buddy_list"]["mobile_friends"];

			for ( Array::const_iterator buddy( mobileFriends.Begin() );	buddy != mobileFriends.End(); ++buddy) {
				const Number& member = *buddy;
				char was_id[32];
				lltoa( member.Value(), was_id, 10 );

				std::string id = was_id;
				if (!id.empty()) {
					current = buddy_list->find( id );
									
					if ( current == NULL) {
						buddy_list->insert( std::make_pair( id, new facebook_user( ) ) );
						current = buddy_list->find( id );
						current->user_id = id;
					}
					
					current->status_id = ID_STATUS_ONTHEPHONE;
				}
			}
		}

		const Object& nowAvailableList = objRoot["payload"]["buddy_list"]["nowAvailableList"];
		// Find now awailable contacts
		for (Object::const_iterator itAvailable(nowAvailableList.Begin());
			itAvailable != nowAvailableList.End(); ++itAvailable)
		{
			const Object::Member& member = *itAvailable;
			const Object& objMember = member.element;
			const Boolean idle = objMember["i"]; // In new version of Facebook "i" means "offline"

			current = buddy_list->find( member.name );
			if ( current == NULL) {
				if (idle) continue; // Just little optimalization

				buddy_list->insert( std::make_pair( member.name, new facebook_user( ) ) );
				current = buddy_list->find( member.name );
				current->user_id = current->real_name = member.name;	
			}
						
			current->status_id = (idle ? ID_STATUS_OFFLINE : ID_STATUS_ONLINE);
		}

		const Object& userInfosList = objRoot["payload"]["buddy_list"]["userInfos"];
		// Get aditional informations about contacts (if available)
		for (Object::const_iterator itUserInfo(userInfosList.Begin());
			itUserInfo != userInfosList.End(); ++itUserInfo)
		{
			const Object::Member& member = *itUserInfo;

			current = buddy_list->find( member.name );
			if ( current == NULL )
				continue;

			const Object& objMember = member.element;
			const String& realName = objMember["name"];
			const String& imageUrl = objMember["thumbSrc"];

			current->real_name = utils::text::slashu_to_utf8(
			    utils::text::special_expressions_decode( realName.Value( ) ) );
			current->image_url = utils::text::slashu_to_utf8(
			    utils::text::special_expressions_decode( imageUrl.Value( ) ) );
		}
	}
	catch (Reader::ParseException& e)
	{
		proto->Log( "!!!!! Caught json::ParseException: %s", e.what() );
		proto->Log( "      Line/offset: %d/%d", e.m_locTokenBegin.m_nLine + 1, e.m_locTokenBegin.m_nLineOffset + 1 );
	}
	catch (const Exception& e)
	{
		proto->Log( "!!!!! Caught json::Exception: %s", e.what() );
	}
	catch (const std::exception& e)
	{
		proto->Log( "!!!!! Caught std::exception: %s", e.what() );
	}

	return EXIT_SUCCESS;
}

int facebook_json_parser::parse_friends( void* data, std::map< std::string, facebook_user* >* friends )
{
	using namespace json;

	try
	{
		std::string buddyData = static_cast< std::string* >( data )->substr( 9 );
		std::istringstream sDocument( buddyData );
		Object objDocument;
		Reader::Read(objDocument, sDocument);

		const Object& objRoot = objDocument;
		const Object& payload = objRoot["payload"];

		for ( Object::const_iterator payload_item( payload.Begin() ); payload_item != payload.End(); ++payload_item)
		{
			const Object::Member& member = *payload_item;

			const Object& objMember = member.element;

			const String& realName = objMember["name"];
			const String& imageUrl = objMember["thumbSrc"];
			//const String& vanity = objMember["vanity"];
			const Number& gender = objMember["gender"];
			
			facebook_user *fbu = new facebook_user();

			fbu->user_id = member.name;
			fbu->real_name = utils::text::slashu_to_utf8(
			    utils::text::special_expressions_decode( realName.Value() ) );
			fbu->image_url = utils::text::slashu_to_utf8(
			    utils::text::special_expressions_decode( imageUrl.Value() ) );

			if (gender.Value() == 1) {
				fbu->gender = 70; // female
			} else if (gender.Value() == 2) {
				fbu->gender = 77; // male
			}

			friends->insert( std::make_pair( member.name, fbu ) );
		}
	}
	catch (Reader::ParseException& e)
	{
		proto->Log( "!!!!! Caught json::ParseException: %s", e.what() );
		proto->Log( "      Line/offset: %d/%d", e.m_locTokenBegin.m_nLine + 1, e.m_locTokenBegin.m_nLineOffset + 1 );
	}
	catch (const Exception& e)
	{
		proto->Log( "!!!!! Caught json::Exception: %s", e.what() );
	}
	catch (const std::exception& e)
	{
		proto->Log( "!!!!! Caught std::exception: %s", e.what() );
	}

	return EXIT_SUCCESS;
}


int facebook_json_parser::parse_notifications( void *data, std::vector< facebook_notification* > *notifications ) 
{
	using namespace json;

	try
	{
		std::string notificationsData = static_cast< std::string* >( data )->substr( 9 );
		std::istringstream sDocument( notificationsData );
		Object objDocument;
		Reader::Read(objDocument, sDocument);

		const Object& objRoot = objDocument;
		const Object& payload = objRoot["payload"]["notifications"];

		for ( Object::const_iterator payload_item( payload.Begin() ); payload_item != payload.End(); ++payload_item)
		{
			const Object::Member& member = *payload_item;
			
			const Object& objMember = member.element;

			const String& content = objMember["markup"];
			const Number& unread = objMember["unread"];
			
			if (unread.Value() == 0) // ignore old notifications
				continue;
			
			std::string text = utils::text::slashu_to_utf8(
								utils::text::special_expressions_decode( content.Value() ) );

			facebook_notification* notification = new facebook_notification( );
			
			notification->text = utils::text::remove_html( utils::text::source_get_value(&text, 1, "<abbr") );
			notification->link = utils::text::source_get_value(&text, 3, "<a ", "href=\"", "\"");

			notifications->push_back( notification );
		}

	}
	catch (Reader::ParseException& e)
	{
		proto->Log( "!!!!! Caught json::ParseException: %s", e.what() );
		proto->Log( "      Line/offset: %d/%d", e.m_locTokenBegin.m_nLine + 1, e.m_locTokenBegin.m_nLineOffset + 1 );
	}
	catch (const Exception& e)
	{
		proto->Log( "!!!!! Caught json::Exception: %s", e.what() );
	}
	catch (const std::exception& e)
	{
		proto->Log( "!!!!! Caught std::exception: %s", e.what() );
	}

	return EXIT_SUCCESS;
}

int facebook_json_parser::parse_messages( void* data, std::vector< facebook_message* >* messages, std::vector< facebook_notification* >* notifications )
{
	using namespace json;

	try
	{
		std::string messageData = static_cast< std::string* >( data )->substr( 9 );
		std::istringstream sDocument( messageData );
		Object objDocument;
		Reader::Read(objDocument, sDocument);

		const Object& objRoot = objDocument;
		const Array& messagesArray = objRoot["ms"];

		std::string last_msg = "";

		for (Array::const_iterator itMessage(messagesArray.Begin());
			itMessage != messagesArray.End(); ++itMessage)
		{
			const Object& objMember = *itMessage;

			const String& type = objMember["type"];

			if ( type.Value( ) == "msg" || type.Value() == "offline_msg" ) // direct message
			{
				const Number& from = objMember["from"];
				char was_id[32];
				lltoa( from.Value(), was_id, 10 );
				
				const Object& messageContent = objMember["msg"];
				const String& text = messageContent["text"];
				//"tab_type":"friend",     objMember["tab_type"]
        
				const Number& time_sent = messageContent["time"];
//				proto->Log("????? Checking time %15.2f > %15.2f", time_sent.Value(), proto->facy.last_message_time_);

				if ((messageContent.Find("truncated") != messageContent.End())
					&& (((const Number &)messageContent["truncated"]).Value() == 1)) {
					// If we got truncated message, we can ignore it, because we should get it again as "messaging" type
					std::string msg = "????? We got truncated message so we ignore it\n";
					msg += utils::text::special_expressions_decode(utils::text::slashu_to_utf8(text.Value()));
					proto->Log(msg.c_str());
				} else if (last_msg != text.Value()) {
					last_msg = text.Value();
  					facebook_message* message = new facebook_message( );
					message->message_text = utils::text::special_expressions_decode(
						utils::text::slashu_to_utf8( text.Value( ) ) );
					message->time = utils::time::fix_timestamp( time_sent.Value() );
					message->user_id = was_id;

					messages->push_back( message );
				} else {
					std::string msg = "????? Got duplicit message?\n";
					msg += utils::text::special_expressions_decode(utils::text::slashu_to_utf8(text.Value()));
					proto->Log(msg.c_str());
				}
			}
			else if ( type.Value( ) == "messaging" ) // inbox message (multiuser or direct)
			{				
				const String& type = objMember["event"];

				if (type.Value() == "deliver") {
					const Object& messageContent = objMember["message"];

					const Number& from = messageContent["sender_fbid"];
					char was_id[32];
					lltoa( from.Value(), was_id, 10 );


					// Ignore if message is from self user
					if (was_id == proto->facy.self_.user_id)
						continue;

				
					const String& text = messageContent["body"];
					//std::string tid = ((const String&)messageContent["tid"]).Value();

					const String& sender_name = messageContent["sender_name"];

					std::string row = ((const String &)objMember["thread_row"]).Value();
        
					const Number& time_sent = messageContent["timestamp"];
					//proto->Log("????? Checking time %15.2f > %15.2f", time_sent.Value(), proto->facy.last_message_time_);

					if (last_msg != text.Value()) {
						last_msg = text.Value();

  						facebook_message* message = new facebook_message( );
						message->message_text = utils::text::special_expressions_decode(
							utils::text::slashu_to_utf8( text.Value( ) ) );

						message->sender_name = utils::text::special_expressions_decode(
							utils::text::slashu_to_utf8( sender_name.Value( ) ) );

						message->time = utils::time::fix_timestamp( time_sent.Value() );
						message->user_id = was_id; // TODO: Check if we have contact with this ID in friendlist and then do something different?

						if (row.find("uiSplitPic",0) != std::string::npos) {
							// This is multiuser message
							
							std::string authors = utils::text::special_expressions_decode(
								utils::text::slashu_to_utf8( row ) );
							authors = utils::text::source_get_value(&authors, 2, "<strong class=\"authors\">", "<");

							const String& to_id = messageContent["tid"];

							std::string popup_text = message->sender_name;
							popup_text += ": ";
							popup_text += message->message_text;

							std::string title = Translate("Multichat");
							title += ": ";
							title += authors;
				
							std::string url = "/?action=read&sk=inbox&page&query&tid=";
							url += to_id.Value();

							proto->Log("      Got multichat message");
		    
							TCHAR* szTitle = mir_a2t_cp(title.c_str(), CP_UTF8);
							TCHAR* szText = mir_a2t_cp(popup_text.c_str(), CP_UTF8);
							TCHAR* szUrl = mir_a2t_cp(url.c_str(), CP_UTF8);
							proto->NotifyEvent(szTitle,szText,NULL,FACEBOOK_EVENT_OTHER, szUrl);
							mir_free(szTitle);
							mir_free(szText);

						} else {
							messages->push_back( message );
						}
					} else {
						std::string msg = "????? Got duplicit inbox message?\n";
						msg += utils::text::special_expressions_decode(utils::text::slashu_to_utf8(text.Value()));
						proto->Log(msg.c_str());
					}
				}
			}
			else if ( type.Value( ) == "group_msg" ) // chat message
			{
				if (!DBGetContactSettingByte(NULL,proto->m_szModuleName,FACEBOOK_KEY_ENABLE_GROUPCHATS, DEFAULT_ENABLE_GROUPCHATS))
					continue;
				
				const String& from_name = objMember["from_name"];

				const Number& to = objMember["to"];
				char group_id[32];
				lltoa( to.Value(), group_id, 10 );
	
				const Number& from = objMember["from"];
				char was_id[32];
				lltoa( from.Value(), was_id, 10 );

				const Object& messageContent = objMember["msg"];
  				const String& text = messageContent["text"];

				std::string msg = utils::text::special_expressions_decode(
						utils::text::slashu_to_utf8( text.Value( ) ) );

				std::string name = utils::text::special_expressions_decode(
						utils::text::slashu_to_utf8( from_name.Value( ) ) );				

				// Add contact into chat, if isn't there already
				if (!proto->IsChatContact(group_id, was_id))
					proto->AddChatContact(group_id, was_id, name.c_str());

				const Number& time_sent = messageContent["time"];
				DWORD timestamp = utils::time::fix_timestamp( time_sent.Value() );

				// Add message into chat
				proto->UpdateChat(group_id, was_id, name.c_str(), msg.c_str(), timestamp);
			}
			else if ( type.Value( ) == "thread_msg" ) // multiuser message
			{
				const String& from_name = objMember["from_name"];
				const String& to_name = objMember["to_name"]["__html"];
				const String& to_id = objMember["to"];

				const Number& from = objMember["from"];
				char was_id[32];
				lltoa( from.Value(), was_id, 10 );

				// Ignore if message is from self user
				if (was_id == proto->facy.self_.user_id)
					continue;

				const Object& messageContent = objMember["msg"];
  				const String& text = messageContent["text"];

				
				last_msg = text.Value();


				std::string popup_text = utils::text::special_expressions_decode(
						utils::text::slashu_to_utf8( from_name.Value( ) ) );
				popup_text += ": ";
				popup_text += utils::text::special_expressions_decode(
						utils::text::slashu_to_utf8( text.Value( ) ) );

				std::string title = Translate("Multichat");
				title += ": ";
				title += utils::text::special_expressions_decode(
						utils::text::slashu_to_utf8( to_name.Value( ) ) );
				
				std::string url = "/?action=read&sk=inbox&page&query&tid=";
				url += to_id.Value();

				proto->Log("      Got multichat message");
		    
				TCHAR* szTitle = mir_a2t_cp(title.c_str(), CP_UTF8);
				TCHAR* szText = mir_a2t_cp(popup_text.c_str(), CP_UTF8);
				TCHAR* szUrl = mir_a2t_cp(url.c_str(), CP_UTF8);
				proto->NotifyEvent(szTitle,szText,NULL,FACEBOOK_EVENT_OTHER, szUrl);
				mir_free(szTitle);
				mir_free(szText);
			}
			else if ( type.Value( ) == "app_msg" ) // event notification
			{
				if (!DBGetContactSettingByte(NULL, proto->m_szModuleName, FACEBOOK_KEY_EVENT_NOTIFICATIONS_ENABLE, DEFAULT_EVENT_NOTIFICATIONS_ENABLE))
					continue;
				
				const String& text = objMember["response"]["payload"]["title"];
				const String& link = objMember["response"]["payload"]["link"];
				// TODO RM: include additional text of notification if exits? (e.g. comment text)
				//const String& text2 = objMember["response"]["payload"]["alert"]["text"];

				const Number& time_sent = objMember["response"]["payload"]["alert"]["time_sent"];        
				if (time_sent.Value() > proto->facy.last_notification_time_) // Check agains duplicit notifications
				{
					proto->facy.last_notification_time_ = time_sent.Value();

					facebook_notification* notification = new facebook_notification( );
					notification->text = utils::text::remove_html(
  						utils::text::special_expressions_decode(
							utils::text::slashu_to_utf8( text.Value( ) ) ) );

  					notification->link = utils::text::special_expressions_decode( link.Value( ) );

					notifications->push_back( notification );
				}
			}
			else if ( type.Value( ) == "typ" ) // chat typing notification
			{
				const Number& from = objMember["from"];
				char user_id[32];
				lltoa( from.Value(), user_id, 10 );

				facebook_user fbu;
				fbu.user_id = user_id;

				HANDLE hContact = proto->AddToContactList(&fbu);
				
				if ( DBGetContactSettingWord(hContact,proto->m_szModuleName,"Status", 0) == ID_STATUS_OFFLINE )
					DBWriteContactSettingWord(hContact,proto->m_szModuleName,"Status",ID_STATUS_ONLINE);

				const Number& state = objMember["st"];
				if (state.Value() == 1)
					CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hContact, (LPARAM)60);
				else
					CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hContact, (LPARAM)PROTOTYPE_CONTACTTYPING_OFF);
			}
			else if ( type.Value( ) == "visibility" ) // change of chat status
			{
				const Boolean visibility = objMember["visibility"];
				proto->Log("      Requested chat switch to %s", visibility ? "Online" : "Offline");
				proto->SetStatus( visibility ? ID_STATUS_ONLINE : ID_STATUS_INVISIBLE );				
			}
			else
				continue;
		}
	}
	catch (Reader::ParseException& e)
	{
		proto->Log( "!!!!! Caught json::ParseException: %s", e.what() );
		proto->Log( "      Line/offset: %d/%d", e.m_locTokenBegin.m_nLine + 1, e.m_locTokenBegin.m_nLineOffset + 1 );
	}
	catch (const Exception& e)
	{
		proto->Log ( "!!!!! Caught json::Exception: %s", e.what() );
	}

	return EXIT_SUCCESS;
}
