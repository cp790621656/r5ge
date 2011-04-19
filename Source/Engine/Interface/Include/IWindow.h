#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Base class for the application window
// Author: Michael Lyashenko
//============================================================================================================

struct IGraphics;
struct IWindow
{
	R5_DECLARE_INTERFACE_CLASS("Window");

	struct Style
	{
		enum
		{
			Undefined = 0,
			Hidden,
			Normal,
			Fixed,
			Child,
			FullScreen
		};
	};

	virtual ~IWindow() {};

	// Creates a window of specified name, style, width, and height
	virtual bool Create(const String&	title,
						short			x			= 0,
						short			y			= 0,
						ushort			width		= 1024,
						ushort			height		= 768,
						uint			style		= Style::Normal)=0;

	virtual void SetTitle		( const String& title  )=0;
	virtual void SetEventHandler( IEventReceiver* ptr   )=0;
	virtual bool SetGraphics	( IGraphics*	 ptr   )=0;
	virtual bool SetPosition	( const Vector2i& pos  )=0;
	virtual bool SetSize		( const Vector2i& size )=0;
	virtual bool SetStyle		( uint style )=0;
	virtual void SetFocus		()=0;

	// Various functions
	virtual bool		IsValid()		const=0;
	virtual String		GetTitle()		const=0;
	virtual Vector2i	GetPosition()	const=0;
	virtual Vector2i	GetSize()		const=0;
	virtual uint		GetStyle()		const=0;
	virtual bool		IsMinimized()	const=0;
	virtual void		ShowCursor(bool show)=0;
	virtual void		Close()=0;
	virtual bool		Update()=0;
	virtual void		BeginFrame()=0;
	virtual void		EndFrame()=0;

	// Retrieves a string from the clipboard
	virtual String GetClipboardText() const=0;

	// Sets the system clipboard text
	virtual void SetClipboardText (const String& text)=0;

	// Serialization
	virtual bool SerializeFrom (const TreeNode& root)=0;
	virtual bool SerializeTo (TreeNode& root) const=0;
};
