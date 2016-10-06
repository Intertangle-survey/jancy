#include "pch.h"
#include "jnc_ct_OperatorMgr.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_Parser.llk.h"

namespace jnc {
namespace ct {

//.............................................................................

llvm::CallInst*
OperatorMgr::memSet (
	const Value& value,
	char c,
	size_t size,
	size_t alignment
	)
{
	Value ptrValue;
	m_module->m_llvmIrBuilder.createBitCast (value, m_module->m_typeMgr.getStdType (StdType_BytePtr), &ptrValue);

	Value argValueArray [5] = 
	{
		ptrValue,
		Value (c, m_module->m_typeMgr.getPrimitiveType (TypeKind_Int8)),
		Value (size, m_module->m_typeMgr.getPrimitiveType (TypeKind_Int32)),
		Value (alignment, m_module->m_typeMgr.getPrimitiveType (TypeKind_Int32)),
		Value ((int64_t) false, m_module->m_typeMgr.getPrimitiveType (TypeKind_Bool)),
	};

	Function* llvmMemset = m_module->m_functionMgr.getStdFunction (StdFunc_LlvmMemset);
	return m_module->m_llvmIrBuilder.createCall (
		llvmMemset,
		llvmMemset->getType (),
		argValueArray,
		countof (argValueArray),
		NULL
		);
}

void
OperatorMgr::zeroInitialize (const Value& value)
{
	ASSERT (value.getType ()->getTypeKindFlags () & TypeKindFlag_DataPtr);
	Type* type = ((DataPtrType*) value.getType ())->getTargetType ();

	if (type->getSize () <= TypeSizeLimit_StoreSize)
		m_module->m_llvmIrBuilder.createStore (type->getZeroValue (), value);
	else
		memSet (value, 0, type->getSize (), type->getAlignment ());
}

bool
OperatorMgr::construct (
	const Value& rawOpValue,
	sl::BoxList <Value>* argList
	)
{
	Type* type = rawOpValue.getType ();
	TypeKind ptrTypeKind = type->getTypeKind ();

	switch (ptrTypeKind)
	{
	case TypeKind_DataRef:
	case TypeKind_DataPtr:
		type = ((DataPtrType*) type)->getTargetType ();
		break;

	case TypeKind_ClassPtr:
	case TypeKind_ClassRef:
		type = ((ClassPtrType*) type)->getTargetType ();
		break;

	default:
		err::setFormatStringError ("'%s' is not a pointer or reference", type->getTypeString ().sz ());
		return false;
	}

	TypeKind typeKind = type->getTypeKind ();

	Function* constructor = NULL;

	switch (typeKind)
	{
	case TypeKind_Struct:
	case TypeKind_Union:
		constructor = ((DerivableType*) type)->getConstructor ();
		break;

	case TypeKind_Class:
		constructor = ((ClassType*) type)->getConstructor ();
		break;
	}

	if (!constructor)
	{
		if (argList && !argList->isEmpty ())
		{
			err::setFormatStringError ("'%s' has no constructor", type->getTypeString ().sz ());
			return false;
		}

		return true;
	}

	DerivableType* derivableType = (DerivableType*) type;
	if (constructor->getAccessKind () != AccessKind_Public &&
		m_module->m_namespaceMgr.getAccessKind (derivableType) == AccessKind_Public)
	{
		err::setFormatStringError ("'%s.construct' is protected", derivableType->getQualifiedName ().sz ());
		return false;
	}

	sl::BoxList <Value> emptyArgList;
	if (!argList)
		argList = &emptyArgList;

	Value opValue = rawOpValue;
	if (ptrTypeKind == TypeKind_DataRef || ptrTypeKind == TypeKind_ClassRef)
	{
		bool result = unaryOperator (UnOpKind_Addr, &opValue);
		if (!result)
			return false;
	}

	argList->insertHead (opValue);

	return callOperator (constructor, argList);
}

bool
OperatorMgr::parseInitializer (
	const Value& rawValue,
	Unit* unit,
	const sl::ConstBoxList <Token>& constructorTokenList,
	const sl::ConstBoxList <Token>& initializerTokenList
	)
{
	if (unit)
		m_module->m_unitMgr.setCurrentUnit (unit);

	bool result;

	Value value = rawValue;

	if (rawValue.getType ()->getTypeKind () == TypeKind_DataRef)
		value.overrideType (((DataPtrType*) rawValue.getType ())->getUnConstPtrType ());
	else if (rawValue.getType ()->getTypeKind () == TypeKind_ClassRef)
		value.overrideType (((ClassPtrType*) rawValue.getType ())->getUnConstPtrType ());

	sl::BoxList <Value> argList;
	if (!constructorTokenList.isEmpty ())
	{
		Parser parser (m_module);
		parser.m_stage = Parser::StageKind_Pass2;

		result = parser.parseTokenList (SymbolKind_expression_or_empty_list_save_list, constructorTokenList);
		if (!result)
			return false;

		argList.takeOver (&parser.m_expressionValueList);
	}

	result = construct (value, &argList);
	if (!result)
		return false;

	if (!initializerTokenList.isEmpty ())
	{
		Parser parser (m_module);
		parser.m_stage = Parser::StageKind_Pass2;

		if (initializerTokenList.getHead ()->m_token == '{')
		{
			parser.m_curlyInitializerTargetValue = value;
			result = parser.parseTokenList (SymbolKind_curly_initializer, initializerTokenList);
			if (!result)
				return false;
		}
		else
		{
			result =
				parser.parseTokenList (SymbolKind_expression_save_value, initializerTokenList) &&
				m_module->m_operatorMgr.binaryOperator (BinOpKind_Assign, value, parser.m_expressionValue);

			if (!result)
				return false;
		}
	}

	return true;
}

bool
OperatorMgr::parseExpression (
	Unit* unit,
	const sl::ConstBoxList <Token>& expressionTokenList,
	uint_t flags,
	Value* resultValue
	)
{
	if (unit)
		m_module->m_unitMgr.setCurrentUnit (unit);

	Parser parser (m_module);
	parser.m_stage = Parser::StageKind_Pass2;
	parser.m_flags |= flags;

	bool result = parser.parseTokenList (SymbolKind_expression_save_value, expressionTokenList);
	if (!result)
		return false;

	*resultValue = parser.m_expressionValue;
	return true;
}

bool
OperatorMgr::parseConstExpression (
	Unit* unit,
	const sl::ConstBoxList <Token>& expressionTokenList,
	Value* resultValue
	)
{
	bool result = parseExpression (unit, expressionTokenList, Parser::Flag_ConstExpression, resultValue);
	if (!result)
		return false;

	ASSERT (resultValue->getValueKind () == ValueKind_Const);
	return true;
}

bool
OperatorMgr::parseConstIntegerExpression (
	Unit* unit,
	const sl::ConstBoxList <Token>& expressionTokenList,
	int64_t* integer
	)
{
	Value value;
	bool result = parseConstExpression (unit, expressionTokenList, &value);
	if (!result)
		return false;

	if (!(value.getType ()->getTypeKindFlags () & TypeKindFlag_Integer))
	{
		err::setFormatStringError ("expression is not integer constant");
		return false;
	}

	*integer = 0;
	memcpy (integer, value.getConstData (), value.getType ()->getSize ());
	return true;
}

bool
OperatorMgr::parseAutoSizeArrayInitializer (
	const sl::ConstBoxList <Token>& initializerTokenList,
	size_t* elementCount
	)
{
	int firstToken = initializerTokenList.getHead ()->m_token;
	switch (firstToken)
	{
	case TokenKind_Literal:
	case TokenKind_HexLiteral:
		*elementCount = parseAutoSizeArrayLiteralInitializer (initializerTokenList);
		break;

	case '{':
		*elementCount = parseAutoSizeArrayCurlyInitializer (initializerTokenList);
		break;

	default:
		err::setFormatStringError ("invalid initializer for auto-size-array");
		return false;
	}

	return true;
}

// it's both more efficient AND easier to parse these by hand

size_t
OperatorMgr::parseAutoSizeArrayLiteralInitializer (const sl::ConstBoxList <Token>& initializerTokenList)
{
	size_t elementCount = 0;

	sl::BoxIterator <Token> token = initializerTokenList.getHead ();
	for (; token; token++)
	{
		switch (token->m_token)
		{
		case TokenKind_Literal:
			elementCount += token->m_data.m_string.getLength ();
			break;

		case TokenKind_HexLiteral:
			elementCount += token->m_data.m_binData.getCount ();
			break;
		}
	}

	if (initializerTokenList.getTail ()->m_token == TokenKind_Literal)
		elementCount++;

	return elementCount;
}

size_t
OperatorMgr::parseAutoSizeArrayCurlyInitializer (const sl::ConstBoxList <Token>& initializerTokenList)
{
	intptr_t level = 0;
	size_t elementCount = 0;

	bool isElement = false;

	sl::BoxIterator <Token> token = initializerTokenList.getHead ();
	for (; token; token++)
	{
		switch (token->m_token)
		{
		case '{':
			if (level == 1)
				isElement = true;

			level++;
			break;

		case '}':
			if (level == 1 && isElement)
			{
				elementCount++;
				isElement = false;
			}

			level--;
			break;

		case ',':
			if (level == 1)
			{
				elementCount++;
				isElement = false;
			}

			break;

		default:
			if (level == 1)
				isElement = true;
		}
	}

	return elementCount;
}

bool
OperatorMgr::evaluateAlias (
	ModuleItemDecl* decl,
	const Value& thisValue,
	const sl::ConstBoxList <Token> tokenList,
	Value* resultValue
	)
{
	Value prevThisValue = m_module->m_functionMgr.overrideThisValue (thisValue);
	bool result = evaluateAlias (decl, tokenList, resultValue);
	if (!result)
		return false;

	m_module->m_functionMgr.overrideThisValue (prevThisValue);
	return true;
}

bool
OperatorMgr::evaluateAlias (
	ModuleItemDecl* decl,
	const sl::ConstBoxList <Token> tokenList,
	Value* resultValue
	)
{
	Parser parser (m_module);
	parser.m_stage = Parser::StageKind_Pass2;

	m_module->m_namespaceMgr.openNamespace (decl->getParentNamespace ());
	m_module->m_namespaceMgr.lockSourcePos ();

	bool result = parser.parseTokenList (SymbolKind_expression_save_value, tokenList);
	if (!result)
		return false;

	m_module->m_namespaceMgr.unlockSourcePos ();
	m_module->m_namespaceMgr.closeNamespace ();

	*resultValue = parser.m_expressionValue;
	return true;
}

bool
OperatorMgr::gcHeapAllocate (
	Type* type,
	const Value& rawElementCountValue,
	Value* resultValue
	)
{
	bool result;

	Value typeValue (&type, m_module->m_typeMgr.getStdType (StdType_BytePtr));

	Function* allocate;
	sl::BoxList <Value> allocateArgValueList;
	allocateArgValueList.insertTail (typeValue);

	Value ptrValue;

	if (type->getTypeKind () == TypeKind_Class)
	{
		if (type->getFlags () & (ClassTypeFlag_HasAbstractMethods | ClassTypeFlag_OpaqueNonCreatable))
		{
			err::setFormatStringError ("cannot instantiate '%s'", type->getTypeString ().sz ());
			return false;
		}

		allocate = m_module->m_functionMgr.getStdFunction (StdFunc_AllocateClass);
	}
	else if (!rawElementCountValue)
	{
		allocate = m_module->m_functionMgr.getStdFunction (StdFunc_AllocateData);
	}
	else
	{
		allocate = m_module->m_functionMgr.getStdFunction (StdFunc_AllocateArray);

		Value countValue;
		result = castOperator (rawElementCountValue, TypeKind_SizeT, &countValue);
		if (!result)
			return false;

		allocateArgValueList.insertTail (countValue);
	}

	m_module->m_operatorMgr.callOperator (
		allocate,
		&allocateArgValueList,
		&ptrValue
		);

	if (type->getTypeKind () == TypeKind_Class)
		m_module->m_llvmIrBuilder.createBitCast (ptrValue, ((ClassType*) type)->getClassPtrType (), resultValue);
	else
		resultValue->overrideType (ptrValue, type->getDataPtrType ());

	return true;
}

bool
OperatorMgr::newOperator (
	Type* type,
	const Value& rawElementCountValue,
	sl::BoxList <Value>* argValueList,
	Value* resultValue
	)
{
	bool result;

	Value ptrValue;
	result = gcHeapAllocate (type, rawElementCountValue, &ptrValue);
	if (!result)
		return false;

	result = construct (ptrValue, argValueList);
	if (!result)
		return false;

	*resultValue = ptrValue;
	return true;
}

//.............................................................................

} // namespace ct
} // namespace jnc
