//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================

#include "CardsClient.h"
using namespace R5;

UITextArea* g_status = 0;

void USMessageLog::Show (const String& text) { if (g_status != 0) { g_status->AddParagraph(text); } }
void USMessageLog::OnInit()					{ g_status = R5_CAST(UITextArea, mWidget); }
void USMessageLog::OnDestroy()				{ if (g_status == mWidget) g_status = 0; }