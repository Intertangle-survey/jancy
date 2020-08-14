//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#include "pch.h"
#include "jnc_CodeAssistThread.h"
#include "moc_jnc_CodeAssistThread.cpp"

namespace jnc {

//..............................................................................

CodeAssistThread::CodeAssistThread(QObject* parent):
	QThread(parent)
{
	printf("CodeAssistThread::CodeAssistThread(%p)\n", this);
	m_offset = 0;
	m_codeAssistKind = CodeAssistKind_Undefined;
}

CodeAssistThread::~CodeAssistThread()
{
	printf("CodeAssistThread::~CodeAssistThread(%p)\n", this);
}

void
CodeAssistThread::request(
	CodeAssistKind kind,
	const QString& text,
	int position
	)
{
	QByteArray text_utf8 = text.toUtf8();
	size_t offset = text.left(position).toUtf8().count();

	request(
		kind,
		sl::StringRef(text_utf8.data(), text_utf8.size()),
		offset
		);
}

void
CodeAssistThread::request(
	CodeAssistKind kind,
	const sl::StringRef& source,
	size_t offset
	)
{
	m_codeAssistKind = kind;
	m_source = source;
	m_offset = offset;

	printf(
		"CodeAssistThread::request(%s, %d)\n%s<<<--->>>%s\n",
		getCodeAssistKindString(kind),
		offset,
		source.getLeftSubString(offset).sz(),
		source.getSubString(offset).sz()
		);

	start();
}

void
CodeAssistThread::cancel()
{
	printf("CodeAssistThread::cancel -- not yet supported\n");
}

void
CodeAssistThread::run()
{
	m_module->initialize(
		"code-assist-module",
		ModuleCompileFlag_DisableCodeGen |
		ModuleCompileFlag_IgnoreOpaqueClassTypeInfo
		);

	m_module->setCompileErrorHandler(compileErrorHandler, this);
	m_module->addStaticLib(jnc::StdLib_getLib());
	m_module->addStaticLib(jnc::SysLib_getLib());

	QStringList::iterator it = m_importDirList.begin();
	for (; it != m_importDirList.end(); it++)
	{
		QByteArray dir = (*it).toUtf8();
		m_module->addImportDir(dir.constData());
	}

	m_module->generateCodeAssist(
		m_codeAssistKind,
		m_offset,
		m_source.cp(),
		m_source.getLength()
		);

	emit ready();
}

bool_t
CodeAssistThread::compileErrorHandler(
	void* context,
	jnc::ModuleCompileErrorKind errorKind
	)
{
	printf("%s\n", err::getLastErrorDescription().sz());
	return true;
}

//..............................................................................

} // namespace jnc
