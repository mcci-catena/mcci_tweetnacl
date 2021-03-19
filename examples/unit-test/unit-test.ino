/*

Module:	unit-test.ino

Function:
	Unit testing for mcci_tweetnacl

Copyright and License:
	This file copyright (C) 2021 by

		MCCI Corporation
		3520 Krums Corners Road
		Ithaca, NY  14850

	See accompanying LICENSE file for copyright and license information.

Author:
	Terry Moore, MCCI Corporation	March 2021

*/

#include "mcci_tweetnacl.h"
#include "mcci_tweetnacl_hash.h"
#include <ArduinoUnit.h>
#include <stdarg.h>
#include <mcciadk_baselib.h>

/****************************************************************************\
|
|   Helpers
|
\****************************************************************************/

// create a version number for comparison
constexpr std::uint32_t
makeVersion(
    std::uint8_t major, std::uint8_t minor, std::uint8_t patch, std::uint8_t local = 0
    )
    {
    return ((std::uint32_t)major << 24u) | ((std::uint32_t)minor << 16u) | ((std::uint32_t)patch << 8u) | (std::uint32_t)local;
    }

// extract major number from version
constexpr std::uint8_t
getMajor(std::uint32_t v)
    {
    return std::uint8_t(v >> 24u);
    }

// extract minor number from version
constexpr std::uint8_t
getMinor(std::uint32_t v)
    {
    return std::uint8_t(v >> 16u);
    }

// extract patch number from version
constexpr std::uint8_t
getPatch(std::uint32_t v)
    {
    return std::uint8_t(v >> 8u);
    }

// extract local number from version
constexpr std::uint8_t
getLocal(std::uint32_t v)
    {
    return std::uint8_t(v);
    }

/****************************************************************************\
|
|   App version
|
\****************************************************************************/

constexpr std::uint32_t kAppVersion = makeVersion(0,1,0,0);

/****************************************************************************\
|
|   Setup
|
\****************************************************************************/

void setup()
    {
    setup_platform();
    setup_printSignOn();
    }

void setup_platform()
    {
    while (!Serial)
	    /* wait for USB attach */
	    yield();
     }

static constexpr const char *filebasename(const char *s)
    {
    const char *pName = s;

    for (auto p = s; *p != '\0'; ++p)
        {
        if (*p == '/' || *p == '\\')
            pName = p + 1;
        }
    return pName;
    }

void 
safePrintf(
	const char *fmt, 
	...
	)
	{
	if (! Serial.dtr())
		return;

	char buf[128];
	va_list ap;

	va_start(ap, fmt);
	(void) vsnprintf(buf, sizeof(buf) - 1, fmt, ap);
	va_end(ap);

	// in case we overflowed:
	buf[sizeof(buf) - 1] = '\0';
	if (Serial.dtr()) Serial.print(buf);
	}

void setup_printSignOn()
    {
    static const char dashes[] = "------------------------------------";

    safePrintf("\n%s%s\n", dashes, dashes);

    safePrintf("This is %s v%d.%d.%d.%d.\n",
        filebasename(__FILE__),
        getMajor(kAppVersion), getMinor(kAppVersion), getPatch(kAppVersion), getLocal(kAppVersion)
        );

    safePrintf("%s%s\n" "\n", dashes, dashes);
    }


/****************************************************************************\
|
|   Loop
|
\****************************************************************************/

void loop()
    {
    Test::run();
    }

/****************************************************************************\
|
|   Test hash
|
\****************************************************************************/

test(10_sha)
	{
	static const uint8_t input[] = "I am a duck";
	mcci_tweetnacl_sha512_t output;
	static const mcci_tweetnacl_sha512_t expected = 
		{
		0xb3, 0xb5, 0xc6, 0xb4, 0xb3, 0x5c, 0x40, 0x93, 0xf6, 0x5f, 0xc6, 0x39, 0xa4, 0x06, 0x39, 0x35, 0x47, 0xca, 0x60, 0x04, 0x62, 0xe2, 0x5b, 0x83, 0x69, 0xd5, 0xbd, 0x76, 0x54, 0xf5, 0xd9, 0xe7, 0x6c, 0xf8, 0xc1, 0x08, 0xa0, 0xcb, 0xb7, 0x69, 0x63, 0x41, 0x7f, 0xe6, 0xa5, 0x3f, 0x6b, 0x68, 0x1a, 0x3b, 0xda, 0xdc, 0x65, 0x8d, 0x3e, 0x80, 0x46, 0x97, 0x68, 0xef, 0x02, 0x6c, 0x59, 0x00,
		};

	mcci_tweetnacl_hash_sha512(&output, input, sizeof(input) - 1);

	safePrintf("sha output:\n");
	for (auto v: output.bytes)
		{
		safePrintf("%02x ", v);
		}
	safePrintf("\n");
	assertEqual(memcmp(&output, &expected, sizeof(output)), 0, "hash didn't match with memcpy");
	assertTrue(mcci_tweetnacl_verify_64(output.bytes, expected.bytes), "verify_64 didn't match");
	}

test(11_sha)
	{
	static const uint8_t input[4096] = { 0 };
	mcci_tweetnacl_sha512_t output;
	static const mcci_tweetnacl_sha512_t expected = 
		{
		0x2d, 0x23, 0x91, 0x3d, 0x37, 0x59, 0xef, 0x01, 0x70, 0x4a, 0x86, 0xb4, 0xbe, 0xe3, 0xac, 0x8a, 0x29, 0x00, 0x23, 0x13, 0xec, 0xc9, 0x8a, 0x74, 0x24, 0x42, 0x5a, 0x78, 0x17, 0x0f, 0x21, 0x95, 0x77, 0x82, 0x2f, 0xd7, 0x7e, 0x4a, 0xe9, 0x63, 0x13, 0x54, 0x76, 0x96, 0xad, 0x7d, 0x59, 0x49, 0xb5, 0x8e, 0x12, 0xd5, 0x06, 0x3e, 0xf2, 0xee, 0x06, 0x3b, 0x59, 0x57, 0x40, 0xa3, 0xa1, 0x2d,
		};

	auto tStart = millis();
	mcci_tweetnacl_hash_sha512(&output, input, sizeof(input));
	auto tElapsed = millis() - tStart;

	safePrintf("Elapsed time for 4096 bytes: %u ms\n", tElapsed);
//	safePrintf("sha output:\n");
//	for (auto v: output.bytes)
//		{
//		safePrintf("%02x ", v);
//		}
//	safePrintf("\n");
	assertEqual(memcmp(&output, &expected, sizeof(output)), 0, "hash didn't match with memcpy");
	assertTrue(mcci_tweetnacl_verify_64(output.bytes, expected.bytes), "verify_64 didn't match");
	}

// skip this test if previous test failed
#define skipIfFailed(t) do { if (! checkTestPass(t)) { skip(); return; } } while (0)

/****************************************************************************\
|
|   Test signatures
|
\****************************************************************************/

// the test vectors in NaCl are huge; 2M. We ought to get these over the
// serial port, but for now, we just have a selection.
#include "mcci_tweetnacl_sign.h"
struct ed25519_test_t
	{
	mcci_tweetnacl_sign_privatekey_t Secret;
	mcci_tweetnacl_sign_publickey_t Public;
	const std::uint8_t *pMessage;
	size_t nMessage;
	const std::uint8_t *pSignature;
	size_t nSignature;
	};

// 1
const uint8_t m1[] = {
	
};
const uint8_t s1[] = {
	0xe5, 0x56, 0x43, 0x00, 0xc3, 0x60, 0xac, 0x72, 0x90, 0x86, 0xe2, 0xcc, 0x80, 0x6e, 0x82, 0x8a, 0x84, 0x87, 0x7f, 0x1e, 0xb8, 0xe5, 0xd9, 0x74, 0xd8, 0x73, 0xe0, 0x65, 0x22, 0x49, 0x01, 0x55, 0x5f, 0xb8, 0x82, 0x15, 0x90, 0xa3, 0x3b, 0xac, 0xc6, 0x1e, 0x39, 0x70, 0x1c, 0xf9, 0xb4, 0x6b, 0xd2, 0x5b, 0xf5, 0xf0, 0x59, 0x5b, 0xbe, 0x24, 0x65, 0x51, 0x41, 0x43, 0x8e, 0x7a, 0x10, 0x0b, 
};
const ed25519_test_t v1 = {
	.Secret = {
	0x9d, 0x61, 0xb1, 0x9d, 0xef, 0xfd, 0x5a, 0x60, 0xba, 0x84, 0x4a, 0xf4, 0x92, 0xec, 0x2c, 0xc4, 0x44, 0x49, 0xc5, 0x69, 0x7b, 0x32, 0x69, 0x19, 0x70, 0x3b, 0xac, 0x03, 0x1c, 0xae, 0x7f, 0x60, 0xd7, 0x5a, 0x98, 0x01, 0x82, 0xb1, 0x0a, 0xb7, 0xd5, 0x4b, 0xfe, 0xd3, 0xc9, 0x64, 0x07, 0x3a, 0x0e, 0xe1, 0x72, 0xf3, 0xda, 0xa6, 0x23, 0x25, 0xaf, 0x02, 0x1a, 0x68, 0xf7, 0x07, 0x51, 0x1a, 
	},
	.Public = {
	0xd7, 0x5a, 0x98, 0x01, 0x82, 0xb1, 0x0a, 0xb7, 0xd5, 0x4b, 0xfe, 0xd3, 0xc9, 0x64, 0x07, 0x3a, 0x0e, 0xe1, 0x72, 0xf3, 0xda, 0xa6, 0x23, 0x25, 0xaf, 0x02, 0x1a, 0x68, 0xf7, 0x07, 0x51, 0x1a, 
	},
	.pMessage = m1,
	.nMessage = sizeof(m1),
	.pSignature = s1,
	.nSignature = sizeof(s1),
};

// 2
const uint8_t m2[] = {
	0x72, 
};
const uint8_t s2[] = {
	0x92, 0xa0, 0x09, 0xa9, 0xf0, 0xd4, 0xca, 0xb8, 0x72, 0x0e, 0x82, 0x0b, 0x5f, 0x64, 0x25, 0x40, 0xa2, 0xb2, 0x7b, 0x54, 0x16, 0x50, 0x3f, 0x8f, 0xb3, 0x76, 0x22, 0x23, 0xeb, 0xdb, 0x69, 0xda, 0x08, 0x5a, 0xc1, 0xe4, 0x3e, 0x15, 0x99, 0x6e, 0x45, 0x8f, 0x36, 0x13, 0xd0, 0xf1, 0x1d, 0x8c, 0x38, 0x7b, 0x2e, 0xae, 0xb4, 0x30, 0x2a, 0xee, 0xb0, 0x0d, 0x29, 0x16, 0x12, 0xbb, 0x0c, 0x00, 0x72, 
};
const ed25519_test_t v2 = {
	.Secret = {
	0x4c, 0xcd, 0x08, 0x9b, 0x28, 0xff, 0x96, 0xda, 0x9d, 0xb6, 0xc3, 0x46, 0xec, 0x11, 0x4e, 0x0f, 0x5b, 0x8a, 0x31, 0x9f, 0x35, 0xab, 0xa6, 0x24, 0xda, 0x8c, 0xf6, 0xed, 0x4f, 0xb8, 0xa6, 0xfb, 0x3d, 0x40, 0x17, 0xc3, 0xe8, 0x43, 0x89, 0x5a, 0x92, 0xb7, 0x0a, 0xa7, 0x4d, 0x1b, 0x7e, 0xbc, 0x9c, 0x98, 0x2c, 0xcf, 0x2e, 0xc4, 0x96, 0x8c, 0xc0, 0xcd, 0x55, 0xf1, 0x2a, 0xf4, 0x66, 0x0c, 
	},
	.Public = {
	0x3d, 0x40, 0x17, 0xc3, 0xe8, 0x43, 0x89, 0x5a, 0x92, 0xb7, 0x0a, 0xa7, 0x4d, 0x1b, 0x7e, 0xbc, 0x9c, 0x98, 0x2c, 0xcf, 0x2e, 0xc4, 0x96, 0x8c, 0xc0, 0xcd, 0x55, 0xf1, 0x2a, 0xf4, 0x66, 0x0c, 
	},
	.pMessage = m2,
	.nMessage = sizeof(m2),
	.pSignature = s2,
	.nSignature = sizeof(s2),
};

// 3
const uint8_t m3[] = {
	0xaf, 0x82, 
};
const uint8_t s3[] = {
	0x62, 0x91, 0xd6, 0x57, 0xde, 0xec, 0x24, 0x02, 0x48, 0x27, 0xe6, 0x9c, 0x3a, 0xbe, 0x01, 0xa3, 0x0c, 0xe5, 0x48, 0xa2, 0x84, 0x74, 0x3a, 0x44, 0x5e, 0x36, 0x80, 0xd7, 0xdb, 0x5a, 0xc3, 0xac, 0x18, 0xff, 0x9b, 0x53, 0x8d, 0x16, 0xf2, 0x90, 0xae, 0x67, 0xf7, 0x60, 0x98, 0x4d, 0xc6, 0x59, 0x4a, 0x7c, 0x15, 0xe9, 0x71, 0x6e, 0xd2, 0x8d, 0xc0, 0x27, 0xbe, 0xce, 0xea, 0x1e, 0xc4, 0x0a, 0xaf, 0x82, 
};
const ed25519_test_t v3 = {
	.Secret = {
	0xc5, 0xaa, 0x8d, 0xf4, 0x3f, 0x9f, 0x83, 0x7b, 0xed, 0xb7, 0x44, 0x2f, 0x31, 0xdc, 0xb7, 0xb1, 0x66, 0xd3, 0x85, 0x35, 0x07, 0x6f, 0x09, 0x4b, 0x85, 0xce, 0x3a, 0x2e, 0x0b, 0x44, 0x58, 0xf7, 0xfc, 0x51, 0xcd, 0x8e, 0x62, 0x18, 0xa1, 0xa3, 0x8d, 0xa4, 0x7e, 0xd0, 0x02, 0x30, 0xf0, 0x58, 0x08, 0x16, 0xed, 0x13, 0xba, 0x33, 0x03, 0xac, 0x5d, 0xeb, 0x91, 0x15, 0x48, 0x90, 0x80, 0x25, 
	},
	.Public = {
	0xfc, 0x51, 0xcd, 0x8e, 0x62, 0x18, 0xa1, 0xa3, 0x8d, 0xa4, 0x7e, 0xd0, 0x02, 0x30, 0xf0, 0x58, 0x08, 0x16, 0xed, 0x13, 0xba, 0x33, 0x03, 0xac, 0x5d, 0xeb, 0x91, 0x15, 0x48, 0x90, 0x80, 0x25, 
	},
	.pMessage = m3,
	.nMessage = sizeof(m3),
	.pSignature = s3,
	.nSignature = sizeof(s3),
};

// 4
const uint8_t m4[] = {
	0xcb, 0xc7, 0x7b, 
};
const uint8_t s4[] = {
	0xd9, 0x86, 0x8d, 0x52, 0xc2, 0xbe, 0xbc, 0xe5, 0xf3, 0xfa, 0x5a, 0x79, 0x89, 0x19, 0x70, 0xf3, 0x09, 0xcb, 0x65, 0x91, 0xe3, 0xe1, 0x70, 0x2a, 0x70, 0x27, 0x6f, 0xa9, 0x7c, 0x24, 0xb3, 0xa8, 0xe5, 0x86, 0x06, 0xc3, 0x8c, 0x97, 0x58, 0x52, 0x9d, 0xa5, 0x0e, 0xe3, 0x1b, 0x82, 0x19, 0xcb, 0xa4, 0x52, 0x71, 0xc6, 0x89, 0xaf, 0xa6, 0x0b, 0x0e, 0xa2, 0x6c, 0x99, 0xdb, 0x19, 0xb0, 0x0c, 0xcb, 0xc7, 0x7b, 
};
const ed25519_test_t v4 = {
	.Secret = {
	0x0d, 0x4a, 0x05, 0xb0, 0x73, 0x52, 0xa5, 0x43, 0x6e, 0x18, 0x03, 0x56, 0xda, 0x0a, 0xe6, 0xef, 0xa0, 0x34, 0x5f, 0xf7, 0xfb, 0x15, 0x72, 0x57, 0x57, 0x72, 0xe8, 0x00, 0x5e, 0xd9, 0x78, 0xe9, 0xe6, 0x1a, 0x18, 0x5b, 0xce, 0xf2, 0x61, 0x3a, 0x6c, 0x7c, 0xb7, 0x97, 0x63, 0xce, 0x94, 0x5d, 0x3b, 0x24, 0x5d, 0x76, 0x11, 0x4d, 0xd4, 0x40, 0xbc, 0xf5, 0xf2, 0xdc, 0x1a, 0xa5, 0x70, 0x57, 
	},
	.Public = {
	0xe6, 0x1a, 0x18, 0x5b, 0xce, 0xf2, 0x61, 0x3a, 0x6c, 0x7c, 0xb7, 0x97, 0x63, 0xce, 0x94, 0x5d, 0x3b, 0x24, 0x5d, 0x76, 0x11, 0x4d, 0xd4, 0x40, 0xbc, 0xf5, 0xf2, 0xdc, 0x1a, 0xa5, 0x70, 0x57, 
	},
	.pMessage = m4,
	.nMessage = sizeof(m4),
	.pSignature = s4,
	.nSignature = sizeof(s4),
};

// 5
const uint8_t m5[] = {
	0x5f, 0x4c, 0x89, 0x89, 
};
const uint8_t s5[] = {
	0x12, 0x4f, 0x6f, 0xc6, 0xb0, 0xd1, 0x00, 0x84, 0x27, 0x69, 0xe7, 0x1b, 0xd5, 0x30, 0x66, 0x4d, 0x88, 0x8d, 0xf8, 0x50, 0x7d, 0xf6, 0xc5, 0x6d, 0xed, 0xfd, 0xb5, 0x09, 0xae, 0xb9, 0x34, 0x16, 0xe2, 0x6b, 0x91, 0x8d, 0x38, 0xaa, 0x06, 0x30, 0x5d, 0xf3, 0x09, 0x56, 0x97, 0xc1, 0x8b, 0x2a, 0xa8, 0x32, 0xea, 0xa5, 0x2e, 0xdc, 0x0a, 0xe4, 0x9f, 0xba, 0xe5, 0xa8, 0x5e, 0x15, 0x0c, 0x07, 0x5f, 0x4c, 0x89, 0x89, 
};
const ed25519_test_t v5 = {
	.Secret = {
	0x6d, 0xf9, 0x34, 0x0c, 0x13, 0x8c, 0xc1, 0x88, 0xb5, 0xfe, 0x44, 0x64, 0xeb, 0xaa, 0x3f, 0x7f, 0xc2, 0x06, 0xa2, 0xd5, 0x5c, 0x34, 0x34, 0x70, 0x7e, 0x74, 0xc9, 0xfc, 0x04, 0xe2, 0x0e, 0xbb, 0xc0, 0xda, 0xc1, 0x02, 0xc4, 0x53, 0x31, 0x86, 0xe2, 0x5d, 0xc4, 0x31, 0x28, 0x47, 0x23, 0x53, 0xea, 0xab, 0xdb, 0x87, 0x8b, 0x15, 0x2a, 0xeb, 0x8e, 0x00, 0x1f, 0x92, 0xd9, 0x02, 0x33, 0xa7, 
	},
	.Public = {
	0xc0, 0xda, 0xc1, 0x02, 0xc4, 0x53, 0x31, 0x86, 0xe2, 0x5d, 0xc4, 0x31, 0x28, 0x47, 0x23, 0x53, 0xea, 0xab, 0xdb, 0x87, 0x8b, 0x15, 0x2a, 0xeb, 0x8e, 0x00, 0x1f, 0x92, 0xd9, 0x02, 0x33, 0xa7, 
	},
	.pMessage = m5,
	.nMessage = sizeof(m5),
	.pSignature = s5,
	.nSignature = sizeof(s5),
};

// 6
const uint8_t m6[] = {
	0x18, 0xb6, 0xbe, 0xc0, 0x97, 
};
const uint8_t s6[] = {
	0xb2, 0xfc, 0x46, 0xad, 0x47, 0xaf, 0x46, 0x44, 0x78, 0xc1, 0x99, 0xe1, 0xf8, 0xbe, 0x16, 0x9f, 0x1b, 0xe6, 0x32, 0x7c, 0x7f, 0x9a, 0x0a, 0x66, 0x89, 0x37, 0x1c, 0xa9, 0x4c, 0xaf, 0x04, 0x06, 0x4a, 0x01, 0xb2, 0x2a, 0xff, 0x15, 0x20, 0xab, 0xd5, 0x89, 0x51, 0x34, 0x16, 0x03, 0xfa, 0xed, 0x76, 0x8c, 0xf7, 0x8c, 0xe9, 0x7a, 0xe7, 0xb0, 0x38, 0xab, 0xfe, 0x45, 0x6a, 0xa1, 0x7c, 0x09, 0x18, 0xb6, 0xbe, 0xc0, 0x97, 
};
const ed25519_test_t v6 = {
	.Secret = {
	0xb7, 0x80, 0x38, 0x1a, 0x65, 0xed, 0xf8, 0xb7, 0x8f, 0x69, 0x45, 0xe8, 0xdb, 0xec, 0x79, 0x41, 0xac, 0x04, 0x9f, 0xd4, 0xc6, 0x10, 0x40, 0xcf, 0x0c, 0x32, 0x43, 0x57, 0x97, 0x5a, 0x29, 0x3c, 0xe2, 0x53, 0xaf, 0x07, 0x66, 0x80, 0x4b, 0x86, 0x9b, 0xb1, 0x59, 0x5b, 0xe9, 0x76, 0x5b, 0x53, 0x48, 0x86, 0xbb, 0xaa, 0xb8, 0x30, 0x5b, 0xf5, 0x0d, 0xbc, 0x7f, 0x89, 0x9b, 0xfb, 0x5f, 0x01, 
	},
	.Public = {
	0xe2, 0x53, 0xaf, 0x07, 0x66, 0x80, 0x4b, 0x86, 0x9b, 0xb1, 0x59, 0x5b, 0xe9, 0x76, 0x5b, 0x53, 0x48, 0x86, 0xbb, 0xaa, 0xb8, 0x30, 0x5b, 0xf5, 0x0d, 0xbc, 0x7f, 0x89, 0x9b, 0xfb, 0x5f, 0x01, 
	},
	.pMessage = m6,
	.nMessage = sizeof(m6),
	.pSignature = s6,
	.nSignature = sizeof(s6),
};

// 7
const uint8_t m7[] = {
	0x89, 0x01, 0x0d, 0x85, 0x59, 0x72, 
};
const uint8_t s7[] = {
	0x6e, 0xd6, 0x29, 0xfc, 0x1d, 0x9c, 0xe9, 0xe1, 0x46, 0x87, 0x55, 0xff, 0x63, 0x6d, 0x5a, 0x3f, 0x40, 0xa5, 0xd9, 0xc9, 0x1a, 0xfd, 0x93, 0xb7, 0x9d, 0x24, 0x18, 0x30, 0xf7, 0xe5, 0xfa, 0x29, 0x85, 0x4b, 0x8f, 0x20, 0xcc, 0x6e, 0xec, 0xbb, 0x24, 0x8d, 0xbd, 0x8d, 0x16, 0xd1, 0x4e, 0x99, 0x75, 0x21, 0x94, 0xe4, 0x90, 0x4d, 0x09, 0xc7, 0x4d, 0x63, 0x95, 0x18, 0x83, 0x9d, 0x23, 0x00, 0x89, 0x01, 0x0d, 0x85, 0x59, 0x72, 
};
const ed25519_test_t v7 = {
	.Secret = {
	0x78, 0xae, 0x9e, 0xff, 0xe6, 0xf2, 0x45, 0xe9, 0x24, 0xa7, 0xbe, 0x63, 0x04, 0x11, 0x46, 0xeb, 0xc6, 0x70, 0xdb, 0xd3, 0x06, 0x0c, 0xba, 0x67, 0xfb, 0xc6, 0x21, 0x6f, 0xeb, 0xc4, 0x45, 0x46, 0xfb, 0xcf, 0xbf, 0xa4, 0x05, 0x05, 0xd7, 0xf2, 0xbe, 0x44, 0x4a, 0x33, 0xd1, 0x85, 0xcc, 0x54, 0xe1, 0x6d, 0x61, 0x52, 0x60, 0xe1, 0x64, 0x0b, 0x2b, 0x50, 0x87, 0xb8, 0x3e, 0xe3, 0x64, 0x3d, 
	},
	.Public = {
	0xfb, 0xcf, 0xbf, 0xa4, 0x05, 0x05, 0xd7, 0xf2, 0xbe, 0x44, 0x4a, 0x33, 0xd1, 0x85, 0xcc, 0x54, 0xe1, 0x6d, 0x61, 0x52, 0x60, 0xe1, 0x64, 0x0b, 0x2b, 0x50, 0x87, 0xb8, 0x3e, 0xe3, 0x64, 0x3d, 
	},
	.pMessage = m7,
	.nMessage = sizeof(m7),
	.pSignature = s7,
	.nSignature = sizeof(s7),
};

// 8
const uint8_t m8[] = {
	0xb4, 0xa8, 0xf3, 0x81, 0xe7, 0x0e, 0x7a, 
};
const uint8_t s8[] = {
	0x6e, 0x0a, 0xf2, 0xfe, 0x55, 0xae, 0x37, 0x7a, 0x6b, 0x7a, 0x72, 0x78, 0xed, 0xfb, 0x41, 0x9b, 0xd3, 0x21, 0xe0, 0x6d, 0x0d, 0xf5, 0xe2, 0x70, 0x37, 0xdb, 0x88, 0x12, 0xe7, 0xe3, 0x52, 0x98, 0x10, 0xfa, 0x55, 0x52, 0xf6, 0xc0, 0x02, 0x09, 0x85, 0xca, 0x17, 0xa0, 0xe0, 0x2e, 0x03, 0x6d, 0x7b, 0x22, 0x2a, 0x24, 0xf9, 0x9b, 0x77, 0xb7, 0x5f, 0xdd, 0x16, 0xcb, 0x05, 0x56, 0x81, 0x07, 0xb4, 0xa8, 0xf3, 0x81, 0xe7, 0x0e, 0x7a, 
};
const ed25519_test_t v8 = {
	.Secret = {
	0x69, 0x18, 0x65, 0xbf, 0xc8, 0x2a, 0x1e, 0x4b, 0x57, 0x4e, 0xec, 0xde, 0x4c, 0x75, 0x19, 0x09, 0x3f, 0xaf, 0x0c, 0xf8, 0x67, 0x38, 0x02, 0x34, 0xe3, 0x66, 0x46, 0x45, 0xc6, 0x1c, 0x5f, 0x79, 0x98, 0xa5, 0xe3, 0xa3, 0x6e, 0x67, 0xaa, 0xba, 0x89, 0x88, 0x8b, 0xf0, 0x93, 0xde, 0x1a, 0xd9, 0x63, 0xe7, 0x74, 0x01, 0x3b, 0x39, 0x02, 0xbf, 0xab, 0x35, 0x6d, 0x8b, 0x90, 0x17, 0x8a, 0x63, 
	},
	.Public = {
	0x98, 0xa5, 0xe3, 0xa3, 0x6e, 0x67, 0xaa, 0xba, 0x89, 0x88, 0x8b, 0xf0, 0x93, 0xde, 0x1a, 0xd9, 0x63, 0xe7, 0x74, 0x01, 0x3b, 0x39, 0x02, 0xbf, 0xab, 0x35, 0x6d, 0x8b, 0x90, 0x17, 0x8a, 0x63, 
	},
	.pMessage = m8,
	.nMessage = sizeof(m8),
	.pSignature = s8,
	.nSignature = sizeof(s8),
};

// 9
const uint8_t m9[] = {
	0x42, 0x84, 0xab, 0xc5, 0x1b, 0xb6, 0x72, 0x35, 
};
const uint8_t s9[] = {
	0xd6, 0xad, 0xde, 0xc5, 0xaf, 0xb0, 0x52, 0x8a, 0xc1, 0x7b, 0xb1, 0x78, 0xd3, 0xe7, 0xf2, 0x88, 0x7f, 0x9a, 0xdb, 0xb1, 0xad, 0x16, 0xe1, 0x10, 0x54, 0x5e, 0xf3, 0xbc, 0x57, 0xf9, 0xde, 0x23, 0x14, 0xa5, 0xc8, 0x38, 0x8f, 0x72, 0x3b, 0x89, 0x07, 0xbe, 0x0f, 0x3a, 0xc9, 0x0c, 0x62, 0x59, 0xbb, 0xe8, 0x85, 0xec, 0xc1, 0x76, 0x45, 0xdf, 0x3d, 0xb7, 0xd4, 0x88, 0xf8, 0x05, 0xfa, 0x08, 0x42, 0x84, 0xab, 0xc5, 0x1b, 0xb6, 0x72, 0x35, 
};
const ed25519_test_t v9 = {
	.Secret = {
	0x3b, 0x26, 0x51, 0x6f, 0xb3, 0xdc, 0x88, 0xeb, 0x18, 0x1b, 0x9e, 0xd7, 0x3f, 0x0b, 0xcd, 0x52, 0xbc, 0xd6, 0xb4, 0xc7, 0x88, 0xe4, 0xbc, 0xaf, 0x46, 0x05, 0x7f, 0xd0, 0x78, 0xbe, 0xe0, 0x73, 0xf8, 0x1f, 0xb5, 0x4a, 0x82, 0x5f, 0xce, 0xd9, 0x5e, 0xb0, 0x33, 0xaf, 0xcd, 0x64, 0x31, 0x40, 0x75, 0xab, 0xfb, 0x0a, 0xbd, 0x20, 0xa9, 0x70, 0x89, 0x25, 0x03, 0x43, 0x6f, 0x34, 0xb8, 0x63, 
	},
	.Public = {
	0xf8, 0x1f, 0xb5, 0x4a, 0x82, 0x5f, 0xce, 0xd9, 0x5e, 0xb0, 0x33, 0xaf, 0xcd, 0x64, 0x31, 0x40, 0x75, 0xab, 0xfb, 0x0a, 0xbd, 0x20, 0xa9, 0x70, 0x89, 0x25, 0x03, 0x43, 0x6f, 0x34, 0xb8, 0x63, 
	},
	.pMessage = m9,
	.nMessage = sizeof(m9),
	.pSignature = s9,
	.nSignature = sizeof(s9),
};

// 10
const uint8_t m10[] = {
	0x4b, 0xaf, 0xda, 0xc9, 0x09, 0x9d, 0x40, 0x57, 0xed, 0x6d, 0xd0, 0x8b, 0xca, 0xee, 0x87, 0x56, 0xe9, 0xa4, 0x0f, 0x2c, 0xb9, 0x59, 0x80, 0x20, 0xeb, 0x95, 0x01, 0x95, 0x28, 0x40, 0x9b, 0xbe, 0xa3, 0x8b, 0x38, 0x4a, 0x59, 0xf1, 0x19, 0xf5, 0x72, 0x97, 0xbf, 0xb2, 0xfa, 0x14, 0x2f, 0xc7, 0xbb, 0x1d, 0x90, 0xdb, 0xdd, 0xde, 0x77, 0x2b, 0xcd, 0xe4, 0x8c, 0x56, 0x70, 0xd5, 0xfa, 0x13, 
};
const uint8_t s10[] = {
	0x57, 0xb9, 0xd2, 0xa7, 0x11, 0x20, 0x7f, 0x83, 0x74, 0x21, 0xba, 0xe7, 0xdd, 0x48, 0xea, 0xa1, 0x8e, 0xab, 0x1a, 0x9a, 0x70, 0xa0, 0xf1, 0x30, 0x58, 0x06, 0xfe, 0xe1, 0x7b, 0x45, 0x8f, 0x3a, 0x09, 0x64, 0xb3, 0x02, 0xd1, 0x83, 0x4d, 0x3e, 0x0a, 0xc9, 0xe8, 0x49, 0x6f, 0x00, 0x0b, 0x77, 0xf0, 0x08, 0x3b, 0x41, 0xf8, 0xa9, 0x57, 0xe6, 0x32, 0xfb, 0xc7, 0x84, 0x0e, 0xee, 0x6a, 0x06, 0x4b, 0xaf, 0xda, 0xc9, 0x09, 0x9d, 0x40, 0x57, 0xed, 0x6d, 0xd0, 0x8b, 0xca, 0xee, 0x87, 0x56, 0xe9, 0xa4, 0x0f, 0x2c, 0xb9, 0x59, 0x80, 0x20, 0xeb, 0x95, 0x01, 0x95, 0x28, 0x40, 0x9b, 0xbe, 0xa3, 0x8b, 0x38, 0x4a, 0x59, 0xf1, 0x19, 0xf5, 0x72, 0x97, 0xbf, 0xb2, 0xfa, 0x14, 0x2f, 0xc7, 0xbb, 0x1d, 0x90, 0xdb, 0xdd, 0xde, 0x77, 0x2b, 0xcd, 0xe4, 0x8c, 0x56, 0x70, 0xd5, 0xfa, 0x13, 
};
const ed25519_test_t v10 = {
	.Secret = {
	0xba, 0x4d, 0x6e, 0x67, 0xb2, 0xce, 0x67, 0xa1, 0xe4, 0x43, 0x26, 0x49, 0x40, 0x44, 0xf3, 0x7a, 0x44, 0x2f, 0x3b, 0x81, 0x72, 0x5b, 0xc1, 0xf9, 0x34, 0x14, 0x62, 0x71, 0x8b, 0x55, 0xee, 0x20, 0xf7, 0x3f, 0xa0, 0x76, 0xf8, 0x4b, 0x6d, 0xb6, 0x75, 0xa5, 0xfd, 0xa5, 0xad, 0x67, 0xe3, 0x51, 0xa4, 0x1e, 0x8e, 0x7f, 0x29, 0xad, 0xd1, 0x68, 0x09, 0xca, 0x01, 0x03, 0x87, 0xe9, 0xc6, 0xcc, 
	},
	.Public = {
	0xf7, 0x3f, 0xa0, 0x76, 0xf8, 0x4b, 0x6d, 0xb6, 0x75, 0xa5, 0xfd, 0xa5, 0xad, 0x67, 0xe3, 0x51, 0xa4, 0x1e, 0x8e, 0x7f, 0x29, 0xad, 0xd1, 0x68, 0x09, 0xca, 0x01, 0x03, 0x87, 0xe9, 0xc6, 0xcc, 
	},
	.pMessage = m10,
	.nMessage = sizeof(m10),
	.pSignature = s10,
	.nSignature = sizeof(s10),
};

const ed25519_test_t * const vecs[] = {
	&v1, &v2, &v3, &v4, &v5, &v6, &v7, &v8, &v9, &v10, 
};

static uint8_t buf[256];

test(20_sign)
	{
	unsigned i = 0;
	safePrintf("Sign checks: ");
	for (auto &&v : vecs)
		{
		size_t nActual;
		++i;
		safePrintf("%u ", i);

		mcci_tweetnacl_sign(
			buf,
			&nActual,
			v->pMessage,
			v->nMessage,
			&v->Secret
			);

		assertEqual(nActual, v->nSignature, "Signature length wrong");
		assertEqual(memcmp(buf, v->pSignature, nActual), 0, "Signature value wrong");
		}
	safePrintf("\n");
	}
