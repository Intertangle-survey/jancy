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
#include "jnc_Edit_p.h"
#include "moc_jnc_Edit.cpp"
#include "moc_jnc_Edit_p.cpp"
#include "qrc_res.cpp"

namespace jnc {

//..............................................................................

Edit::Edit(QWidget *parent):
	QPlainTextEdit(parent),
	d_ptr(new EditPrivate)
{
	Q_D(Edit);

	d->q_ptr = this;
	d->init();
}

Edit::~Edit()
{
}

bool
Edit::isLineNumberMarginEnabled()
{
	Q_D(Edit);
	return d->m_lineNumberMargin != NULL;
}

void
Edit::enableLineNumberMargin(bool isEnabled)
{
	Q_D(Edit);
	d->enableLineNumberMargin(isEnabled);
}

bool
Edit::isCurrentLineHighlightingEnabled()
{
	Q_D(Edit);
	return d->m_isCurrentLineHighlightingEnabled;
}

void
Edit::enableCurrentLineHighlighting(bool isEnabled)
{
	Q_D(Edit);
	return d->enableCurrentLineHighlighting(isEnabled);
}

bool
Edit::isSyntaxHighlightingEnabled()
{
	Q_D(Edit);
	return d->m_syntaxHighlighter != NULL;
}

void
Edit::enableSyntaxHighlighting(bool isEnabled)
{
	Q_D(Edit);
	return d->enableSyntaxHighlighting(isEnabled);
}

Edit::CodeAssistTriggers
Edit::codeAssistTriggers()
{
	Q_D(Edit);
	return d->m_codeAssistTriggers;
}

void
Edit::setCodeAssistTriggers(CodeAssistTriggers triggers)
{
	Q_D(Edit);
	d->m_codeAssistTriggers = triggers;
}

QStringList
Edit::importDirList()
{
	Q_D(Edit);
	return d->m_importDirList;
}

void
Edit::setImportDirList(const QStringList& dirList)
{
	Q_D(Edit);
	d->m_importDirList = dirList;
}

void
Edit::setTextCursorLineCol(
	int line,
	int col
	)
{
	Q_D(Edit);
	setTextCursor(d->getCursorFromLineCol(lex::LineCol(line, col)));
}

void
Edit::quickInfoTip()
{
	Q_D(Edit);
	d->requestCodeAssist(CodeAssistKind_QuickInfoTip, true);
}

void
Edit::argumentTip()
{
	Q_D(Edit);
	d->requestCodeAssist(CodeAssistKind_ArgumentTip, true);
}

void
Edit::autoComplete()
{
	Q_D(Edit);
	d->requestCodeAssist(CodeAssistKind_AutoComplete, true);
}

void
Edit::autoCompleteList()
{
	Q_D(Edit);
	d->requestCodeAssist(CodeAssistKind_AutoCompleteList, true);
}

void
Edit::gotoDefinition()
{
	Q_D(Edit);
	d->requestCodeAssist(CodeAssistKind_GotoDefinition, true);
}

void
Edit::resizeEvent(QResizeEvent *e)
{
	Q_D(Edit);

	QPlainTextEdit::resizeEvent(e);

	if (d->m_lineNumberMargin)
		d->updateLineNumberMarginGeometry();
}

void
Edit::keyPressEvent(QKeyEvent* e)
{
	Q_D(Edit);

	int key = e->key();
	QString text = e->text();
	QChar ch = text.isEmpty() ? QChar() : text.at(0);
	bool isHandled = true; // assume handled

	if (!d->isCompleterVisible())
		switch (key)
		{
		case Qt::Key_Escape:
			d->hideCodeAssist();
			break;

		case Qt::Key_Home:
			isHandled = d->keyPressHome(e->modifiers());
			break;

		case Qt::Key_Tab:
			isHandled = d->keyPressTab(e->modifiers());
			break;

		case Qt::Key_Space:
			isHandled = d->keyPressSpace(e->modifiers());
			break;

		case Qt::Key_Enter:
		case Qt::Key_Return:
			isHandled = d->keyPressEnter(e->modifiers());
			break;

		default:
			QPlainTextEdit::keyPressEvent(e);
			d->requestCodeAssistOnChar(ch.toLatin1());
		}
	else
		switch (key)
		{
		case Qt::Key_Enter:
		case Qt::Key_Return:
		case Qt::Key_Escape:
		case Qt::Key_Tab:
		case Qt::Key_Backtab:
		case Qt::Key_Up:
		case Qt::Key_Down:
			e->ignore(); // let the completer do the default processing
			break;

		case Qt::Key_Home:
			d->keyPressHome(e->modifiers());
			break;

		case Qt::Key_Space:
			if (d->keyPressSpace(e->modifiers()))
				break;

			// fall through

		default:
			if (!ch.isPrint() || ch.isLetterOrNumber() || ch == '_')
			{
				isHandled = false;
				break;
			}

			d->applyCompleter();
			QPlainTextEdit::keyPressEvent(e);
			d->requestCodeAssistOnChar(ch.toLatin1());
		}

	if (!isHandled)
		QPlainTextEdit::keyPressEvent(e);

}

void
Edit::mousePressEvent(QMouseEvent* e)
{
	Q_D(Edit);

	// check for triggers first

	QPlainTextEdit::mousePressEvent(e);
}

void
Edit::mouseMoveEvent(QMouseEvent* e)
{
	Q_D(Edit);

	QPlainTextEdit::mouseMoveEvent(e);

	if (!d->isCompleterVisible() &&
		(d->m_codeAssistTriggers & QuickInfoTipOnMouseOverIdentifier))
		d->requestQuickInfoTip(e->pos());
}

void
Edit::enterEvent(QEvent* e)
{
	Q_D(Edit);

	QPlainTextEdit::enterEvent(e);

	if (!d->isCompleterVisible() &&
		d->m_lastCodeAssistKind == CodeAssistKind_QuickInfoTip &&
		(d->m_codeAssistTriggers & QuickInfoTipOnMouseOverIdentifier))
	{
		QPoint pos = mapFromGlobal(QCursor::pos());
		d->requestQuickInfoTip(pos);
	}
}

//..............................................................................

EditPrivate::EditPrivate()
{
    q_ptr = NULL;
	m_syntaxHighlighter = NULL;
	m_lineNumberMargin = NULL;
	m_isCurrentLineHighlightingEnabled = false;
	m_thread = NULL;
	m_completer = NULL;
	m_lastCodeAssistKind = CodeAssistKind_Undefined;
	m_lastCodeAssistOffset = -1;
	m_lastCodeAssistPosition = -1;
	m_pendingCodeAssistPosition = -1;

	m_codeAssistTriggers =
//		Edit::QuickInfoTipOnMouseOverIdentifier | // doesn't work quite well yet
		Edit::ArgumentTipOnCtrlShiftSpace |
		Edit::ArgumentTipOnTypeLeftParenthesis |
		Edit::ArgumentTipOnTypeComma |
		Edit::AutoCompleteOnCtrlSpace |
		Edit::AutoCompleteListOnTypeDot |
		Edit::AutoCompleteListOnTypeIdentifier |
		Edit::GotoDefinitionOnCtrlClick;
}

QImage createSubImage(QImage* image, const QRect & rect) {
    size_t offset = rect.x() * image->depth() / 8
                    + rect.y() * image->bytesPerLine();
    return QImage(image->bits() + offset, rect.width(), rect.height(),
                  image->bytesPerLine(), image->format());
}

void
EditPrivate::init()
{
	Q_Q(Edit);

#if (_JNC_OS_DARWIN)
	QFont font("Menlo", 11);
#elif (_JNC_OS_WIN)
	QFont font("Consolas", 10);
#else
	QFont font("Monospace", 9);
#endif

	font.setFixedPitch(true);
	font.setKerning(false);
	font.setStyleHint(
		QFont::Monospace,
		(QFont::StyleStrategy)(QFont::NoFontMerging | QFont::ForceIntegerMetrics)
		);

	QPalette palette = q->palette();
	palette.setColor(QPalette::Highlight, Color_SelectionBack);
	palette.setBrush(QPalette::HighlightedText, QBrush(Qt::NoBrush));

	q->setFont(font);
	q->setTabStopWidth(q_ptr->fontMetrics().width(' ') * 4);
	q->setWordWrapMode(QTextOption::NoWrap);
	q->setMouseTracking(true);
	q->setPalette(palette);

	enableSyntaxHighlighting(true);
	enableLineNumberMargin(true);
	enableCurrentLineHighlighting(true);

	QObject::connect(
		q, SIGNAL(cursorPositionChanged()),
		this, SLOT(onCursorPositionChanged())
		);

	static const size_t iconIdxTable[] =
	{
		0,  // Icon_Object
		1,  // Icon_Namespace
		2,  // Icon_Event
		4,  // Icon_Function
		6,  // Icon_Property
		7,  // Icon_Variable
		12, // Icon_Field
		11, // Icon_Const
		10, // Icon_Type
		9,  // Icon_Typedef
 	};

	QPixmap imageList(":/Images/ObjectIcons");
	int iconSize = imageList.height();

	for (size_t i = 0; i < countof(m_iconTable); i++)
		m_iconTable[i] = imageList.copy(iconIdxTable[i] * iconSize, 0, iconSize, iconSize);
}

void
EditPrivate::enableSyntaxHighlighting(bool isEnabled)
{
	Q_Q(Edit);

	if (isEnabled)
	{
		if (!m_syntaxHighlighter)
			m_syntaxHighlighter = new JancyHighlighter(q->document());
	}
	else if (m_syntaxHighlighter)
	{
		m_syntaxHighlighter->setDocument(NULL);
		delete m_syntaxHighlighter;
		m_syntaxHighlighter = NULL;
	}
}

void
EditPrivate::enableLineNumberMargin(bool isEnabled)
{
	Q_Q(Edit);

	if (isEnabled)
	{
		if (m_lineNumberMargin)
			return;

		m_lineNumberMargin = new LineNumberMargin(q);
		q->setViewportMargins(m_lineNumberMargin->width(), 0, 0, 0);
		updateLineNumberMarginGeometry();
		m_lineNumberMargin->show();

		QObject::connect(
			q, SIGNAL(updateRequest(const QRect&, int)),
			this, SLOT(updateLineNumberMargin(const QRect&, int))
			);
	}
	else
	{
		if (!m_lineNumberMargin)
			return;

		QObject::disconnect(
			q, SIGNAL(updateRequest(const QRect&, int)),
			this, SLOT(updateLineNumberMargin(const QRect&, int))
			);

		q->setViewportMargins(0, 0, 0, 0);
		delete m_lineNumberMargin;
		m_lineNumberMargin = NULL;
	}
}

void
EditPrivate::enableCurrentLineHighlighting(bool isEnabled)
{
	Q_Q(Edit);

	if (isEnabled == m_isCurrentLineHighlightingEnabled)
		return;

	if (isEnabled)
		highlightCurrentLine();
	else
		q->setExtraSelections(QList<QTextEdit::ExtraSelection>());

	m_isCurrentLineHighlightingEnabled = isEnabled;
}

void
EditPrivate::updateLineNumberMarginGeometry()
{
	ASSERT(m_lineNumberMargin);

	Q_Q(Edit);

	QRect rect = q->contentsRect();

	m_lineNumberMargin->setGeometry(
		rect.left(),
		rect.top(),
		m_lineNumberMargin->width(),
		rect.height()
		);
}

void
EditPrivate::requestCodeAssistOnChar(char c)
{
	switch (c)
	{
	case '.':
		if (m_codeAssistTriggers & Edit::AutoCompleteListOnTypeDot)
			requestCodeAssist(CodeAssistKind_AutoCompleteList);
		break;

	case '(':
		if (m_codeAssistTriggers & Edit::ArgumentTipOnTypeLeftParenthesis)
			requestCodeAssist(CodeAssistKind_ArgumentTip);
		break;

	case ',':
		if (m_codeAssistTriggers & Edit::ArgumentTipOnTypeComma)
			requestCodeAssist(CodeAssistKind_ArgumentTip);
		break;
	}
}

void
EditPrivate::requestCodeAssist(
	CodeAssistKind kind,
	bool isSync
	)
{
	Q_Q(Edit);

	QTextCursor cursor = q->textCursor();
	requestCodeAssist(kind, cursor.position(), isSync);
}

void
EditPrivate::requestCodeAssist(
	CodeAssistKind kind,
	int position,
	bool isSync
	)
{
	Q_Q(Edit);

	if (m_thread)
		m_thread->cancel();

	m_thread = new CodeAssistThread(this);
	m_thread->m_importDirList = m_importDirList;

	QObject::connect(
		m_thread, SIGNAL(ready()),
		this, SLOT(onCodeAssistReady())
		);

	QObject::connect(
		m_thread, SIGNAL(finished()),
		this, SLOT(onThreadFinished())
		);

	m_thread->request(kind, ref::g_nullPtr, position, q->toPlainText());
}

void
EditPrivate::requestQuickInfoTip(const QPoint& pos)
{
	Q_Q(Edit);

	QTextCursor cursor = q->cursorForPosition(pos);
	m_pendingCodeAssistPosition = cursor.position();
	m_quickInfoTipTimer.start(Timeout_QuickInfo, this);
}

void
EditPrivate::hideCodeAssist()
{
	if (m_completer)
		m_completer->popup()->hide();

	QToolTip::hideText();

	m_lastCodeAssistKind = CodeAssistKind_Undefined;
	m_lastCodeAssistPosition = -1;
	m_thread = NULL;
}

void
EditPrivate::updateLineNumberMargin(
	const QRect& rect,
	int dy
	)
{
	ASSERT(m_lineNumberMargin);

	if (dy)
		m_lineNumberMargin->scroll(0, dy);
	else
		m_lineNumberMargin->update(
			0,
			rect.y(),
			m_lineNumberMargin->width(),
			rect.height()
			);
}

void
EditPrivate::highlightCurrentLine()
{
	Q_Q(Edit);

	QTextEdit::ExtraSelection selection;
	selection.cursor = q->textCursor();
	selection.cursor.clearSelection();
	selection.format.setBackground(QColor(Color_CurrentLineBack));
	selection.format.setProperty(QTextFormat::FullWidthSelection, true);

	QList<QTextEdit::ExtraSelection> extraSelections;
	extraSelections.append(selection);
	q->setExtraSelections(extraSelections);
}

void
EditPrivate::ensureCompleter()
{
	if (m_completer)
		return;

	Q_Q(Edit);

	QTreeView* popup = new QTreeView;
	CompleterItemDelegate* itemDelegate = new CompleterItemDelegate(popup);
	popup->setHeaderHidden(true);
	popup->setRootIsDecorated(false);
	popup->setSelectionBehavior(QAbstractItemView::SelectRows);
	popup->setFont(q->font());
	popup->setItemDelegateForColumn(Column_Name, itemDelegate);
	popup->setItemDelegateForColumn(Column_Synopsis, itemDelegate);

	m_completer = new QCompleter(q);
	m_completer->setWidget(q);
	m_completer->setCompletionMode(QCompleter::PopupCompletion);
	m_completer->setMaxVisibleItems(Limit_MaxVisibleItemCount);
	m_completer->setPopup(popup);

    QObject::connect(
		m_completer, SIGNAL(activated(const QString&)),
		this, SLOT(onCompleterActivated(const QString&))
		);
}

void
EditPrivate::updateCompleter(bool isForced)
{
	ASSERT(m_completer);

	Q_Q(Edit);

	QTextCursor cursor = q->textCursor();
	int position = cursor.position();
	int anchorPosition = getLastCodeAssistPosition();
	if (position < anchorPosition)
	{
		hideCodeAssist();
		return;
	}

	cursor.setPosition(position, QTextCursor::MoveAnchor);
	cursor.setPosition(anchorPosition, QTextCursor::KeepAnchor);
	QString prefix = cursor.selectedText();

	if (m_lastCodeAssistKind == CodeAssistKind_ImportAutoCompleteList)
		prefix.remove(0, 1); // opening quotation mark

	if (!isForced && prefix == m_completer->completionPrefix())
		return;

	QAbstractItemView* popup = (QTreeView*)m_completer->popup();
	QTreeView* treeView = (QTreeView*)popup;
	m_completer->setCompletionPrefix(prefix);
	popup->setCurrentIndex(m_completer->completionModel()->index(0, 0));

	QMargins margins = treeView->contentsMargins();
	int marginWidth = margins.left() + margins.right();
	int scrollWidth = popup->verticalScrollBar()->sizeHint().width();
	int nameWidth = popup->sizeHintForColumn(Column_Name);
	int synopsisWidth = popup->sizeHintForColumn(Column_Synopsis);

	if (nameWidth > Limit_MaxNameWidth)
		nameWidth = Limit_MaxNameWidth;

	if (synopsisWidth > Limit_MaxSynopsisWidth)
		synopsisWidth = Limit_MaxSynopsisWidth;

	treeView->setColumnWidth(Column_Name, nameWidth);
	treeView->setColumnWidth(Column_Synopsis, synopsisWidth);

	int fullWidth = nameWidth + synopsisWidth + scrollWidth + marginWidth;
	m_completerRect.setWidth(fullWidth);
	m_completer->complete(m_completerRect);
}

void
EditPrivate::applyCompleter()
{
	Q_Q(Edit);

	ASSERT(m_completer);

	QModelIndex index = m_completer->popup()->currentIndex();
	QString completion = index.isValid() ? m_completer->popup()->model()->data(index).toString() : QString();
	if (!completion.isEmpty())
		onCompleterActivated(completion);

	hideCodeAssist();
}

lex::LineCol
EditPrivate::getLineColFromCursor(const QTextCursor& cursor0)
{
	QTextCursor cursor = cursor0;
	cursor.movePosition(QTextCursor::StartOfLine);

	int line = 0;
	while (cursor.positionInBlock() > 0)
	{
		line++;
		cursor.movePosition(QTextCursor::Up);
	}

	QTextBlock block = cursor.block().previous();
	while (block.isValid())
	{
		line += block.lineCount();
		block = block.previous();
	}

	return lex::LineCol(line, cursor0.columnNumber());
}

QTextCursor
EditPrivate::getCursorFromLineCol(const lex::LineCol& pos)
{
	Q_Q(Edit);

	QTextCursor cursor = q->textCursor();
	cursor.setPosition(0);
	cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, pos.m_line);
	cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, pos.m_col);
	return cursor;
}

QTextCursor
EditPrivate::getCursorFromOffset(size_t offset)
{
	Q_Q(Edit);

	QString prefix = q->toPlainText().toUtf8().left(offset);
	int position = prefix.length();
	QTextCursor cursor = q->textCursor();
	cursor.setPosition(position);
	return cursor;
}

QTextCursor
EditPrivate::getLastCodeAssistCursor()
{
	Q_Q(Edit);

	int position = getLastCodeAssistPosition();
	QTextCursor cursor = q->textCursor();
	cursor.setPosition(position);
	return cursor;
}

QRect
EditPrivate::getLastCodeAssistCursorRect()
{
	Q_Q(Edit);

	QTextCursor cursor = getLastCodeAssistCursor();
	QRect rect = q->cursorRect(cursor);

#if (QT_VERSION >= 0x050500)
	rect.translate(q->viewportMargins().left(), q->viewportMargins().top());
#else
	rect.translate(m_lineNumberMargin ? m_lineNumberMargin->width() : 0, 0);
#endif

	return rect;
}

int
EditPrivate::calcLastCodeAssistPosition()
{
	ASSERT(m_lastCodeAssistKind && m_lastCodeAssistPosition == -1);

	QTextCursor cursor = getCursorFromOffset(m_lastCodeAssistOffset);
	m_lastCodeAssistPosition = cursor.position();
	return m_lastCodeAssistPosition;
}

QPoint
EditPrivate::getLastCodeAssistToolTipPoint(bool isBelowCurrentCursor)
{
	Q_Q(Edit);

	enum
	{
		QtToolTipOffset_X = 2,
		QtToolTipOffset_Y = 16,
	};

	QRect rect = getLastCodeAssistCursorRect();

	if (isBelowCurrentCursor)
		rect.moveTop(q->cursorRect().top());

	QPoint point = q->mapToGlobal(rect.bottomLeft());
	point -= QPoint(QtToolTipOffset_X, QtToolTipOffset_Y);
	return point;
}

void
EditPrivate::createQuickInfoTip(ModuleItem* item)
{
	Q_Q(Edit);

	QString text = item->getSynopsis_v();
	QPoint point = getLastCodeAssistToolTipPoint();
	QToolTip::showText(point, text, q);
}

void
EditPrivate::createArgumentTip(
	FunctionType* functionType,
	size_t argumentIdx
	)
{
	Q_Q(Edit);

	#define ML_ARG_INDENT "&nbsp;&nbsp;&nbsp;&nbsp;"

	Type* returnType = functionType->getReturnType();
	size_t argCount = functionType->getArgCount();
	size_t lastArgIdx = argCount - 1;

	QString text = "<p style='white-space:pre'>";
	text += returnType->getTypeString();

	if (argCount >= 2)
		text += " (<br>" ML_ARG_INDENT;
	else
		text += " (";

	for (size_t i = 0; i < argCount; i++)
	{
		FunctionArg* arg = functionType->getArg(i);
		Type* argType = arg->getType();

		if (i == argumentIdx)
			text += "<b>";

		text += argType->getTypeStringPrefix();
		text += ' ';
		text += arg->getDecl()->getName();
		text += argType->getTypeStringSuffix();

		if (i == argumentIdx)
			text += "</b>";

		if (i != lastArgIdx)
			text += ",<br>" ML_ARG_INDENT;
	}

	if (argCount >= 2)
		text += "<br>" ML_ARG_INDENT ")</p>";
	else
		text += ")</p>";

	QPoint point = getLastCodeAssistToolTipPoint(true);
	if (point != m_lastToolTipPoint)
	{
		QToolTip::showText(point, " ", q); // ensure tipChanged
		m_lastToolTipPoint = point;
	}

	QToolTip::showText(point, text, q);
}

size_t
EditPrivate::getItemIconIdx(ModuleItem* item)
{
	static const size_t iconIdxTable[ModuleItemKind__Count] =
	{
		Icon_Object,    // ModuleItemKind_Undefined
		Icon_Namespace, // ModuleItemKind_Namespace
		Icon_Object,    // ModuleItemKind_Attribute
		Icon_Object,    // ModuleItemKind_AttributeBlock
		Icon_Object,    // ModuleItemKind_Scope
		Icon_Type,      // ModuleItemKind_Type
		Icon_Typedef,   // ModuleItemKind_Typedef
		Icon_Object,    // ModuleItemKind_Alias
		Icon_Const,     // ModuleItemKind_Const
		Icon_Variable,  // ModuleItemKind_Variable
		Icon_Function,  // ModuleItemKind_Function
		Icon_Variable,  // ModuleItemKind_FunctionArg
		Icon_Function,  // ModuleItemKind_FunctionOverload
		Icon_Property,  // ModuleItemKind_Property
		Icon_Property,  // ModuleItemKind_PropertyTemplate
		Icon_Const,     // ModuleItemKind_EnumConst
		Icon_Variable,  // ModuleItemKind_Field
		Icon_Type,      // ModuleItemKind_BaseTypeSlot
		Icon_Function,  // ModuleItemKind_Orphan
		Icon_Object,    // ModuleItemKind_LazyImport
	};

	ModuleItemKind itemKind = item->getItemKind();
	return ((size_t)itemKind < countof(iconIdxTable)) ? iconIdxTable[itemKind] : Icon_Object;
}

void
EditPrivate::addAutoCompleteNamespace(
	QStandardItemModel* model,
	Namespace* nspace
	)
{
	NamespaceKind namespaceKind = nspace->getNamespaceKind();
	if (namespaceKind == NamespaceKind_Type)
	{
		NamedType* namedType = (NamedType*)nspace->getParentItem();
		if (namedType->getTypeKindFlags() & TypeKindFlag_Derivable)
		{
			DerivableType* derivableType = (DerivableType*)namedType;

			size_t count = derivableType->getBaseTypeCount();
			for (size_t i = 0; i < count; i++)
			{
				BaseTypeSlot* slot = derivableType->getBaseType(i);
				DerivableType* baseType = slot->getType();
				if (!(baseType->getTypeKindFlags() & TypeKindFlag_Import)) // still unresolved
					addAutoCompleteNamespace(model, baseType->getNamespace());
			}
		}
	}

	size_t count = nspace->getItemCount();
	for (size_t i = 0; i < count; i++)
	{
		ModuleItem* item = nspace->getItem(i);
		ModuleItemKind itemKind = item->getItemKind();
		Type* type = item->getType();
		QString name = item->getDecl()->getName();
		QString synopsis = item->getSynopsis_v();
		size_t iconIdx = getItemIconIdx(item);

		QStandardItem* nameItem = new QStandardItem;
		nameItem->setText(name);
		nameItem->setData(name.toLower(), Role_CaseInsensitiveSort);

		QStandardItem* synopsisItem = new QStandardItem;
		synopsisItem->setText(synopsis);

		if (iconIdx != -1)
			synopsisItem->setIcon(m_iconTable[iconIdx]);

		QList<QStandardItem*> row;
		row.append(nameItem);
		row.append(synopsisItem);

		model->appendRow(row);
	}
}

void
EditPrivate::createAutoCompleteList(
	Namespace* nspace,
	uint_t flags
	)
{
	Q_Q(Edit);

	QStandardItemModel* model = new QStandardItemModel(m_completer);
	addAutoCompleteNamespace(model, nspace);

	if (flags & CodeAssistNamespaceFlag_IncludeParentNamespace)
	{
		nspace = nspace->getParentNamespace();

		while (nspace)
		{
			addAutoCompleteNamespace(model, nspace);
			nspace = nspace->getParentNamespace();
		}
	}

	ensureCompleter();

	model->setSortRole(Role_CaseInsensitiveSort);
	model->sort(0);

	m_completer->setModel(model);
	m_completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
	m_completer->setCaseSensitivity(Qt::CaseInsensitive);
	m_completer->setWrapAround(false);
	m_completer->setCompletionPrefix(QString());

	m_completerRect = getLastCodeAssistCursorRect();
	updateCompleter(true);
}

void
EditPrivate::addFile(
	QStandardItemModel* model,
	const QString& fileName
	)
{
	QStandardItem* qtItem = new QStandardItem;
	qtItem->setText(fileName);
	qtItem->setData(fileName.toLower(), Role_CaseInsensitiveSort);
	qtItem->setIcon(m_fileIconProvider.icon(QFileIconProvider::File));

	model->appendRow(qtItem);
}

void
EditPrivate::createImportAutoCompleteList(Module* module)
{
	Q_Q(Edit);

	QStandardItemModel* model = new QStandardItemModel(m_completer);

	QStringList importDirFilter;
	importDirFilter.append("*.jnc");
	importDirFilter.append("*.jncx");

	handle_t it = module->getImportDirIterator();
	while (it)
	{
		const char* dir = module->getNextImportDir(&it);
		QDirIterator dirIt(dir, importDirFilter);

		while (dirIt.hasNext())
		{
			dirIt.next();
			addFile(model, dirIt.fileName());
		}
	}

	it = module->getExtensionSourceFileIterator();
	while (it)
	{
		const char* fileName = module->getNextExtensionSourceFile(&it);
		addFile(model, fileName);
	}

	ensureCompleter();

	model->setSortRole(Role_CaseInsensitiveSort);
	model->sort(0);

	m_completer->setModel(model);
	m_completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
	m_completer->setCaseSensitivity(Qt::CaseInsensitive);
	m_completer->setWrapAround(false);
	m_completer->setCompletionPrefix(QString());

	m_completerRect = getLastCodeAssistCursorRect();
	updateCompleter(true);
}

bool
EditPrivate::keyPressSpace(Qt::KeyboardModifiers modifiers)
{
	if (!(modifiers & Qt::ControlModifier))
		return false;

	if (modifiers & Qt::ShiftModifier)
	{
		if (m_codeAssistTriggers & Edit::ArgumentTipOnCtrlShiftSpace)
			requestCodeAssist(CodeAssistKind_ArgumentTip, true);
	}
	else
	{
		if (m_codeAssistTriggers & Edit::AutoCompleteOnCtrlSpace)
			requestCodeAssist(CodeAssistKind_AutoComplete, true);
	}

	return true;
}

bool
EditPrivate::keyPressHome(Qt::KeyboardModifiers modifiers)
{
	Q_Q(Edit);

	if (modifiers & Qt::ControlModifier)
		return false;

	// don't select indent

	QTextCursor cursor = q->textCursor();
	int position = cursor.position();
	cursor.setPosition(position, QTextCursor::MoveAnchor);
	cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);

	bool isNextWord;

	if (cursor.position() == position)
	{
		isNextWord = true;
	}
	else
	{
		QString line = cursor.selectedText();
		isNextWord = !line.isEmpty() && line.at(0).isSpace() && !line.trimmed().isEmpty();
	}

	QTextCursor::MoveMode moveMode = (modifiers & Qt::ShiftModifier) ?
		QTextCursor::KeepAnchor :
		QTextCursor::MoveAnchor;

	cursor = q->textCursor();
	cursor.movePosition(QTextCursor::StartOfLine, moveMode);

	if (isNextWord)
		cursor.movePosition(QTextCursor::NextWord, moveMode);

	q->setTextCursor(cursor);
	return true;
}

bool
EditPrivate::keyPressTab(Qt::KeyboardModifiers modifiers)
{
	return false;
}

bool
EditPrivate::keyPressEnter(Qt::KeyboardModifiers modifiers)
{
	return false;
}


void
EditPrivate::timerEvent(QTimerEvent* e)
{
	if (e->timerId() != m_quickInfoTipTimer.timerId())
		return;

	m_quickInfoTipTimer.stop();
	requestCodeAssist(CodeAssistKind_QuickInfoTip, m_pendingCodeAssistPosition);
}

void
EditPrivate::onCursorPositionChanged()
{
	if (m_isCurrentLineHighlightingEnabled)
		highlightCurrentLine();

	switch (m_lastCodeAssistKind)
	{
	case CodeAssistKind_QuickInfoTip:
		hideCodeAssist();
		break;

	case CodeAssistKind_ArgumentTip:
		requestCodeAssist(CodeAssistKind_ArgumentTip);
		break;

	case CodeAssistKind_AutoCompleteList:
	case CodeAssistKind_ImportAutoCompleteList:
		if (isCompleterVisible())
			updateCompleter();
		break;
	}
}

void
EditPrivate::onCompleterActivated(const QString& completion)
{
	Q_Q(Edit);

    QTextCursor cursor = q->textCursor();
	int basePosition = getLastCodeAssistPosition();

	if (m_lastCodeAssistKind == CodeAssistKind_ImportAutoCompleteList)
	{
		QString quotedCompletion = '"' + completion + '"';
		cursor.setPosition(basePosition, QTextCursor::MoveAnchor);
		cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
		cursor.insertText(quotedCompletion);
	}
	else
	{
		cursor.select(QTextCursor::WordUnderCursor);

		int position = cursor.position();
		int anchor = cursor.anchor();

		if (anchor < basePosition)
			cursor.setPosition(basePosition, QTextCursor::MoveAnchor);

		if (position < basePosition)
			cursor.setPosition(basePosition, QTextCursor::KeepAnchor);

		cursor.insertText(completion);
	}

    q->setTextCursor(cursor);
}

void
EditPrivate::onCodeAssistReady()
{
	Q_Q(Edit);

	CodeAssistThread* thread = (CodeAssistThread*)sender();
	ASSERT(thread);

	if (thread != m_thread)
		return;

	CodeAssist* codeAssist = thread->getCodeAssist();
	if (!codeAssist)
	{
		if (thread->getCodeAssistKind() != CodeAssistKind_QuickInfoTip ||
			m_lastCodeAssistKind == CodeAssistKind_QuickInfoTip) // don't let failed quick-info to ruin existing code-assist
			hideCodeAssist();

		return;
	}

	m_lastCodeAssistKind = codeAssist->getCodeAssistKind();
	m_lastCodeAssistOffset = codeAssist->getOffset();
	m_lastCodeAssistPosition = -1;

	switch (m_lastCodeAssistKind)
	{
	case CodeAssistKind_QuickInfoTip:
		createQuickInfoTip(codeAssist->getModuleItem());
		break;

	case CodeAssistKind_ArgumentTip:
		createArgumentTip(codeAssist->getFunctionType(), codeAssist->getArgumentIdx());
		break;

	case CodeAssistKind_AutoCompleteList:
		createAutoCompleteList(codeAssist->getNamespace(), codeAssist->getNamespaceFlags());
		break;

	case CodeAssistKind_ImportAutoCompleteList:
		createImportAutoCompleteList(codeAssist->getModule());
		break;

	case CodeAssistKind_GotoDefinition:
		break;

	default:
		hideCodeAssist();
	}
}

void
EditPrivate::onThreadFinished()
{
	CodeAssistThread* thread = (CodeAssistThread*)sender();
	ASSERT(thread);

	if (thread == m_thread)
		m_thread = NULL;

	thread->deleteLater();
}

//..............................................................................

void
CompleterItemDelegate::paint(
	QPainter* painter,
	const QStyleOptionViewItem& option,
	const QModelIndex& index
	) const
{
	if (index.column() != EditPrivate::Column_Synopsis)
	{
		QStyledItemDelegate::paint(painter, option, index);
		return;
	}

	QStyleOptionViewItem altOption = option;
	altOption.palette.setColor(QPalette::Text, EditPrivate::Color_SynopsisColumnText);
	altOption.palette.setColor(QPalette::WindowText, EditPrivate::Color_SynopsisColumnText);
	QStyledItemDelegate::paint(painter, altOption, index);
}

//..............................................................................

} // namespace jnc
