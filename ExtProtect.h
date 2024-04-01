#ifndef __ExtProtect_h__
#define __ExtProtect_h__

const char *GetProtectError(void);
char * ObfuscateJass(char *data, size_t len);
bool IsW3M(const char *w3m_filename);
bool IsProtected(const char *w3m_filename);
bool HasExtBackup(const char *w3m_filename);
bool ProtectW3M(const char *w3m_filename, const char *password);
bool UnprotectW3M(const char *w3m_filename, const char *password);
bool CompressMPQ(const char *mpq_filename);

#endif