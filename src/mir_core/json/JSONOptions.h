#ifndef JSON_OPTIONS_H
#define JSON_OPTIONS_H

/**
 *  This file holds all of the compiling options for easy access and so
 *  that you don't have to remember them, or look them up all the time
 */

/*
 *  JSON_LIBRARY must be declared if libjson is compiled as a static or dynamic 
 *  library.  This exposes a C-style interface, but none of the inner workings of libjson
 */
#define JSON_LIBRARY

/*
 *  JSON_DEBUG is used to perform extra error checking.  Because libjson usually 
 *  does on the fly parsing, validation is impossible, so this option will allow
 *  you to register an error callback so that you can record what is going wrong 
 *  before the library crashes.  This option does not protect from these errors,
 *  it simply tells you about them, which is nice for debugging, but not preferable
 *  for release candidates
 */
//#define JSON_DEBUG

/*
 *  JSON_SAFE performs similarly to JSON_DEBUG, except this option does protect
 *  from the errors that it encounters.  This option is recommended for those who
 *  feel it's possible for their program to encounter invalid json.
 */
#define JSON_SAFE

/*
 *  JSON_STDERROR routes error messages to cerr instead of a callback, this 
 *  option hides the callback registering function.  This will usually display
 *  messages in the console
 */
//#define JSON_STDERROR

/*
 *  JSON_PREPARSE causes all parsing to be done immediately.  By default, libjson
 *  parses nodes on the fly as they are needed, this makes parsing much faster if
 *  your program gets a lot of information that it doesn't need.  An example of
 *  this would be a client application communicating with a server if the server
 *  returns things like last modified date and other things that you don't use.
 */
//#define JSON_PREPARSE

/*
 *  JSON_LESS_MEMORY will force libjson to let go of memory as quickly as it can
 *  this is recommended for software that has to run on less than optimal machines.
 *  It will cut libjson's memory usage by about 20%, but also run slightly slower.
 *  It's recommended that you also compile using the -Os option, as this will also
 *  reduce the size of the library
 */
//#define JSON_LESS_MEMORY

/*
 *  JSON_UNICODE tells libjson to use wstrings instead of regular strings, this
 *  means that libjson supports the full array of unicode characters, but also takes
 *  much more memory and processing power.
 */
//#define JSON_UNICODE

/*
 *  JSON_REF_COUNT causes libjson to reference count JSONNodes, which makes copying
 *  and passing them around much faster.  It is recommended that this stay on for
 *  most uses
 */
#define JSON_REF_COUNT

/*
 *  JSON_BINARY is used to support binary, which is base64 encoded and decoded by libjson,
 *  if this option is not turned on, no base64 support is included
 */
//#define JSON_BINARY

/*
 *  JSON_MEMORY_CALLBACKS exposes functions to register callbacks for allocating, resizing,
 *  and freeing memory.  Because libjson is designed for costomizability, it is feasible
 *  that some users would like to further add speed by having the library utilize a memory
 *  pool.  With this option turned on, the default behavior is still done internally unless
 *  a callback is registered.  So you can have this option on and mot use it.
 */
#define JSON_MEMORY_CALLBACKS

/*
 *  JSON_MEMORY_MANAGE is used to create functionality to automatically track and clean
 *  up memory that has been allocated by the user.  This includes strings, binary data, and
 *  nodes.  It also exposes bulk delete functions.
 */
//#define JSON_MEMORY_MANAGE

/*
 *  JSON_MUTEX_CALLBACKS exposes functions to register callbacks to lock and unlock
 *  mutexs and functions to lock and unlock JSONNodes and all of it's children.  This 
 *  does not prevent other threads from accessing the node, but will prevent them from
 *  locking it. It is much easier for the end programmer to allow libjson to manage
 *  your mutexs because of reference counting and manipulating trees, libjson automatically
 *  tracks mutex controls for you, so you only ever lock what you need to
 */
//#define JSON_MUTEX_CALLBACKS

/*
 *  JSON_MUTEX_MANAGE lets you set mutexes and forget them, libjson will not only keep
 *  track of the mutex, but also keep a count of how many nodes are using it, and delete
 *  it when there are no more references
 */
//#define JSON_MUTEX_MANAGE

/*
 *  JSON_ISO_STRICT turns off all code that uses non-standard C++.  This removes all
 *  references to long long and long double as well as a few others
 */
//#define JSON_ISO_STRICT

/*
 *  JSON_ITERATORS turns on all of libjson's iterating functionality.  This would usually
 *  only be turned off while compiling for use with C
 */
//#define JSON_ITERATORS

/*
 *  JSON_WRITER turns on libjson's writing capabilties.  Without this libjson can only
 *  read and parse json, this allows it to write back out
 */
#define JSON_WRITER

/*
 *  JSON_NEWLINE affects how libjson writes.  If this option is turned on, libjson
 *  will use whatever it's defined as for the newline signifier, otherwise, it will use 
 *  standard unix \n.
 */
//#define JSON_NEWLINE "\r\n"  //\r\n is standard for most windows and dos programs

/*
 *  JSON_COMMENTS tells libjson to store and write comments.  libjson always supports
 *  parsing json that has comments in it as it simply ignores them, but with this option
 *  it keeps the comments and allows you to insert further comments
 */
//#define JSON_COMMENTS

/*
 *  JSON_INDENT affects how libjson writes.  If this option is turned on, libjson
 *  will use \t to indent formatted json, otherwise it will use the number of characters
 *  that you specify.  If this is not turned on, then it will use the tab (\t) character
 */
//#define JSON_INDENT "    "

/*
 *  JSON_WRITE_BASH_COMMENTS will cause libjson to write all comments in bash (#) style
 *  if this option is not turned on, then it will use C-style comments.  Bash comments are
 *  all single line
 */
//#define JSON_WRITE_BASH_COMMENTS

/*
 *  JSON_WRITE_SINGLE_LINE_COMMENTS will cause libjson to write all comments in using //
 *  notation, or (#) if that option is on.  Some parsers do not support multiline C comments
 *  although, this option is not needed for bash comments, as they are all single line anyway
 */
//#define JSON_WRITE_SINGLE_LINE_COMMENTS

/*
 *  JSON_VALIDATE turns on validation features of libjson.  This option requires JSON_SAFE
 */
//#define JSON_VALIDATE

/*
 *  JSON_CASE_INSENSITIVE_FUNCTIONS turns on funtions for finding child nodes in a case-
 *  insenititve way
 */
//#define JSON_CASE_INSENSITIVE_FUNCTIONS

/*
 *  JSON_UNIT_TEST is used to maintain and debug the libjson.  It makes all private
 *  members and functions public so that tests can do checks of the inner workings
 *  of libjson.  This should not be turned on by end users.
 */
//#define JSON_UNIT_TEST

/*
 *  JSON_INDEX_TYPE allows you th change the size type for the children functions. If this 
 *  option is not used then unsigned int is used.  This option is useful for cutting down
 *  on memory, or using huge numbers of child nodes (over 4 billion)
 */
//#define JSON_INDEX_TYPE unsigned int

#endif
