#include "_All.h"

//============================================================================================================
// Some functions have to be exported with exact names so 3ds max can recognize them
//============================================================================================================

#define AS_IS	extern "C"
#define EXPORT	__declspec(dllexport)

//============================================================================================================
// This needs to be global in order to be accessible by the library description functions below
//============================================================================================================

HINSTANCE g_hInstance = 0;

//============================================================================================================
// Exporter descriptor, needed by 3ds Max
//============================================================================================================

struct R5ExporterClassDesc : public ClassDesc2
{
	virtual int 			IsPublic()					{ return 1; }
	virtual void*			Create(BOOL loading = 0)	{ return new R5MaxExporter(); }
	virtual const char*		ClassName()					{ return "R5 Max Exporter"; }
	virtual SClass_ID		SuperClassID()				{ return SCENE_EXPORT_CLASS_ID; }
	virtual Class_ID		ClassID()					{ return Class_ID(0x94ee1a39, 0xa2df183a); }
	virtual const char* 	Category()					{ return "R5"; }
	virtual const char*		InternalName()				{ return "R5 Max Exporter"; }
	virtual HINSTANCE		HInstance()					{ return g_hInstance; }
};

//============================================================================================================
// Functions needed by 3ds Max used to query this plugin's information
//============================================================================================================

AS_IS EXPORT const char*	LibDescription()			{ return "R5 Max Exporter"; }
AS_IS EXPORT int			LibNumberClasses()			{ return 1; }
AS_IS EXPORT unsigned long	LibVersion()				{ return VERSION_3DSMAX; }
AS_IS EXPORT ClassDesc*		LibClassDesc(int i)
{
	static R5ExporterClassDesc exporterDesc;
	return &exporterDesc;
	return 0;
}

//============================================================================================================
// DLL entry point
//============================================================================================================

AS_IS int WINAPI DllMain( HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved )
{
	static bool initDone = false;

	if ( !initDone )
	{
		initDone = true;
		g_hInstance = hinstDLL;
		::DisableThreadLibraryCalls(g_hInstance);
		//::InitCustomControls(g_hInstance);
		::InitCommonControls();
	}
	return 1;
}