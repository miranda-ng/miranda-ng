/*

dbx_tree: tree database driver for Miranda IM

Copyright 2007-2010 Michael "Protogenes" Kunz,

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef M_DBX_TREE_H__

#define M_DBX_TREE_H__ 1

#include <stdint.h>
#pragma pack(push, 8)


/**
	\brief general return value if invalid param or invalid combination of params specified
**/
static const unsigned int DBT_INVALIDPARAM = 0xFFFFFFFF;


///////////////////////////////////////////////////////////
// Entities
///////////////////////////////////////////////////////////

/**
	\brief A handle to a Entity
**/
typedef uint32_t TDBTEntityHandle;

static const uint32_t DBT_NF_IsRoot      = 0x00000001;  /// Entity is the Root
static const uint32_t DBT_NF_IsGroup     = 0x00000002;  /// Entity is a   group
static const uint32_t DBT_NF_IsAccount   = 0x00000004;  /// Entity is an  account

static const uint32_t DBT_NF_HasChildren = 0x00010000;  /// Entity has Children (for Groups and Metacontacts)
static const uint32_t DBT_NF_HasVirtuals = 0x00020000;  /// Entity has at least one Virtual duplicate
static const uint32_t DBT_NF_IsVirtual   = 0x00040000;  /// Entity is a Virtual duplicate

static const uint32_t DBT_NFM_SpecialEntity = DBT_NF_IsRoot | DBT_NF_IsGroup | DBT_NF_IsAccount | DBT_NF_IsVirtual;

///////////////////////////////////////////////////////////
// Entities
///////////////////////////////////////////////////////////

/**
	\brief
	\param wParam = 0
	\param lParam = 0

	\return Handle to root Entity
**/
#define MS_DBT_ENTITY_GETROOT "DBT/Entity/GetRoot"


/**
	\brief
	\param wParam = hEntity
	\param lParam = 0

	\return ChildCount of specified Entity
**/
#define MS_DBT_ENTITY_CHILDCOUNT "DBT/Entity/ChildCount"


/**
	\brief
	\param wParam = hEntity
	\param lParam = 0

	\return Parent hEntity of specified Entity
**/
#define MS_DBT_ENTITY_GETPARENT "DBT/Entity/GetParent"


/**
	\brief
  \param wParam = hEntity
  \param lParam = hNewEntity

	\return 0 on success
**/
#define MS_DBT_ENTITY_MOVE "DBT/Entity/Move"

/**
	\brief
	\param wParam = hEntity
	\param lParam = hNewParent
**/
#define ME_DBT_ENTITY_MOVING "DBT/Entity/Moving"
/**
	\brief
	\param wParam = hEntity
	\param lParam = hOldParent
**/
#define ME_DBT_ENTITY_MOVED "DBT/Entity/Moved"

/**
	\brief Read the flags of an Entity
  \param wParam = hEntity
  \param lParam = 0

	\return Flags
**/
#define MS_DBT_ENTITY_GETFLAGS "DBT/Entity/GetFlags"



static const uint32_t DBT_NIFO_OSC_AC   = 0x00000001;                       /// onStartEntity - AddChildren
static const uint32_t DBT_NIFO_OSC_AP   = 0x00000002;                       /// onStartEntity - AddParent
static const uint32_t DBT_NIFO_OSC_AO   = 0x00000004;                       /// onStartEntity - AddOriginal (only if Entity is virtual)
static const uint32_t DBT_NIFO_OSC_AOC  = 0x00000008 | DBT_NIFO_OSC_AO;     /// onStartEntity - AddOriginalChildren (only if Entity is virtual)
static const uint32_t DBT_NIFO_OSC_AOP  = 0x00000010 | DBT_NIFO_OSC_AO;     /// onStartEntity - AddOriginalParent (only if Entity is virtual)
static const uint32_t DBT_NIFO_OSC_USEACCOUNT  = 0x00000080;                /// onStartEntity - use Account as fallback, only for settings

static const uint32_t DBT_NIFO_OC_AC    = 0x00000001 <<8;                   /// onChildEntity - AddChildren
//static const uint32_t DBT_LC_OC_AP      = 0x00000002 <<8;                 /// invalid for children
static const uint32_t DBT_NIFO_OC_AO    = 0x00000004 <<8;                   /// onChildEntity - AddOriginal (only if Entity is virtual)
static const uint32_t DBT_NIFO_OC_AOC   = 0x00000008 <<8 | DBT_NIFO_OC_AO;  /// onChildEntity - AddOriginalChildren (only if Entity is virtual)
static const uint32_t DBT_NIFO_OC_AOP   = 0x00000010 <<8 | DBT_NIFO_OC_AO;  /// onChildEntity - AddOriginalParent (only if Entity is virtual)
static const uint32_t DBT_NIFO_OC_USEACCOUNT  = 0x00000080 <<8;             /// onStartEntity - use Account as fallback, only for settings

static const uint32_t DBT_NIFO_OP_AC    = 0x00000001 <<16;                  /// onParentEntity - AddChildren
static const uint32_t DBT_NIFO_OP_AP    = 0x00000002 <<16;                  /// onParentEntity - AddParent
static const uint32_t DBT_NIFO_OP_AO    = 0x00000004 <<16;                  /// onParentEntity - AddOriginal (only if Entity is virtual)
static const uint32_t DBT_NIFO_OP_AOC   = 0x00000008 <<16 | DBT_NIFO_OP_AO; /// onParentEntity - AddOriginalChildren (only if Entity is virtual)
static const uint32_t DBT_NIFO_OP_AOP   = 0x00000010 <<16 | DBT_NIFO_OP_AO; /// onParentEntity - AddOriginalParent (only if Entity is virtual)
static const uint32_t DBT_NIFO_OP_USEACCOUNT  = 0x00000080 <<16;           /// onStartEntity - use Account as fallback, only for settings

static const uint32_t DBT_NIFO_GF_DEPTHFIRST = 0x01000000;  /// general flags - depth first iteration instead of breath first
static const uint32_t DBT_NIFO_GF_USEROOT    = 0x02000000;  /// general flags - use root as fallback, only for settings
static const uint32_t DBT_NIFO_GF_VL1        = 0x10000000;  /// general flags - limit virtual lookup depth to 1
static const uint32_t DBT_NIFO_GF_VL2        = 0x20000000;  /// general flags - limit virtual lookup depth to 2
static const uint32_t DBT_NIFO_GF_VL3        = 0x30000000;  /// general flags - limit virtual lookup depth to 3
static const uint32_t DBT_NIFO_GF_VL4        = 0x40000000;  /// general flags - limit virtual lookup depth to 4

/**
	\brief Entityfilter options for Entity iteration
**/
typedef
	struct TDBTEntityIterFilter
	{
		uint32_t cbSize;					/// size of the structur in bytes
		uint32_t Options;					/// Options for iteration: DB_EIFO_*
		uint32_t fHasFlags;				/// flags an Entity must have to be iterated
		uint32_t fDontHasFlags;		/// flags an Entity have not to have to be iterated
	} TDBTEntityIterFilter, *PDBTEntityIterFilter;

/**
	\brief Handle of an Entity-Iteration
**/
typedef uintptr_t TDBTEntityIterationHandle;
/**
	\brief initialize an iteration of Entities
	\param wParam = PDBTEntityIterFilter, NULL to iterate all Entities (breadthfirst, all but root)
	\param lParam = TDBTEntityHandle Entity, where iteration starts

	\return EnumID
**/
#define MS_DBT_ENTITY_ITER_INIT "DBT/Entity/Iter/Init"


/**
	\brief get the next Entity
	\param wParam = EnumID returned by MS_DBT_ENTITY_ITER_INIT
	\param lParam = 0

	\return hEntity, 0 at the end
**/
#define MS_DBT_ENTITY_ITER_NEXT "DBT/Entity/Iter/Next"

/**
	\brief closes an iteration and frees its ressourcs
	\param wParam = IterationHandle returned by MS_DBT_ENTITY_ITER_INIT
	\param lParam = 0

	\return 0 on success
**/
#define MS_DBT_ENTITY_ITER_CLOSE "DBT/Entity/Iter/Close"

/**
	\brief Deletes an Entity.

	All children will be moved to its parent.
	If the Entity has virtual copies, history and settings will be transfered to the first duplicate.

	\param wParam = hEntity
	\param lParam = 0

	\return 0 on success
**/
#define MS_DBT_ENTITY_DELETE  "DBT/Entity/Delete"


typedef struct TDBTEntity
{
	uint32_t bcSize;
	TDBTEntityHandle hParentEntity;
	uint32_t fFlags;                 /// Flags DBT_NF_
	TDBTEntityHandle hAccountEntity; /// Needed for normal Entities, reference to AccountEntity for the created one
} TDBTEntity, *PDBTEntity;

/**
	\brief Creates a new Entity.
  \param wParam = PDBTEntity
  \param lParam = 0

	\return hEntity on success, 0 otherwise
**/
#define MS_DBT_ENTITY_CREATE  "DBT/Entity/Create"


/**
	\brief returns the account entity handle specified during creation
  \param wParam = TDBTEntityHandle
  \param lParam = 0

	\return hEntity on success, 0 otherwise
**/
#define MS_DBT_ENTITY_GETACCOUNT "DBT/Entity/GetAccount"


///////////////////////////////////////////////////////////
// Virtual Entities
///////////////////////////////////////////////////////////

/**
	\brief Creates a virtual duplicate of an Entity
  \param wParam = hEntity to duplicate, couldn't be a group (DBT_NF_IsGroup set to 0)
  \param lParam = hParentEntity to place duplicate

	\return hEntity of created duplicate
**/
#define MS_DBT_VIRTUALENTITY_CREATE  "DBT/VirtualEntity/Create"

/**
	\brief Retrieves the original Entity, which this is a duplicate of
  \param wParam = hEntity of virtual Entity
  \param lParam = 0

	\return hEntity of original Entity
**/
#define MS_DBT_VIRTUALENTITY_GETPARENT  "DBT/VirtualEntity/GetParent"

/**
	\brief Retrieves the first virtual duplicate of an Entity (if any)
  \param wParam = hEntity with virtual copies
  \param lParam

	\return hEntity of first virtual duplicate
**/
#define MS_DBT_VIRTUALENTITY_GETFIRST  "DBT/VirtualEntity/GetFirst"

/**
	\brief Retrieves the following duplicate
  \param wParam = hVirtualEntity of virtual Entity
  \param lParam = 0

	\return hEntity of next duplicate, 0 if hVirtualEntity was the last duplicate
**/
#define MS_DBT_VIRTUALENTITY_GETNEXT  "DBT/VirtualEntity/GetNext"


///////////////////////////////////////////////////////////
// Settings
///////////////////////////////////////////////////////////

/**
	\brief Handle of a Setting
**/
typedef uint32_t TDBTSettingHandle;


static const uint16_t DBT_ST_BYTE   = 0x01;
static const uint16_t DBT_ST_WORD   = 0x02;
static const uint16_t DBT_ST_DWORD  = 0x03;
static const uint16_t DBT_ST_QWORD  = 0x04;

static const uint16_t DBT_ST_CHAR   = 0x11;
static const uint16_t DBT_ST_SHORT  = 0x12;
static const uint16_t DBT_ST_INT    = 0x13;
static const uint16_t DBT_ST_INT64  = 0x14;

static const uint16_t DBT_ST_BOOL   = 0x20;
static const uint16_t DBT_ST_FLOAT  = 0x21;
static const uint16_t DBT_ST_DOUBLE = 0x22;

static const uint16_t DBT_ST_ANSI   = 0xff;
static const uint16_t DBT_ST_BLOB   = 0xfe;
static const uint16_t DBT_ST_UTF8   = 0xfd;
static const uint16_t DBT_ST_WCHAR  = 0xfc;

#if (defined(_UNICODE) || defined(UNICODE))
	static const uint16_t DBT_ST_TCHAR  = DBT_ST_WCHAR;
#else
	static const uint16_t DBT_ST_TCHAR  = DBT_ST_ANSI;
#endif

static const uint16_t DBT_STF_Signed         = 0x10;
static const uint16_t DBT_STF_VariableLength = 0x80;



static const uint32_t DBT_SDF_FoundValid  = 0x00000001;
static const uint32_t DBT_SDF_HashValid   = 0x00000002;

/**
	\brief Describes a setting, its name and location
**/
typedef
	struct TDBTSettingDescriptor {
		uint32_t cbSize;                               /// size of the structure in bytes
		TDBTEntityHandle Entity;                      /// Entityhandle where the setting can be found, or where searching starts
		char * pszSettingName;                         /// Setting name
		uint32_t Options;                              /// options describing where the setting can be found DBT_NIFO_*
		uint32_t Flags;                                /// Valid Flags. DBT_SDF_* describes which following values are valid (internal use)

		TDBTEntityHandle FoundInEntity;               /// internal use to avoid to do the searching twice
		uint32_t Hash;                                 /// internal used HashValue for settingname
		TDBTSettingHandle FoundHandle;                  /// internal used SettingHandle
	} TDBTSettingDescriptor, * PDBTSettingDescriptor;

/**
	\brief Describes a settings value

	it is never used alone, without a type qualifier
**/
typedef
	union TDBTSettingValue {
		bool Bool;
		int8_t  Char;  uint8_t  Byte;
		int16_t Short; uint16_t Word;
		uint32_t Int;   uint32_t DWord;
		int64_t Int64; uint64_t QWord;
		float Float;
		double Double;

		struct {
			uint32_t Length;  // length in bytes of pBlob, length in characters of char types including trailing null
			union {
				uint8_t * pBlob;
				char * pAnsi;
				char * pUTF8;
				wchar_t * pWide;
				TCHAR * pTChar;
			};
		};
	} TDBTSettingValue;

/**
	\brief Describes a setting
**/
typedef
	struct TDBTSetting {
		uint32_t cbSize;		          /// size of the structure in bytes
		PDBTSettingDescriptor Descriptor;  /// pointer to a Setting descriptor used to locate the setting
		uint16_t Type;			        /// type of the setting, see DBT_ST_*
		TDBTSettingValue Value;		        /// Value of the setting according to Type
	} TDBTSetting, * PDBTSetting;



/**
	\brief retrieves the handle of the setting
  \param wParam = PDBTSettingDescriptor
  \param lParam = 0

	\return hSetting when found, 0 otherwise
**/
#define MS_DBT_SETTING_FIND  "DBT/Setting/Find"


/**
	\brief deletes the specified Setting
  \param wParam = PDBTSettingDescriptor
  \param lParam = 0

	\return hSetting when found, 0 otherwise
**/
#define MS_DBT_SETTING_DELETE  "DBT/Setting/Delete"

/**
	\brief deletes the specified Setting
  \param wParam = TDBTSettingHandle
  \param lParam = 0

	\return 0 on success
**/
#define MS_DBT_SETTING_DELETEHANDLE  "DBT/Setting/DeleteHandle"


/**
	\brief Write a setting (and creates it if neccessary)
  \param wParam = PDBTSetting
  \param lParam = 0

	\return TDBTSettingHandle on success, 0 otherwise
**/
#define MS_DBT_SETTING_WRITE  "DBT/Setting/Write"

/**
	\brief retrieves the handle of the setting
  \param wParam = PDBTSetting
  \param lParam = TDBTSettingHandle, could be 0 to create new setting, but needs wParam->Descriptor with valid data

	\return hSetting when found or created, 0 otherwise
**/
#define MS_DBT_SETTING_WRITEHANDLE  "DBT/Setting/WriteHandle"

/**
	\brief retrieves the value of the setting
  \param wParam = PDBTSetting
  \param lParam = 0

	\return SettingHandle
**/
#define MS_DBT_SETTING_READ  "DBT/Setting/Read"

/**
	\brief retrieves the value of the setting

	Also retrieves the SettingDescriptor if it is set and prepared correctly (name buffers set etc)
  \param wParam = PDBTSetting
  \param lParam = TDBTSettingHandle

	\return original settings type
**/
#define MS_DBT_SETTING_READHANDLE  "DBT/Setting/ReadHandle"



/**
	\brief Settings Filter Options for setting iteration
**/
typedef
	struct TDBTSettingIterFilter {
		uint32_t cbSize;								  /// size in bytes of this structure
		uint32_t Options;                 /// DBT_NIFO_* flags
		TDBTEntityHandle hEntity;        /// hEntity which settings should be iterated (or where iteration begins)
		char * NameStart;                 /// if set != NULL the iteration will only return settings which name starts with this string
		uint32_t ExtraCount;              /// count of additional Entities which settings are enumerated, size of the array pointed by ExtraEntities
		TDBTEntityHandle * ExtraEntities; /// pointer to an array with additional Entity handles in prioritized order

		PDBTSettingDescriptor Descriptor;  /// if set, the iteration will fill in the correct data, you may set SettingsNameLength and SettingName to a buffer to recieve the name of each setting
		PDBTSetting Setting;	              /// if set, iteration loads every settings value, except variable length data (blob, strings) but returns their length

	} TDBTSettingIterFilter, *PDBTSettingIterFilter;


/**
	\brief Handle of a Setting-Iteration
**/
typedef uintptr_t TDBTSettingIterationHandle;
/**
	\brief initialize an iteration of settings
	\param wParam = PDBTSettingIterFilter
	\param lParam = 0

	\return EnumID
**/
#define MS_DBT_SETTING_ITER_INIT "DBT/Setting/Iter/Init"


/**
	\brief get the next setting
	\param wParam = EnumID returned by MS_DBT_SETTING_ITER_INIT
	\param lParam = 0

	\return hSetting, 0 at the end
**/
#define MS_DBT_SETTING_ITER_NEXT "DBT/Setting/Iter/Next"

/**
	\brief closes an iteration and frees its ressourcs
	\param wParam = IterationHandle returned by MS_DBT_SETTING_ITER_INIT
	\param lParam = 0

	\return 0 on success
**/
#define MS_DBT_SETTING_ITER_CLOSE "DBT/Setting/Iter/Close"


///////////////////////////////////////////////////////////
// Events
///////////////////////////////////////////////////////////

typedef uint32_t TDBTEventHandle;


/**
	\brief this event was sent by the user. If not set this event was received.
**/
static const uint32_t DBT_EF_SENT  = 0x00000002;

/**
	\brief event has been read by the user. It does not need to be processed any more except for history.
**/
static const uint32_t DBT_EF_READ  = 0x00000004;

/**
	\brief event contains the right-to-left aligned text
**/
static const uint32_t DBT_EF_RTL   = 0x00000008;

/**
	\brief event contains a text in utf-8
**/
static const uint32_t DBT_EF_UTF   = 0x00000010;

/**
	\brief event is virtual. it is not stored to db file yet.
**/
static const uint32_t DBT_EF_VIRTUAL   = 0x00000020;

/**
	\brief describes an event
**/
typedef struct TDBTEvent {
	uint32_t    cbSize;     /// size of the structure in bytes
	char *			ModuleName; /// 
	uint32_t    Timestamp;  /// seconds since 00:00, 01/01/1970. Gives us times until 2106 unless you use the standard C library which is signed and can only do until 2038. In GMT.
	uint32_t    Flags;	    /// the omnipresent flags	
	uint32_t    EventType;  /// module-unique event type ID
	uint32_t    cbBlob;	    /// size of pBlob in bytes
	uint8_t  *  pBlob;	    /// pointer to buffer containing module-defined event data
} TDBTEvent, *PDBTEvent;

static const uint32_t DBT_EventType_Message     = 0;
static const uint32_t DBT_EventType_URL         = 1;
static const uint32_t DBT_EventType_Contacts    = 2;
static const uint32_t DBT_EventType_Added       = 1000;
static const uint32_t DBT_EventType_AuthRequest = 1001;  //specific codes, hence the module-
static const uint32_t DBT_EventType_File        = 1002;  //specific limit has been raised to 2000


/**
	\brief retrieves the blobsize of an event in bytes
  \param wParam = hEvent
  \param lParam = 0

	\return blobsize
**/
#define MS_DBT_EVENT_GETBLOBSIZE "DBT/Event/GetBlobSize"



/**
	\brief retrieves all information of an event
  \param wParam = hEvent
  \param lParam = PDBTEvent

	\return 0 on success
**/
#define MS_DBT_EVENT_GET "DBT/Event/Get"

/**
	\brief retrieves all information of an event
  \param wParam = hEntity
  \param lParam = 0

	\return Event count of specified Entity on success, DBT_INVALIDPARAM on error
**/
#define MS_DBT_EVENT_GETCOUNT "DBT/Event/GetCount"


/**
	\brief Deletes the specfied event
  \param wParam = hEntity
  \param lParam = hEvent

	\return 0 on success
**/
#define MS_DBT_EVENT_DELETE "DBT/Event/Delete"

/**
	\brief Creates a new Event
  \param wParam = hEntity
  \param lParam = PDBTEvent

	\return hEvent on success, 0 otherwise
**/

#define MS_DBT_EVENT_ADD "DBT/Event/Add"


/**
	\brief Changes the flags for an event to mark it as read.
  \param wParam = hEntity
  \param lParam = hEvent

	\return New flags
**/
#define MS_DBT_EVENT_MARKREAD "DBT/Event/MarkRead"

/**
	\brief Saves a virtual event to file and changes the flags.
  \param wParam = hEntity
  \param lParam = hEvent

	\return 0 on success
**/
#define MS_DBT_EVENT_WRITETODISK  "DBT/Event/WriteToDisk"

/**
	\brief Retrieves a handle to a Entity that owns hEvent.
  \param wParam = hEvent
  \param lParam = 0

	\return NULL is a valid return value, meaning, as usual, the user.
					DBT_INVALIDPARAM if hDbEvent is invalid, or the handle to the Entity on
					success
**/
#define MS_DBT_EVENT_GETENTITY  "DBT/Event/GetEntity"

/**
	\brief Event Filter Options for event iteration
**/
typedef
	struct TDBTEventIterFilter {
		uint32_t cbSize;										/// size in bytes of this structure
		uint32_t Options;										/// DBT_NIFO_* flags
		TDBTEntityHandle hEntity;					/// hEntity which events should be iterated (or where iteration begins)
		uint32_t ExtraCount;								/// count of additional Entities which settings are enumerated, size of the array pointed by ExtraEntities
		TDBTEntityHandle * ExtraEntities;   /// pointer to an array with additional Entity handles in prioritized order

		uint32_t tSince;                    /// timestamp when to start iteration, 0 for first item
		uint32_t tTill;                     /// timestamp when to stop iteration, 0 for last item

		PDBTEvent Event;	                    /// if set every events data gets stored there

	} TDBTEventIterFilter, *PDBTEventIterFilter;


/**
	\brief Handle of a Event-Iteration
**/
typedef uintptr_t TDBTEventIterationHandle;
/**
	\brief initialize an iteration of events
	\param wParam = PDBTEventIterFilter
	\param lParam = 0

	\return EnumID
**/
#define MS_DBT_EVENT_ITER_INIT "DBT/Event/Iter/Init"


/**
	\brief get the next event
	\param wParam = EnumID returned by MS_DBT_EVENT_ITER_INIT
	\param lParam = 0

	\return hSetting, 0 at the end
**/
#define MS_DBT_EVENT_ITER_NEXT "DBT/Event/Iter/Next"

/**
	\brief closes an iteration and frees its resourcs
	\param wParam = IterationHandle returned by MS_DBT_EVENT_ITER_INIT
	\param lParam = 0

	\return 0 on success
**/
#define MS_DBT_EVENT_ITER_CLOSE "DBT/Event/Iter/Close"



#pragma pack(pop)

#endif
