#include "pch.h"
#include "MyLib.h"

//.............................................................................

enum Error
{
	Error_Success = 0,
	Error_CmdLine = -1,
	Error_Io      = -2,
	Error_Compile = -3,
	Error_Runtime = -4,
};

//.............................................................................

#if (_JNC_ENV == JNC_ENV_WIN)

std::auto_ptr <char>
convertToUtf8 (
	const wchar_t* string,
	size_t length = -1
	)
{
	int requiredLength = ::WideCharToMultiByte (CP_UTF8, 0, string, (int) length, NULL, 0, NULL, NULL);
	if (!requiredLength)
		return std::auto_ptr <char> ();

	char* p = new char [requiredLength + 1];
	p [requiredLength] = 0; // ensure zero-termination

	::WideCharToMultiByte (CP_UTF8, 0, string, (int) length, p, requiredLength, NULL, NULL);
	return std::auto_ptr <char> (p);
}

#endif

//.............................................................................

#if (_JNC_ENV == JNC_ENV_WIN)
int
wmain (
	int argc,
	wchar_t* argv []
	)
#else
int
main (
	int argc,
	char* argv []
	)
#endif
{
	bool result;

	printf ("Initializing...\n");
	
	if (argc < 2)
	{
		printf ("usage: 01_export <script.jnc>\n");
		return Error_CmdLine;
	}

	jnc::initialize ();

#if (_JNC_ENV == JNC_ENV_WIN)
	std::auto_ptr <char> fileName_utf8 = convertToUtf8 (argv [1]);
	const char* fileName = fileName_utf8.get ();
#else
	const char* fileName = argv [1];
#endif

	jnc::AutoModule module;

	module->initialize (fileName);
	module->addStaticLib (jnc::StdLib_getLib ());
	module->addStaticLib (MyLib_getLib ());

	printf ("Parsing '%s'...\n", fileName);

	result = 
		module->parseFile (fileName) &&
		module->parseImports ();

	if (!result)
	{
		printf ("%s\n", jnc::getLastErrorDescription_v ());
		return Error_Compile;
	}

	printf ("Compiling...\n");

	result = module->compile ();
	if (!result)
	{
		printf ("%s\n", jnc::getLastErrorDescription_v ());
		return Error_Compile;
	}

	printf ("JITting...\n");

	result = module->jit ();
	if (!result)
	{
		printf ("%s\n", jnc::getLastErrorDescription_v ());
		return Error_Compile;
	}

	jnc::Namespace* nspace = module->getGlobalNamespace ()->getNamespace ();
	jnc::Function* mainFunction = nspace->findFunction ("main");
	if (!mainFunction)
	{
		printf ("%s\n", jnc::getLastErrorDescription_v ());
		return Error_Compile;
	}

	printf ("Running...\n");

	jnc::AutoRuntime runtime;

	result = runtime->startup (module);
	if (!result)
	{
		printf ("%s\n", jnc::getLastErrorDescription_v ());
		return Error_Runtime;
	}

	Error finalResult = Error_Success;

	int returnValue;
	result = jnc::callFunction (runtime, mainFunction, &returnValue);
	if (!result)
	{
		printf ("Runtime error: %s\n", jnc::getLastErrorDescription_v ());
		finalResult = Error_Runtime;
	}

	printf ("Shutting down...\n");

	runtime->shutdown ();

	printf ("Done.\n");

	return finalResult;
}

//.............................................................................
