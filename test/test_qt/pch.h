#pragma once

#define _CRT_SECURE_NO_WARNINGS // disable useless warnings about "unsafe" string functions
#define _SCL_SECURE_NO_WARNINGS // disable useless warnings about "unsafe" iterators

#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#include "axl_g_Pch.h"

#undef min
#undef max

// QT

#include <QtGui>
#include <QAction>
#include <QApplication>
#include <QDockWidget>
#include <QFileDialog>
#include <QHeaderView>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QPlainTextEdit>
#include <QStatusBar>
#include <QSyntaxHighlighter>
#include <QTextBlock>
#include <QToolBar>
#include <QTreeWidget>
#include <QWidget>

// Jancy

#include "jnc_Module.h"
#include "jnc_Runtime.h"
#include "jnc_ExtensionLib.h"
#include "jnc_CallSite.h"

#include "axl_io_SockAddr.h"
#include "axl_sys_Time.h"

using namespace axl;

#if (_JNC_ENV == JNC_ENV_WIN)

// Memory Leak Detection

#	define _CRTDBG_MAP_ALLOC
#	include <stdlib.h>
#	include <crtdbg.h>

#	ifdef _DEBUG
#		ifndef DBG_NEW
#			define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#			define new DBG_NEW
#		endif
#	endif

#elif (_JNC_ENV == JNC_ENV_POSIX)
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <netinet/ip.h>
#endif