#if !defined(DEFINITIONS_H)
#define DEFINITIONS_H

#define FLAG_CONTAINS(x,y)      ( ( x & y ) == y )
#define REMOVE_FLAG(x,y)        ( x = ( x & ~y ))

#define LOG(fmt, ...) Log(__FUNCTION__, fmt, ##__VA_ARGS__)
#define CODE_BLOCK_TRY              try {
#define CODE_BLOCK_CATCH(ex)        } catch (ex& e) { LOG("Exception: %s", e.what());
#define CODE_BLOCK_CATCH_UNKNOWN    } catch (...) { LOG("Unknown exception");
#define CODE_BLOCK_CATCH_ALL        } catch (WAException& e) { LOG("Exception: %s", e.what()); \
                                    } catch (exception& e) { LOG("Exception: %s", e.what()); \
                                    } catch (...) { LOG("Unknown exception"); }
#define CODE_BLOCK_END              }

#define NIIF_INTERN_TCHAR NIIF_INTERN_UNICODE

#endif