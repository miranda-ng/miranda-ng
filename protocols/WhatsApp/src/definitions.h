#if !defined(DEFINITIONS_H)
#define DEFINITIONS_H

#define FLAG_CONTAINS(x,y)      ( ( x & y ) == y )
#define REMOVE_FLAG(x,y)        ( x = ( x & ~y ))

#define CODE_BLOCK_CATCH_ALL \
		    catch (WAException& e) { debugLogA("Exception: %s", e.what()); \
        } catch (exception& e) { debugLogA("Exception: %s", e.what()); \
        } catch (...) { debugLogA("Unknown exception"); }

#define NIIF_INTERN_TCHAR NIIF_INTERN_UNICODE

#endif