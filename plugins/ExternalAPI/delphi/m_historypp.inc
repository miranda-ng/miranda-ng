{
    History++ plugin for Miranda IM: the free IM client for Microsoft* Windows*

    Copyright (‘) 2006-2007 theMIROn, 2003-2006 Art Fedorov.
    History+ parts (C) 2001 Christian Kastner

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
}

{-----------------------------------------------------------------------------
 m_historypp (historypp project)

 Version:   1.5
 Created:   06.08.2004
 Author:    Oxygen

 [ Description ]

 Header with History++ services declaration

 [ History ]

 1.5 (05.08.2004)
   First version

 [ Modifications ]
 none

 [ Known Issues ]
 none

 Contributors: theMIROn, Art Fedorov
-----------------------------------------------------------------------------}

{$IFNDEF M_HISTORYPP}
{$DEFINE M_HISTORYPP}

const
  //** VALUES FOR TItemRenderDetails.dwHistoryWindow
  //** Used in ME_HPP_RICHEDIT_ITEMPROCESS event
  //** This is one of the following params,
  //** saying what kind of history window displays
  //** processed item.

  // IRDHW_CONTACTHISTORY. The window is ordinary
  // contact history.
  IRDHW_CONTACTHISTORY = $0001;
  // IRDHW_GLOBALHISTORY. The window is global
  // history (system history). Invokes by
  // Menu -> System History
  IRDHW_GLOBALHISTORY  = $0002;
  // IRDHW_GLOBALSEARCH. The window is search
  // window and the processed item is the
  // result of the global search.
  IRDHW_GLOBALSEARCH   = $0003;
  // IRDHW_EXTERNAL. The window is external window
  IRDHW_EXTERNALGRID   = $0004;

  //** VALUES FOR TItemRenderDetails.dwFlags
  //** Used in ME_HPP_RICHEDIT_ITEMPROCESS event
  //** These flags inform you about what you are
  //** processing. Like saying that the item is
  //** selected or it's inline or such stuff

  // IRDF_SELECTED. The processed item is selected.
  // The background color will always be clHighlight
  // even if you change it (i will change it back).
  // Though, I will not touch font or font color.
  IRDF_SELECTED = $0001;
  // IRDF_INLINE. The RichEdit provided is not one
  // that is used for drawing a cell, but the one
  // used for "inline editing".
  IRDF_INLINE   = $0002;
  // IRDF_EVENT. The RichEdit provided is from "Open Event"
  // window. It's the window which opens when you right-click
  // item in contact's history and select "Open"
  IRDF_EVENT    = $0004;

type
  TItemRenderDetails = record
    cbSize        :dword;     // size of the structure in bytes
    hContact      :THANDLE;   // handle to the contact for which the event is processed
    hDBEvent      :THANDLE;   // handle to the event which is processed
    dwEventTime   :dword;     // timestamp of the event
    wEventType    :word;      // Event's flags (see m_database, EVENTTYPE_*; m_icq, ICQEVENTTYPE_*)
    IsEventSent   :ByteBool;  // Outgoing event. True if DBEF_SENT event flag is present (see m_database)
    dwFlags       :dword;     // Any reasonable combination of IRDF_* flags.
    bHistoryWindow:byte;      // What kind of window history. See IRDHW_* values
    pProto        :PAnsiChar;     // Proto of the event, if available
    pModule       :PAnsiChar;     // Module of the event, if available
    pText         :PWideChar; // Text of the event, not used now
    pExtended     :PAnsiChar;     // Extended text, used for storing urls, paths and so on
  end;

  PItemRenderDetails = ^TItemRenderDetails;

const
  // ME_HPP_RICHEDIT_ITEMPROCESS
  // (supported from 1.5.0)
  // Called when next RichEdit history item
  // is rendered. Third-party plugins can alter it
  // like adding smileys, changing color and so on.
  // wParam - RichEdit control handle
  // lParam - pointer to TItemRenderDetails structure, information
  //          about processed item, see TItemRenderDetails for details
  //
  // Note: Changing richedit background color will change the background
  //       color of the whole cell! Additionally, the background color
  //       of the *selected* cell and richedit is ALWAYS clHighlight,
  //       no matter what you do. But font is untouched, so if your
  //       plugin changes font color, you may need to handle selected
  //       cells differently (use IF_SELECTED flag).
  //
  // Warn: Remeber about changing fonts. You CAN NOT have *different* fonts
  //       for the *same* item, depening on your wish. For example, the
  //       first time event is fired you set font for Item1 'Arial 10',
  //       and the next time you set font for the same Item1 'Arial 12'.
  //       Because height is calculated only once, and you may have
  //       problems with text painting (when you change font text can
  //       become too large and be cut or override another cell)
  //
  // See:  hpp_itemprocess_samples.pas for some sample event handlers
  //       like SmileyAdd and TextFormat support and special handler
  //       kinda of conversation separation
  ME_HPP_RICHEDIT_ITEMPROCESS = 'History++/RichEdit/ItemProcessEvent';

  // MS_HPP_SHOWGLOBALSEARCH
  // (supported from 1.5.0)
  // Show Global history search window
  // If already opened, bring it to front
  // wParam - zero
  // lParam - zero
  MS_HPP_SHOWGLOBALSEARCH = 'History++/ShowGlobalSearch';

type
  TOpenEventParams = record
    cbSize   : dword;
    hContact : THANDLE;
    hDBEvent : THANDLE;
    pPassword: PAnsiChar;
  end;

  POpenEventParams = ^TOpenEventParams;

const

  // MS_HPP_OPENHISTORYEVENT
  // (supported from 1.5.0)
  // (changed in 1.5.110)
  //
  // Opens contact's history and selects
  // provided event
  // wParam - pointer to TOpenEventParams structure
  // lParam - zero
  // Return - bool, True if contact opened, False if password
  //          field opened
  // Note: if you just want to show contact's history,
  //       use system service MS_HISTORY_SHOWCONTACTHISTORY
  MS_HPP_OPENHISTORYEVENT = 'History++/OpenHistoryEvent2';

  // MS_HPP_GETVERSION
  // (supported from 1.5.0)
  // Get current History++ version
  // Third-party plugins can use it to know if installed
  // version of History++ supports particular feature
  // wParam - zero
  // lParam - zero
  // Return - current version, via PLUGIN_MAKE_VERSION macro
  MS_HPP_GETVERSION = 'History++/GetVersion';

  // MS_HPP_EMPTYHISTORY
  // (supported from 1.5.0.118)
  // Erases contact's history
  // wParam - hContact
  // lParam - zero
  // Notes  - hContact can be NULL(0) to empty system history
  MS_HPP_EMPTYHISTORY = 'History++/EmptyHistory';

{$ENDIF}
