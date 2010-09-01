#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Bundle is a collection of assets packed into a single file
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

	// Extract the specified file from the bundle
	bool Extract (const char* filename, Memory& mem) const;

public:

	// Retrieves all bundles that can be found
	static Array<Bundle>& GetAllBundles();
};