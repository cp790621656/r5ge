#include "../Include/_All.h"

#ifdef _WINDOWS
  #include <direct.h>
#else
  #include <unistd.h>
  #define _chdir chdir
#endif

#include <stdarg.h>

using namespace R5;

//============================================================================================================
// File container that has a destructor that will properly close the file
//============================================================================================================

struct File
{
	FILE* fp;
	Thread::Lockable lock;

	File (const char* filename) { fp = fopen(filename, "w"); }
	~File() { if (fp) { fflush(fp); fclose(fp); fp = 0; } }

	operator FILE* () { return fp; }
	operator bool () const { return fp != 0; }

	void Lock()   const { lock.Lock(); }
	void Unlock() const { lock.Unlock(); }
};

//============================================================================================================
// Changes the local working directory
//============================================================================================================

bool System::SetCurrentPath(const char* path)
{
	return (_chdir(path) == 0);
}

//============================================================================================================
// Dumps a string into the log file
//============================================================================================================

void System::Log(const char *format, ...)
{
	static File log ("log.txt");
	static ulong last = 0;

	FILE* file = log;

	// In rare cases the log may need to be used after the static variable has been released.
	// If this happens, open the log manually and simply append to it.
	if (file == 0) file = fopen("log.txt", "a");

	if (file != 0)
	{
		if (log) log.Lock();
		{
			va_list args;
			va_start(args, format);
			
#ifdef _WINDOWS
			uint iLength = _vscprintf(format, args) + 1;
			char* text = new char[iLength];
#else
			static char text[2048];
#endif
			vsprintf(text, format, args);
			va_end(args);

			Time::Update();
			ulong totalMS	= Time::GetMilliseconds();
			ulong remMS		= totalMS % 1000;
			ulong totalSEC	= totalMS / 1000;
			ulong remSEC	= totalSEC % 60;
			ulong totalMIN	= totalSEC / 60;
			ulong remMIN	= totalMIN % 60;
			ulong totalHRS	= totalMIN / 60;
			
			fprintf(file, "[%3d.%02d.%02d.%03d]: ", totalHRS, remMIN, remSEC, remMS);
			fputs(text, file);
			fputc('\n', file);

#ifdef _WINDOWS
			delete [] text;
#endif
			// Write to file no more frequently than every 100 milliseconds
			if (log && totalMS - last > 100)
			{
				last = totalMS;
				fflush(file);
			}
		}
		if (log) log.Unlock();
		else fclose(file);
	}
}

//============================================================================================================
// Checks if the file exists
//============================================================================================================

bool System::FileExists(const char *filename)
{
	FILE *fp = fopen(filename, "r");
	if (fp) { fclose(fp); return true; }
	return false;
}

//============================================================================================================
// Gets the file's header
//============================================================================================================

uint System::GetFileHeader(const char* filename)
{
	// The first 4 bytes are enough to tell what type of file it is
	FILE* fp = fopen(filename, "rb");
	if (!fp) return 0;
	uint fd;
	fread(&fd, sizeof(uint), 1, fp);
	fclose(fp);
	return fd;
}

//============================================================================================================
// "c:\temp\test.abc" becomes "test.abc"
//============================================================================================================

String System::GetFilenameFromPath (const String& in)
{
	uint i = 0;

	for ( i = in.GetLength(); i > 0; )
	{
		char character = in[--i];
		if (character == '/' || character == '\\')
		{
			String out;
			in.GetString(out, i + 1);
			return out;
		}
	}
	return in;
}

//============================================================================================================
// "c:\temp\test.abc" becomes "c:\temp\"
//============================================================================================================

String System::GetPathFromFilename (const String& in)
{
	uint i = 0;

	for ( i = in.GetLength(); i > 0; )
	{
		char character = in[--i];
		if (character == '/' || character == '\\')
		{
			String out;
			in.GetString(out, 0, i + 1);
			return out;
		}
	}
	return "";
}

//============================================================================================================
// "c:\temp\test.abc" becomes "abc"
//============================================================================================================

String System::GetExtensionFromFilename (const String& in)
{
	uint i = 0;

	for ( i = in.GetLength(); i > 0; )
	{
		char character = in[--i];
		if (character == '.')
		{
			String out;
			in.GetString(out, i + 1);
			return out;
		}
	}
	return "";
}