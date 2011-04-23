#include "../Include/_All.h"

//============================================================================================================
// Built-in shaders
//============================================================================================================

#include "../Shaders/ProjectedTexture.h"
#include "../Shaders/Deferred.h"
#include "../Shaders/Lights.h"
#include "../Shaders/SSAO.h"
#include "../Shaders/Blur.h"
#include "../Shaders/Bloom.h"
#include "../Shaders/DOF.h"
#include "../Shaders/Shadow.h"

using namespace R5;

//============================================================================================================
// Preprocess an internal shader
//============================================================================================================

uint GLGetInternalShaderCode (const String& name, String& code)
{
	if (name.BeginsWith("[R5] Projected Texture"))
	{
		if (name.Contains("Replace"))
		{
			code = g_projectedTexture;
			code << g_projectedTextureReplace;
		}
		else if (name.Contains("Modulate"))
		{
			code = g_projectedTexture;
			code << g_projectedTextureModulate;
		}
		else
		{
			code = g_projectedTexture;
			code << g_projectedTextureAddSubtract;
		}
	}
	else if	(name == "[R5] Combine Deferred")	code = g_combineDeferred;
	else if (name == "[R5] Horizontal Blur")	code = g_blurH;
	else if (name == "[R5] Vertical Blur")		code = g_blurV;
	else if (name == "[R5] Bloom Blur")			code = g_blurBloom;
	else if (name == "[R5] Combine Bloom")		code = g_combineBloom;
	else if (name == "[R5] Depth of Field")		code = g_dof;
	else if (name == "[R5] Sample SSAO")		code = g_ssaoSample;
	else if (name == "[R5] Horizontal Depth-Respecting Blur")
	{
		code  = g_depthRespectingBlur;
		code << g_depthRespectingBlurH;
	}
	else if (name == "[R5] Vertical Depth-Respecting Blur")
	{
		code  = g_depthRespectingBlur;
		code << g_depthRespectingBlurV;
	}
	else if (name == "[R5] Horizontal SSAO Blur")
	{
		code  = g_ssaoBlur;
		code << g_ssaoBlurH;
	}
	else if (name == "[R5] Vertical SSAO Blur")
	{
		code  = g_ssaoBlur;
		code << g_ssaoBlurV;
	}
	else if (name.Contains("Light"))
	{
		// Light shaders are compiled from multiple sources in order to reduce code repetition
		bool ao = name.Contains("AO");
		bool shadow = name.Contains("Shadow");

		code << g_lightCommon;

		// Light-specific code
		if (name.Contains("Directional"))
		{
			code << g_lightDirectional;
			code << g_lightBody;

			if (ao)		code << g_lightAO;
			if (shadow)	code << g_lightShadow;

			code << (ao ? g_lightDirAOEnd : g_lightDirEnd);
		}
		else
		{
			code << g_lightPoint;
			code << g_lightBody;

			if (ao)		code << g_lightAO;
			if (shadow)	code << g_lightShadow;

			code << (ao ? g_lightPointAOEnd : g_lightPointEnd);
		}
	}
	else if (name.BeginsWith("[R5] Shadow"))
	{
		// Expected syntax: "[R5] Shadow %u"
		String out;
		name.GetString(out, 12);
		uint cascades (0);

		// The number of supported shadow cascades must be valid
		if (out >> cascades && cascades < 5 && cascades > 0)
		{
			// Regardless of cascades, the first texture is always camera's depth
			code << "uniform sampler2D R5_texture0;\n";

			// Light's depth textures follow
			for (uint i = 0; i < cascades; ++i)
			{
				code << "uniform sampler2DShadow R5_texture";
				code << (i + 1);
				code << ";\n";
			}

			// Next come the shadow matrices
			for (uint i = 0; i < cascades; ++i)
			{
				code << "uniform mat4 shadowMatrix";
				code << i;
				code << ";\n";
			}

			// Shadow cascades need an additional parameter specifying where the shadows start
			if (cascades > 1) code << "uniform vec3 shadowStart;\n";

			// Next comes the common component
			code << g_shadow;
			code << "vec4 pos4 = shadowMatrix0 * worldPos;\n";
			code << "float final = SamplePCF(R5_texture1, pos4.xyz / pos4.w);\n";

			// Run through all remaining cascades and add them up
			for (uint i = 1; i < cascades; ++i)
			{
				// Set the new shadow matrix
				code << "pos4 = shadowMatrix";
				code << i;
				code << " * worldPos;\n";

				if (i == 1) code << "float ";

				// Sample the shadow texture
				code << "shadow = SamplePCF(R5_texture";
				code << (i + 1);
				code << ", pos4.xyz / pos4.w);\n";

				// Sample the texture and choose the smallest value
				code << "final = mix(final, shadow, clamp((depth - shadowStart.";

				// Doing a ceil() on the value here would produce sharp discontinuities between
				// shadows. Doing a division by half a percent here creates a smooth transition.
				if		(i == 1) code << "x) * 200.0, 0.0, 1.0));\n";
				else if (i == 2) code << "y) * 200.0, 0.0, 1.0));\n";
				else if (i == 3) code << "z) * 200.0, 0.0, 1.0));\n";
			}

			// Set the color and end the function
			code << "gl_FragColor = vec4(final);\n}";
		}
	}
#ifdef _DEBUG
	else ASSERT(false, "Unrecognized internal shader request");
#endif
	return code.IsValid() ? IShader::Flag::Fragment : 0;
}