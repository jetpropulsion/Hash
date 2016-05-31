///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Compile with /EHa, Project => C/C++ => Code Generation => Enable C++ Exceptions => Yes, with SEH exceptions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define NATURAL_ALIGNMENT		max(__alignof(__int64), __alignof(long double))
#define ALIGNMENT					16																				//8 is not enough if using SSE2 instructions
#pragma pack(ALIGNMENT)
#define ALIGN								__declspec(align(ALIGNMENT))
#define	INLINE							__forceinline
//#define	INLINE							__inline
#pragma warning(disable: 4505)																			//unreferenced local function has been removed
#pragma warning(disable: 4731)																			//frame pointer register EBX modified by inline assembly code
#pragma warning(disable: 4146)																			//unary minus applied to unsigned, result still unsigned
#pragma warning(disable: 4324)																			//structure was padded due to __declspec(align())
#undef _CRT_DISABLE_PERFCRIT_LOCKS																	//if defined use fread_nolock() and similar functions

/*
//Fast float control mode
#pragma float_control(except, off)
#pragma fenv_access(off)
#pragma float_control(precise, off)
#pragma fp_contract(on)
*/
/*
//Strict float control mode
#pragma float_control(precise, on)
#pragma fenv_access(on)
#pragma float_control(except, on)
#pragma fp_contract(off)
*/

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define _WIN32_LEAN_AND_MEAN
#define _WIN32_LEAN_AND_MEAN_
#define __WIN32_LEAN_AND_MEAN__

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <intrin.h>
#include <emmintrin.h>
#include <xmmintrin.h>
#include <fcntl.h>
#include <io.h>

//#pragma comment(linker, "/ENTRY:main /FILEALIGN:0x200 /MERGE:.data=.text /MERGE:.rdata=.text /SECTION:.text,EWR /IGNORE:4078")
#pragma comment(exestr, "Compiled on " __DATE__ " at " __TIME__ )

#if( defined(_MSC_VER) && (!defined(__INTEL_COMPILER)) )
	#define amalloc(s)				_aligned_malloc(s, ALIGNMENT)
	#define arealloc(p,s)			_aligned_realloc(p, s, ALIGNMENT)
	#define afree(p)					_aligned_free(p)
#endif

#define burn(x)							memset(&(x), 0, sizeof(x))
#define lengthof(arr)				(sizeof(arr) / sizeof(arr[0]))
#define lastof(arr)					(lengthof(arr) - 1)
#define indexof(x, arr)			(((LPBYTE)(x)) - ((LPBYTE)(arr))) / sizeof((arr)[0])

#if ( defined(_MSC_VER) )
	#define rol8(i, bits)			_rotl8((i), (bits))
	#define ror8(i, bits)			_rotr8((i), (bits))
	#define rol16(i, bits)		_rotl16((i), (bits))
	#define ror16(i, bits)		_rotr16((i), (bits))
	#define rol64(i, bits)		_rotl64((i), (bits))
	#define ror64(i, bits)		_rotr64((i), (bits))
#endif
typedef const BYTE *LPCBYTE;

const TCHAR cszStringNo[] = _T("No");
const TCHAR cszStringYes[] = _T("Yes");

#define FASTSHIFT		32 | 64 | 128

static LARGE_INTEGER liFreq;
static HANDLE hStdOut = NULL;
static CONSOLE_SCREEN_BUFFER_INFO csbi;
static const char cszStringDblDashes[256] =
	"================================================================"
	"================================================================"
	"================================================================"
	"===============================================================";

static void ConsoleReadParams()
{
	burn(csbi);
	if((hStdOut = ::GetStdHandle(STD_OUTPUT_HANDLE)) == INVALID_HANDLE_VALUE)
	{
		return;
	}
	if(!::GetConsoleScreenBufferInfo(hStdOut, &csbi))
	{
		burn(csbi);
		csbi.dwSize.X = 80;
		csbi.dwSize.Y = 25;
	}
}

static void WriteHeader(LPCTSTR Text)
{
	ConsoleReadParams();
	_ftprintf_s(stdout, _T("%.*hs\n"), csbi.dwSize.X - 1, cszStringDblDashes);
	_ftprintf_s(stdout, _T("%s\n"), Text);
	_ftprintf_s(stdout, _T("%.*hs\n"), csbi.dwSize.X - 1, cszStringDblDashes);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef unsigned (__cdecl *PHASH)(LPCBYTE Value, unsigned Length, unsigned Seed);
typedef struct TESTHASH
{
	unsigned __int64 Ticks;			//Cumulative ticks
	LPCTSTR Name;								//Function name
	LPCTSTR Description;				//Detailed description
	PHASH pFn;									//Generic function pointer
	unsigned Collisions;				//Cumulative collisions found
	unsigned Seed;							//Initialization seed
}
TESTHASH, *PTESTHASH;
typedef const TESTHASH *PCTESTHASH;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//Forward and extern declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" unsigned Jenkins_x86(LPCBYTE pcbValue, unsigned dwLength, unsigned dwSeed);
extern "C" unsigned FNV1a_x86(LPCBYTE pcbValue, unsigned dwLength, unsigned dwSeed);
extern "C" unsigned DJB2_x86(LPCBYTE pcbValue, unsigned dwLength, unsigned dwSeed);
extern "C" unsigned Murmur_x86(LPCBYTE pcbValue, unsigned dwLength, unsigned dwSeed);
extern "C" unsigned CRC32_x86(LPCBYTE pcbValue, unsigned dwLength, unsigned dwSeed);
unsigned CRC32(LPCBYTE Value, unsigned Length, unsigned Seed);
unsigned Jenkins(LPCBYTE Value, unsigned Length, unsigned Seed);
unsigned SimpleSum(LPCBYTE Value, unsigned Length, unsigned Seed);
unsigned DJB2(LPCBYTE Value, unsigned Length, unsigned Seed);
unsigned SDBM(LPCBYTE Value, unsigned Length, unsigned Seed);
unsigned Rotating(LPCBYTE Value, unsigned Length, unsigned Seed);
unsigned EMACS(LPCBYTE Value, unsigned Length, unsigned Seed);
unsigned PJW(LPCBYTE Value, unsigned Length, unsigned Seed);
unsigned AVL(LPCBYTE Value, unsigned Length, unsigned Seed);
unsigned Murmur(LPCBYTE Value, unsigned Length, unsigned Seed);
unsigned FNV1a(LPCBYTE Value, unsigned Length, unsigned Seed);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Initialization of test structures
///////////////////////////////////////////////////////////////////////////////////////////////////////////

static TESTHASH Hashes[] =
{
	{	0, _T("SimpleSum"), _T("Simple addition of all octets in array"), SimpleSum, 0, 0 },
	{	0, _T("Murmur"), _T("by Austin Appleby"), Murmur, 0, 0 },
	{	0, _T("Murmur_x86"), _T("by Austin Appleby in x86 assembly"), Murmur_x86, 0, 0 },
	{	0, _T("Jenkins"), _T("Bob Jenkin's hashlittle() from \"lookup3.c\""), Jenkins, 0, 0 },
	{	0, _T("Jenkins_x86"), _T("Bob Jenkin's hashlittle() from \"lookup3.c\" in x86 assembly"), Jenkins_x86, 0, 0 },
	{	0, _T("DJB2"), _T("Daniel J. Bernstein's hash function #2"), DJB2, 0, 0 },
	{	0, _T("DJB2_x86"), _T("Daniel J. Bernstein's hash function #2 in x86 assembly"), DJB2_x86, 0, 0 },
	{	0, _T("SDBM"), _T("<no information>"), SDBM, 0, 0 },
	{	0, _T("Rotating"), _T("<no information>"), Rotating, 0, 0 },
	{	0, _T("EMACS"), _T("Used in UNIX environment application EMACS"), EMACS, 0, 0 },
	{	0, _T("PJW"), _T("<no information>"), PJW, 0, 0 },
	{	0, _T("AVL"), _T("AVL hash"), AVL, 0, 0 },
	{	0, _T("CRC32"), _T("Cyclic-Redundancy-Check 32bit"), CRC32, 0, (unsigned)-1 },
	{	0, _T("CRC32_x86"), _T("Cyclic-Redundancy-Check 32bit in assembly"), CRC32_x86, 0, (unsigned)-1 },
	{	0, _T("FNV1a"), _T("by Fowler/Noll/Vo"), FNV1a, 0, 0 },
	{	0, _T("FNV1a_x86"), _T("by Fowler/Noll/Vo in x86 assembly"), FNV1a_x86, 0, 0 },
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Hash sort and compare
///////////////////////////////////////////////////////////////////////////////////////////////////////////

static unsigned HashIx;
static int qsortCompareHash(const void *pA, const void *pB)
{
	return ((const unsigned *)pA)[1 + HashIx] - ((const unsigned *)pB)[1 + HashIx];
}

static void *Hashed = NULL;
static unsigned HashedCount = 0;
static unsigned HashedFree = 0;
static const unsigned HashedRowSize = sizeof(unsigned) + (lengthof(Hashes) * sizeof(unsigned));

static void HashTextLine(LPCTSTR pcszLine, unsigned cchLine, unsigned uIndex)
{
	//_ftprintf_s(stdout, _T("%u(%6u):\"%.*s\"\n"), ulIndex, cchLine, cchLine, pcszLine);

	if(HashedFree == 0)
	{
		void *NewHashed;
		unsigned HashedSize = HashedCount * HashedRowSize;
		if(HashedSize == 0)
		{
			if((NewHashed = amalloc(1024 * HashedRowSize)) == NULL)
			{
				_ftprintf_s(stdout, _T("ERROR: Failed to amalloc() %u bytes\n"), 1024 * HashedRowSize);
				exit(-1);
			}
			HashedFree = 1024;
		}
		else
		{
			if((NewHashed = arealloc(Hashed, 2 * HashedSize)) == NULL)
			{
				_ftprintf_s(stdout, _T("ERROR: Failed to arealloc() to %u /doubling the size/\n"), 2 * HashedSize);
				exit(-1);
			}
			HashedFree = HashedCount;
		}
		Hashed = NewHashed;
	}

	HashedFree--;

	unsigned *Current = (unsigned *)((LPBYTE)Hashed + (HashedCount++ * HashedRowSize));
	Current[0] = uIndex;

	LARGE_INTEGER liStart;
	LARGE_INTEGER liStop;
	ULONG_PTR i;
	for(i = 0; i < lengthof(Hashes); i++)
	{
		if(!::QueryPerformanceCounter(&liStart))
		{
			_ftprintf_s(stdout, _T("ERROR:QueryPerformanceCounter(): %d\n"), ::GetLastError());
			exit(-1);
		}
		ULONG_PTR Hash = Hashes[i].pFn((LPCBYTE)pcszLine, cchLine * sizeof(TCHAR), Hashes[i].Seed);
		if(!::QueryPerformanceCounter(&liStop))
		{
			_ftprintf_s(stdout, _T("ERROR:QueryPerformanceCounter(): %d\n"), ::GetLastError());
			exit(-1);
		}
		Hashes[i].Ticks += liStop.QuadPart - liStart.QuadPart;
		Current[1 + i] = Hash;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Hash algorithms' implementations, copied from the net
///////////////////////////////////////////////////////////////////////////////////////////////////////////


unsigned Jenkins(LPCBYTE Value, unsigned Length, unsigned Seed)
{
	unsigned a, b, c;
	a = b = c = 0xDEADBEEF + ((unsigned)Length) + ((unsigned)Seed);
	while(Length > 12)
	{
		a += Value[0];
		a += ((unsigned)Value[1]) << 8;
		a += ((unsigned)Value[2]) << 16;
		a += ((unsigned)Value[3]) << 24;
		b += Value[4];
		b += ((unsigned)Value[5]) << 8;
		b += ((unsigned)Value[6]) << 16;
		b += ((unsigned)Value[7]) << 24;
		c += Value[8];
		c += ((unsigned)Value[9]) << 8;
		c += ((unsigned)Value[10]) << 16;
		c += ((unsigned)Value[11]) << 24;

		//mix(a,b,c);
		a -= c; a ^= _lrotl(c, 4); c += b;
		b -= a; b ^= _lrotl(a, 6); a += c;
		c -= b; c ^= _lrotl(b, 8); b += a;
		a -= c; a ^= _lrotl(c, 16); c += b;
		b -= a; b ^= _lrotl(a, 19); a += c;
		c -= b; c ^= _lrotl(b, 4); b += a;

		Length -= 12;
		Value += 12;
	}
	//last block
	switch(Length)
	{
		case 12: c += ((unsigned)Value[11]) << 24;
		case 11: c += ((unsigned)Value[10]) << 16;
		case 10: c += ((unsigned)Value[9]) << 8;
		case 9 : c += Value[8];
		case 8 : b += ((unsigned)Value[7]) << 24;
		case 7 : b += ((unsigned)Value[6]) << 16;
		case 6 : b += ((unsigned)Value[5]) << 8;
		case 5 : b += Value[4];
		case 4 : a += ((unsigned)Value[3]) << 24;
		case 3 : a += ((unsigned)Value[2]) << 16;
		case 2 : a += ((unsigned)Value[1]) << 8;
		case 1 : a += Value[0];
		break;
		case 0:
		return c;
	}
	//final(a,b,c);
  c ^= b; c -= _lrotl(b, 14);
  a ^= c; a -= _lrotl(c, 11);
  b ^= a; b -= _lrotl(a, 25);
  c ^= b; c -= _lrotl(b, 16);
  a ^= c; a -= _lrotl(c, 4);
  b ^= a; b -= _lrotl(a, 14);
  c ^= b; c -= _lrotl(b, 24);

	return c;
}


unsigned SimpleSum(LPCBYTE Value, unsigned Length, unsigned Seed)
{
	UNREFERENCED_PARAMETER(Seed);

	unsigned u = (unsigned)-1;
	for(unsigned i = 0; i < Length; i++)
	{
		u += *(Value++);
	}
	return u;
}


unsigned DJB2(LPCBYTE Value, unsigned Length, unsigned Seed)
{
	unsigned u = Seed + Length + 5381;		//5381 is "magic" value
	while(Length--)
	{
		u = ((u << 5) + u) + *(Value++);
	}
	return u;
}


unsigned SDBM(LPCBYTE Value, unsigned Length, unsigned Seed)
{
	unsigned u = Seed + Length;
	for(unsigned i = 0; i < Length; i++)
	{
		u = Value[i] + (u << 6) + (u << 16) - u;
	}
	return u;
}


unsigned Rotating(LPCBYTE Value, unsigned Length, unsigned Seed)
{
	unsigned u = Seed + Length;
	for(unsigned i = 0; i < Length; i++)
	{
		u = (u << 5) ^ (u >> 27) ^ Value[i];
	}
	return u;
}


unsigned EMACS(LPCBYTE Value, unsigned Length, unsigned Seed)
{
	unsigned u = Seed + Length;
	for(unsigned i = 0; i < Length; i++)
	{
		BYTE c = Value[i];
		if(c >= 140)
			c -= 40;
		u = (u << 3) + (u >> 28) + c;
	}
	return u;
}


unsigned PJW(LPCBYTE Value, unsigned Length, unsigned Seed)
{
	unsigned u = Seed + Length;
	for(unsigned i = 0; i < Length; i++)
	{
		u = (u << 4) + Value[i];
		unsigned g;
		if((g = (u & 0xF0000000)) != 0)
			u = ((u ^ (g >> 24)) ^ g);
	}
	return u;
}


unsigned AVL(LPCBYTE Value, unsigned Length, unsigned Seed)
{
	unsigned u = Seed + Length;
	for(unsigned i = 0; i < Length; i++)
	{
		u ^= _lrotl(u, 3);
		u = 0x1bbb * u + Value[i];
	}
	return u % 0xbbb1;
}


unsigned Murmur(LPCBYTE Value, unsigned Length, unsigned Seed)
{
	const unsigned m = 0x5bd1e995;

	unsigned h = Seed ^ Length;
	while(Length >= 4)
	{
		unsigned k  = _byteswap_ulong(*((unsigned *)Value));
		//TODO:	Despite that this is obvious solution, it produces differences (?????!!!!!?????)
/*
		k = Value[0];
		k |= Value[1] << 8;
		k |= Value[2] << 16;
		k |= Value[3] << 24;
*/
		k *= m; 
		k ^= k >> 24; 
		k *= m;

		h *= m;
		h ^= k;

		Value += 4;
		Length -= 4;
	}
	switch(Length)
	{
		case 3:
			h ^= Value[2] << 16;
		case 2:
			h ^= Value[1] << 8;
		case 1:
			h ^= Value[0];
			h *= m;
	}

	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
}


unsigned FNV1a(LPCBYTE Value, unsigned Length, unsigned Seed)
{
	#define NO_FNV_GCC_OPTIMIZATION
	const unsigned FNV1_32_INIT = 0x811C9DC5;
	const unsigned FNV_32_PRIME = 0x01000193;

	LPCBYTE End = Value + Length;
	unsigned u = FNV1_32_INIT;
	while(Value < End)
	{
		#if(defined(NO_FNV_GCC_OPTIMIZATION))
		u *= FNV_32_PRIME;
		#else
		u += (u << 1) + (u << 4) + (u << 7) + (u << 8) + (u << 24);
		#endif
		u ^= (unsigned)*(Value++);
	}
	return u ^ (Seed + Length);
}


extern "C" __declspec(dllexport) unsigned CRC32Table[256];
unsigned CRC32Table[256];
unsigned CRC32(LPCBYTE Value, unsigned Length, unsigned Seed)
{
	//#define update_crc32(c, crc) ((crc >> 8) ^ CRC32Table[(c ^ (int)crc) & 0xff])
	unsigned CRC32 = 0xbaadf00d;
	while(Length--)
	{
		CRC32 = (Seed >> 8) ^ CRC32Table[(unsigned char)(*(Value++) ^ Seed)];
		Seed = CRC32;
	}
	return CRC32;
}
static void CRC32Init(unsigned *pCRC32Table)
{
	for(unsigned i = 0; i < 256; i++)
	{
		unsigned entry32 = i;
		for(unsigned j = 0; j < 8; j++)
		{
			unsigned carry32 = entry32 & 1;
			entry32 >>= 1;
			if(carry32)
			{
				entry32 ^= 0xedb88320;
			}
		}
		pCRC32Table[i] = entry32;
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Windows OS tuning for test
///////////////////////////////////////////////////////////////////////////////////////////////////////////


static void Setup()
{
	HANDLE const hProcess = ::GetCurrentProcess();
	if(::SetProcessAffinityMask(hProcess, 3))
	{
		DWORD_PTR dwPAM = 0;
		DWORD_PTR dwSAM = 0;
		if(::GetProcessAffinityMask(hProcess, &dwPAM, &dwSAM))
			_ftprintf_s(stderr, _T("Affinity mask: Process AM=%#08x, System AM=%#08x\n"), dwPAM, dwSAM);
	}

	HANDLE const hThread = ::GetCurrentThread();
	//if(::SetThreadAffinityMask(hThread, 3))
	//{
	//	_ftprintf_s(stderr, _T("Thead affinity mask is set\n"));
	//}

	if(::SetProcessPriorityBoost(hProcess, TRUE))
	{
		_ftprintf_s(stderr, _T("Priority boost is set\n"));
	}
	if(::SetPriorityClass(hProcess, HIGH_PRIORITY_CLASS))
	{
		_ftprintf_s(stderr, _T("Process high priority is set\n"));
	}
	if(::SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST))
	{
		_ftprintf_s(stderr, _T("Thread high priority is set\n"));
	}

	BOOL const tRedirOut = !(_isatty(_fileno(stdout)));
	_setmode(_fileno(stdout), _O_U8TEXT);
	_ftprintf_s(stderr, _T("Standard output redirection [%s]\n"), tRedirOut ? cszStringYes : cszStringNo);
}

static BOOL SanityCheck()
{
	static char szTest[4096];
	memset(szTest, 'a', sizeof(szTest));
	unsigned errors = 0;
	for(unsigned i = 1; i < sizeof(szTest); i++)
	{
		unsigned a, b;

		a = Jenkins_x86((LPCBYTE)szTest, i, 0);
		b = Jenkins((LPCBYTE)szTest, i, 0);
		if(a != b)
		{
			_ftprintf_s(stdout, _T("[Jenkins('a', %u)] CPP: %08x, ASM: %08x\n"), i, b, a);
			errors++;
		}
		b = FNV1a((LPCBYTE)szTest, i, 0);
		a =	FNV1a_x86((LPCBYTE)szTest, i, 0);
		if(a != b)
		{
			_ftprintf_s(stdout, _T("[FNV1a('a', %u)] CPP: %08x, ASM: %08x\n"), i, b, a);
			errors++;
		}
		b = DJB2((LPCBYTE)szTest, i, 0);
		a = DJB2_x86((LPCBYTE)szTest, i, 0);
		if(a != b)
		{
			_ftprintf_s(stdout, _T("[DJB2('a', %u)] CPP: %08x, ASM: %08x\n"), i, b, a);
			errors++;
		}
		b = Murmur((LPCBYTE)szTest, i, 0);
		a = Murmur_x86((LPCBYTE)szTest, i, 0);
		if(a != b)
		{
			_ftprintf_s(stdout, _T("[Murmur('a', %u)] CPP: %08x, ASM: %08x\n"), i, b, a);
			errors++;
		}
		a = CRC32_x86((LPCBYTE)szTest, i, 0);
		b = CRC32((LPCBYTE)szTest, i, 0);
		if(a != b)
		{
			_ftprintf_s(stdout, _T("[CRC32('a', %u)] CPP: %08x, ASM: %08x\n"), i, b, a);
			errors++;
		}
	}
	return (errors == 0);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Windows main(), include <tchar.h>
///////////////////////////////////////////////////////////////////////////////////////////////////////////

int _tmain(int argc, LPCTSTR *argv)
{
	errno_t err;
	if(argc != 2)
	{
		_ftprintf_s(stderr, _T("\nHash 1.0\nCreated in 2008 by Vladimir Kraljevic, for various testing purposes\n"));
		_ftprintf_s(stderr, _T("\nUsage: %s <path>\n"), argv[0]);
		return ERROR_INVALID_PARAMETER;		//"Parameter is incorrect"
	}
	CRC32Init(CRC32Table);
	if (!SanityCheck())
	{
		_ftprintf_s(stderr, _T("Insane.\n"));
		return -1;
	}
	if(!::QueryPerformanceFrequency(&liFreq))
	{
		_ftprintf_s(stdout, _T("ERROR:QueryPerformanceFrequency(): %d\n"), ::GetLastError());
		return -1;
	}
	_ftprintf_s(stdout, _T("Performance Frequency: %I64d\n"), liFreq.QuadPart);

	ULONG_PTR i;
	for(i = 0; i < lengthof(Hashes); i++)
	{
		Hashes[i].Ticks = 0;
		_ftprintf_s(stdout, _T("Hash function %u: %s (%s)\n"), i, Hashes[i].Name, Hashes[i].Description);
	}

	FILE *f = NULL;
	BYTE bTemp[131072];
	TCHAR szTemp[8192];
	unsigned const cuTemp = sizeof(bTemp) / sizeof(TCHAR);			//Do NOT use lengthof() because it will yield wrong size (in bytes)
	if((err = _tfopen_s(&f, argv[1], _T("rtcSN,ccs=UTF-8"))) != 0)
	{
		if(!_tcserror_s(szTemp, lengthof(szTemp), err))
			_ftprintf_s(stderr, _T("ERROR: Failed to open \"%s\": %d\n\t%s\n"), argv[1], err, szTemp);
		else
			_ftprintf_s(stderr, _T("ERROR: Failed to open \"%s\": %d\n"), argv[1], err);
		return err;
	}

	int result = -1;
	__try
	{
		if( (err = setvbuf(f, NULL, _IOFBF, cuTemp * 2)) != 0 )
		{
			_ftprintf_s(stderr, _T("ERROR: Failed to setbuf() to %u for \"%s\": %d\n"), cuTemp * 2, argv[1], ferror(f));
			result = err;
			__leave;
		}
		if( (err = clearerr_s(f)) != 0 )
		{
			_ftprintf_s(stderr, _T("ERROR: Failed to clearerr_s() for \"%s\": %d\n"), argv[1], ferror(f));
			result = err;
			__leave;
		}
		///////////////////////////////////////////////////////////////////////////////////////////////////////
		// History Buffer
		///////////////////////////////////////////////////////////////////////////////////////////////////////
		Hashed = NULL;
		HashedCount = 0;
		HashedFree = 0;
		///////////////////////////////////////////////////////////////////////////////////////////////////////
		__try
		{
			Setup();

			LPTSTR pszLine = (LPTSTR)bTemp;
			unsigned uIndex = 0;
			while(_fgetts(pszLine, cuTemp - 1, f))
			{
				uIndex++;

				unsigned uTemp = cuTemp - 1;
				/***************************************************************************************************
				****************************************************************************************************
				***	Skip leading CR's and LF's as long as these occur
				***	we are finished when either encounter; EOL or we exceed line length boundary
				****************************************************************************************************
				***************************************************************************************************/
				while( (uTemp != 0) && (*pszLine != _T('\0')) && ((*pszLine == _T('\r')) || (*pszLine == _T('\n'))) )
				{
					uTemp--;
					pszLine++;
				}
				LPCTSTR pcszTemp = pszLine;
				while( (uTemp != 0) && (*pcszTemp != _T('\0')) && (*pcszTemp != _T('\r')) && (*pcszTemp != _T('\n')) )
				{
					uTemp--;
					pcszTemp++;
				}
				unsigned uLine = (unsigned)(pcszTemp - pszLine);
				if(uLine != 0)
				{
					HashTextLine(pszLine, uLine, uIndex);
				}
				pszLine = (LPTSTR)bTemp;
			}
			if( (err = ferror(f)) != 0 )
			{
				_ftprintf_s(stderr, _T("ERROR: File \"%s\" possibly not read correctly: %d\n"), argv[1], err);
				result = err;
				__leave;
			}
			_ftprintf_s(stdout, _T("Lines:%u, Hashed Count:%u, Hashed Free:%u\n"), uIndex, HashedCount, HashedFree);
			if(HashedCount == 0)
			{
				_fputts(_T("INFO: No lines were hashed\n"), stderr);
				result = 0;
				__leave;
			}

			for(HashIx = 0; HashIx < lengthof(Hashes); HashIx++)
			{
				qsort(Hashed, HashedCount, HashedRowSize, qsortCompareHash);

				unsigned *Previous = (unsigned *)Hashed;
				unsigned HashedRowIndex;
				for(HashedRowIndex = 1; HashedRowIndex < HashedCount; HashedRowIndex++)
				{
					unsigned *Current = (unsigned *)((LPBYTE)Hashed + (HashedRowIndex * HashedRowSize));
					if(Current[1 + HashIx] == Previous[1 + HashIx])
					{
						//_ftprintf_s(stdout, _T("Collision in lines %u and %u: %08x:%08x\n"), Previous[0], Current[0], Previous[1 + HashIx], Current[1 + HashIx]);
						Hashes[HashIx].Collisions++;
					}
					Previous = Current;
				}
				_ftprintf_s(	stdout,
											_T("%-40s: Ticks=%-9.2f Collisions=%-16u (%5.2f%%)\n"),
											Hashes[HashIx].Name,
											(double)Hashes[HashIx].Ticks * 1000.e0 / ((double)liFreq.QuadPart),
											Hashes[HashIx].Collisions,
											(double)Hashes[HashIx].Collisions * 100.0e0 / ((double)HashedCount)
										);
			}
			result = 0;
		}
		__finally
		{
			if((Hashed != NULL) || (HashedCount != 0) || (HashedFree != 0))
			{
				if(!::IsBadWritePtr(Hashed, (HashedCount + HashedFree) * HashedRowSize))
				{
					afree(Hashed);
				}
				else
				{
					_ftprintf_s(stdout, _T("ERROR: Bad pointer\n"));
					exit(-1);
				}
			}
		}
	}
	__finally
	{
		if((err = fclose(f)) != 0)
		{
			_ftprintf_s(stderr, _T("ERROR: File \"%s\" not closed correctly: %d\n"), argv[1], ferror(f));
		}
		f = NULL;
	}

	return result;
};


