//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================

#include "CardsClient.h"
using namespace R5;

UITextArea* g_status = 0;

void USMessageLog::Show (const String& text) { if (g_status != 0) { g_status->AddParagraph(text); } }
void USMessageLog::OnInit()					{ g_status = R5_CAST(UITextArea, mWidget); }
void USMessageLog::OnDestroy()				{ if (g_status == mWidget) g_status = 0; }