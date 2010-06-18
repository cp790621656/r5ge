#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Find the root of the specified parent object, creating a new one if necessary
//============================================================================================================

OSSceneRoot* OSSceneRoot::FindRootOf (Object* parent)
{
	if (parent == 0) return 0;

	do
	{
		if (parent->GetParent() == 0) break;

		OSSceneRoot* root = parent->GetScript<OSSceneRoot>(false);
		if (root != 0) return root;

		parent = parent->GetParent();
	}
	while (parent != 0);

	return parent->AddScript<OSSceneRoot>(true);
}