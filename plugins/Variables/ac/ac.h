// AMIP public API
#ifndef _AC_H_
#define _AC_H_

enum ac_StartMode {
  AC_START_ALL = 0,
  AC_START_CLIENT,
  AC_START_SERVER,
  AC_START_NONE
};

enum ac_ErrorCode {
  AC_ERR_NOERROR = 0,
  AC_ERR_CLIENTISNULL,
  AC_ERR_EXCEPTION,
  AC_ERR_CONNECTIONFAILED,
  AC_ERR_SERVERNOTRUNNING
};

#define AC_BUFFER_SIZE       2048

// flags for event listener
#define AC_EVT_PLAY          0x0001
#define AC_EVT_PAUSE         0x0002
#define AC_EVT_STOP          0x0004
#define AC_EVT_START         0x0008
#define AC_EVT_EXIT          0x0010

#define AC_EVT_TIMER         0x0020
#define AC_EVT_MSG           0x0040

#define AC_EVT_CHANGE        0x0080

#define AC_EVT_PLCHANGE      0x0100
#define AC_EVT_PLREADY       0x0200

// doesn't include AC_EVT_TIMER, because it can be expensive and usually not necessary to use
// doesn't include AC_EVT_MSG. It's delivered to the message callback function and is never delivered to
// event callback function
#define AC_EVT_ALL           AC_EVT_PLAY | AC_EVT_PAUSE | AC_EVT_STOP | AC_EVT_START | AC_EVT_EXIT | AC_EVT_CHANGE | AC_EVT_PLCHANGE | AC_EVT_PLREADY

typedef VOID (CALLBACK* AC_MSG_CALLBACK) (const char *);
typedef VOID (CALLBACK* AC_EVT_CALLBACK) (int);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

  // Initializes client part of the component. Parameters specify host and port client
  // should connect to (where AMIP is running), connection timeout in milliseconds, number
  // of seconds (dsec) to suspend connections for if failed to connect dcount times.
  // returns 1 if client was initialized properly, 0 otherwise
  int WINAPI ac_init_client(const char *host, int port, int timeout, int dsec, int dcount);

  // Initializes and starts the server part of the component. Server part listens for
  // incoming connections from AMIP, it receives messages and events on specified host and port.
  // returns 1 if server was started successfully, 0 otherwise
  int WINAPI ac_init_server(const char *host, int port);

  // Alternative method to start services, differs from 2 specified above in the way
  // where it gets the configuration data. Startup data is read from the ac.ini file.
  // Don't use ac_init() together with ac_init_client() and ac_init_server()!
  // Call to start services (AC_START_ALL to start both client and server)
  // AC_START_CLIENT will start client only (you can query and control AMIP)
  // AC_START_SERVER will start server only (you can accept song announcements from AMIP)
  void WINAPI ac_init(int mode);
  
  // Call when you finished working with AMIP (e.g. on exit)
  void WINAPI ac_uninit();

  // Useful if you need to uninit the client to init it later with different options
  void WINAPI ac_uninit_client();

  // Stops the server, you can start it again with different options later
  // Subsequent calls to ac_uninit_client() and ac_stop_server() can be replaced with the
  // single call to ac_uninit()
  void WINAPI ac_stop_server();

  // Passes command to AMIP. For the list of commands see AMIP Help.
  // Remove '/dde mplug' prefix to get the real command, for instance
  // in the help you see '/dde mplug announce preset 1' command, this
  // function will accept 'announce preset 1' as the parameter
  int WINAPI ac_exec(const char *cmd);

  // Evaluates AMIP's variable and puts the result into the result buffer
  // cmd can be var_<variable>, where variable is any of the AMIP variables
  // without %, for instance %name becomes var_name. Also cfg_<parameter>
  // variables are supported (cfg_enabled, cfg_rmienabled, etc.)
  // Basically, all the $dde variables from help can be evaluated via this
  // function ('$dde mplug format "%name"' becomes 'format "%name"')
  // Warning: result buffer must have AC_BUFFER_SIZE capacity
  int WINAPI ac_eval(const char *cmd, char *result);

  // same as ac_eval but takes a format spec string and evaluates it all, the format
  // spec may look like "%1 - %2 (%br~kbps)"
  int WINAPI ac_format(const char *cmd, char *result);
  // ac_exec and ac_eval return one of the AC_ERR_* codes
  // if function succeeds, the return code is AC_ERR_NOERROR (0)
  // if ac_eval fails, empty string is placed into the result buffer

  // Registers callback function which will receive all messages from AMIP.
  // Pass address of your function taking char* as an argument and it will
  // be called every time AMIP has something to say you
  void WINAPI ac_register_msg_callback(AC_MSG_CALLBACK);

  // Event callback will accept events from AMIP if listener for events was added
  void WINAPI ac_register_evt_callback(AC_EVT_CALLBACK);
  
  // Adds listener to AMIP, AMIP will notify host:port about the events specified by flags
  // until listener is removed or fail_count limit is reached. If notification fails
  // fail_count times, AMIP automatically removes listener for the specified host:port.
  // AMIP keeps listeners even between restarts (in plugin.ini file)
  int WINAPI ac_add_event_listener(const char *host, int port, int timeout, UINT flags, UINT fail_count);
  
  // You must unregister all listeners that you have registered before your application exits
  int WINAPI ac_remove_event_listener(const char *host, int port);


  // Ping server on the specified address and port
  // returns true if there is AMIP server running
  // returns false if server cannot be reached within the specified timeout (ms)
  BOOL WINAPI ac_pingServer(const char *host, int port, int timeout);


  // Playlist related functions:
  // Gets playlist from AMIP and caches it, you should use this function every time playlist changes
  // (AC_EVT_PLCHANGE event received) and before using any other playlist related functions.
  // The correct usage sequence:
  //   1. Register listener for AC_EVT_PLCHANGE and AC_EVT_PLREADY events
  //   2. When you receive AC_EVT_PLCHANGE event via callback or upon first usage you must re-index playlist
  //      using ac_exec("reindexq") function call (AMIP builds playlist cache)
  //   3. When playlist is re-indexed, you will receive AC_EVT_PLREADY event, only at this moment you should
  //      call ac_get_pl() function (this function gets cached playlist from AMIP)
  // Return code is the same as for ac_exec and ac_eval functions, see ac_ErrorCode enum
  int WINAPI ac_get_pl();
  
  // Returns the size of playlist cached by client. You can compare it with the size of real playlist, the
  // size of playlist cached by AMIP and do re-index and ac_get_pl to be in sync if necessary
  int WINAPI ac_get_plsize();
  
  // Returns 1 if title is not NULL and within playlist bounds, 0 otherwise
  // Title with the specified zero-based idx is copied to buff. buff must have at least AC_BUFFER_SIZE size
  // Make sure to prepare playlist first, see ac_get_pl() function comments, use ac_get_plsize() to determine
  // playlist size
  int WINAPI ac_get_title(UINT idx, char *buff);


  // configuring Client
  // AMIP port client will try to connect to (default 60333)
  void WINAPI ac_setDestPort(int port);

  // AMIP host client will try to connect to (default 127.0.0.1)
  void WINAPI ac_setDestHost(const char *host);
  
  // Client timeout
  void WINAPI ac_setTimeout(int ms);

  // Source port the client will listen for AMIP commands on (default 60334)
  void WINAPI ac_setSrcPort(int port);
  
  // Source host interface which will accept AMIP connections (default 127.0.0.1)
  void WINAPI ac_setSrcHost(const char *host);

  // get configuration
  int WINAPI ac_getSrcPort();
  int WINAPI ac_getDestPort();
  void WINAPI ac_getSrcHost(char *out_host);
  void WINAPI ac_getDestHost(char *out_host);

  // Reload the configuration and restart services
  void WINAPI ac_rehash();

  // Returns the major part of API version (for ac 1.2 will return 1)
  int WINAPI ac_version_major();

  // Returns the mainor part of API version (for ac 1.2 will return 2)
  int WINAPI ac_version_minor();

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*_AC_H_*/
