    //auth
    //db event added to NULL contact
    //blob format is:
    //ASCIIZ    text
    //DWORD     uin
    //HANDLE    hContact
    ICQEVENTTYPE_AUTH_GRANTED   = 2004;    //database event type
    ICQEVENTTYPE_AUTH_DENIED    = 2005;    //database event type
    ICQEVENTTYPE_SELF_REMOVE    = 2007;    //database event type
    ICQEVENTTYPE_FUTURE_AUTH    = 2008;    //database event type
    ICQEVENTTYPE_CLIENT_CHANGE  = 2009;    //database event type
    ICQEVENTTYPE_CHECK_STATUS   = 2010;    //database event type
    ICQEVENTTYPE_IGNORECHECK_STATUS = 2011;//database event type

    //broadcast from server
    //ASCIIZ    text
    //ASCIIZ    from name
    //ASCIIZ    from e-mail
    ICQEVENTTYPE_BROADCAST      = 2006;    //database event type
