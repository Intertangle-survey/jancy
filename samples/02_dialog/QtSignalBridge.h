#pragma once

//.............................................................................

class QtSignalBridge: public QObject
{
	Q_OBJECT;

protected:
	jnc::Multicast* m_jncEvent;

public:
	QtSignalBridge (QObject* parent = NULL):
		QObject (parent)
	{
	}

	QtSignalBridge (
		QObject* sender,
		const char* signal,
		jnc::Multicast* jncEvent,
		QObject* parent = NULL
		):
		QObject (parent)
	{
		connect (sender, signal, jncEvent);
	}

	void 
	connect (
		QObject* sender,
		const char* signal,
		jnc::Multicast* jncEvent
		)
	{
		m_jncEvent = jncEvent;
		QObject::connect (sender, signal, this, SLOT (onQtSignal ()));
	}

public slots:
	void 
	onQtSignal ();
};

//.............................................................................
