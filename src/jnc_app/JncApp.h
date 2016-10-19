#pragma once

struct CmdLine;

//..............................................................................

class JncApp
{
protected:
	CmdLine* m_cmdLine;
	jnc::AutoModule m_module;
	jnc::AutoRuntime m_runtime;

public:
	JncApp (CmdLine* cmdLine);

	bool
	parse ();

	bool
	compile ()
	{
		return m_module->compile ();
	}

	bool
	jit ()
	{
		return m_module->jit ();
	}

	void
	printLlvmIr ()
	{
		printf ("%s", m_module->getLlvmIrString_v ());
	}

	bool
	generateDocumentation ();

	bool
	runFunction (int* returnValue = NULL);
};

//..............................................................................
