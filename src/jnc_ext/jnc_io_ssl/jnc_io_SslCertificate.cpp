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
#include "jnc_io_SslCertificate.h"
#include "jnc_io_SslLib.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_TYPE(
	SslCertEntry,
	"io.SslCertEntry",
	g_sslLibGuid,
	SslLibCacheSlot_SslCertEntry
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(SslCertEntry)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	SslCertName,
	"io.SslCertName",
	g_sslLibGuid,
	SslLibCacheSlot_SslCertName,
	SslCertName,
	&SslCertificate::markOpaqueGcRoots
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(SslCertName)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<SslCertName>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<SslCertName>)

	JNC_MAP_CONST_PROPERTY("m_entryTable", &SslCertName::getEntryTable)
	JNC_MAP_CONST_PROPERTY("m_oneLine", &SslCertName::getOneLine)

	JNC_MAP_FUNCTION("findEntry", &SslCertName::findEntry)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	SslCertificate,
	"io.SslCertificate",
	g_sslLibGuid,
	SslLibCacheSlot_SslCertificate,
	SslCertificate,
	&SslCertificate::markOpaqueGcRoots
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(SslCertificate)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<SslCertificate>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<SslCertificate>)

	JNC_MAP_CONST_PROPERTY("m_serialNumber", &SslCertificate::getSerialNumber)
	JNC_MAP_CONST_PROPERTY("m_validFromDate", &SslCertificate::getValidFromDate)
	JNC_MAP_CONST_PROPERTY("m_validToDate", &SslCertificate::getValidToDate)
	JNC_MAP_CONST_PROPERTY("m_subject", &SslCertificate::getSubject)
	JNC_MAP_CONST_PROPERTY("m_issuer", &SslCertificate::getIssuer)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

void
JNC_CDECL
SslCertName::markOpaqueGcRoots(jnc::GcHeap* gcHeap)
{
	size_t count = m_entryArray.getCount();
	for (size_t i = 0; i < count; i++)
		gcHeap->markDataPtr(m_entryArray[i]);

	gcHeap->markDataPtr(m_oneLinePtr);
}

DataPtr
JNC_CDECL
SslCertName::getEntryTable(
	SslCertName* self,
	size_t i
	)
{
	X509_NAME_ENTRY* entry = X509_NAME_get_entry(self->m_name, i);
	if (!entry)
		return g_nullDataPtr;

	size_t count = self->m_entryArray.getCount();
	if (i < count && self->m_entryArray[i].m_p)
		return self->m_entryArray[i];

	if (i >= count)
		self->m_entryArray.setCount(i + 1);

	self->m_entryArray[i] = createEntry(entry);
	return self->m_entryArray[i];
}

DataPtr
JNC_CDECL
SslCertName::getOneLine(SslCertName* self)
{
	if (self->m_oneLinePtr.m_p)
		return self->m_oneLinePtr;

    char* p = X509_NAME_oneline(self->m_name, NULL, 0);
	self->m_oneLinePtr = strDup(p);
	OPENSSL_free(p);

	return self->m_oneLinePtr;
}

DataPtr
JNC_CDECL
SslCertName::findEntry(
	SslCertName* self,
	int nid
	)
{
	int i = X509_NAME_get_index_by_NID(self->m_name, nid, -1);
	return i >= 0 ? getEntryTable(self, i) : g_nullDataPtr;
}

DataPtr
SslCertName::createEntry(X509_NAME_ENTRY* srcEntry)
{
	Runtime* runtime = getCurrentThreadRuntime();
	runtime->getGcHeap()->enterNoCollectRegion();

	DataPtr ptr = createData<SslCertEntry>(runtime);
	SslCertEntry* dstEntry = (SslCertEntry*)ptr.m_p;
	dstEntry->m_nid = OBJ_obj2nid(srcEntry->object);
	dstEntry->m_namePtr = strDup(cry::getAsn1ObjectString(srcEntry->object));
	dstEntry->m_valuePtr = strDup(cry::getAsn1StringString(srcEntry->value));

	runtime->getGcHeap()->leaveNoCollectRegion(false);
	return ptr;
}

//..............................................................................

SslCertificate*
SslCertificate::create(X509* cert)
{
	Runtime* runtime = getCurrentThreadRuntime();
	SslCertificate* self = createClass<SslCertificate>(runtime);
	self->m_cert = cert;
	return self;
}

void
JNC_CDECL
SslCertificate::markOpaqueGcRoots(jnc::GcHeap* gcHeap)
{
	gcHeap->markDataPtr(m_serialNumberPtr);
	gcHeap->markClassPtr(m_subject);
	gcHeap->markClassPtr(m_issuer);
}

DataPtr
JNC_CDECL
SslCertificate::getSerialNumber(SslCertificate* self)
{
	if (self->m_serialNumberPtr.m_p)
		return self->m_serialNumberPtr;

	cry::BigNum bigNum;
	bigNum.create();

	ASN1_INTEGER* serialNumber = X509_get_serialNumber(self->m_cert);
	ASN1_INTEGER_to_BN(serialNumber, bigNum);

	self->m_serialNumberPtr = strDup(bigNum.getHexString());
	return self->m_serialNumberPtr;
}

uint64_t
JNC_CDECL
SslCertificate::getValidFromDate()
{
	return m_validFromDate ?
		m_validFromDate :
		m_validFromDate = getTimestamp(X509_get_notBefore(m_cert));
}

uint64_t
JNC_CDECL
SslCertificate::getValidToDate()
{
	return m_validFromDate ?
		m_validFromDate :
		m_validFromDate = getTimestamp(X509_get_notAfter(m_cert));
}

SslCertName*
JNC_CDECL
SslCertificate::getSubject()
{
	return m_subject ?
		m_subject :
		m_subject = createSslCertName(X509_get_subject_name(m_cert));
}

SslCertName*
JNC_CDECL
SslCertificate::getIssuer()
{
	return m_issuer ?
		m_issuer :
		m_issuer = createSslCertName(X509_get_issuer_name(m_cert));
}

SslCertName*
SslCertificate::createSslCertName(X509_NAME* srcName)
{
	Runtime* runtime = getCurrentThreadRuntime();
	SslCertName* dstName = createClass<SslCertName>(runtime);
	dstName->m_name = srcName;
	dstName->m_entryCount = X509_NAME_entry_count(srcName);
	return dstName;
}

uint64_t
SslCertificate::getTimestamp(const ASN1_TIME* time)
{
	int days;
	int secs;

	ASN1_TIME* epoch = ASN1_TIME_set(NULL, 0);
	ASN1_TIME_diff(&days, &secs, epoch, time);
	ASN1_TIME_free(epoch);

	ASSERT(days >= 0 && secs >= 0);

	time_t unixTime = days * 24 * 60 * 60 + secs;
	return (unixTime + AXL_SYS_EPOCH_DIFF) * 10000000;
}

//..............................................................................

DataPtr
JNC_CDECL
getSslNidShortName(uint_t nid)
{
	return createForeignStringPtr(OBJ_nid2sn(nid), false);
}

DataPtr
JNC_CDECL
getSslNidLongName(uint_t nid)
{
	return createForeignStringPtr(OBJ_nid2ln(nid), false);
}

//..............................................................................

} // namespace io
} // namespace jnc
