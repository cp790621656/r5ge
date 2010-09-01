#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Extracts the specified amount of data from the file
//============================================================================================================

bool Extract (FILE* fp, uint& size, void* data, uint bytes)
{
	if (size < bytes) return false;

	if (bytes > 0)
	{
		if (bytes != fread(data, 1, bytes, fp)) return false;
		size -= bytes;
	}
	return true;
}

//============================================================================================================
// Extracts a uint32 from the file
//============================================================================================================

bool ExtractSize (FILE* fp, uint& size, uint& val)
{
	if (fp == 0 || size < 1) return false;

	// Try to read the size as a byte
	byte ch;
	if (1 != fread(&ch, 1, 1, fp)) return false;

	--size;

	// If the value is 255, it means that the following 4 bytes define the size
	if (ch == 255)
	{
		if (size < 4) return false;

		uint32 length (0);
		if (4 != fread(&length, 1, 4, fp)) return false;

		size -= 4;
		val = length;
	}
	else
	{
		val = ch;
	}
	return true;
}

//============================================================================================================
// Extracts a string from the file
//============================================================================================================

bool Extract (FILE* fp, uint& size, String& val)
{
	uint length;

	if (::ExtractSize(fp, size, length))
	{
		return ::Extract(fp, size, val.Resize(length), length);
	}
	return false;
}

//============================================================================================================
// Load the specified file
//============================================================================================================

bool Bundle::Load (const String& filename)
{
	FILE* fp = fopen(filename.GetBuffer(), "rb");
	if (fp == 0) return false;

	// Remember the filename and the path
	mFilename = filename;
	mDir = System::GetPathFromFilename(filename);
	mAssets.Clear();

	// Determine the size of the file
	fseek(fp, 0, SEEK_END);
	uint size = (uint)ftell(fp);
	fseek(fp, 0, SEEK_SET);
	byte header[5];

	// Extract the header
	if (::Extract(fp, size, header, 5))
	{
		// Ensure that the header is proper
		if (header[0] != '/' ||
			header[1] != '/' ||
			header[2] != 'R' ||
			header[3] != '5' ||
			header[4] != 'D')
		{
			String name;

			while (size > 0)
			{
				// The asset's name comes first
				if (!::Extract(fp, size, name)) break;

				// Create a new asset entry within the bundle
				Asset& asset = mAssets.Expand();
				asset.name	 = name;
				asset.offset = (uint)ftell(fp);
			}
			fclose(fp);
			return true;
		}
	}
	fclose(fp);
	return false;
}

//============================================================================================================
// Extract the specified file from the bundle
//============================================================================================================

bool Bundle::Extract (const char* filename, Memory& mem) const
{
	String dir  (System::GetPathFromFilename(filename));
	String name (System::GetFilenameFromPath(filename, false));
	String ext	(System::GetExtensionFromFilename(filename));

	String match (dir);
	if (name != "*") match << name;

	String current;

	FOREACH(i, mAssets)
	{
		const Asset& asset = mAssets[i];
		current = mDir;
		current << asset.name;

		if (match.IsValid() && !current.BeginsWith(match)) continue;
		if (ext.IsValid() && !current.EndsWith(ext)) continue;

		// If this point was reached, then a matching asset has been located -- try to open it
		FILE* fp = fopen(mFilename.GetBuffer(), "rb");

		if (fp != 0)
		{
			fseek(fp, 0, SEEK_END);
			uint size = (uint)ftell(fp);

			// Just a safety precaution
			if (asset.offset < size)
			{
				// Jump directly to the offset within the file
				fseek(fp, asset.offset, SEEK_SET);
				size -= asset.offset;

				// Extract the size of the asset
				uint length (0);
				if (!::ExtractSize(fp, size, length)) break;

				// If the matching asset gets located, read it into memory and exit
				if (length <= size)
				{
					fread(mem.Resize(length), 1, length, fp);
					fclose(fp);
					return true;
				}
			}
			fclose(fp);
		}
	}
	return false;
}

//============================================================================================================
// Retrieves all bundles that can be found
//============================================================================================================

Array<Bundle>& Bundle::GetAllBundles()
{
	// The file cannot be found -- try to locate it within the asset bundles
	static bool doOnce = true;
	static Array<Bundle> bundles;

	// If it's the first time this function is executed, collect the paths to all asset bundles
	if (doOnce)
	{
		doOnce = false;

		bundles.Lock();
		{
			Array<String> files;
			String assetName;

			// Find all asset bundles
			System::GetFiles("*.r5d", files, true);

			// Run through all located bundles
			for (uint i = files.GetSize(); i > 0; --i)
			{
				Bundle& bundle = bundles.Expand();
				if (!bundle.Load(files[i])) bundles.Shrink();
			}
		}
		bundles.Unlock();
	}
	return bundles;
}