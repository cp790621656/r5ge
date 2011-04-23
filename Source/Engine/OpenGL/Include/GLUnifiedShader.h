#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Unified shader program
// Author: Michael Lyashenko
//============================================================================================================

class GLUnifiedShader
{
	Flags mFlags;
	CodeNode mCode;

public:

	// Returns the shader flags associated with the shader
	const Flags& GetBasicFlags() const { return mFlags; }

	// Load the specified code and return its shader type
	void SerializeFrom (const String& code);

	// Save the shader code into the specified string
	void SerializeTo (String& code) const { code.Clear(); mCode.SerializeTo(code); }

	// Save a variation of the shader into the specified string
	Flags GetVariation (String& out, const Flags& flags) const;
};
