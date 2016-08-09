#include "pch.h"
#include "jnc_ct_DataPtrType.h"
#include "jnc_ct_Module.h"
#include "jnc_rt_Runtime.h"

namespace jnc {
namespace ct {

//.............................................................................

DataPtrType::DataPtrType ()
{
	m_typeKind = TypeKind_DataPtr;
	m_ptrTypeKind = DataPtrTypeKind_Normal;
	m_targetType = NULL;
	m_anchorNamespace = NULL;
	m_size = sizeof (DataPtr);
}

bool
DataPtrType::isConstPtrType ()
{
	return 
		(m_flags & PtrTypeFlag_Const) != 0 || 
		(m_flags & PtrTypeFlag_ReadOnly) != 0 && 
		m_module->m_namespaceMgr.getAccessKind (m_anchorNamespace) == AccessKind_Public;
}

sl::String
DataPtrType::createSignature (
	Type* baseType,
	TypeKind typeKind,
	DataPtrTypeKind ptrTypeKind,
	uint_t flags
	)
{
	sl::String signature = typeKind == TypeKind_DataRef ? "RD" : "PD";

	switch (ptrTypeKind)
	{
	case DataPtrTypeKind_Lean:
		signature += 'l';
		break;

	case DataPtrTypeKind_Thin:
		signature += 't';
		break;
	}

	signature += getPtrTypeFlagSignature (flags);
	signature += baseType->getSignature ();
	return signature;
}

void
DataPtrType::prepareTypeString ()
{
	m_typeString = m_targetType->getTypeString ();

	if (m_flags & PtrTypeFlag__AllMask)
	{
		m_typeString += ' ';
		m_typeString += getPtrTypeFlagString (m_flags);
	}

	if (m_ptrTypeKind != DataPtrTypeKind_Normal)
	{
		m_typeString += ' ';
		m_typeString += getDataPtrTypeKindString (m_ptrTypeKind);
	}

	m_typeString += m_typeKind == TypeKind_DataRef ? "&" : "*";
}

void
DataPtrType::prepareLlvmType ()
{
	m_llvmType = 
		m_ptrTypeKind == DataPtrTypeKind_Normal ? m_module->m_typeMgr.getStdType (StdType_DataPtrStruct)->getLlvmType () : 
		m_targetType->getTypeKind () != TypeKind_Void ? llvm::PointerType::get (m_targetType->getLlvmType (), 0) :
		m_module->m_typeMgr.getStdType (StdType_BytePtr)->getLlvmType ();
}

void
DataPtrType::prepareLlvmDiType ()
{
	m_llvmDiType = 
		m_ptrTypeKind == DataPtrTypeKind_Normal ? m_module->m_typeMgr.getStdType (StdType_DataPtrStruct)->getLlvmDiType () :
		m_targetType->getTypeKind () != TypeKind_Void ? m_module->m_llvmDiBuilder.createPointerType (m_targetType) :
		m_module->m_typeMgr.getStdType (StdType_BytePtr)->getLlvmDiType ();
}

void
DataPtrType::markGcRoots (
	const void* p,
	rt::GcHeap* gcHeap
	)
{
	ASSERT (m_ptrTypeKind == DataPtrTypeKind_Normal);

	DataPtr* ptr = (DataPtr*) p;		
	if (!ptr->m_validator)
		return;

	gcHeap->weakMark (ptr->m_validator->m_validatorBox);
	gcHeap->markData (ptr->m_validator->m_targetBox);
}

//.............................................................................

} // namespace ct
} // namespace jnc