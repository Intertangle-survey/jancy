#ifndef _MODULEPANE_H
#define _MODULEPANE_H

class MdiChild;

class ModulePane : public QTreeWidget
{
	Q_OBJECT

public:
	ModulePane(QWidget *parent);
	
	QSize sizeHint() const { return QSize(300, 50); }

	bool build(jnc::Module *module, MdiChild *document);
	void clear();

private slots:
	void onItemDoubleClicked(QTreeWidgetItem *treeItem, int column);

private:
	QTreeWidgetItem *insertItem(const QString &text,
		QTreeWidgetItem *parent = 0, void *data = 0);

	bool addItemAttributes(QTreeWidgetItem *parent, jnc::ModuleItem *item);

	void addNamespace(QTreeWidgetItem *parent, jnc::GlobalNamespace *globalNamespace);
	void addItem(QTreeWidgetItem *parent, jnc::ModuleItem *item);
	void addType(QTreeWidgetItem *parent, jnc::Type *type);
	void addTypedef(QTreeWidgetItem *parent, jnc::Typedef *typed);
	void addVariable(QTreeWidgetItem *parent, jnc::Variable *variable);
	void addEnumConst(QTreeWidgetItem *parent, jnc::EnumConst *member);
	void addValue(QTreeWidgetItem *parent, const char* name, jnc::Type *type, jnc::ModuleItem *item);
	void addFunction(QTreeWidgetItem *parent, jnc::Function *function);
	void addFunctionImpl(QTreeWidgetItem *parent, jnc::Function *function);
	void addProperty(QTreeWidgetItem *parent, jnc::Property *prop);
	void addEnumTypeMembers(QTreeWidgetItem *parent, jnc::EnumType *type);
	void addStructTypeMembers(QTreeWidgetItem *parent, jnc::StructType *type);
	
	void addStructField(QTreeWidgetItem *parent, jnc::StructField *field)
	{
		addValue (parent, field->getName (), field->getType (), field);
	}

	void addUnionTypeMembers(QTreeWidgetItem *parent, jnc::UnionType *type);
	void addClassTypeMembers(QTreeWidgetItem *parent, jnc::ClassType *type);

	MdiChild *document;
};

#endif