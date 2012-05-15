
        Version history:

0.0.2.6 - 2006/08/03

	  * added file hashing (pescuma)
	  * rewrote the picture loading code. It is now running in a background thread
	    at low priority. The loader thread is the only instance which actually writes 
	    to the cache entries. It does this with a small delay and notifies the hook
	    subscribers when a picture has been loaded. While it is not fully loaded,
	    the fallback protocol picture may be used. This makes connecting or 
	    loading the clist with lots of avatars significantly faster at the cost
	    of delayed picture appearance.
	    It may also help with the sporadic lockups since the part of the code which
	    runs in the callers thread context is now much simpler.

0.0.2.5 - 2006/07/30

	  * fixed updater support
	  * moved InitPolls() to ModulesLoaded() to avoid a problem with missing
	    core services.
	  * fixed keyboard navigation glitch in the Customize->Contact pictures dialog.

0.0.2.4 - 2006/07/30

      * bugfix (missing protocol avatars after migrating to new relpath)
      * added proper version info resource record (showing ansi/unicode)      
      * added project files for Visual C++ 6 (source code release only)

0.0.2.3 - 2006/07/11

      * should now work better with metacontacts. Subcontact avatar changes are
        "forwarded" to the master contact.
        
      * relative path names are now relative to the DATABASE directory, not
        Mirandas root directory.
        
      * missing picture files and/or invalid database entries will now result
        in a refresh request.                  

0.0.2.0 introduced big internal changes. New request queue system (written by Pescuma)
        loadavatars no longer depends on imgdecoder.dll, but needs a recent png2dib
        plugin (supplied with Miranda 0.4.3.x nightly builds)
        
0.0.1.12 - 0.0.2.2 - released via nightlys (see SVN changelog)

0.0.1.11 - 2005/11/10

        + added patch by pescuma for making normal avatars transparent (or semi-
          transparent).
          There are global (under Customize->Contact pictures) and per contact settings
          (contact menu -> Contact picture...). In order to get good results, you will need
          to tweak the settings a bit - it works for most avatar pictures with a uni-color
          background, but it really depends on the image.
          
		+ built with Visual Studio 2005 (Visual C++ 8.0). The DLL is now statically
		  linked, so they are somewhat bigger, but it avoids error messages because of the
		  new VC++ 8 runtime which is not installed on most PCs.

		+ added unicode version (loadavatarsW.dll) which will display unicode nicknames in
		  the "per user" contact picture dialog.
		  
		+ added services and events to manage own avatar pictures. See m_avatars.h for more
		  information. Also, the drawing service was enhanced to allow drawing your own
		  avatars.
		  
0.0.1.10 - 2005/10/19

        ! added support for jabber avatar updates.

        + better cleanup on Unload()
        
0.0.1.9 - 2005/10/18

        * accept .dat as valid image extension (sometimes, icq avatars are saved as .dat 
          files, even if they are actually JPG images (reason unknown, but we can load
          them anyway. MS_UTILS_LOADBITMAP will return an error when the file is not
          really a valid image - the extension doesn't matter).

0.0.1.8 - 2005/10/17

        ! fix. Don't show tray warning messages about invalid avatar file size when
          the option "Show warning messages" on Customize->Contact pictures is unchecked.
          
        * changed way how protocols are enabled/disabled. After protocols have been added
          or removed, "new" protocols are automatically enabled, so you don't have to do
          that manually any more.
          
        * you can completely disable the size limit check by setting the size limit on
          Customize->Contact List to 0 (zero).

0.0.1.7 - 2005/10/16

        * bugfix: check filenames more aggressivly.

        * bugfix: premultiply was broken (thanks FYR)
        
        + added a service to draw a contacts picture to a target device context.
          See m_avatars.h for description on how to use that service.
          
        + added file size limitation to the picture loader. The default are 70Kbytes,
          should be enough for most avatar pictures (they have to be small images).
          The limit can be increased on the option page (Customize->contact pictures).

0.0.1.5 - 2005/09/15

        + added updater support
        
        * moved option page to Customize->Contact pictures

0.0.1.4 - 2005/08/31

        * cache reallocs will now send avatar change notifies (realloc() may move the
          cache in memory, so pointers to avatar cache entries may become invalid).

        ! bugfix - badly written protocols which load themself too late are now
          skipped and won't cause troubles because of the protocol list reallocation.

0.0.1.3 - 2005/08/30

        + ability to lock the avatar (protect from automatic updates). Useful, if you 
          have set a local contact picture and don't want it to be overwritten.
          Actually, the feature does not really lock the avatar updating - it will
          only save the picture from being overwritten.

        + added UI to set/change/lock avatar. DEVELOPERS please check m_avatars.h on how
          to use the service from your own plugins. There are now services to lock and/or
          set the avatar, either by providing a valid image filename or by using a file
          selection dialog. Also, there is a service to call the avatar dialog for a
          hContact.

        * struct avatarCacheEntry has changed too. It now provides szFilename which
          will contain the full image filename of the current avatar in use.
          
        * added a menu item to the contact menu which allows to set avatar options (local
          picture, locked avatar, and a "hidden" attribute).
          The hidden attribute is set in the struct avatarCacheEntry

        + added support for PNG images (imgdecoder.dll needs to be present in either the 
          main miranda directory or the \Plugins subfolder. Transparent PNG images are
          supported and the avatar service will perform the premultiplication of alpha
          values so that the image can (and should) be rendered with the AlphaBlend()
          API.

0.0.1.2 - 2005/08/20

        + the service now creates and uses relative filenames for all contact pictures,
          including protocol avatars, if possible.
          
        * for protocols which are in "invisible" status, no active avatar fetching
          is performed. This is to avoid problems with MSN and privacy issues for
          other protocols (if you're invisible, then the protocol should not initiate
          any outbound communications which may require direct p2p connections).

        * an option page has been added. Currently, you can:
        
          + select, for which protocols the service should ACTIVELY fetch avatars.
            If you set a protocols to "inactive" (uncheck it), already existing
            avatars for contacts of that protocol will continue to work, but the
            service will no longer actively refresh avatars.
            
          + set protocol pictures (pseudo avatars). You can select local pictures
            for each installed protocol. These will be used as a fallback then, if
            no avatar is available for a contact. You can also remove these pictures.

        * don't fetch avatars for contacts which are either blocked or set on the
          invisibility list (ApparentMode == ID_STATUS_OFFLINE). Reason: No active
          outbound communications w/o user intervention should be initiated for blocked 
          contacts.

        + added support for the updater plugin.
        
0.0.1.1 - 2005/08/06

        * changed API. Don't return HBITMAP directly, but instead, it returns
          a pointer to a struct avatarCacheEntry which can be used to obtain
          the bitmap handle.

0.0.1.0 - 2005/08/05

        * initial release, cvs import.
        
