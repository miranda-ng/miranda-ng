#ifndef __KEYS_H__
#define __KEYS_H__

#define publickeyuserid 0
#define secretkeyuserid 1

void initKeyUserIDs(const int atype);
void updateKeyUserIDs(const int atype);
void releaseKeyUserIDs(const int atype);
char *getKeyUserID(const int atype, const int aindex);
int getKeyUserIDCount(const int atype);

#endif // __KEYS_H__
