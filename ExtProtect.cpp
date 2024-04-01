#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>

#include <string>
#include <map>
using namespace std;

#include "SFmpq_static.h"
#include "md5.h"
#include "aes.h"

#include "ExtProtect.h"

const char BackupName[] = "ExtProtect\\Backup";
const char *RemoveFiles[] = {"(attributes)"};
const int numRemoveFiles = 1;

const char *ProtectedFiles[] = {"war3map.j", "war3map.wtg", "war3map.wct", "war3map.w3c",
								"war3map.w3r", "(listfile)", "war3mapUnits.doo", "war3map.w3s", "war3map.imp"};
const int numProtectedFiles = 9;
const int deleteProtectedFiles = 1;
const int createProtectedFiles = 3;
const int optionalProtectedFiles = 6;
const char *RenameFilesFrom[] = {"war3Map.j"};
const char *RenameFilesTo[] = {"Scripts\\war3map.j"};
const int numRenameFiles = 1;
const char *MapFiles[] = {"war3map.w3e", "war3map.w3i", "war3map.wtg", "war3map.wct", "war3map.wts",
						"war3map.j", "war3map.shd", "war3mapMap.blp", "war3map.mmp", "war3map.wpm",
						"war3map.doo", "war3mapUnits.doo", "war3map.w3r", "war3map.w3c", "war3map.w3u",
						"scripts\\war3map.j", "war3mapPreview.tga", "war3mapMap.tga"};
const int numMapFiles = 18;

const char *IDFollows[] = {"array", "handle","widget","boolexpr","eventid","gamestate",
	"event","player","unit","destructable","item","force","group","trigger",
	"triggercondition","triggeraction","timer","location","region","rect","boolean",
	"sound","conditionfunc","filterfunc","unitpool","itempool","race","alliancetype",
	"racepreference","gamestate","igamestate","fgamestate","playerstate","playergameresult",
	"unitstate","eventid","gameevent","playerevent","playerunitevent","unitevent","limitop",
	"widgetevent","dialogevent","unittype","gamespeed","gamedifficulty","gametype","mapflag",
	"mapvisibility","mapsetting","mapdensity","mapcontrol","playerslotstate","volumegroup",
	"camerafield","camerasetup","playercolor","placement","startlocprio","raritycontrol",
	"blendmode","texmapflags","effect","effecttype","weathereffect","fogstate","fogmodifier",
	"dialog","button","quest","questitem","defeatcondition","timerdialog","leaderboard",
	"trackable","gamecache","integer","string","real","unreal","multiboarditem","itemtype",
	NULL
};
const char IDChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_";
const char Delimiters[] = "(),=+-*<>/[]!~`@#$%^&:;{}<>?|";
const char *RequiredIDs[] = {"main", "config"};
const int numRequiredIDs = 2;
const char *EndLineSeps = "\n\r";

enum ERROR_TYPE {ERROR_NONE=0, ERROR_OPENFILE=1, ERROR_MEMORY=2, ERROR_ACCESS=3, ERROR_PASSWORD=4,
	ERROR_TEMPFILE=5, ERROR_NOBACKUP=6} ProtectError = ERROR_NONE;
const char *ErrorStrings[] = {"", "Unable to open map file", "Unable to allocate memory",
	"Unable to access files in map archive", "Incorrect Password", "Unable to create temporary file",
	"The map contains no backup"};

#define error(x) {ProtectError = x; ShowError(__LINE__); return 0;}


const char *GetProtectError(void)
{
	return ErrorStrings[ProtectError];
}

void ShowError(int line)
{
	/*char b[1000];
	sprintf(b, "Error on Line %i: %s", line, GetProtectError());
	MessageBox(0, b, "Error", MB_ICONERROR);*/
}

unsigned int genrand_int8b(void)
{
	return (rand() >> 8) & 0xFFlu;
}

unsigned char *MD5_Hash(const char *string)
{
	MD_CTX context;
	static unsigned char digest[16];
	unsigned int len = (unsigned int)strlen(string);

	MD5Init(&context);
	MD5Update(&context, (unsigned char *)string, len);
	MD5Final(digest, &context);

	return digest;
}

long filesize(const char *filename)
{
	long s;
	int h;
	h = _open(filename, _O_RDONLY);
	if(!h)
		return -1;
	s = _filelength(h);
	_close(h);
	return s;
}

char * ObfuscateJass(char *data, size_t len)
{
	typedef map<string, unsigned int> varmap;
	unsigned int id_base;
	char **prefix;
	char *ptr, *ptr2;
	char *buffer;
	size_t id_len;
	string output;
	varmap vars;
	varmap::const_iterator vpos;

	id_base = rand();

	buffer = new char[len+1];
	if(buffer == NULL)
	{
		error(ERROR_MEMORY);
	}

	//Copy data to buffer, stripping comments and excessive whitespace
	ptr = data;
	ptr2 = buffer;
	while(*ptr != 0)
	{
		if((*ptr == ' ') || (*ptr == '\t'))
		{
			bool preceding_delimiter = (ptr != data) && (strchr(Delimiters, *(ptr-1)) != NULL);
			while((*(ptr+1) == ' ') || (*(ptr+1) == '\t'))
			{
				ptr++;
			}
			bool following_delimiter = (strchr(Delimiters, *(ptr+1)) != NULL);
			//whitespace with a delimiter on only one side can be eliminated
			if((!following_delimiter || !preceding_delimiter) && (following_delimiter || preceding_delimiter))
			{
				ptr++;
			}
		}
		if((*ptr == '/') && (*(ptr+1) == '/'))
		{
			ptr += strcspn(ptr, EndLineSeps);
			continue;
		}
		if(*ptr == '\'')
		{
			*ptr2 = *ptr;
			ptr++;
			ptr2++;
			while((*ptr != '\'') && (*ptr != 0))
			{
				*ptr2 = *ptr;
				ptr++;
				ptr2++;
			}
			if(*ptr != 0)
			{
				*ptr2 = *ptr;
				ptr++;
				ptr2++;
			}
			continue;
		}
		if(*ptr == '"')
		{
			*ptr2 = *ptr;
			ptr++;
			ptr2++;
			while(((*ptr != '"') || (*(ptr-1) == '\\')) && (*ptr != 0))
			{
				*ptr2 = *ptr;
				ptr++;
				ptr2++;
			}
			if(*ptr != 0)
			{
				*ptr2 = *ptr;
				ptr++;
				ptr2++;
			}
			continue;
		}
		if(*ptr == '\r')
			*ptr = '\n';
		*ptr2 = *ptr;
		ptr++;
		ptr2++;
	}
	*ptr2 = 0;

	//Copy Back To data, removing multiple newlines
	ptr = buffer;
	ptr2 = data;
	while(*ptr != 0)
	{
		if(*ptr == '\n')
		{
			ptr += strspn(ptr, "\n \t");
			*ptr2 = '\r';
			ptr2++;
			*ptr2 = '\n';
			ptr2++;
			continue;
		}
		*ptr2 = *ptr;
		ptr++;
		ptr2++;
	}
	*ptr2 = 0;

	//Scan In Variable IDs
	prefix = (char **)IDFollows;
	while((*prefix) != NULL)
	{
		ptr = data;
		do{
			ptr = strstr(ptr, *prefix);
			if(ptr == NULL)
				break;
			if((ptr != data) && (strchr(IDChars, *(ptr-1)) != NULL))
			{
				ptr += strlen(*prefix);
				continue;
			}
			ptr += strlen(*prefix);
			if(*ptr != ' ')
				continue;
			ptr += strspn(ptr, " ");

			id_len = strspn(ptr, IDChars);
			if(id_len == 0)
				break;
			sprintf(buffer, "%*.*s", id_len, id_len, ptr);
			if(_stricmp(buffer, "array") == 0) //array follows other identifiers
				continue;
			id_base += genrand_int8b() + 1;
			vars[string(buffer)] = id_base;
			ptr += id_len;
		}while(ptr != NULL);
		prefix++;
	}
	//Scan in Function IDs (Special Case - Only convert functions declared in the file and not external callbacks)
	const char *const func_str = "function";
	ptr = data;
	do{
		ptr = strstr(ptr, func_str);
		if(ptr == NULL)
			break;
		if((ptr != data) && (strchr(IDChars, *(ptr-1)) != NULL))
		{
			ptr += strlen(func_str);
			continue;
		}
		ptr2 = ptr;
		while(ptr2 != data)
		{
			ptr2--;
			if((*ptr2 == '\n') || (*ptr2 == '\r'))
				break;
			if(*ptr2 > ' ')//not a whitespace character
				break;
		}
		if((ptr2 != data) && (*ptr2 > ' '))
		{
			//Not a function declarations
			ptr += strlen(func_str);
			continue;
		}
		ptr += strlen(func_str);
		if(*ptr != ' ')
			continue;
		ptr += strspn(ptr, " ");

		id_len = strspn(ptr, IDChars);
		if(id_len == 0)
			break;
		sprintf(buffer, "%.*s", id_len, ptr);
		if(_stricmp(buffer, "array") == 0) //array follows other identifiers
			continue;
		id_base += genrand_int8b() + 1;
		vars[string(buffer)] = id_base;
		ptr += id_len;
	}while(ptr != NULL);

	//Don't Rename Certain Required IDs
	for(int i = 0; i < numRequiredIDs; ++i)
	{
		vars.erase(RequiredIDs[i]);
	}

	/*
	//Save a mapping of variable names to integers for debugging
	FILE *fp = fopen("ExtProtect.log", "wt");
	for(vpos = vars.begin(); vpos != vars.end(); ++vpos)
	{
		fprintf(fp, "%s = O%u\n", (vpos->first).c_str(), (vpos->second));
	}
	fclose(fp);
	//*/

	//Replace IDs with ID #s
	ptr = data;
	output.reserve(len);
	do{
		ptr2 = ptr;
		while(*ptr && !strchr(IDChars, *ptr))
		{
			
			if(*ptr == '"')
			{
				len = 1;
				while(*(ptr+len) && ((*(ptr+len) != '"') || (*(ptr+len-1) == '\\')))
				{
					len++;
				}
				sprintf(buffer, "%*.*s", len+1, len+1, ptr);
				output += string(buffer);
				ptr += len;
				if(*ptr)
				{
					ptr++;
				}
			}
			else if(*ptr == '\'')
			{
				len = 1;
				while(*(ptr+len) && (*(ptr+len) != '\''))
				{
					len++;
				}
				sprintf(buffer, "%*.*s", len+1, len+1, ptr);
				output += string(buffer);
				ptr+=len;
				if(*ptr)
				{
					ptr++;
				}
			}
			else
			{
				sprintf(buffer, "%c", *ptr);
				output += string(buffer);
				ptr++;
			}
		}
		if(*ptr == 0)
		{
			break;
		}
		len = strspn(ptr, IDChars);
		if(len > 0)
		{
			sprintf(buffer, "%*.*s", len, len, ptr);
			vpos = vars.find(string(buffer));
			if(vpos == vars.end())
			{
				output += string(buffer);
			}
			else	
			{
				sprintf(buffer, "O%lu", vpos->second);
				output += string(buffer);
			}
			ptr += len;
		}
	}while((ptr != ptr2) && (*ptr != 0));
	delete[] buffer;

	buffer = new char [output.length() + 1];
	if(buffer == NULL)
	{
		error(ERROR_MEMORY);
		return NULL;
	}

	memmove(buffer, output.c_str(), output.length());
	buffer[output.length()] = 0;


	//Replace IDs in quotes with ID #s
	ptr = buffer;	
	len = output.length()+1;
	output.clear();
	output.reserve(len);
	do{
		ptr2 = ptr;
		len = 1;
		if(*ptr == '"')
		{
			while(*(ptr+len) && ((*(ptr+len) != '"') || (*(ptr+len-1) == '\\')))
			{
				len++;
			}
			sprintf(data, "%*.*s", len-1, len-1, ptr+1);

			vpos = vars.find(string(data));
			//only names >= 6 characters long
			if((len-1 < 6) || (vpos == vars.end()))
			{
				sprintf(data, "%*.*s", len+1, len+1, ptr);
				output += string(data);
			}
			else	
			{
				sprintf(data, "\"O%lu\"", vpos->second);
				output += string(data);
			}
			ptr += len;
			if(*ptr)
			{
				ptr++;
			}
		}
		else
		{
			while(*(ptr+len) && (*(ptr+len) != '\"'))
			{
				len++;
			}
			sprintf(data, "%*.*s", len, len, ptr);
			output += string(data);
			ptr += len;
		}
	}while((ptr != ptr2) && (*ptr != 0));
	delete[] buffer;

	buffer = new char [output.length() + 1];
	if(buffer == NULL)
	{
		error(ERROR_MEMORY);
		return NULL;
	}

	memmove(buffer, output.c_str(), output.length());
	buffer[output.length()] = 0;

	return buffer;
}

bool IsW3M(const char *w3m_filename)
{
	MPQHANDLE hMain = 0;
	MPQHANDLE hFile = 0;
	unsigned int i;
	int is_map = 0;
	FILE *fp;

	if((w3m_filename == NULL) || (w3m_filename[0] == 0))
		return false;

	fp = fopen(w3m_filename, "rb");
	if(!fp)
		return false;
	fclose(fp);

	hMain = MpqOpenArchiveForUpdate(w3m_filename, MOAU_OPEN_EXISTING, 0);
	if(hMain == 0)
		return false;
	for(i = 0; i < numMapFiles; ++i)
	{
		if(SFileOpenFileEx(hMain, ProtectedFiles[i], SFILE_SEARCH_CURRENT_ONLY, &hFile) == TRUE)
		{
			is_map++;
			SFileCloseFile(hFile);
		}
	}
	MpqCloseUpdatedArchive(hMain, 0);
	if(is_map >= 1)
		return true;
	return false;
}

bool IsProtected(const char *w3m_filename)
{
	MPQHANDLE hMain = 0;
	MPQHANDLE hFile = 0;
	unsigned int i;
	DWORD file_len, file_len2;
	bool map_protected = false;

	if((w3m_filename == NULL) || (w3m_filename[0] == 0))
		return false;

	hMain = MpqOpenArchiveForUpdate(w3m_filename, MOAU_OPEN_EXISTING, 0);
	if(hMain == 0)
		return false;
	for(i = 0; i < numProtectedFiles; ++i)
	{
		if(SFileOpenFileEx(hMain, MapFiles[i], SFILE_SEARCH_CURRENT_ONLY, &hFile) == FALSE)
		{
			map_protected = true;
			SFileCloseFile(hFile);
			break;
		}
		file_len = SFileGetFileSize(hFile, &file_len2);
		if(file_len == 0)
		{
			map_protected = true;
			SFileCloseFile(hFile);
			break;
		}
		SFileCloseFile(hFile);
	}
	if(map_protected != true)
	{
		for(i = 0; i < numRenameFiles; ++i)
		{
			if(SFileOpenFileEx(hMain, RenameFilesTo[i], SFILE_SEARCH_CURRENT_ONLY, &hFile) == TRUE)
			{
				map_protected = true;
				SFileCloseFile(hFile);
				break;
			}
		}
	}
	MpqCloseUpdatedArchive(hMain, 0);
	return map_protected;
}

bool HasExtBackup(const char *w3m_filename)
{
	MPQHANDLE hMain = 0;
	MPQHANDLE hFile = 0;
	bool hasExtBackup = false;

	if((w3m_filename == NULL) || (w3m_filename[0] == 0))
		return false;

	hMain = MpqOpenArchiveForUpdate(w3m_filename, MOAU_OPEN_EXISTING, 0);
	if(hMain == 0)
		return false;
	
	if(SFileOpenFileEx(hMain, BackupName, SFILE_SEARCH_CURRENT_ONLY, &hFile) == TRUE)
	{
		hasExtBackup = true;
		SFileCloseFile(hFile);
	}

	MpqCloseUpdatedArchive(hMain, 0);
	return hasExtBackup;
}

bool ProtectW3M(const char *w3m_filename, const char *password)
{
	MPQHANDLE hMain = 0;
	unsigned int i;
	
	DWORD file_len, file_len2, bytes_read;
	char temp_filename[256];
	unsigned char *key;
	unsigned char *data, *data2;
	aes encrypter;
	FILE *fp;
	MPQHANDLE hProt = 0;
	MPQHANDLE hFile = 0;

	if((w3m_filename == NULL) || (w3m_filename[0] == 0))
		error(ERROR_OPENFILE);

	hMain = MpqOpenArchiveForUpdate(w3m_filename, MOAU_OPEN_EXISTING | MOAU_OPEN_ALWAYS, 0);
	if(hMain == 0)
		error(ERROR_OPENFILE);
	if((password != NULL) && (password[0] != 0)) //No Password Means No Backup
	{
		key = MD5_Hash(password);

		strcpy(temp_filename, _tempnam(".", "Ext"));
		hProt = MpqOpenArchiveForUpdate(temp_filename, MOAU_CREATE_NEW | MOAU_CREATE_ALWAYS, numProtectedFiles);
		if(hProt == 0)
			error(ERROR_TEMPFILE);

		//Add Protected Files To Backup Archive
		for(i = 0; i < numProtectedFiles; ++i)
		{
			if(SFileOpenFileEx(hMain, ProtectedFiles[i], SFILE_SEARCH_CURRENT_ONLY, &hFile) == FALSE)
			{
				if(i >= optionalProtectedFiles)
					continue;
				MpqCloseUpdatedArchive(hProt,0);
				_unlink(temp_filename);
				MpqCloseUpdatedArchive(hMain, 0);
				error(ERROR_ACCESS);
			}
			file_len = SFileGetFileSize(hFile, &file_len2);
			data = new unsigned char[file_len];
			if(data == NULL)
			{
				SFileCloseFile(hFile);
				MpqCloseUpdatedArchive(hProt,0);
				_unlink(temp_filename);
				MpqCloseUpdatedArchive(hMain, 0);
				error(ERROR_MEMORY);
			}
			if((SFileReadFile(hFile, (LPVOID)data, file_len, &bytes_read, NULL) == FALSE) || (bytes_read != file_len))
			{
				delete[] data;
				SFileCloseFile(hFile);
				MpqCloseUpdatedArchive(hProt,0);
				_unlink(temp_filename);
				MpqCloseUpdatedArchive(hMain, 0);
				error(ERROR_ACCESS);
			}
			SFileCloseFile(hFile);
			MpqAddFileFromBufferEx(hProt, (LPVOID)data, file_len, ProtectedFiles[i], MAFA_COMPRESS | MAFA_ENCRYPT | MAFA_REPLACE_EXISTING | MAFA_EXISTS, MAFA_COMPRESS_DEFLATE, Z_BEST_COMPRESSION);
			delete[] data;
			Sleep(0);
		}
		//Close & Compress
		MpqCompactArchive(hProt);
		MpqCloseUpdatedArchive(hProt,0);

		//Encrypt & Insert Into Map
		file_len = filesize(temp_filename);
		file_len2 = (file_len + 15) & 0xFFFFFFF0ul;
		data = new unsigned char[file_len2];
		if(data == NULL)
		{
			_unlink(temp_filename);
			MpqCloseUpdatedArchive(hMain, 0);
			error(ERROR_MEMORY);
		}
		memset((LPVOID)data, 0, file_len2);
		data2 = new unsigned char[file_len2];
		if(data2 == NULL)
		{
			delete[] data;
			_unlink(temp_filename);
			MpqCloseUpdatedArchive(hMain, 0);
			error(ERROR_MEMORY);
		}

		fp = fopen(temp_filename, "rb");
		if(fp == NULL)
		{
			delete[] data2;
			delete[] data;
			_unlink(temp_filename);
			MpqCloseUpdatedArchive(hMain, 0);
			error(ERROR_TEMPFILE);
		}
		if(fread((LPVOID)data, 1, file_len, fp) != file_len)
		{
			fclose(fp);
			delete[] data2;
			delete[] data;
			_unlink(temp_filename);
			MpqCloseUpdatedArchive(hMain, 0);
			error(ERROR_TEMPFILE);
		}
		fclose(fp);
		_unlink(temp_filename);
		encrypter.set_key(key, 16, both);
		for(i = 0; i < file_len2; ++i)
			data[i] ^= key[i & 0xF];
		for(i = 0; i < file_len2; i += 16)
		{
			encrypter.encrypt(&data[i], &data2[i]);
		}
		delete[] data;
		MpqAddFileFromBufferEx(hMain, (LPVOID)data2, file_len2, BackupName, MAFA_COMPRESS | MAFA_ENCRYPT | MAFA_REPLACE_EXISTING | MAFA_EXISTS, MAFA_COMPRESS_DEFLATE, Z_BEST_COMPRESSION);
		delete[] data2;
	}
	//Obfuscate Jass
	if(SFileOpenFileEx(hMain, "war3map.j", SFILE_SEARCH_CURRENT_ONLY, &hFile) == FALSE)
	{
		MpqCloseUpdatedArchive(hMain, 0);
		error(ERROR_ACCESS);
	}
	file_len = SFileGetFileSize(hFile, &file_len2);
	data = new unsigned char[file_len+1];
	if(data == NULL)
	{
		SFileCloseFile(hFile);
		MpqCloseUpdatedArchive(hMain, 0);
		error(ERROR_MEMORY);
	}
	if((SFileReadFile(hFile, (LPVOID)data, file_len, &bytes_read, NULL) == FALSE) || (bytes_read != file_len))
	{
		delete[] data;
		SFileCloseFile(hFile);
		MpqCloseUpdatedArchive(hMain, 0);
		error(ERROR_ACCESS);
	}
	data[file_len] = 0;
	SFileCloseFile(hFile);
	data2 = (unsigned char *)ObfuscateJass((char *)data, file_len);
	delete[] data;
	if(data2 == NULL)
	{
		MpqCloseUpdatedArchive(hMain, 0);
		error(ERROR_ACCESS);
	}

	MpqAddFileFromBufferEx(hMain, (LPVOID)data2, (DWORD)strlen((char *)data2), "war3map.j", MAFA_COMPRESS | MAFA_ENCRYPT | MAFA_REPLACE_EXISTING | MAFA_EXISTS, MAFA_COMPRESS_DEFLATE, Z_BEST_COMPRESSION);
	delete[] data2;
	//Delete File After Being Backed Up
	for(i = deleteProtectedFiles; i < numProtectedFiles; ++i)
	{
		if((MpqDeleteFile(hMain, ProtectedFiles[i]) == FALSE) && (i < optionalProtectedFiles))
		{
			MpqCompactArchive(hMain);
			MpqCloseUpdatedArchive(hMain, 0);
			error(ERROR_ACCESS);
		}
		if(i < createProtectedFiles)
		{
			char ch = 0;
			MpqAddFileFromBufferEx(hMain, &ch, 0, ProtectedFiles[i],  MAFA_REPLACE_EXISTING | MAFA_EXISTS, MAFA_COMPRESS_STANDARD, 0);
		}
	}
	for(i = 0; i < numRemoveFiles; ++i)
	{
		MpqDeleteFile(hMain, RemoveFiles[i]);
	}
	//Rename Some Files In Original Archive
	for(i = 0; i < numRenameFiles; ++i)
		MpqRenameFile(hMain, RenameFilesFrom[i], RenameFilesTo[i]);

	_unlink(temp_filename);
	MpqCompactArchive(hMain);
	MpqCloseUpdatedArchive(hMain, 0);
	return true;
}
bool UnprotectW3M(const char *w3m_filename, const char *password)
{
	MPQHANDLE hMain = 0;
	unsigned int i;
	DWORD file_len, file_len2, bytes_read;
	char temp_filename[256];
	unsigned char *key;
	unsigned char *data, *data2;
	aes decrypter;
	FILE *fp;
	MPQHANDLE hProt = 0;
	MPQHANDLE hFile = 0;

	if((w3m_filename == NULL) || (w3m_filename[0] == 0))
		error(ERROR_OPENFILE);

	if((password == NULL) || (password[0] == 0)) //No Password Means No Backup, Means No Unprotect
		error(ERROR_NOBACKUP);
	
	key = MD5_Hash(password);

	hMain = MpqOpenArchiveForUpdate(w3m_filename, MOAU_OPEN_EXISTING | MOAU_OPEN_ALWAYS, 0);
	if(hMain == 0)
		error(ERROR_OPENFILE);
	
	for(i = 0; i < numRenameFiles; ++i)
		MpqRenameFile(hMain, RenameFilesTo[i], RenameFilesFrom[i]);

	strcpy(temp_filename, _tempnam(".", "Ext"));

	//Decrypt Backup Data With Supplied Password
	if(SFileOpenFileEx(hMain, BackupName, SFILE_SEARCH_CURRENT_ONLY, &hFile) == FALSE)
	{
		_unlink(temp_filename);
		MpqCloseUpdatedArchive(hMain, 0);
		error(ERROR_ACCESS);
	}
	file_len = SFileGetFileSize(hFile, &file_len2);
	data = new unsigned char[file_len];
	if(data == NULL)
	{
		SFileCloseFile(hFile);
		_unlink(temp_filename);
		MpqCloseUpdatedArchive(hMain, 0);
		error(ERROR_MEMORY);
	}
	data2 = new unsigned char[file_len];
	if(data2 == NULL)
	{
		delete[] data;
		SFileCloseFile(hFile);
		_unlink(temp_filename);
		MpqCloseUpdatedArchive(hMain, 0);
		error(ERROR_MEMORY);
	}
    if((SFileReadFile(hFile, (LPVOID)data, file_len, &bytes_read, NULL) == FALSE) || (bytes_read != file_len))
	{
		delete[] data;
		delete[] data2;
		SFileCloseFile(hFile);
		_unlink(temp_filename);
		MpqCloseUpdatedArchive(hMain, 0);
		error(ERROR_ACCESS);
	}
	SFileCloseFile(hFile);

	decrypter.set_key(key, 16, both);
	for(i = 0; i < file_len; i += 16)
		decrypter.decrypt(&data[i], &data2[i]);
	for(i = 0; i < file_len; ++i)
		data2[i] ^= key[i & 0xF];

	fp = fopen(temp_filename, "wb");
	if(fp == NULL)
	{
		delete[] data;
		delete[] data2;
		_unlink(temp_filename);
		MpqCloseUpdatedArchive(hMain, 0);
		error(ERROR_TEMPFILE);
	}
	if(fwrite((LPVOID)data2, 1, file_len, fp) != file_len)
	{
		fclose(fp);
		delete[] data;
		delete[] data2;
		_unlink(temp_filename);
		MpqCloseUpdatedArchive(hMain, 0);
		error(ERROR_TEMPFILE);
	}
	fclose(fp);	
	delete[] data;
	delete[] data2;

	hProt = MpqOpenArchiveForUpdate(temp_filename, MOAU_OPEN_EXISTING | MOAU_OPEN_ALWAYS | MOAU_READ_ONLY, 0);
	if(hProt == 0)
	{
		_unlink(temp_filename);
		MpqCloseUpdatedArchive(hMain, 0);
		error(ERROR_PASSWORD);
	}

	//Extract Protected Files From Backup
	for(i = 0; i < numProtectedFiles; ++i)
	{
		if(SFileOpenFileEx(hProt, ProtectedFiles[i], SFILE_SEARCH_CURRENT_ONLY, &hFile) == FALSE)
		{
			if(i >= optionalProtectedFiles)
				continue;
			MpqCloseUpdatedArchive(hProt,0);
			_unlink(temp_filename);
			MpqCloseUpdatedArchive(hMain, 0);
			error(ERROR_ACCESS);
		}
		file_len = SFileGetFileSize(hFile, &file_len2);
		data = new unsigned char[file_len];
		if(data == NULL)
		{
			SFileCloseFile(hFile);
			MpqCloseUpdatedArchive(hProt,0);
			_unlink(temp_filename);
			MpqCloseUpdatedArchive(hMain, 0);
			error(ERROR_MEMORY);
		}
		if((SFileReadFile(hFile, (LPVOID)data, file_len, &bytes_read, NULL) == FALSE) || (bytes_read != file_len))
		{
			delete[] data;
			SFileCloseFile(hFile);
			MpqCloseUpdatedArchive(hProt,0);
			_unlink(temp_filename);
			MpqCloseUpdatedArchive(hMain, 0);
			error(ERROR_ACCESS);
		}
		SFileCloseFile(hFile);
		MpqAddFileFromBufferEx(hMain, (LPVOID)data, file_len, ProtectedFiles[i], MAFA_COMPRESS | MAFA_ENCRYPT | MAFA_REPLACE_EXISTING | MAFA_EXISTS, MAFA_COMPRESS_DEFLATE, Z_BEST_COMPRESSION);
		delete[] data;
	}
	for(i = 0; i < numRemoveFiles; ++i)
	{
		MpqDeleteFile(hMain, RemoveFiles[i]);
	}

	//Close & Deletee Backup Data File
	MpqCloseUpdatedArchive(hProt,0);
	_unlink(temp_filename);
	MpqDeleteFile(hMain, BackupName);

	MpqCompactArchive(hMain);
	MpqCloseUpdatedArchive(hMain, 0);
	return true;
}

bool CompressMPQ(const char *mpq_filename)
{
	MPQHANDLE hMain = 0;
	MPQHANDLE hFile = 0;
	DWORD file_len, file_len2, bytes_read;
	char *sub_filename, *listfile;
	unsigned char *data;

	if((mpq_filename == NULL) || (mpq_filename[0] == 0))
		error(ERROR_OPENFILE);

	hMain = MpqOpenArchiveForUpdate(mpq_filename, MOAU_OPEN_EXISTING | MOAU_OPEN_ALWAYS, 0);
	if(hMain == 0)
		error(ERROR_OPENFILE);

	//Read Listfile
	if(SFileOpenFileEx(hMain, "(listfile)", SFILE_SEARCH_CURRENT_ONLY, &hFile) == FALSE)
	{
		MpqCloseUpdatedArchive(hMain, 0);
		error(ERROR_ACCESS);
	}	
	file_len = SFileGetFileSize(hFile, &file_len2);
	listfile = new char[file_len+1];	
	if(listfile == NULL)
	{
		SFileCloseFile(hFile);
		MpqCloseUpdatedArchive(hMain, 0);
		error(ERROR_MEMORY);
	}
	if((SFileReadFile(hFile, (LPVOID)listfile, file_len, &bytes_read, NULL) == FALSE) || (bytes_read != file_len))
	{
		delete[] listfile;
		SFileCloseFile(hFile);
		MpqCloseUpdatedArchive(hMain, 0);
		error(ERROR_ACCESS);
	}
	SFileCloseFile(hFile);
	listfile[file_len] = 0;

	//Process Each File In ListFile
	sub_filename = strtok(listfile, "\n\r");
	while(sub_filename != NULL)
	{
		if(_stricmp(sub_filename+strlen(sub_filename)-4, ".wav") == 0) //Skip Over '.Wav's
			goto CompressMPQ_DoNextFile;

		if(SFileOpenFileEx(hMain, sub_filename, SFILE_SEARCH_CURRENT_ONLY, &hFile) == FALSE)
			goto CompressMPQ_DoNextFile;

		file_len = SFileGetFileSize(hFile, &file_len2);
		data = new unsigned char[file_len];
		if(data == NULL)
		{
			SFileCloseFile(hFile);
			goto CompressMPQ_DoNextFile;
		}
		if((SFileReadFile(hFile, (LPVOID)data, file_len, &bytes_read, NULL) == FALSE) || (bytes_read != file_len))
		{
			delete[] data;
			SFileCloseFile(hFile);
			goto CompressMPQ_DoNextFile;
		}
		SFileCloseFile(hFile);
		MpqAddFileFromBufferEx(hMain, (LPVOID)data, file_len, sub_filename, MAFA_COMPRESS | MAFA_REPLACE_EXISTING | MAFA_EXISTS, MAFA_COMPRESS_DEFLATE, Z_BEST_COMPRESSION);
		delete[] data;

CompressMPQ_DoNextFile:
		sub_filename = strtok(NULL, "\n\r");
	}

	delete[] listfile;
	MpqCompactArchive(hMain);
	MpqCloseUpdatedArchive(hMain, 0);
	return true;
}