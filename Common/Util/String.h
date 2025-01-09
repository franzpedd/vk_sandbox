#pragma once

namespace Cosmos
{
	// safely copies a string from a destination into a source
	inline void strcpy_safe(char* dest, char const* src, size_t maxLen)
	{
		size_t nCount = maxLen;
		char* pstrDest = dest;
		const char* pstrSource = src;

		while (0 < nCount && 0 != (*pstrDest++ = *pstrSource++)) {
			nCount--;
		}

		if (maxLen > 0) {
			pstrDest[-1] = 0;
		}
	}
}