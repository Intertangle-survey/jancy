#pragma once

#include "MyWidget.h"

//.............................................................................

class MyLabel: public MyWidget
{
public:
	JNC_BEGIN_OPAQUE_CLASS_TYPE (MyLabel, "Label", ApiSlot_Label)
		JNC_CONSTRUCTOR (&(rtl::construct <MyLabel, jnc::DataPtr>))
		JNC_AUTOGET_PROPERTY ("m_text", &MyLabel::setText)
		JNC_AUTOGET_PROPERTY ("m_color", &MyLabel::setColor)
		JNC_AUTOGET_PROPERTY ("m_backColor", &MyLabel::setBackColor)
		JNC_AUTOGET_PROPERTY ("m_alignment", &MyLabel::setAlignment)
	JNC_END_CLASS_TYPE ()

public: 
	jnc::DataPtr m_text;
	int m_color;
	int m_backColor;
	Qt::Alignment m_alignment;
	
public:
	QLabel* m_qtLabel;

public:
	MyLabel (jnc::DataPtr textPtr);

	void
	AXL_CDECL
	setText (jnc::DataPtr textPtr)
	{
		m_text = textPtr;
		m_qtLabel->setText ((const char*) textPtr.m_p);
	}

	void
	AXL_CDECL
	setColor (int color)
	{	
		m_color = color;
		updateStyleSheet ();
	}

	void
	AXL_CDECL
	setBackColor (int color)
	{	
		m_backColor = color;
		updateStyleSheet ();
	}

	void
	AXL_CDECL
	setAlignment (Qt::Alignment alignment)
	{	
		m_alignment = alignment;
		m_qtLabel->setAlignment (alignment);
	}

protected:
	void
	updateStyleSheet ();
};

//.............................................................................
