#include "launcher.h"
#include <cstring>
#include <cstdint>
#include <cstddef>
#include <sysfn/sha1.h>
#include <windows.h>

namespace trace {

typedef char T_Sha1[SHA1_DIGEST_SIZE];

const char * in1 = "abcdefgh";
const char * out1 = "\x42\x5a\xf1\x2a\x07\x43\x50\x2b" "\x32\x2e\x93\xa0\x15\xbc\xf8\x68\xe3\x24\xd5\x6a";
char buf[SHA1_DIGEST_SIZE];

bool getHashFromFile (char const * fname, T_Sha1 sha)
{
	FILE * file;
	if (0 == fopen_s(&file, fname, "rb"))
	{
		printf("launcher: hashing file %s\n", fname);
		char buf[SHA1_DIGEST_SIZE];
		int const res = sha1_stream(file, sha);
		fclose(file);
		return res == 0;
	}
	return false;
}

bool compareFiles (char const * filename0, char const * filename1)
{
	T_Sha1 hash0;
	memset(hash0, 0, SHA1_DIGEST_SIZE);
	T_Sha1 hash1;
	memset(hash1, 0, SHA1_DIGEST_SIZE);

	bool const do_compare = getHashFromFile(filename0, hash0) && getHashFromFile(filename1, hash1);
	if (do_compare)
		return 0 == memcmp(hash0, hash1, SHA1_DIGEST_SIZE);
	return false;
}

bool fileExists (char const * fname)
{
	FILE * file;
	if (0 == fopen_s(&file, fname, "rb"))
	{
		fclose(file);
		return true;
	}
	return false;
}

void runTraceServer (char const * runname, char * args)
{
	STARTUPINFO startupInfo;
	PROCESS_INFORMATION processInformation;

	ZeroMemory(&startupInfo, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);
	ZeroMemory(&processInformation, sizeof(processInformation));

	if (CreateProcess(runname, args, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &startupInfo, &processInformation))
	{
		CloseHandle(processInformation.hProcess);
		CloseHandle(processInformation.hThread);
	}
}

bool tryCopyTraceServer (char const * origname, char const * runname)
{
	if (BOOL res = CopyFile(origname, runname, false))
	{
		printf("launcher: copied to destination.\n");
		return true;
	}
	printf("launcher: copy failed with error=0x%08x\n", GetLastError());
	return false;
}

bool tryUpdateTraceServer (char const * origname, char const * runname)
{
	bool const orig_exists = fileExists(origname);
	if (!orig_exists)
	{
		printf("launcher: Error: source file not present, nothing to do\n");
		return false;
	}
	if (!fileExists(runname))
	{
		printf("launcher: dest file not existing, copying...\n");
		tryCopyTraceServer(origname, runname);
		return false;
	}

	printf("launcher: both files exists, checking...\n");
	if (false == compareFiles(origname, runname))
	{
		printf("launcher: files differ, trying simple copy...\n");
		return true;
	}
	else
	{
		printf("launcher: both files are up to date.\n");
		return false;
	}
}

} // namespace

