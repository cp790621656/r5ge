#include "../Include/_All.h"
#include "../Include/_Filters.h"
using namespace R5;

//============================================================================================================
// Helpful macros that shorten the code below
//============================================================================================================

#define INCLUDE(entry)				\
	{								\
		val = entry;				\
		if (val < min) min = val;	\
		if (val > max) max = val;	\
	}

//============================================================================================================

#define FINALIZE(result, var)				\
	range  =  max - min;					\
	center = (min + max) * 0.5f;			\
	offset = r.GenerateRangeFloat() * var;	\
	result = Float::Clamp(center + offset, max - range, min + range);

//============================================================================================================

namespace R5
{
namespace Filter
{

//============================================================================================================
// Helper function that mirrors values above a certain point
//============================================================================================================

inline float MirrorOver(float val, float threshold)
{
	return (val > threshold) ? threshold - (val - threshold) : val;
}

//============================================================================================================
// Generates a non-seamless fractal field of specified size
//============================================================================================================

void GenerateFractal (Random& r, float* out, uint width, uint height, float threshold)
{
	uint size(0);

	for (uint i = 2; i <= width;  i <<= 1) size  = i;
	for (uint i = 2; i <= height; i <<= 1) if (i > size) size = i;

	if (size == 0) return;

	size += 1;

	float* temp = new float[size * size];
	memset(temp, 0, size * size * sizeof(float));

	// Start with the 4 corners
	temp[0] = r.GenerateRangeFloat();
	temp[size - 1] = r.GenerateRangeFloat();
	temp[size * (size - 1)] = r.GenerateRangeFloat();
	temp[size *  size - 1 ] = r.GenerateRangeFloat();

	uint index, half, halfSize, yw;
	float variance = 1.0f, val, range, center, offset, min, max, diamondVariance;

	for (uint current = size - 1; current > 1; current >>= 1, variance *= 0.5f)
	{
		half = current >> 1;
		halfSize = size * half;

		// Generate all mid-points (square)
		for (uint y = half; y < size; y += current)
		{
			yw = yw = y * size;

			for (uint x = half; x < size; x += current)
			{
				index = x + yw;

				min =  1024.0f;
				max = -1024.0f;

				INCLUDE(temp[index - half - halfSize]);
				INCLUDE(temp[index - half + halfSize]);
				INCLUDE(temp[index + half - halfSize]);
				INCLUDE(temp[index + half + halfSize]);

				FINALIZE(temp[index], variance);
			}
		}

		bool even = true;
		diamondVariance = variance / 1.4f;

		// Generate all side-points (diamond)
		for (uint y = 0; y < size; y += half, even = !even)
		{
			yw = y * size;

			for (uint x = even ? half : 0; x < size; x += current)
			{
				index = x + yw;

				min =  1024.0f;
				max = -1024.0f;

				if (x >= half)			INCLUDE(temp[index - half]);
				if (x + half < size)	INCLUDE(temp[index + half]);
				if (y >= half)			INCLUDE(temp[index - halfSize]);
				if (y + half < size)	INCLUDE(temp[index + halfSize]);

				FINALIZE(temp[index], diamondVariance);
			}
		}
	}

	// Copy rows one at a time (buffer widths differ)
	for (uint i = 0, row = width * sizeof(float); i < height; ++i)
		memcpy(out + i * height, temp + i * size, row);

	// Cleanup
	delete [] temp;

	// Fractal fields should be normalized at the end as they're not in 0-1 range
	size = width * height;
	R5::Normalize(out, size);

	// If there is a mirror threshold, create some ridges
	if (threshold != 1.0f)
	{
		for (uint i = 0; i < size; ++i) out[i] = MirrorOver(out[i], threshold);
		R5::Normalize(out, size);
	}
}

//============================================================================================================
// Generates a seamless fractal pattern of specified size
//============================================================================================================

void GenerateSeamlessFractal (Random& r, float* out, uint width, uint height, float threshold)
{
	uint size = width * height;
	if (size == 0) return;

	// Buffer of values indicating whether the pixel has been calculated
	bool* done = new bool[size];
	memset(done, 0, size);

	// Starting value for the sides
	out [0] = r.GenerateRangeFloat();
	done[0] = true;

	uint centerIndex = Float::RoundToUInt((0.5f * height + 0.5f) * width);

	// Starting value for the center
	out [centerIndex] = r.GenerateRangeFloat();
	done[centerIndex] = true;

	float variance = 1.0f, diamondVariance, strideX, strideY, halfX, halfY,
		min, max, val, range, center, offset;

	uint splits = 1, botY, minY, midY, maxY, topY, botYW, minYW, midYW, maxYW, topYW,
		botX, minX, midX, maxX, topX, index, left, right, top, bottom, bl, br, tl, tr, ll, rr, bb, tt;

	for (;; splits <<= 1, variance *= 0.5f)
	{
		// Actual width of this stride
		strideX = variance * width;
		strideY = variance * height;

		// Nothing left to split
		if (strideX < 1.0f && strideY < 1.0f) break;

		// Half width of the stride
		halfX = 0.5f * strideX;
		halfY = 0.5f * strideY;

		// Maximum height variance
		diamondVariance = variance / 1.4f;

		for (uint iy = 0; iy < splits; ++iy)
		{
			botY = WrapIndex(Float::RoundToInt(strideY *  iy	- halfY ),	height);
			minY = WrapIndex(Float::RoundToInt(strideY *  iy			),	height);
			midY = WrapIndex(Float::RoundToInt(strideY *  iy	+ halfY ),	height);
			maxY = WrapIndex(Float::RoundToInt(strideY * (iy+1)			),	height);
			topY = WrapIndex(Float::RoundToInt(strideY * (iy+1)	+ halfY ),	height);

			botYW = botY * width;
			minYW = minY * width;
			midYW = midY * width;
			maxYW = maxY * width;
			topYW = topY * width;

			for (uint ix = 0; ix < splits; ++ix)
			{
				botX = WrapIndex(Float::RoundToInt(strideX *  ix	- halfX ), width);
				minX = WrapIndex(Float::RoundToInt(strideX *  ix			), width);
				midX = WrapIndex(Float::RoundToInt(strideX *  ix	+ halfX ), width);
				maxX = WrapIndex(Float::RoundToInt(strideX * (ix+1)			), width);
				topX = WrapIndex(Float::RoundToInt(strideX * (ix+1)	+ halfX ), width);

				index	= midYW + midX;
				left	= midYW + minX;
				right	= midYW + maxX;
				top		= maxYW + midX;
				bottom	= minYW + midX;

				bl = minYW + minX;
				br = minYW + maxX;
				tl = maxYW + minX;
				tr = maxYW + maxX;

				ll = midYW + botX;
				rr = midYW + topX;
				bb = botYW + midX;
				tt = topYW + midX;

				if (!done[index])
				{
					min =  1024.0f;
					max = -1024.0f;

					INCLUDE(out[bl]);
					INCLUDE(out[br]);
					INCLUDE(out[tl]);
					INCLUDE(out[tr]);

					FINALIZE(out[index], variance);
					done[index] = true;
				}

				if (!done[left] && done[ll] && done[tl] && done[bl])
				{
					min =  1024.0f;
					max = -1024.0f;

					INCLUDE(out[index]);
					INCLUDE(out[ll]);
					INCLUDE(out[tl]);
					INCLUDE(out[bl]);

					FINALIZE(out[left], diamondVariance);
					done[left] = true;
				}

				if (!done[right] && done[rr] && done[tr] && done[br])
				{
					min =  1024.0f;
					max = -1024.0f;

					INCLUDE(out[index]);
					INCLUDE(out[rr]);
					INCLUDE(out[tr]);
					INCLUDE(out[br]);

					FINALIZE(out[right], diamondVariance);
					done[right] = true;
				}

				if (!done[bottom] && done[bb] && done[bl] && done[br])
				{
					min =  1024.0f;
					max = -1024.0f;

					INCLUDE(out[index]);
					INCLUDE(out[bb]);
					INCLUDE(out[bl]);
					INCLUDE(out[br]);

					FINALIZE(out[bottom], diamondVariance);
					done[bottom] = true;
				}

				if (!done[top] && done[tt] && done[tl] && done[tr])
				{
					min =  1024.0f;
					max = -1024.0f;

					INCLUDE(out[index]);
					INCLUDE(out[tt]);
					INCLUDE(out[tl]);
					INCLUDE(out[tr]);

					FINALIZE(out[top], diamondVariance);
					done[top] = true;
				}
			}
		}
	}
	
	// Cleanup
	delete [] done;

	// Fractal fields should be normalized at the end as they're not in 0-1 range
	R5::Normalize(out, size);

	// If there is a mirror threshold, create some ridges
	if (threshold != 1.0f)
	{
		for (uint i = 0; i < size; ++i) out[i] = MirrorOver(out[i], threshold);
		R5::Normalize(out, size);
	}
}

//============================================================================================================
// Helper class used by Perlin Noise generation
//============================================================================================================

struct PerlinBuffer
{
	const float*	mBuffer;
	uint			mWidth;
	uint			mHeight;
	uint			mAllocated;
	float			mFactor;

	float Sample (float x, float y, float threshold, bool seamless)
	{
		float f = seamless ? Interpolation::HermiteTile (mBuffer, mWidth, mHeight, x, y) :
							 Interpolation::HermiteClamp(mBuffer, mWidth, mHeight, x, y);
		return mFactor * MirrorOver(f, threshold);
	}
};

//============================================================================================================
// Generates simple random noise of specified size
//============================================================================================================

FILTER(Simple)
{
	uint allocated = (uint)size.x * size.y;
	for (uint i = 0; i < allocated; ++i)
		data[i] = r.GenerateFloat();
}

//============================================================================================================
// Fractal noise filter
//============================================================================================================

FILTER(Fractal)
{
	// Get the number of octaves -- stored as a float, same as any other parameter
	uint octaves = (params.GetCount() > 0) ? Float::RoundToUInt(params[0]) : 1;
	if (octaves == 0 || size.x < 4 || size.y < 4) return;

	// Noise smoothness and threshold for ridged fractal noise are passed as optional parameters
	float threshold		= (params.GetCount() > 1) ? params[1] : 1.0f;
	float smoothness	= (params.GetCount() > 2) ? params[2] : 1.0f;

	// Limit the smoothness within reasonable values
	smoothness = Float::Clamp(smoothness, 0.01f, 1.0f);

	// Allocated buffer length is obvious
	uint width = size.x;
	uint height = size.y;
	uint allocated = width * height;
	float contribution = 1.0f;

	// Clear the initial buffer
	memset(data, 0, sizeof(float) * allocated);

	// Generate the seed for the first octave
	uint seed = r.GenerateUint();

	// Generate the noise octaves
	for (uint o = 0; o < octaves; ++o, contribution *= smoothness)
	{
		// Generate a random number using the last saved seed -- this becomes our octave's seed
		r.SetSeed(seed);
		seed = r.GenerateUint();

		if (seamless)
		{
			// Seamless fractal noise
			GenerateSeamlessFractal(r, aux, width, height, threshold);
		}
		else
		{
			// Clamp-at-edges fractal noise
			GenerateFractal(r, aux, width, height, threshold);
		}

		// Add the result to the final buffer, creating ridges in the process
		if (Float::IsZero(contribution - 1.0f))
		{
			for (uint i = 0; i < allocated; ++i)
			{
				data[i] = Max(data[i], aux[i]);
			}
		}
		else
		{
			for (uint i = 0; i < allocated; ++i)
			{
				data[i] = Max(data[i], contribution * aux[i]);
			}
		}
	}

	// R5::Normalize the final result
	if (octaves > 1) R5::Normalize(data, allocated);
}

//============================================================================================================
// Perlin noise filter
//============================================================================================================

FILTER(Perlin)
{
	// Get the number of octaves -- stored as a float, same as any other parameter
	uint octaves = (params.GetCount() > 0) ? Float::RoundToUInt(params[0]) : 1;
	if (octaves == 0 || size.x < 4 || size.y < 4) return;

	// Noise smoothness and threshold for ridged perlin noise are passed as optional parameters
	float threshold  = (params.GetCount() > 1) ? params[1] : 1.0f;
	float smoothness = (params.GetCount() > 2) ? params[2] : 1.0f;

	// Allocated buffer length is obvious
	uint width = size.x;
	uint height = size.y;
	uint allocated = width * height;

	// Figure out if the requested number of octaves is even possible
	for (uint i = 1; i < octaves; ++i)
	{
		width  = width  >> 1;
		height = height >> 1;

		if ((width + height) < 3)
		{
			octaves = i;
			break;
		}
	}

	// Reset width and height
	width  = size.x;
	height = size.y;

	// Create a new set of buffers, one per octave
	PerlinBuffer* sample = new PerlinBuffer[octaves];

	float contribution		= 1.0f;
	float contributionScale = 1.0f + smoothness;
	float totalContribution = 0.0f;

	// Figure out the total contribution based on the scale provided
	for (uint i = 0; i < octaves; ++i)
	{
		totalContribution += contribution;
		contribution *= contributionScale;
	}

	float contributionFactor = 1.0f / totalContribution;

	// Set the contribution factors
	for (uint i = 0; i < octaves; ++i)
	{
		sample[i].mFactor = contributionFactor;
		contributionFactor *= contributionScale;
	}

	// Generate a simple noise field that will be used to generate the Perlin noise
	for (uint i = 0; i < allocated; ++i)
		data[i] = r.GenerateFloat();

	// First sample buffer is a direct copy of the original buffer
	sample[0].mWidth		= width;
	sample[0].mHeight		= height;
	sample[0].mAllocated	= allocated;
	sample[0].mBuffer		= data;

	// Calculate the dimensions of the downsampled noise and bind the pointers to buffer segments.
	// The advantage of doing this rather than simply generating new floats every time is that this way
	// far fewer numbers need to be generated by simply moving the offset inside the already generated array.
	for (uint i = 1; i < octaves; ++i)
	{
		sample[i].mWidth		= sample[i-1].mWidth		>> 1;
		sample[i].mHeight		= sample[i-1].mHeight		>> 1;
		sample[i].mAllocated	= sample[i-1].mAllocated	>> 2;
		sample[i].mBuffer		= data + (r.GenerateUint() & (allocated - sample[i].mAllocated - 1));
	}

	// Combine the generated noise
	for (uint y = 0; y < height; ++y)
	{
		uint yw = y * width;
		float fy = (float)y / height;

		for (uint x = 0; x < width; ++x)
		{
			uint index = yw + x;
			float fx = (float)x / width;

			// Add the first octave
			aux[index] = sample[0].mFactor * MirrorOver(data[index], threshold);

			// Add other octaves
			for (uint i = 1; i < octaves; ++i)
				aux[index] += sample[i].Sample(fx, fy, threshold, seamless);
		}
	}

	delete [] sample;
	Swap(data, aux);

	// R5::Normalize the final result
	R5::Normalize(data, allocated);
}

//============================================================================================================
// Normalizes the specified noise, putting it in range of 0 to 1
//============================================================================================================

FILTER(Normalize)
{
	uint allocated = (uint)size.x * size.y;
	R5::Normalize(data, allocated);
}

//============================================================================================================
// Gaussian blur filter
//============================================================================================================

FILTER(GaussianBlur)
{
	uint passes = (params.GetCount() > 0) ? (uint)params[0] : 1;
	uint width  = size.x;
	uint height = size.y;

	for (uint pass = 0; pass < passes; ++pass)
	{
		// Horizontal blur pass
		for (uint y = 0; y < height; ++y)
		{
			uint yw = y * width;

			for (uint x = 0; x < width; ++x)
			{
				uint index = yw + x;

				float val = 0.509434f * data[index];

				if (seamless)
				{
					val += 0.169811f * ( data[yw + WrapIndex(x-1, width)] +
										 data[yw + WrapIndex(x+1, width)] );
					val += 0.075472f * ( data[yw + WrapIndex(x-2, width)] +
										 data[yw + WrapIndex(x+2, width)] );
				}
				else
				{
					val += 0.169811f * ( data[yw + ClampIndex(x-1, width)] +
										 data[yw + ClampIndex(x+1, width)] );
					val += 0.075472f * ( data[yw + ClampIndex(x-2, width)] +
										 data[yw + ClampIndex(x+2, width)] );
				}

				aux[index] = val;
			}
		}

		// Vertical blur pass
		for (uint y = 0; y < height; ++y)
		{
			uint yw = y * width, y0, y1, y2, y3;

			if (seamless)
			{
				y0 = WrapIndex(y-1, height) * width;
				y1 = WrapIndex(y+1, height) * width;
				y2 = WrapIndex(y-2, height) * width;
				y3 = WrapIndex(y+2, height) * width;
			}
			else
			{
				y0 = ClampIndex(y-1, height) * width;
				y1 = ClampIndex(y+1, height) * width;
				y2 = ClampIndex(y-2, height) * width;
				y3 = ClampIndex(y+2, height) * width;
			}

			for (uint x = 0; x < width; ++x)
			{
				uint index = yw + x;

				float val = 0.509434f * aux[index];

				val += 0.169811f * ( aux[y0 + x] +
									 aux[y1 + x] );

				val += 0.075472f * ( aux[y2 + x] +
									 aux[y3 + x] );

				data[index] = val;
			}
		}
	}
}

//============================================================================================================
// Blur filter -- Gaussian if 2nd and 3rd parameters were skipped, weighted gaussian otherwise
//============================================================================================================

FILTER(Blur)
{
	float low  = (params.GetCount() > 1) ? params[1] : 1.0f;
	float high = (params.GetCount() > 2) ? params[2] : 1.0f;
	float diff = high - low;

	// If range is not valid, do a normal blur pass instead
	if (diff < 0.0001f)
	{
		GaussianBlur(r, data, aux, size, params, seamless);
		return;
	}

	// Number of passes, width and height of the noise
	uint passes = (params.GetCount() > 0) ? (uint)params[0] : 1;
	uint width  = size.x;
	uint height = size.y;

	for (uint pass = 0; pass < passes; ++pass)
	{
		// Horizontal blur pass
		for (uint y = 0; y < height; ++y)
		{
			uint yw = y * width;

			for (uint x = 0; x < width; ++x)
			{
				uint index = yw + x;

				float height = data[index];
				float factor = (height - low) / diff;

				if (factor < 1.0f)
				{
					if (factor < 0.0f) factor = 0.0f;
					float val = 0.509434f * height;

					if (seamless)
					{
						val += 0.169811f * ( data[yw + WrapIndex(x-1, width)] +
											 data[yw + WrapIndex(x+1, width)] );
						val += 0.075472f * ( data[yw + WrapIndex(x-2, width)] +
											 data[yw + WrapIndex(x+2, width)] );
					}
					else
					{
						val += 0.169811f * ( data[yw + ClampIndex(x-1, width)] +
											 data[yw + ClampIndex(x+1, width)] );
						val += 0.075472f * ( data[yw + ClampIndex(x-2, width)] +
											 data[yw + ClampIndex(x+2, width)] );
					}

					aux[index] = val * (1.0f - factor) + height * factor;
				}
				else aux[index] = height;
			}
		}

		// Vertical blur pass
		for (uint y = 0; y < height; ++y)
		{
			uint yw = y * width, y0, y1, y2, y3;

			if (seamless)
			{
				y0 = WrapIndex((int)y-1, height) * width;
				y1 = WrapIndex((int)y+1, height) * width;
				y2 = WrapIndex((int)y-2, height) * width;
				y3 = WrapIndex((int)y+2, height) * width;
			}
			else
			{
				y0 = ClampIndex((int)y-1, height) * width;
				y1 = ClampIndex((int)y+1, height) * width;
				y2 = ClampIndex((int)y-2, height) * width;
				y3 = ClampIndex((int)y+2, height) * width;
			}

			for (uint x = 0; x < width; ++x)
			{
				uint index = yw + x;

				float height = aux[index];
				float factor = (height - low) / diff;

				if (factor < 1.0f)
				{
					if (factor < 0.0f) factor = 0.0f;
					float val = 0.509434f * height;

					val += 0.169811f * ( aux[y0 + x] +
										 aux[y1 + x] );
					val += 0.075472f * ( aux[y2 + x] +
										 aux[y3 + x] );
					data[index] = val * (1.0f - factor) + height * factor;
				}
				else data[index] = height;
			}
		}
	}
}

//============================================================================================================
// Power of # filter
//============================================================================================================

FILTER(Power)
{
	float power = (params.GetCount() == 0) ? 2.0f : params[0];
	uint allocated = (uint)size.x * size.y;
	for (uint i = 0; i < allocated; ++i)
		data[i] = ::pow(data[i], power);
}

//============================================================================================================
// Square root filter
//============================================================================================================

FILTER(Sqrt)
{
	uint allocated = (uint)size.x * size.y;
	for (uint i = 0; i < allocated; ++i)
	{
		float val = data[i];
		data[i] = (val < 0.0f) ? 0.0f : Float::Sqrt(val);
	}
}

//============================================================================================================
// Add/subtract filter
//============================================================================================================

FILTER(Add)
{
	if (params.GetCount() > 0)
	{
		float val = params[0];
		uint allocated = (uint)size.x * size.y;
		for (uint i = 0; i < allocated; ++i)
			data[i] += val;
	}
}

//============================================================================================================
// Multiply filter
//============================================================================================================

FILTER(Multiply)
{
	if (params.GetCount() > 0)
	{
		float val = params[0];
		uint allocated = (uint)size.x * size.y;
		for (uint i = 0; i < allocated; ++i)
			data[i] *= val;
	}
}

//============================================================================================================
// Rounds all values down to the specified precision. Ie: 0.1 means values are 0.1, 0.2, etc
//============================================================================================================

FILTER(Round)
{
	if (params.GetCount() > 0)
	{
		float precision = params[0];
		uint allocated = (uint)size.x * size.y;
		for (uint i = 0; i < allocated; ++i)
			data[i] = Float::Round(data[i], precision);
	}
}

//============================================================================================================
// Clamps all pixels to be within the specified range
//============================================================================================================

FILTER(Clamp)
{
	if (params.GetCount() > 1)
	{
		float low  = params[0];
		float high = params[1];

		uint allocated = (uint)size.x * size.y;

		for (uint i = 0; i < allocated; ++i)
		{
			if		(data[i] < low)		data[i] = low;
			else if (data[i] > high)	data[i] = high;
		}
	}
}

//============================================================================================================
// Mirrors all values above or below the specified point
//============================================================================================================

FILTER(Mirror)
{
	uint allocated = (uint)size.x * size.y;

	if (params.GetCount() == 0)
	{
		// Optimization, equivalent of passing 1 for the first parameter
		for (uint i = 0; i < allocated; ++i)
			data[i] = 1.0f - data[i];
	}
	else
	{
		float low  = params[0];
		float high = (params.GetCount() > 1) ? params[1] : 1.0f;

		for (uint i = 0; i < allocated; ++i)
		{
			if		(data[i] > high)	data[i] = high - (data[i] - high);
			else if (data[i] < low)		data[i] = low  + (low - data[i]);
		}
	}
}

//============================================================================================================
// Thermal erosion filter
//============================================================================================================

FILTER(Erode)
{
	int		passes		= (params.GetCount() > 0) ? Float::RoundToUInt(params[0]) : 10;
	float	strength	= (params.GetCount() > 1) ? params[1] : 0.5f;
	float	deposit		= (params.GetCount() > 2) ? params[2] : 0.0f;

	Vector2i neighbor[8];
	neighbor[0] = Vector2i( 0,  1);
	neighbor[1] = Vector2i(-1,  1);	// Top-left
	neighbor[2] = Vector2i(-1,  0);
	neighbor[3] = Vector2i( 1,  1);	// Top-right
	neighbor[4] = Vector2i( 1,  0);
	neighbor[5] = Vector2i(-1, -1);	// Bottom-left
	neighbor[6] = Vector2i( 0, -1);
	neighbor[7] = Vector2i( 1, -1);	// Bottom-right

	int width  = size.x;
	int height = size.y;

	// Only 50% should be transferred as this will even out the 2 pixels.
	// As such, we can't transfer more than 50%.
	strength = Min(strength * 0.5f, 0.5f);

	int yw, index, target, idx;
	float max, diff;

	for (int i = 0; i < passes; ++i)
	{
		for (int y = 0; y < height; ++y)
		{
			yw = y * width;

			for (int x = 0; x < width; ++x)
			{
				index = yw + x;
				target = index;
				max = 0.0f;

				float& current = data[index];

				for (int n = 0; n < 8; ++n)
				{
					if (seamless)
					{
						idx =	WrapIndex(y + neighbor[n].y, height) * width +
								WrapIndex(x + neighbor[n].x, width);
					}
					else
					{
						idx =	ClampIndex(y + neighbor[n].y, height) * width +
								ClampIndex(x + neighbor[n].x, width);
					}

					// Difference in height between the two pixels
					diff = current - data[idx];

					// Diagonal pixels are farther away, so take that into consideration
					if ((n & 1) != 0) diff *= 0.7071f;

					// Choose the lowest
					if (diff > max)
					{
						max = diff;
						target = idx;
					}
				}

				// If we found a lower pixel to move to
				if (target != index)
				{
					// Move some percentage from one pixel to another
					max *= strength;

					// Remove the full deposit
					current -= max;

					// Deposit the adjusted amount
					data[target] += max * deposit;
				}
			}
		}
	}
}

} // namespace Filter
} // namespace R5