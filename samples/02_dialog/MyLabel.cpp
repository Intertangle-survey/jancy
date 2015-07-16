#include "pch.h"
#include "MyLabel.h"

//.............................................................................

void
MyLabel::enumGcRoots (
	jnc::GcHeap* gcHeap,
	MyLabel* self
	)
{
}

MyLabel*
MyLabel::operatorNew (
	jnc::ClassType* type,
	jnc::DataPtr textPtr
	)
{
	jnc::ApiClassBox <MyLabel>* label = (jnc::ApiClassBox <MyLabel>*) jnc::StdLib::allocateClass (type);
	label->prime (type);
	label->construct (textPtr);
	return label;
}

void
MyLabel::construct (jnc::DataPtr textPtr)
{
	m_qtLabel = new QLabel;
	m_color = -1;
	m_backColor = -1;
	m_alignment = m_qtLabel->alignment ();

	MyWidget::construct (m_qtLabel);
	setText (textPtr);
}

void
MyLabel::updateStyleSheet ()
{
	QString styleSheet = "QLabel { ";

	if (m_color != -1)
		styleSheet += QString ("color: #%1; ").arg (m_color, 6, 16, (QChar) '0');
	else
		styleSheet += "color: black; ";

	if (m_backColor != -1)
		styleSheet += QString ("background-color: #%1; ").arg (m_backColor, 6, 16, (QChar) '0');
	else
		styleSheet += "background-color: transparent; ";

	styleSheet += "}";

	m_qtLabel->setStyleSheet (styleSheet);
}

//.............................................................................
