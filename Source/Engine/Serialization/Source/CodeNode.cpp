#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Load the contents of the specified string into memory, stripping out C++ comments and extra spaces
//============================================================================================================

void CodeNode::Load (const String& source)
{
	uint start(0), end (source.GetSize());

	while (start < end)
	{
		start = mChildren.Expand().Load(source, start, end);
	}
}

//============================================================================================================
// Load the specified segment
//============================================================================================================

uint CodeNode::Load (const String& source, uint start, uint end)
{
	if (end > source.GetSize()) end = source.GetSize();
	while (start < end && source[start] < 33) ++start;

	uint lineStart (start);

	for (; start < end; ++start)
	{
		if (source[start] == ';')
		{
			source.GetTrimmed(mLine, lineStart, start++);
			mLine.TrimCode();
			return start;
		}

		if (source[start] == '{')
		{
			source.GetTrimmed(mLine, lineStart, start++);
			mLine.TrimCode();

			while (start < end)
			{
				CodeNode& code = mChildren.Expand();
				start = code.Load(source, start, end);

				if (code.mLine.IsEmpty())
				{
					mChildren.Shrink();
					break;
				}
			}
			return start;
		}

		if (start < end && source[start] == '}')
		{
			return ++start;
		}
	}
	return end;
}

//============================================================================================================
// Save the structure's contents out into the specified string
//============================================================================================================

void CodeNode::Save (String& out, uint tabs)
{
	bool root = mLine.IsEmpty();

	if (!root)
	{
		// Add a new line in front of new scope statements for clarity
		if (mChildren.IsValid() && out.IsValid()) out << "\n";

		// Add tabs, also for clarity
		for (uint i = 0; i < tabs; ++i) out << "\t";

		out << mLine;
		out << (mChildren.IsEmpty() ? ";\n" : "\n");
	}

	if (mChildren.IsValid())
	{
		if (!root)
		{
			for (uint i = 0; i < tabs; ++i) out << "\t";
			out << "{\n";
		}

		FOREACH(i, mChildren) mChildren[i].Save(out, root ? 0 : tabs+1);

		if (!root)
		{
			for (uint i = 0; i < tabs; ++i) out << "\t";
			out << "}\n";
		}
	}
}