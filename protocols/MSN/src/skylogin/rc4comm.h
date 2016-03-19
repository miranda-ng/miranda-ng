int RC4Comm_Init(LSConnection *pConn);
int RC4Comm_Send (LSConnection *pConn, const char * buf, int len);
int RC4Comm_Recv (LSConnection *pConn, char * buf, int len);
