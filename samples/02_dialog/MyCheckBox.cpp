#include "pch.h"
#include "MyCheckBox.h"

//.............................................................................

void
MyCheckBox::construct (jnc::DataPtr textPtr)
{
	m_qtCheckBox = new QCheckBox;
	MyWidget::construct (m_qtCheckBox);
	setText (textPtr);

	m_onIsCheckedChangedBridge = new QtSignalBridge;
	m_onIsCheckedChangedBridge->connect (m_qtCheckBox, SIGNAL (stateChanged (int)), &m_onIsCheckedChanged);
}

MyCheckBox*
MyCheckBox::operatorNew (
	jnc::ClassType* type,
	jnc::DataPtr textPtr
	)
{
	MyCheckBox* checkBox = (MyCheckBox*) jnc::StdLib::allocateClass (type);
	checkBox->construct (textPtr);
	return checkBox;
}

//.............................................................................
