#...............................................................................
#
#  This file is part of the Jancy toolkit.
#
#  Jancy is distributed under the MIT license.
#  For details see accompanying license.txt file,
#  the public copy of which is also available at:
#  http://tibbo.com/downloads/archive/jancy/license.txt
#
#...............................................................................

#
# lib folder
#

set (
	LIB_H_LIST
	jnc_io_UsbDesc.h
	jnc_io_UsbEndpoint.h
	jnc_io_UsbInterface.h
	jnc_io_UsbDevice.h
	jnc_io_UsbLib.h
	)

set (
	LIB_CPP_LIST
	jnc_io_UsbDesc.cpp
	jnc_io_UsbEndpoint.cpp
	jnc_io_UsbInterface.cpp
	jnc_io_UsbDevice.cpp
	jnc_io_UsbLib.cpp
	)

source_group (
	lib
	FILES
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# jnc folder
#

set (
	JNC_LIST
	jnc/io_Usb.jnc
	)

source_group (
	jnc
	FILES
	${JNC_LIST}
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# pch folder
#

set (PCH_H     pch.h)
set (PCH_CPP   pch.cpp)

source_group (
	pch
	FILES
	${PCH_H}
	${PCH_CPP}
	)

#...............................................................................
#
# jnc_io_usb library
#

include_directories (
	${LIBUSB_INC_DIR}
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	)

link_directories (
	${LIBUSB_LIB_DIR}
	${AXL_LIB_DIR}
	)

add_library (
	jnc_io_usb
	SHARED
	${PCH_H}
	${PCH_CPP}
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	${JNC_LIST}
	)

axl_set_pch (
	jnc_io_usb
	${PCH_H}
	${PCH_CPP}
	)

set (CMAKE_SHARED_LIBRARY_PREFIX)

target_compile_definitions (
	jnc_io_usb
	PUBLIC
	-D_JNC_DYNAMIC_EXTENSION_LIB
	)

set_target_properties (
	jnc_io_usb
	PROPERTIES
	OUTPUT_NAME io_usb-${AXL_CPU}
	SUFFIX .bin
	)

target_link_libraries (
	jnc_io_usb
	jnc_api_dynamic
	axl_io
	axl_core
	)

if (UNIX AND NOT APPLE)
	target_link_libraries (
		jnc_io_usb
		rt
		)
endif ()

if (WIN32)
	target_link_libraries (
		jnc_io_usb
		libusb-1.0
		)
else ()
	target_link_libraries (
		jnc_io_usb
		usb-1.0
		)
endif ()

set_target_properties (
	jnc_io_usb
	PROPERTIES
	FOLDER extensions
	)

#...............................................................................
#
# io_usb.jncx archive
#

set (BIN_DIR ${JANCY_BIN_BASE_DIR}/${CONFIGURATION_SUFFIX})

add_custom_target (
	jnc_io_usb_jncx
	ALL
	COMMAND ${CMAKE_COMMAND} -E remove ${BIN_DIR}/io_usb.jncx
	COMMAND ${7Z_EXE} a -tzip -y ${BIN_DIR}/io_usb.jncx ${CMAKE_CURRENT_LIST_DIR}/jnc/*.jnc ${BIN_DIR}/io_usb-${AXL_CPU}.bin
	DEPENDS
	jnc_io_usb
	)

set_target_properties (
	jnc_io_usb_jncx
	PROPERTIES
	FOLDER extensions
	)

#...............................................................................