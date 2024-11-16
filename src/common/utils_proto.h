/*
 *  The contents of this file are subject to the Initial
 *  Developer's Public License Version 1.0 (the "License");
 *  you may not use this file except in compliance with the
 *  License. You may obtain a copy of the License at
 *  http://www.ibphoenix.com/main.nfs?a=ibphoenix&page=ibp_idpl.
 *
 *  Software distributed under the License is distributed AS IS,
 *  WITHOUT WARRANTY OF ANY KIND, either express or implied.
 *  See the License for the specific language governing rights
 *  and limitations under the License.
 *
 *  The Original Code was created by Claudio Valderrama on 25-Dec-2003
 *  for the Firebird Open Source RDBMS project.
 *
 *  Copyright (c) 2003 Claudio Valderrama
 *  and all contributors signed below.
 *
 *  All Rights Reserved.
 *  Contributor(s): ______________________________________.
 *
 *  Nickolay Samofatov <nickolay@broadviewsoftware.com>
 */


// =====================================
// Utility functions

#ifndef INCLUDE_UTILS_PROTO_H
#define INCLUDE_UTILS_PROTO_H

#include <cctype>
#include <string.h>
#include <type_traits>

#include "../common/classes/fb_string.h"
#include "../common/classes/array.h"
#include "iberror.h"
#include "firebird/Interface.h"
#include "memory_routines.h"

#ifdef SFIO
#include <stdio.h>
#endif

namespace fb_utils
{
	char* copy_terminate(char* dest, const char* src, size_t bufsize);
	char* exact_name(char* const name);
	inline void exact_name(Firebird::string& str)
	{
		str.rtrim();
	}
	char* exact_name_limit(char* const name, size_t bufsize);
	bool implicit_domain(const char* domain_name);
	bool implicit_integrity(const char* integ_name);
	bool implicit_pk(const char* pk_name);
	int name_length(const TEXT* const name);
	int name_length_limit(const TEXT* const name, size_t bufsize);
	bool readenv(const char* env_name, Firebird::string& env_value);
	bool readenv(const char* env_name, Firebird::PathName& env_value);
	bool setenv(const char* name, const char* value, bool overwrite);
	int snprintf(char* buffer, size_t count, const char* format...);
	char* cleanup_passwd(char* arg);
	inline char* get_passwd(char* arg)
	{
		return cleanup_passwd(arg);
	}
	typedef char* arg_string;

	// Warning: Only wrappers:

	// ********************
	// s t r i c m p
	// ********************
	// Abstraction of incompatible routine names
	// for case insensitive comparison.
	inline int stricmp(const char* a, const char* b)
	{
#if defined(HAVE_STRCASECMP)
		return ::strcasecmp(a, b);
#elif defined(HAVE_STRICMP)
		return ::stricmp(a, b);
#else
#error dont know how to compare strings case insensitive on this system
#endif
	}


	// ********************
	// s t r n i c m p
	// ********************
	// Abstraction of incompatible routine names
	// for counted length and case insensitive comparison.
	inline int strnicmp(const char* a, const char* b, size_t count)
	{
#if defined(HAVE_STRNCASECMP)
		return ::strncasecmp(a, b, count);
#elif defined(HAVE_STRNICMP)
		return ::strnicmp(a, b, count);
#else
#error dont know how to compare counted length strings case insensitive on this system
#endif
	}

	// std::isspace behavior is undefined with char and signed char.
	// https://en.cppreference.com/w/cpp/string/byte/isspace
	static inline int isspace(const char c)
	{
		return std::isspace((int)(UCHAR)c);
	}

#ifdef WIN_NT
	bool prefix_kernel_object_name(char* name, size_t bufsize);
	bool isGlobalKernelPrefix();
	bool private_kernel_object_name(char* name, size_t bufsize);
	bool privateNameSpaceReady();
#endif

	// Compare the absolute value of two SINT64 numbers.
	// Return 0 if they are equal, <0 if n1 < n2 and >0 if n1 > n2.
	inline int abs64Compare(SINT64 n1, SINT64 n2)
	{
#ifndef FB_INT64_COMPARE_FAILED
#define FB_INT64_COMPARE_FAILED 1
#endif

#if FB_INT64_COMPARE_FAILED
		// avoid compiler bug when comparing minimum INT64
		const SINT64 minInt64 = 0x8000000000000000;
		if (n1 == minInt64)
			return n2 == minInt64 ? 0 : 2;
		if (n2 == minInt64)
			return -2;
#endif

		n1 = n1 > 0 ? -n1 : n1;
		n2 = n2 > 0 ? -n2 : n2;
		return n1 == n2 ? 0 : n1 < n2 ? 1 : -1;
	}

	Firebird::PathName get_process_name();
	SLONG genUniqueId();
	void getCwd(Firebird::PathName& pn);

	void inline initStatusTo(ISC_STATUS* status, ISC_STATUS to)
	{
		status[0] = isc_arg_gds;
		status[1] = to;
		status[2] = isc_arg_end;
	}

	void inline init_status(ISC_STATUS* status)
	{
		initStatusTo(status, FB_SUCCESS);
	}

	void inline statusBadAlloc(ISC_STATUS* status)
	{
		initStatusTo(status, isc_virmemexh);
	}

	void inline statusUnknown(ISC_STATUS* status)
	{
		initStatusTo(status, isc_exception_sigill);		// Any better ideas? New error code?
	}

	void inline init_status(Firebird::CheckStatusWrapper* status)
	{
		status->init();
	}

	unsigned int copyStatus(ISC_STATUS* const to, const unsigned int space,
							const ISC_STATUS* const from, const unsigned int count) noexcept;
	void copyStatus(Firebird::CheckStatusWrapper* to, const Firebird::IStatus* from) noexcept;
	unsigned int mergeStatus(ISC_STATUS* const to, unsigned int space, const Firebird::IStatus* from) noexcept;
	void setIStatus(Firebird::CheckStatusWrapper* to, const ISC_STATUS* from) noexcept;
	unsigned int statusLength(const ISC_STATUS* const status) noexcept;
	unsigned int subStatus(const ISC_STATUS* in, unsigned int cin,
						   const ISC_STATUS* sub, unsigned int csub) noexcept;
	bool cmpStatus(unsigned int len, const ISC_STATUS* a, const ISC_STATUS* b) noexcept;
	const ISC_STATUS* nextCode(const ISC_STATUS* v) noexcept;

	inline unsigned nextArg(const ISC_STATUS v) noexcept
	{
		return v == isc_arg_cstring ? 3 : 2;
	}

	inline bool isStr(const ISC_STATUS v) noexcept
	{
		switch (v)
		{
		case isc_arg_cstring:
		case isc_arg_string:
		case isc_arg_interpreted:
		case isc_arg_sql_state:
			return true;
		}

		return false;
	}

	// Check does vector contain particular code or not
	bool containsErrorCode(const ISC_STATUS* v, ISC_STATUS code);

	enum FetchPassResult {
		FETCH_PASS_OK,
		FETCH_PASS_FILE_OPEN_ERROR,
		FETCH_PASS_FILE_READ_ERROR,
		FETCH_PASS_FILE_EMPTY
	};
	FetchPassResult fetchPassword(const Firebird::PathName& name, const char*& password);

	// Returns current value of performance counter
	SINT64 query_performance_counter();

	// Returns frequency of performance counter in Hz
	SINT64 query_performance_frequency();

	void get_process_times(SINT64 &userTime, SINT64 &sysTime);

	void exactNumericToStr(SINT64 value, int scale, Firebird::string& target, bool append = false);

	// Returns true if called from firebird build process (appr. environment is set)
	bool bootBuild();

	// Add appropriate file prefix.
	Firebird::PathName getPrefix(unsigned prefType, const char* name);

	// moves DB path information (from limbo transaction) to another buffer
	void getDbPathInfo(unsigned int& itemsLength, const unsigned char*& items,
		unsigned int& bufferLength, unsigned char*& buffer,
		Firebird::Array<unsigned char>& newItemsBuffer, const Firebird::PathName& dbpath);

	// returns true if passed info items work with running svc thread
	bool isRunningCheck(const UCHAR* items, unsigned int length);

	// converts bytes to BASE64 representation
	void base64(Firebird::string& b64, const Firebird::UCharBuffer& bin);

	// generate random string in BASE64 representation
	void random64(Firebird::string& randomValue, FB_SIZE_T length);

	void logAndDie(const char* text);

	// On incorrect sqlType returns dsc_unknown
	UCHAR sqlTypeToDscType(SSHORT sqlType);

	// Returns next offset value
	unsigned sqlTypeToDsc(unsigned prevOffset, unsigned sqlType, unsigned sqlLength,
		unsigned* dtype, unsigned* len, unsigned* offset, unsigned* nullOffset);

	bool inline isNetworkError(ISC_STATUS code)
	{
		return code == isc_network_error ||
			code == isc_net_write_err ||
			code == isc_net_read_err ||
			code == isc_lost_db_connection;
	}

	// Uppercase/strip string according to login rules
	const char* dpbItemUpper(const char* s, FB_SIZE_T l, Firebird::string& buf);

	// Uppercase/strip string according to login rules
	template <typename STR>
	void dpbItemUpper(STR& name)
	{
		Firebird::string buf;
		const char* up = dpbItemUpper(name.c_str(), name.length(), buf);
		if (up)
			name = up;
	}

	// Frequently used actions with clumplets
	bool isBpbSegmented(unsigned parLength, const unsigned char* par);


	// Workaround, to be removed with C++ 23
	template <typename... T>
	constexpr bool fb_always_false_v = false;

	// Put integer value into info buffer
	template<typename T>
	inline unsigned char* putInfoItemInt(const unsigned char item, T value,
		unsigned char* ptr, const unsigned char* end)
	{
		static_assert(std::is_integral_v<T>, "Integral type expected");

		constexpr auto len = sizeof(T);

		if (ptr + len + 1 + 2 > end)
		{
			if (ptr < end)
			{
				*ptr++ = isc_info_truncated;
				if (ptr < end)
					*ptr++ = isc_info_end;
			}
			return nullptr;
		}

		*ptr++ = item;
		*ptr++ = len;
		*ptr++ = 0;

		if constexpr (len == sizeof(SINT64))
			put_vax_int64(ptr, value);
		else if constexpr (len == sizeof(SLONG))
			put_vax_long(ptr, value);
		else if constexpr (len == sizeof(SSHORT))
			put_vax_short(ptr, value);
		else if constexpr (len == sizeof(char))
			*ptr = value;
		else
			static_assert(fb_always_false_v<T>, "unknown data type");

		ptr += len;
		return ptr;
	}


	// RAII to call fb_shutdown() in utilities
	class FbShutdown
	{
	public:
		FbShutdown(int r)
			: reason(r)
		{ }

		~FbShutdown();

	private:
		int reason;
	};
} // namespace fb_utils

#endif // INCLUDE_UTILS_PROTO_H
