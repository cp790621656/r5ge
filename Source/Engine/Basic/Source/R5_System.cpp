#include "../Include/_All.h"

#ifdef _WINDOWS
  #include <direct.h>
  #include <windows.h>
#else
  #include <unistd.h>
  #include <dirent.h>
  #include <stdlib.h>
  #include <dlfcn.h>
  #define _chdir chdir
  #define _getcwd getcwd
#endif

#include <stdarg.h>

using namespace R5;

//============================================================================================================
// Global access
//============================================================================================================

FILE* g_file = 0;

//============================================================================================================
// File container that has a destructor that will properly close the file
//============================================================================================================

struct LogFile
{
	Thread::Lockable lock;

	LogFile (const char* filename) { g_file = fopen(filename, "w"); }
	~LogFile() { if (g_file) { fflush(g_file); fclose(g_file); g_file = 0; } }

	operator FILE* () { return g_file; }
	operator bool () const { return g_file != 0; }

	void Lock()   const { lock.Lock(); }
	void Unlock() const { lock.Unlock(); }
};

//============================================================================================================
// Returns the OS type
//============================================================================================================

#ifdef _WINDOWS
uint System::GetOS() { return System::OS::Windows; }
#elif defined _MACOS
uint System::GetOS() { return System::OS::MacOS; }
#elif defined _LINUX
uint System::GetOS() { return System::OS::Linux; }
#else
uint System::GetOS() { return System::OS::Unknown; }
#endif

//============================================================================================================
// Executes the specified command via shell
//============================================================================================================

int System::Execute (const char* command) { return ::system(command); }

//============================================================================================================
// Changes the local working directory
//============================================================================================================

bool System::SetCurrentPath (const char* path)
{
	return (_chdir(path) == 0);
}

//============================================================================================================
// Flush the log file, saving out the contents
//============================================================================================================

void System::FlushLog()
{
	if (g_file != 0) fflush(g_file);
}

//============================================================================================================
// Dumps a string into the log file
//============================================================================================================

void System::Log (const char *format, ...)
{
	static LogFile log ("log.txt");
	static ulong last = 0;

	// In rare cases the log may need to be used after the static variable has been released.
	// If this happens, open the log manually and simply append to it.
	if (g_file == 0) g_file = fopen("log.txt", "a");

	if (g_file != 0)
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
			
			fprintf(g_file, "[%3u.%02u.%02u.%03u]: ", (uint)totalHRS, (uint)remMIN, (uint)remSEC, (uint)remMS);
			fputs(text, g_file);
			fputc('\n', g_file);

#ifdef _WINDOWS
			delete [] text;
#endif
			// Write to file no more frequently than every 100 milliseconds
			if (log && totalMS - last > 100)
			{
				last = totalMS;
				fflush(g_file);
			}
		}
		if (log) log.Unlock();
		else
		{
			fclose(g_file);
			g_file = 0;
		}
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
// Gets the current folder path (ends with '/')
//============================================================================================================

String System::GetCurrentPath()
{
	char temp[256] = {0};
	_getcwd(temp, 255);
	String path(temp);
	path.Replace("\\", "/", true);
	if (!path.EndsWith("/")) path << "/";
	return path;
}

//============================================================================================================
// "c:\temp\test.abc" becomes "test.abc"
//============================================================================================================

String System::GetFilenameFromPath (const String& in, bool extension)
{
	String out;
	uint last = in.Find("[");
	while (last > 0 && in[last-1] < 33) --last;

	for (uint i = last; i > 0; )
	{
		char character = in[--i];

		if (character == '/' || character == '\\')
		{
			in.GetString(out, i + 1, last);

			if (!extension)
			{
				for (uint b = last; b > i; )
				{
					char character = in[--b];

					if (character == '.')
					{
						in.GetString(out, i + 1, b);
						break;
					}
				}
			}
			return out;
		}
	}

	if (!extension)
	{
		for (uint b = last; b > 0; )
		{
			char character = in[--b];

			if (character == '.')
			{
				in.GetString(out, 0, b);
				return out;
				break;
			}
		}
	}
	return in;
}

//============================================================================================================
// "c:\temp\test.abc" becomes "c:/temp/"
//============================================================================================================

String System::GetPathFromFilename (const String& in)
{
	for (uint i = in.GetLength(); i > 0; )
	{
		char character = in[--i];

		if (character == '/' || character == '\\')
		{
			String out;
			in.GetString(out, 0, i + 1);
			out.Replace("\\", "/", true);
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
	for (uint i = in.GetLength(); i > 0; )
	{
		char character = in[--i];

		if (character == '/' || character == '\\') break;

		if (character == '.')
		{
			String out;
			in.GetString(out, i + 1);
			return out;
		}
	}
	return "";
}

//============================================================================================================

uint GetFileOrFolder (const String& in, String& out, uint offset)
{
	while (offset < in.GetSize() && in[offset] == '/') ++offset;
	uint last = offset;
	while (last < in.GetSize() && in[last] != '/') ++last;
	if (offset < last) in.GetString(out, offset, last);
	else out.Clear();
	return last;
}

//============================================================================================================
// "c:/temp/../test.abc" becomes "c:/test.abc"
//============================================================================================================

String System::GetNormalizedFilename (const String& file)
{
	String out = file;
	out.Replace("\\", "/", true);
	out.Replace("//", "/", true);

	String word;
	Array<String> list;

	for (uint i = 0; i < out.GetLength(); )
	{
		i = GetFileOrFolder(out, word, i);

		if ((list.GetSize() > 0) && (word == "..") && (list.Back() != ".."))
		{
			list.Shrink();
		}
		else
		{
			list.Expand() = word;
		}
	}

	if (list.IsEmpty()) return file;

	out.Clear();

	FOREACH(i, list)
	{
		if (i > 0) out << "/";
		out << list[i];
	}
	return out;
}

//============================================================================================================
// Reads the contents of the specified folder, populating file and folder lists
//============================================================================================================

bool System::ReadFolder (const String& dir, Array<String>& folders, Array<String>& files)
{
	String file;
	String path (dir);
	path.Replace("\\", "/", true);
	if (path.IsValid() && !path.EndsWith("/")) path << "/";
	else if (path == "/") path.Clear();

#ifdef _WINDOWS
	WIN32_FIND_DATA info;

	String match (path);
	match << "*.*";

	void* fileHandle = FindFirstFile(match.GetBuffer(), &info);
	if (fileHandle == INVALID_HANDLE_VALUE) return false;

	do 
	{
		file = info.cFileName;

		if (!file.BeginsWith("."))
		{
			if ((info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
			{
				String& out = folders.Expand();
				out << path;
				out << file;
				out << "/";
			}
			else
			{
				String& out = files.Expand();
				out << path;
				out << file;
			}
		}
	}
	while (FindNextFile(fileHandle, &info));

#else
	DIR* dp = opendir(dir.IsValid() ? dir.GetBuffer() : "./");
	if (dp == 0) return false;
	struct dirent* dirp = 0;

	while ((dirp = readdir(dp)) != 0)
	{
		String file (dirp->d_name);

		if (!file.BeginsWith("."))
		{
			if (dirp->d_type == DT_DIR)
			{
				String& out = folders.Expand();
				out << path;
				out << file;
				out << "/";
			}
			else
			{
				String& out = files.Expand();
				out << path;
				out << file;
			}
		}
	}
    closedir(dp);
#endif
	folders.Sort();
	files.Sort();
    return true;
}

//============================================================================================================
// Recursive file search function
//============================================================================================================

uint FindFiles (const String& dir, const String& name, const String& ext, Array<String>& files, bool recursive)
{
	uint count (0);
	String match (dir);
	if (name != "*") match << name;

	Array<String> fd, fl;

	if (System::ReadFolder(dir, fd, fl))
	{
		FOREACH(i, fl)
		{
			const String& s = fl[i];

			if (match.IsValid() && !s.BeginsWith(match)) continue;
			if (ext.IsValid() && !s.EndsWith(ext)) continue;
			files.Expand() << s;
			++count;
		}

		if (recursive)
		{
			FOREACH(i, fd)
			{
				count += FindFiles(fd[i], name, ext, files, true);
			}
		}
	}
	return count;
}

//============================================================================================================
// Fills out a list of all files with the partial path matching 'path'. Returns 'true' if one was found.
//============================================================================================================

bool System::GetFiles (const String& path, Array<String>& files, bool recursive)
{
	uint count (0);

	if (FileExists(path))
	{
		++count;
		files.Expand() = path;
	}
	else
	{
		String dir  (GetPathFromFilename(path));
		String name (GetFilenameFromPath(path, false));
		String ext	(GetExtensionFromFilename(path));

		count += ::FindFiles(dir, name, ext, files, recursive);

		if (!path.EndsWith(".r5d"))
		{
			const Array<Bundle>& bundles = Bundle::GetAllBundles();

			FOREACH(i, bundles)
			{
				bundles[i].FindFiles(path, files);
			}
		}
	}
	return (count > 0);
}

//============================================================================================================
// Whether the filename is close enough to be a match
//============================================================================================================

bool System::IsFilenameCloseEnough (const String& filename, const String& dir,
									const String& name, const String& ext, byte flag)
{
	// Match the folder
	if (dir.IsValid() && !filename.BeginsWith(dir)) return false;

	// Extract the filename
	String currentName (System::GetFilenameFromPath(filename, false));

	// If we have a name to work with, match it
	if (name.IsValid())
	{
		if (flag == 1 && !currentName.BeginsWith(name)) return false;
		else if (flag == 2 && !currentName.EndsWith(name)) return false;
		else if (currentName != name) return false;
	}

	// If an extension was specified, we need to match it
	if (ext.IsValid() && !filename.EndsWith(ext))
	{
		// Automatic similar extension conversion
		if (ext == "r5a" || ext == "r5b" || ext == "r5c")
		{
			if (!filename.EndsWith("r5a") &&
				!filename.EndsWith("r5b") &&
				!filename.EndsWith("r5c")) return false;
		}
		else if (ext == "tga" || ext == "png" || ext == "jpg" || ext == "r5t")
		{
			if (!filename.EndsWith(".tga") &&
				!filename.EndsWith(".png") &&
				!filename.EndsWith(".jpg") &&
				!filename.EndsWith(".r5t")) return false;
		}
		else return false;
	}
	return true;
}

//============================================================================================================
// Returns the best matching filename that exists. Allows specifying a different extension than
// that of the existing file. "c:/temp/test.abc" will match "c:/temp/test.txt" if it exists instead.
//============================================================================================================

String System::GetBestMatch (const String& filename)
{
	if (FileExists(filename)) return filename;

	String dir  (GetPathFromFilename(filename));
	String name (GetFilenameFromPath(filename, false));
	String ext	(GetExtensionFromFilename(filename));

	byte flag (0);

	if (name == "*")
	{
		name.Clear();
	}
	else
	{
		if (name.BeginsWith("*")) flag = 1;
		else if (name.EndsWith("*")) flag = 2;
		name.Replace("*", "", true);
	}

	if (name.IsValid())
	{
		Array<String> fd, fl;

		if (ReadFolder(dir, fd, fl))
		{
			FOREACH(i, fl)
			{
				const String& s = fl[i];
				if (IsFilenameCloseEnough(s, dir, name, ext, flag)) return s;
			}
		}
	}
	return "";
}

//============================================================================================================
// Load the specified system library
//============================================================================================================

bool System::Library::Load (const String& path)
{
	if (mHandle) Release();
#ifdef _WINDOWS
	mHandle = LoadLibrary(path.GetBuffer());
#else
	mHandle = dlopen(path.GetBuffer(), RTLD_NOW);
#endif
	return (mHandle != 0);
}

//============================================================================================================
// Get the address of the specified function from the library
//============================================================================================================

void* System::Library::GetFunction (const String& name)
{
#ifdef _WINDOWS
	return (mHandle != 0) ? GetProcAddress((HMODULE)mHandle, name.GetBuffer()) : 0;
#else
	return (mHandle != 0) ? dlsym(mHandle, name.GetBuffer()) : 0;
#endif
}

//============================================================================================================
// Release the library
//============================================================================================================

void System::Library::Release()
{
	if (mHandle)
	{
#ifdef _WINDOWS
		FreeLibrary((HMODULE)mHandle);
#else
		dlclose(mHandle);
#endif
		mHandle = 0;
	}
}
