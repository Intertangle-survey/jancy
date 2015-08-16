#pragma once

#include "MyWidget.h"

//.............................................................................

class MyLayout: public jnc::IfaceHdr
{
public:
	JNC_BEGIN_CLASS ("Layout", ApiSlot_Layout)
		JNC_OPAQUE_CLASS (MyLayout, NULL)
		JNC_OPERATOR_NEW (&operatorNew)
		JNC_FUNCTION ("addWidget", &MyLayout::addWidget)
		JNC_FUNCTION ("addLayout", &MyLayout::addLayout)
		JNC_FUNCTION ("addSpacer", &MyLayout::addSpacer)
	JNC_END_CLASS ()

public: 
	QBoxLayout::Direction m_direction;
	
public:
	QBoxLayout* m_qtLayout;

public:
	void
	construct (
		QBoxLayout::Direction direction,
		QWidget* parent = NULL
		);

	static 
	MyLayout*
	operatorNew (
		jnc::ClassType* type,
		QBoxLayout::Direction direction
		);

	void
	AXL_CDECL
	addWidget (MyWidget* widget);

	void
	AXL_CDECL
	addLayout (MyLayout* layout);

	void
	AXL_CDECL
	addSpacer ();
};

//.............................................................................
