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

#pragma once

#include <QPlainTextEdit>

namespace jnc {

class EditPrivate;
class LineNumberMargin;

#if (_JNC_EDIT_DLL)
#  define JNC_EDIT_EXPORT Q_DECL_EXPORT
#else
#  define JNC_EDIT_EXPORT Q_DECL_IMPORT
#endif

//..............................................................................

class JNC_EDIT_EXPORT Edit: public QPlainTextEdit
{
	Q_OBJECT
	Q_DECLARE_PRIVATE(Edit)
	Q_DISABLE_COPY(Edit)
	Q_PROPERTY(bool isLineNumberMarginEnabled READ isLineNumberMarginEnabled WRITE enableLineNumberMargin)
	Q_PROPERTY(bool isCurrentLineHighlightingEnabled READ isCurrentLineHighlightingEnabled WRITE enableCurrentLineHighlighting)
	Q_PROPERTY(bool isSyntaxHighlightingEnabled READ isSyntaxHighlightingEnabled WRITE enableSyntaxHighlighting)
	Q_PROPERTY(CodeAssistTriggers codeAssistTriggers READ codeAssistTriggers WRITE setCodeAssistTriggers)
	Q_PROPERTY(QStringList importDirList READ importDirList WRITE setImportDirList)

	friend class LineNumberMargin;

public:
    enum CodeAssistTrigger
	{
		QuickInfoTipOnHoverOverIdentifier      = 0x0001,
		QuickInfoTipOnCursorOverIdentifier     = 0x0002,
		ArgumentTipOnCtrlShiftSpace            = 0x0004,
		ArgumentTipOnTypeLeftParenthesis       = 0x0008,
		ArgumentTipOnTypeComma                 = 0x0010,
		ArgumentTipOnHoverOverLeftParenthesis  = 0x0020,
		ArgumentTipOnCursorOverLeftParenthesis = 0x0040,
		ArgumentTipOnHoverOverComma            = 0x0080,
		ArgumentTipOnCursorOverComma           = 0x0100,
		AutoCompleteOnCtrlSpace                = 0x0200,
		AutoCompleteListOnTypeDot              = 0x0400,
		AutoCompleteListOnTypeIdentifier       = 0x0800,
		GotoDefinitionOnCtrlClick              = 0x1000,
    };

    Q_DECLARE_FLAGS(CodeAssistTriggers, CodeAssistTrigger)

public:
	Edit(QWidget* parent = NULL);
	~Edit();

	// properties

	bool isLineNumberMarginEnabled();
	void enableLineNumberMargin(bool isEnabled);
	bool isCurrentLineHighlightingEnabled();
	void enableCurrentLineHighlighting(bool isEnabled);
	bool isSyntaxHighlightingEnabled();
	void enableSyntaxHighlighting(bool isEnabled);
	CodeAssistTriggers codeAssistTriggers();
	void setCodeAssistTriggers(CodeAssistTriggers triggers);
	QStringList importDirList();
	void setImportDirList(const QStringList& dirList);

	// selection operations

	void setTextCursorLineCol(
		int line,
		int col
		);

public slots:
	// code-assist

	void quickInfoTip();
	void argumentTip();
	void autoComplete();
	void autoCompleteList();
	void gotoDefinition();

protected:
	virtual void resizeEvent(QResizeEvent* e);
    virtual void keyPressEvent(QKeyEvent* e);
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void mouseMoveEvent(QMouseEvent* e);

protected:
	QScopedPointer<EditPrivate> d_ptr;
};

//..............................................................................

Q_DECLARE_OPERATORS_FOR_FLAGS(Edit::CodeAssistTriggers)

} // namespace jnc
