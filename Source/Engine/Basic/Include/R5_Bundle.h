#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Bundle is a collection of assets packed into a single file
// Author: Michael Lyashenko
//============================================================================================================

class Bundle
{
public:

	struct Asset
	{
		String name;
		uint offset;
	};

private:

	String mFilename;
	String mDir;
	Array<Asset> mAssets;

public:

	// Bundle's name is its filename
	const String& GetName() const { return mFilename; }

	// Bundle is valid as long as it has assets
	bool IsValid() const { return mAssets.IsValid(); }

	// Read-only access to asset records within the bundle
	const Array<Asset>& GetAllAssets() const { return mAssets; }

	// Load the specified asset bundle
	bool Load (const String& filename);

	// Finds the file of specified name within the bundle
	bool FindFiles (const String& filename, Array<String>& files) const;

	// Extract the specified file from the bundle
	bool Extract (const String& filename, Memory& mem, String* actualFilename = 0) const;

public:

	// Retrieves all bundles that can be found
	static const Array<Bundle>& GetAllBundles();
};