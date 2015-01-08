#include "pch.h"
#include "MainWindow.h"
#include "moc_MainWindow.cpp"
#include "MyLib.h"

//.............................................................................

static MainWindow* g_mainWindow = NULL;

MainWindow* getMainWindow ()
{
	return g_mainWindow;
}

//.............................................................................

MainWindow::MainWindow (
	QWidget *parent, 
	Qt::WindowFlags flags
	): 
	QMainWindow (parent, flags)
{
	ASSERT (!g_mainWindow);
	g_mainWindow = this;

	m_body = new QWidget (this);
	m_body->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
	setCentralWidget (m_body);

	QFont font ("Monospace", 9);
	font.setStyleHint (QFont::TypeWriter);

	m_output = new QPlainTextEdit (this);
	m_output->setReadOnly(true);
	m_output->setTextInteractionFlags (Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	m_output->setLineWrapMode (QPlainTextEdit::NoWrap);
	m_output->setFont (font);

	QDockWidget* dockWidget = new QDockWidget ("Output", this);
	dockWidget->setWidget (m_output);
	addDockWidget (Qt::BottomDockWidgetArea, dockWidget);

	mt::setTlsSlotValue (&m_module);
	mt::setTlsSlotValue (&m_runtime);
}

int MainWindow::output_va (const char* format, va_list va)
{
	QString string;
	string.vsprintf (format, va);
	m_output->moveCursor (QTextCursor::End);
	m_output->insertPlainText (string);
	return string.length ();
}

bool MainWindow::runScript (const QString& fileName_qt)
{
	if (fileName_qt.isEmpty ())
	{
		output ("usage: 02_dialog <script.jnc>\n");
		return false;
	}

	rtl::String fileName = (const utf16_t*) fileName_qt.unicode ();

	output ("Opening '%s'...\n", fileName.cc ());

	io::SimpleMappedFile file;
	bool result = file.open (fileName, io::FileFlag_ReadOnly);
	if (!result)
	{
		output ("%s\n", err::getError ()->getDescription ().cc ());
		return false;
	}

	output ("Parsing...\n");

	m_module.create (fileName);

	result = m_module.parse (
		fileName,
		(const char*) file.p (),
		file.getSize ()
		);

	if (!result)
	{
		output ("%s\n", err::getError ()->getDescription ().cc ());
		return false;
	}

	output ("Compiling...\n");

	result = m_module.compile ();
	if (!result)
	{
		output ("%s\n", err::getError ()->getDescription ().cc ());
		return false;
	}

	output ("Mapping...\n");

	result =
		m_module.createLlvmExecutionEngine () &&
		MyLib::mapFunctions (&m_module);

	if (!result)
	{
		output ("%s\n", err::getError ()->getDescription ().cc ());
		return false;
	}

	output ("JITting...\n");

	result = m_module.jit ();
	if (!result)
	{
		output ("%s\n", err::getError ()->getDescription ().cc ());
		return false;
	}

	jnc::Function* mainFunction = m_module.getFunctionByName ("main");
	if (!mainFunction)
	{
		output ("%s\n", err::getError ()->getDescription ().cc ());
		return false;
	}

	output ("Running...\n");

	jnc::ScopeThreadRuntime scopeRuntime (&m_runtime);

	result = 
		m_runtime.create (16 * 1024, 16 * 1024) &&
		m_runtime.addModule (&m_module); // 16K gc heap, 16K stack
		m_runtime.startup ();

	if (!result)
	{
		output ("%s\n", err::getError ()->getDescription ().cc ());
		return false;
	}

	m_layout.prime ();	
	m_layout.construct (QBoxLayout::TopToBottom, m_body);

	jnc::Function* constructor = m_module.getConstructor ();
	jnc::Function* destructor = m_module.getDestructor ();

	if (destructor)
		m_runtime.addStaticDestructor ((jnc::StaticDestructor*) destructor->getMachineCode ());

	try
	{
		typedef void ConstructorProc ();
		typedef int MainProc (MyLayout*);

		if (constructor)
			((ConstructorProc*) constructor->getMachineCode ()) ();

		int returnValue = ((MainProc*) mainFunction->getMachineCode ()) (&m_layout);
		output ("'main' returned (%d)\n", returnValue);
	}
	catch (err::Error error)
	{
		output ("Runtime error: %s\n", error.getDescription ().cc ());
	}
	catch (...)
	{
		output ("Unexpected runtime exception\n");
	}

	output ("Done.\n");
	return true;
}

void MainWindow::closeEvent (QCloseEvent* e)
{
	output ("Shutting down...\n");
	m_runtime.shutdown ();
}

//.............................................................................
