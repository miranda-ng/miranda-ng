#pragma once

class AnnounceDatabase
{
  public:
	AnnounceDatabase();
	~AnnounceDatabase();

	enum StatusFlag
	{
		StatusFlag_Offline = 0,
		StatusFlag_Online,
		StatusFlag_Away,
		StatusFlag_Dnd,
		StatusFlag_Na,
		StatusFlag_Occupied,
		StatusFlag_FreeForChat,
		StatusFlag_Invisible,
		StatusFlag_SpeakStatusMsg,
		StatusFlag_SuppressConnect,
	};

	enum EventFlag
	{
		EventFlag_Message = 0,
		EventFlag_Url,
		EventFlag_Added,
		EventFlag_AuthRequest,
		EventFlag_File,
		EventFlag_ReadMsgLength,
		EventFlag_DialogOpen,
		EventFlag_DialogFocused,
	};

	//--------------------------------------------------------------------------
	// Description : get/set a status flags
	//--------------------------------------------------------------------------
	bool getStatusFlag(StatusFlag flag) const;
	void setStatusFlag(StatusFlag flag, bool state);

	//--------------------------------------------------------------------------
	// Description : get/set an event flags
	//--------------------------------------------------------------------------
	bool getEventFlag(EventFlag flag) const;
	void setEventFlag(EventFlag flag, bool state);
	
	//--------------------------------------------------------------------------
	// Description : get/set an event flags
	//--------------------------------------------------------------------------
	unsigned int getMaxMsgSize() const    { return m_max_msg; }
	void setMaxMsgSize(unsigned int size) { m_max_msg = size; }
	
	//--------------------------------------------------------------------------
	// Description : load/save the settings from the miranda database
	//--------------------------------------------------------------------------
	void load();
	void save();

  private:
	unsigned int m_status_flags;
	unsigned int m_event_flags;

	unsigned int m_max_msg;
};