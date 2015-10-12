#pragma once

//.............................................................................

enum JncFlag
{
	JncFlag_Help              = 0x0001,
	JncFlag_Version           = 0x0002,
	JncFlag_LlvmIr            = 0x0004,
	JncFlag_Jit               = 0x0010,
	JncFlag_McJit             = 0x0020,
	JncFlag_SimpleGcSafePoint = 0x0040,
	JncFlag_RunFunction       = 0x0080,
	JncFlag_Server            = 0x0100,
	JncFlag_DebugInfo         = 0x0200,
	JncFlag_StdInSrc          = 0x0400,
	JncFlag_Extension         = 0x1000,
	JncFlag_ExtensionList     = 0x2000,
	JncFlag_ExtensionSrcFile  = 0x4000,
};

struct CmdLine
{
	uint_t m_flags;
	uint16_t m_serverPort;
	size_t m_stackSizeLimit;
	size_t m_gcAllocSizeTrigger;
	size_t m_gcPeriodSizeTrigger;

	sl::String m_fileName;
	sl::String m_srcNameOverride;
	sl::String m_functionName;
	sl::String m_extensionSrcFileName;

	sl::BoxList <sl::String> m_importDirList;

	CmdLine ();
};

//.............................................................................

enum CmdLineSwitch
{
	CmdLineSwitch_Undefined = 0,
	CmdLineSwitch_Help,
	CmdLineSwitch_Version,
	CmdLineSwitch_StdInSrc,
	CmdLineSwitch_LlvmIr,
	CmdLineSwitch_DebugInfo,
	CmdLineSwitch_Jit,
	CmdLineSwitch_McJit,
	CmdLineSwitch_SimpleGcSafePoint,
	CmdLineSwitch_Run,
	CmdLineSwitch_ExtensionInfo,
	CmdLineSwitch_ExtensionList,

	CmdLineSwitch_RunFunction = sl::CmdLineSwitchFlag_HasValue,
	CmdLineSwitch_Server,
	CmdLineSwitch_GcAllocSizeTrigger,
	CmdLineSwitch_GcPeriodSizeTrigger,
	CmdLineSwitch_StackSizeLimit,
	CmdLineSwitch_SrcNameOverride,
	CmdLineSwitch_ImportDir,
	CmdLineSwitch_ExtensionSrcFile,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

AXL_SL_BEGIN_CMD_LINE_SWITCH_TABLE (CmdLineSwitchTable, CmdLineSwitch)
	AXL_SL_CMD_LINE_SWITCH_GROUP ("General options")
	AXL_SL_CMD_LINE_SWITCH_2 (
		CmdLineSwitch_Help,
		"h", "help", NULL,
		"Display this help"
		)
	AXL_SL_CMD_LINE_SWITCH_2 (
		CmdLineSwitch_Version,
		"v", "version", NULL,
		"Display compiler version"
		)
	AXL_SL_CMD_LINE_SWITCH_2 (
		CmdLineSwitch_Server,
		"s", "server", "<port>",
		"Run compiler server on TCP port <port>"
		)
	AXL_SL_CMD_LINE_SWITCH (
		CmdLineSwitch_StdInSrc,
		"stdin", NULL,
		"Get source from STDIN rather than from the file"
		)
	AXL_SL_CMD_LINE_SWITCH_2 (
		CmdLineSwitch_SrcNameOverride,
		"n", "source-name", "<name>",
		"Override source name (defaults to full-path/'stdin')"
		)

	AXL_SL_CMD_LINE_SWITCH_GROUP ("Compilation options")
	AXL_SL_CMD_LINE_SWITCH_2 (
		CmdLineSwitch_ImportDir,
		"I", "import-dir", NULL,
		"Add import directory"
		)
	AXL_SL_CMD_LINE_SWITCH_2 (
		CmdLineSwitch_LlvmIr,
		"l", "llvm-ir", NULL,
		"Emit LLVM IR (lli-compatible)"
		)
	AXL_SL_CMD_LINE_SWITCH_2 (
		CmdLineSwitch_DebugInfo,
		"g", "debug-info", NULL,
		"Generate debug information (does not work with legacy JIT)"
		)
	AXL_SL_CMD_LINE_SWITCH_2 (
		CmdLineSwitch_Jit,
		"j", "jit", NULL,
		"JIT compiled module"
		)
	AXL_SL_CMD_LINE_SWITCH_2 (
		CmdLineSwitch_McJit,
		"m", "mcjit", NULL,
		"Use MC-JIT engine (does not work on Windows)"
		)
	AXL_SL_CMD_LINE_SWITCH (
		CmdLineSwitch_SimpleGcSafePoint,
		"simple-gc-safe-point", NULL,
		"Use simple GC safe-point call (rather than write barrier)"
		)

	AXL_SL_CMD_LINE_SWITCH_GROUP ("Runtime options")
		AXL_SL_CMD_LINE_SWITCH_2 (
		CmdLineSwitch_Run,
		"r", "run", NULL,
		"Run function 'main' (implies JITting)"
		)
	AXL_SL_CMD_LINE_SWITCH_2 (
		CmdLineSwitch_RunFunction,
		"run-func", "run-function", "<function>",
		"Run function <function> (implies JITting)"
		)
	AXL_SL_CMD_LINE_SWITCH (
		CmdLineSwitch_GcAllocSizeTrigger,
		"gc-alloc-size-trigger", "<size>",
		"Specify the GC alloc size trigger"
		)
	AXL_SL_CMD_LINE_SWITCH (
		CmdLineSwitch_GcPeriodSizeTrigger,
		"gc-period-size-trigger", "<size>",
		"Specify the GC period size trigger"
		)
	AXL_SL_CMD_LINE_SWITCH (
		CmdLineSwitch_StackSizeLimit,
		"stack-size-limit", "<size>",
		"Specify the stack size limit"
		)

	AXL_SL_CMD_LINE_SWITCH_GROUP ("Extension (jncx) options")
	AXL_SL_CMD_LINE_SWITCH_3 (
		CmdLineSwitch_ExtensionList,
		"x", "ext", "extension", NULL,
		"Treat input file as extension"
		)

	AXL_SL_CMD_LINE_SWITCH (
		CmdLineSwitch_ExtensionList,
		"ext-list", NULL,
		"List the contents of extension"
		)

	AXL_SL_CMD_LINE_SWITCH (
		CmdLineSwitch_ExtensionSrcFile,
		"ext-src", "<file>",
		"Extract source file <file> from extension"
		)
AXL_SL_END_CMD_LINE_SWITCH_TABLE ()

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CmdLineParser: public sl::CmdLineParser <CmdLineParser, CmdLineSwitchTable>
{
	friend class sl::CmdLineParser <CmdLineParser, CmdLineSwitchTable>;

protected:
	CmdLine* m_cmdLine;

public:
	CmdLineParser (CmdLine* cmdLine)
	{
		m_cmdLine = cmdLine;
	}

protected:
	bool
	onValue (const char* value);

	bool
	onSwitch (
		SwitchKind switchKind,
		const char* value
		);

	bool
	finalize ();
};

//.............................................................................
