#ifndef CRC_H
#define CRC_H
/* This computes a 32 bit CRC of the data in the buffer, and returns the
   CRC.  The polynomial used is 0xedb88320. */
unsigned int crc32(const unsigned char *buf, unsigned int len, int salt);
#endif /* CRC_H */ 
