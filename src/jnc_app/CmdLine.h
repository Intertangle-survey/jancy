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
};

struct CmdLine
{
	uint_t m_flags;
	uint16_t m_serverPort;
	size_t m_stackSizeLimit;
	size_t m_gcAllocSizeTrigger;
	size_t m_gcPeriodSizeTrigger;

	sl::String m_srcFileName;
	sl::String m_srcNameOverride;
	sl::String m_functionName;

	sl::BoxList <sl::String> m_importDirList;

	CmdLine ();
};

//.............................................................................

enum CmdLineSwitchKind
{
	CmdLineSwitchKind_Undefined = 0,
	CmdLineSwitchKind_Help,
	CmdLineSwitchKind_Version,
	CmdLineSwitchKind_StdInSrc,
	CmdLineSwitchKind_LlvmIr,
	CmdLineSwitchKind_DebugInfo,
	CmdLineSwitchKind_Jit,
	CmdLineSwitchKind_McJit,
	CmdLineSwitchKind_SimpleGcSafePoint,
	CmdLineSwitchKind_Run,
	CmdLineSwitchKind_RunFunction = sl::CmdLineSwitchFlag_HasValue,
	CmdLineSwitchKind_Server,
	CmdLineSwitchKind_GcAllocSizeTrigger,
	CmdLineSwitchKind_GcPeriodSizeTrigger,
	CmdLineSwitchKind_StackSizeLimit,
	CmdLineSwitchKind_SrcNameOverride,
	CmdLineSwitchKind_ImportDir,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

AXL_SL_BEGIN_CMD_LINE_SWITCH_TABLE (CmdLineSwitchTable, CmdLineSwitchKind)
	AXL_SL_CMD_LINE_SWITCH_GROUP ("General options")
	AXL_SL_CMD_LINE_SWITCH_2 (
		CmdLineSwitchKind_Help,
		"h", "help", NULL,
		"Display this help"
		)
	AXL_SL_CMD_LINE_SWITCH_2 (
		CmdLineSwitchKind_Version,
		"v", "version", NULL,
		"Display compiler version"
		)
	AXL_SL_CMD_LINE_SWITCH_2 (
		CmdLineSwitchKind_Server,
		"s", "server", "<port>",
		"Run compiler server on TCP port <port>"
		)
	AXL_SL_CMD_LINE_SWITCH (
		CmdLineSwitchKind_StdInSrc,
		"stdin", NULL,
		"Get source from STDIN rather than from the file"
		)
	AXL_SL_CMD_LINE_SWITCH_2 (
		CmdLineSwitchKind_SrcNameOverride,
		"n", "source-name", "<name>",
		"Override source name (defaults to full-path/'stdin')"
		)

	AXL_SL_CMD_LINE_SWITCH_GROUP ("Compilation options")
	AXL_SL_CMD_LINE_SWITCH_2 (
		CmdLineSwitchKind_ImportDir,
		"I", "import-dir", NULL,
		"Add import directory"
		)
	AXL_SL_CMD_LINE_SWITCH_2 (
		CmdLineSwitchKind_LlvmIr,
		"l", "llvm-ir", NULL,
		"Emit LLVM IR (lli-compatible)"
		)
	AXL_SL_CMD_LINE_SWITCH_2 (
		CmdLineSwitchKind_DebugInfo,
		"g", "debug-info", NULL,
		"Generate debug information (does not work with legacy JIT)"
		)
	AXL_SL_CMD_LINE_SWITCH_2 (
		CmdLineSwitchKind_Jit,
		"j", "jit", NULL,
		"JIT compiled module"
		)
	AXL_SL_CMD_LINE_SWITCH_2 (
		CmdLineSwitchKind_McJit,
		"m", "mcjit", NULL,
		"Use MC-JIT engine (does not work on Windows)"
		)
	AXL_SL_CMD_LINE_SWITCH (
		CmdLineSwitchKind_SimpleGcSafePoint,
		"simple-gc-safe-point", NULL,
		"Use simple GC safe-point call (rather than write barrier)"
		)

	AXL_SL_CMD_LINE_SWITCH_GROUP ("Runtime options")
		AXL_SL_CMD_LINE_SWITCH_2 (
		CmdLineSwitchKind_Run,
		"r", "run", NULL,
		"Run function 'main' (implies JITting)"
		)
	AXL_SL_CMD_LINE_SWITCH (
		CmdLineSwitchKind_RunFunction,
		"run-function", "<function>",
		"Run function <function> (implies JITting)"
		)
	AXL_SL_CMD_LINE_SWITCH (
		CmdLineSwitchKind_GcAllocSizeTrigger,
		"gc-alloc-size-trigger", "<size>",
		"Specify the GC alloc size trigger"
		)
	AXL_SL_CMD_LINE_SWITCH (
		CmdLineSwitchKind_GcPeriodSizeTrigger,
		"gc-period-size-trigger", "<size>",
		"Specify the GC period size trigger"
		)
	AXL_SL_CMD_LINE_SWITCH (
		CmdLineSwitchKind_StackSizeLimit,
		"stack-size-limit", "<size>",
		"Specify the stack size limit"
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
