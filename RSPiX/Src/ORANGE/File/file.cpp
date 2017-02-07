////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2016 RWS Inc, All Rights Reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of version 2 of the GNU General Public License as published by
// the Free Software Foundation
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110FAILURE301 USA
//
//////////////////////////////////////////////////////////////////////////////
//
// File.CPP
// 
// History:
//		07/29/95 JMI	Started.
//
//		08/14/95	JMI	Changed name from CFile to CNFile.
//
//		08/17/95	JMI	Added memory file ability.
//
//		08/28/95	JMI	Added use of m_sMemError to allow Error() function to
//							flag errors.
//
//		09/19/95	JMI	Altered ASSERT in Close to work with memory files.
//
//		09/22/95	JMI	Added Open and Close hooks.
//		
//		10/04/95	JMI	Added automagic reopen ability.
//
//		10/14/96 MJR	Removed <SMRTHEAP.HPP> since it will be taken care of
//							by system.h, which is included via blue.h
//
//		10/15/96	JMI	Added Read/Write((float*)...) and Read/Write((double*)...).
//
//		10/18/96 MJR	Fixed memory leaks on Write() functions.
//
//		10/21/96	JMI	Changed Write() functions to utilize a shared static buf-
//							fer and write in sizeof(ms_u8SwapBuf) byte chunks.  Also,
//							fixed possible problem introduced by the #if 1 blocks
//							added to Write(uint32_t/uint16_t) that would have caused twice the
//							info to have been written.
//
//		10/21/96	JMI	Added ability to read & write in new "ASCII mode".
//
//		10/23/96	JMI	Open() for memory was initializing m_sFlags to 0 instead
//							of NFILE_BINARY.
//
//		10/23/96	JMI	Byte swapping was always doing at least a buffers worth
//							and was overrunning the swap buffer.  The fix to this
//							was merely implementing the slight performance improvement
//							that was listed in comment before each swap algorithm.
//
//		10/25/96	JMI	Now reads and writes PIXEL24 which is a 24 bit type
//							defined in <Sys>System.h.  It can vary in order on big 
//							and little endian systems, but it may, in some cases,
//							not vary.  I'm just not sure.  For now, this module
//							byte swaps if the file's endianness varies from the
//							systems, but this may not always be the case for 24 bit
//							pixel data.
//
//		10/28/96	JMI	Disabled warning on constant conditional expression so
//							this file would compile nicely on warning level 4.
//
//		10/30/96	JMI	Changed:
//							Old label:			New label:
//							=========			=========
//							CNFile				RFile
//							*_CNFILE_*			*_RFILE_*
//							CList					RList
//							ENDIAN_BIG			RFile::BigEndian
//							ENDIAN_LITTLE		RFile::LittleEndian
//							NFILE_BINARY		RFile::Binary
//							NFILE_ASCII			RFile::Ascii
//							PIXEL24				RPixel24
//							CNFILEOPENHOOK		RFile::OpenHook
//							CNFILECLOSEHOOK	RFile::CloseHook
//
//		12/11/96	JMI	Now calls ms_criticall with the number of bytes that are
//							about to be read or written when reading/writing a disk
//							file.
//
//		01/22/97	JMI	New Open() call allows one to make a file that grows
//							when Write()s exceed its size.
//
//		01/28/97	JMI	Added yet another Open() overload that allows one to open
//							an existing FILE* stream with an RFile.
//							And yet another Open() which opens from another RFile
//							that is already attached to disk or memory.  This is use-
//							ful for descended classes such as RIff (so they can sub-
//							class an existing RFile).
//
//		05/30/97	JMI	Added FILE_VERBOSE.  Which, when defined, 
//							causes the resmgr to output TRACEs when a file
//							is not found.  Although we all love this feature
//							for debugging purposes, it gets bad when we're
//							loading a file only if it exists.
//
//		07/17/97 MJR	Added code to set default buffer size for RFile's to
//							16k, but commented it out pending further testing to
//							determine whether it's a good idea.
//
//		08/23/97	JMI	Now will call the criticallback for disk files every 
//							MAX_CALLBACK_GRANULARITY_BYTES bytes at a maximum to
//							ensure a certain amount of callback granularity.
//
//		08/23/97	JMI	Fixed bug with not incrementing dest pointers in Read()
//							and Write().
//
//////////////////////////////////////////////////////////////////////////////
//
// This module handles file stuff, providing services such as Open, Close,
// Read, and Write.  It also can transparently convert data as read in based
// on the endian nature of the current platform and the endian macro supplied
// by the app to the Open() or SetEndian() (BigEndian or LittleEndian).
//
// There are two modes in which this module can operate (Binary or
// Ascii (Ascii does not work for memory files)).  In order to
// hide ASCII operation, the ASCII versions of these functions return values
// that would, hopefully, be acceptable to a binary reader/writer using
// CNFile.  The big suck is that you cannot make a placeholder for places you
// want to seek back to with intent of updating a field since the field's size,
// in ASCII mode, is dependent on the value.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Headers.
//////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <limits.h>
#include <float.h>	// For float and double limits.

#if PLATFORM_UNIX
#include <unistd.h>
#include <dirent.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#endif

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <io.h>
#include <direct.h>
#include <malloc.h>
#define PATH_MAX MAX_PATH
#ifndef F_OK
#define F_OK 00
#endif
#ifndef R_OK
#define R_OK 04
#endif
typedef HRESULT (WINAPI *fnSHGetFolderPathW)(HWND hwnd, int nFolder, HANDLE hToken, DWORD dwFlags, LPWSTR pszPath);
#endif

#include "Blue.h"

#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/File/file.h"
#else
	#include "file.h"
#endif	// PATHS_IN_INCLUDES


//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////
#ifdef SYS_ENDIAN_BIG
	#ifdef SYS_ENDIAN_LITTLE
		#error SYS_ENDIAN_LITTLE & SYS_ENDIAN_BIG cannot both be defined!
	#else
		#define ENDIAN_CONSISTENT	(m_endian != LittleEndian)
	#endif // SYS_ENDIAN_LITTLE
#else
	#ifdef SYS_ENDIAN_LITTLE
		#define ENDIAN_CONSISTENT	(m_endian != BigEndian)
	#else
		#error SYS_ENDIAN_LITTLE or SYS_ENDIAN_BIG must be defined!
	#endif	// SYS_ENDIAN_LITTLE
#endif	// SYS_ENDIAN_BIG

// Defines what is whitespace to this module.
#define WHITE_SPACE	"\t \n"

// Granularity of callbacks in bytes.
#define MAX_CALLBACK_GRANULARITY_BYTES	1024

//////////////////////////////////////////////////////////////////////////////
// Module specific pragmas.
//////////////////////////////////////////////////////////////////////////////
#if defined(WIN32)
	#pragma warning (disable: 4127) // conditional expression is constant.
#endif

//////////////////////////////////////////////////////////////////////////////
// Module specific typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) variables.
//////////////////////////////////////////////////////////////////////////////

// Called on every read and write with
// the amount that is about to be
// processed.
RFile::CritiCall	RFile::ms_criticall		= nullptr;

// For hooking Open(char*, ...) calls.
RFile::OpenHook	RFile::ms_hOpen			= nullptr;
uintptr_t					RFile::ms_lOpenUser		= 0;
RFile::CloseHook	RFile::ms_hClose			= nullptr;
uintptr_t					RFile::ms_lCloseUser		= 0;

// Used to byte swap by Write().
uint8_t					RFile::ms_au8SwapBuf[RFILE_SWAP_SIZE];

// String description of endian (index by Endian enum).
static const char*	ms_apszEndian[]	=
	{
	"BigEndian",
	"NeutralEndian",
	"LittleEndian",
	};


#ifdef ALLOW_RFILE_REOPEN
	RList <RFile> RFile::ms_listOpen;	// List of open RFiles connected to
													// disk.
#endif	// ALLOW_RFILE_REOPEN

//////////////////////////////////////////////////////////////////////////////
// Construction/Destruction.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Default constructor.
//
//////////////////////////////////////////////////////////////////////////////
RFile::RFile(void)
	{
	m_fs				= nullptr;
	m_endian			= BigEndian;
	m_flags			= NoFlags;
	m_pucFile		= nullptr;
	m_sOwnMem		= FALSE;
	m_pucCur			= nullptr;
   m_lSize			= 0;
   m_lGrowSize		= 0;
	m_sMemError		= 0;
	m_sOpenSem		= 0;
	m_sCloseSem		= 0;
	m_lUser			= 0;
	m_pfileSynch	= nullptr;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
//////////////////////////////////////////////////////////////////////////////
RFile::~RFile(void)
	{
	if (m_fs != nullptr || m_pucFile != nullptr)
		{
		Close();
		TRACE("~RFile(): Closed the file that you forgot to, hoser!\n");
		}
	}

//////////////////////////////////////////////////////////////////////////////
// Methods.
//////////////////////////////////////////////////////////////////////////////

#if PLATFORM_UNIX
// this is from PhysicsFS originally ( http://icculus.org/physfs/ )
//  (also zlib-licensed.)
static int locateOneElement(char *buf)
{
	char *ptr = nullptr;
	DIR *dirp = nullptr;
	struct dirent *dent = nullptr;

	if (access(buf, F_OK) == 0)
		return 1;  /* quick rejection: exists in current case. */

	ptr = strrchr(buf, '/');  /* find entry at end of path. */
	if (ptr == nullptr)
	{
		dirp = opendir(".");
		ptr = buf;
	}
	else
	{
		*ptr = '\0';
		dirp = opendir(buf);
		*ptr = '/';
		ptr++;  /* point past dirsep to entry itself. */
	}

	while ((dent = readdir(dirp)) != nullptr)
	{
		if (strcasecmp(dent->d_name, ptr) == 0)
		{
			strcpy(ptr, dent->d_name); /* found a match. Overwrite with this case. */
			closedir(dirp);
			return 1;
		}
	}

	/* no match at all... */
	closedir(dirp);
	return 0;
}
#endif

static void locateCorrectCase(char *buf)
{
#if PLATFORM_UNIX
	char *ptr = buf;
//	char *prevptr = buf;

   while ((ptr = strchr(ptr + 1, '/')))
	{
		*ptr = '\0';  /* block this path section off */
		if (!locateOneElement(buf))
		{
			*ptr = '/'; /* restore path separator */
			return;  /* missing element in path. */
		}
		*ptr = '/'; /* restore path separator */
	}

	/* check final element... */
	locateOneElement(buf);
#endif
}


extern const char *FindCorrectFile(const char *_pszName, const char *pszMode)
{
    char *pszName = (char *) alloca(strlen(_pszName) + 1);
    strcpy(pszName, _pszName);

    static bool initialized = false;
    static bool nohomedir = false;
    static char prefpath[PATH_MAX];
    if (!initialized)
    {
        TRACE("FindCorrectFile initializing...\n");
        if (rspCommandLine("nohomedir"))
        {
            TRACE("--nohomedir is on the command line.\n");
            nohomedir = true;
        }
        else
        {
            #ifdef WIN32
            /*
             * Vista and later has a new API for this, but SHGetFolderPath works there,
             *  and apparently just wraps the new API. This is the new way to do it:
             *
             *     SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_CREATE,
             *                          nullptr, &wszPath);
             */
            strcpy(prefpath, ".\\");  // a default for failure case.

            HMODULE lib = LoadLibraryA("Shell32.dll");
            if (lib != nullptr)
            {
                fnSHGetFolderPathW pSHGetFolderPathW = (fnSHGetFolderPathW) GetProcAddress(lib, "SHGetFolderPathW");
                if (pSHGetFolderPathW != nullptr)
                {
        			WCHAR path[MAX_PATH];
                    if (SUCCEEDED(pSHGetFolderPathW(nullptr, 0x001a/*CSIDL_APPDATA*/ | 0x8000/*CSIDL_FLAG_CREATE*/, nullptr, 0, path)))
                    {
                        // !!! FIXME: screwed if there's a unicode path for now.
                        snprintf(prefpath, sizeof (prefpath), "%S\\RunningWithScissors", (const wchar_t *) path);
                        mkdir(prefpath);
                        snprintf(prefpath, sizeof (prefpath), "%S\\RunningWithScissors\\Postal Plus", (const wchar_t *) path);
                        mkdir(prefpath);
                        snprintf(prefpath, sizeof (prefpath), "%S\\RunningWithScissors\\Postal Plus\\", (const wchar_t *) path);
                    }
                }
                FreeLibrary(lib);
            }
            #elif defined(PLATFORM_MACOSX)
            const char *homedir = getenv("HOME");
            if ( (!homedir) || ((strlen(homedir) + 32) >= sizeof (prefpath)) )
                homedir = "./";  // oh well.

            strcpy(prefpath, homedir);
            if (prefpath[strlen(prefpath)FAILURE] != '/') strcat(prefpath, "/");

            strcat(prefpath, "Library/Application Support/Postal Plus/");
            #else
            const char *homedir = getenv("HOME");
            const char *xdghomedir = getenv("XDG_DATA_HOME");
            const char *append = "";

            if (xdghomedir == nullptr)
            {
                if (homedir == nullptr)
                    xdghomedir = ".";  // oh well.
                else
                {
                    xdghomedir = homedir;
                    append = "/.local/share";
                }
            }

            snprintf(prefpath, sizeof (prefpath), "%s%s/PostalPlus/", xdghomedir, append);

            if (homedir != nullptr)
            {
                char oldpath[PATH_MAX];
                snprintf(oldpath, sizeof (oldpath), "%s/.postal1", homedir);
                if (access(oldpath, F_OK) == 0)
                {
                    TRACE("using oldschool prefpath at \"%s\"\n", oldpath);
                    snprintf(prefpath, sizeof (prefpath), "%s/", oldpath);
                }
            }

            // try to make sure the dirs exist...
            for (char *i = prefpath; *i; i++)
            {
                if (*i == '/')
                {
                    *i = '\0';
                    mkdir(prefpath, 0700);
                    *i = '/';
                }
            }
            mkdir(prefpath, 0700);
            #endif

            TRACE("prefpath is \"%s\"\n", prefpath);
        }
        initialized = true;
    }

    static char finalname[PATH_MAX];
    static bool bail_early = true;

    if (nohomedir)
        strcpy(finalname, pszName);

    else if ((strlen(pszName) + strlen(prefpath)) > sizeof (finalname))
        strcpy(finalname, pszName); // oh well.

    else
    {
        bail_early = false;
        sprintf(finalname, "%s%s", prefpath, pszName);
    }

    locateCorrectCase(finalname);

    if (bail_early)  // don't choose between prefpath and basedir?
        return(finalname);

    // writing? Always use prefpath.
    if (strcspn(pszMode, "aAwW+") < strlen(pszMode))
    {
        // build directories...
        for (char *ptr = finalname; *ptr; ptr++)
        {
            if (((*ptr == '/') || (*ptr == '\\')) && (ptr != finalname))
            {
                *ptr = '\0';
                if (access(finalname, F_OK) == FAILURE)
                {
                    TRACE("Making directory \"%s\"\n", finalname);
                    #ifdef WIN32
                    mkdir(finalname);
                    #else
                    mkdir(finalname, S_IRWXU);
                    #endif
                }
                *ptr = '/';
            }
        }

        // read AND write.  :/   Copy the file if it's not there.
        if ((strchr(pszMode, '+')) && (access(finalname, F_OK) == FAILURE))
        {
            FILE *in = fopen(pszName, "rb");
            FILE *out = fopen(finalname, "wb");
            if (in && out)
            {
                int ch = 0;
                while (1)  // !!! FIXME: this is really lame.
                {
                    ch = fgetc(in);
                    if (ch == EOF) break;
                    fputc(ch, out);
                }
            }
            if (in) fclose(in);
            if (out) fclose(out);
        }

        return finalname;
    }

    else  // reading.
    {
        if (access(finalname, R_OK) == FAILURE)  // favor prefpath?
        {
            strcpy(finalname, pszName); // nope, use original name.
            locateCorrectCase(finalname);
        }
    }

    return finalname;
}


//////////////////////////////////////////////////////////////////////////////
//
// Open file pszFileName with fopen flags pszFlags and endian format endian
// { BigEndian, LittleEndian }.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
int16_t RFile::Open(		// Returns 0 on success.
	const char* pszFileName,	// Filename to open.
	const char* pszFlags,		// fopen flags to use for opening.
	Endian endian,			// { BigEndian | LittleEndian | NeutralEndian }.
	Flags flags)			// See comments in Typedefs & Enums section in .h.
	{
   int16_t sResult	= SUCCESS;	// Assume success.

	// If not already open . . .
	if (m_fs == nullptr && m_pucFile == nullptr)
		{
		int16_t sOpen	= TRUE;

		// Store flags for this file.
		// Make sure Ascii and Binary are not both specified.
		ASSERT( (flags & (Ascii | Binary)) != (Ascii | Binary) );
		// Make sure either Ascii or Binary is specified.
		ASSERT( (flags & (Ascii | Binary)) != 0);

		m_flags	= flags;
		
		// If hook defined . . .
		if (ms_hOpen != nullptr)
			{
			// If not re-entered . . .
			if (m_sOpenSem == 0)
				{
				m_sOpenSem++;
				
				sOpen = (*ms_hOpen)(this, pszFileName, pszFlags, endian, ms_lOpenUser);
				
				m_sOpenSem--;
				}
			}

		// If no hook or hook told use to continue as normal . . .
		if (sOpen == TRUE)
			{
			// Set endian type.
			SetEndian(endian);

			// Attempt to open file.
			m_fs	= fopen(FindCorrectFile(pszFileName, pszFlags), pszFlags);
			// If successful . . .
			if (m_fs != nullptr)
				{
				// Attempt to set a better buffer size
				int setres = 0;
				#if 0
					size_t bufsize = BUFSIZ;
					if (bufsize < 16384)
						bufsize = (16384 / BUFSIZ) * BUFSIZ;
					setres = setvbuf(m_fs, nullptr, _IOFBF, bufsize);
				#endif
				if (setres == 0)
					{
					// Success.
					#ifdef ALLOW_RFILE_REOPEN
						// Store file flags.
						ASSERT(strlen(pszFlags) < sizeof(m_szFlags));
						strcpy(m_szFlags, pszFlags);
						
						// Store file name.
						ASSERT(strlen(pszFileName) < sizeof(m_szFileName));
						strcpy(m_szFileName, pszFileName);

						// Update access.
						m_lLastAccess		= Blu_GetTime();
						// Connected.
						m_sDisconnected	= FALSE;

						// Add to open list.
                  if (ms_listOpen.Add(this) == SUCCESS)
							{
							}
						else
							{
							TRACE("Open(\"%s\", \"%s\", %s): Unable to add to open list.\n",
									pszFileName, pszFlags, 
									ms_apszEndian[endian]);
                     sResult = FAILURE - 2;
							}
					#endif // ALLOW_RFILE_REOPEN
					}
				else
					{
					TRACE("Open(\"%s\", \"%s\", %s): Error returned by setvbuf()!\n",
							pszFileName, pszFlags, 
							ms_apszEndian[endian]);
               sResult = FAILURE - 4; // Is there any REAL sense to these error numbers???
					}

				// If an error occurrs after fopen . . .
            if (sResult != SUCCESS)
					{
					fclose(m_fs);
					m_fs	= nullptr;
					}
				}
			else
				{
#ifdef FILE_VERBOSE
				TRACE("Open(\"%s\", \"%s\", %s): Unable to open file.\n",
						pszFileName, pszFlags, 
						ms_apszEndian[endian]);
#endif // FILE_VERBOSE
            sResult = FAILURE;
				}
			}
		}
	else
		{
		TRACE("Open(\"%s\", \"%s\", %s): File already open.\n",
				pszFileName, pszFlags,
				ms_apszEndian[endian]);
      sResult = FAILURE - 1;
		}

   return sResult;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Open memory pFile of size lSize and endian format endian
// { BigEndian, LittleEndian }.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
int16_t RFile::Open(		// Returns 0 on success.
	void* pFile,			// Pointer to memory to open.
   size_t lSize,				// Size of *pFile in bytes.
	Endian endian)			// { BigEndian | LittleEndian | NeutralEndian }.
	{
   int16_t sResult	= SUCCESS;	// Assume success.

	// If not already open . . .
	if (m_fs == nullptr && m_pucFile == nullptr)
		{
		// Store flags for this file.
		m_flags	= Binary;
		
		// Set endian type.
		SetEndian(endian);

		// Open memory.
		m_pucFile	= m_pucCur	= (uint8_t*)pFile;
		// Do not own buffer.
		m_sOwnMem	= FALSE;

		// Set size of file.
		m_lSize		= lSize;
		// Clear error flag.
		m_sMemError	= 0;
		}
	else
		{
		TRACE("Open(%08lX, %i, %s): File already open.\n",
				pFile, lSize,
				ms_apszEndian[endian]);
      sResult = FAILURE - 1;
		}

   return sResult;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Open memory pFile of size lSize and endian format endian.
// RFile may size and or relocate the memory in order to expand the memory file.
// Deallocates on Close().
// { RFile::Big, RFile::Little }.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
int16_t RFile::Open(	// Returns 0 on success.
   size_t	lSize,		// Size in bytes to begin with.
   size_t	lGrowSize,	// Min amount in bytes to grow memory file when written passed end.
							// Note: The larger of lGrowSize and the amount overwritten will
							// be allocated in the case of an overrun.
	Endian endian)		// { BigEndian | LittleEndian | NeutralEndian }.
	{
   int16_t sResult	= SUCCESS;	// Assume success.

	// If not already open . . .
	if (m_fs == nullptr && m_pucFile == nullptr)
		{
		// Store flags for this file.
		m_flags	= Binary;
		
		// Set endian type.
		SetEndian(endian);

		// Open memory.
		m_pucFile	= m_pucCur	= (uint8_t*)malloc(lSize);
		if (m_pucFile != nullptr)
			{
			// Do own buffer.
			m_sOwnMem	= TRUE;

			// Set size of file.
			m_lSize		= lSize;
			// Set minimum grow size for overwrites.
			m_lGrowSize	= lGrowSize;
			// Clear error flag.
			m_sMemError	= 0;
			}
		else
			{
			TRACE("Open(%i, %i, %s): File too large for free memory.\n",
					lSize, lGrowSize,
					ms_apszEndian[endian]);
         sResult = FAILURE - 2;
			}
		}
	else
		{
		TRACE("Open(%i, %i, %s): File already open.\n",
				lSize, lGrowSize,
				ms_apszEndian[endian]);
      sResult = FAILURE - 1;
		}

   return sResult;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Open an existing FILE* stream.
// Once a FILE* is opened, you can use this class's Close() instead of fclose(),
// if that is more convenient.
//
//////////////////////////////////////////////////////////////////////////////
int16_t RFile::Open(		// Returns 0 on success.
	FILE*	fs,				// FILE* stream to open.
	Endian endian,			// { RFile::BigEndian | RFile::LittleEndian | RFile::NeutralEndian }.
	Flags flags	/*=		// See comments in Typedefs & Enums section 
		Binary*/)			// above.
	{
   int16_t sResult	= SUCCESS;	// Assume success.

	if (fs != nullptr)
		{
		// Store flags for this file.
		// Make sure Ascii and Binary are not both specified.
		ASSERT( (flags & (Ascii | Binary)) != (Ascii | Binary) );
		// Make sure either Ascii or Binary is specified.
		ASSERT( (flags & (Ascii | Binary)) != 0);

		m_flags	= flags;
		
		// Set endian.
		SetEndian(endian);

		// Store FILE* stream.
		m_fs	= fs;
		}
	else
		{
		TRACE("Open(0x%08lX, %s): Invalid FILE*.\n",
				fs,
				ms_apszEndian[endian]);
      sResult	= FAILURE;
		}

   return sResult;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Open an existing RFile.
// "Ack!" you say ... and I agree.
// This basically begins what I like to think of as a synchronization between two RFiles.
// This RFile snags the current state (basically copies the members) from the specified
// RFile.  Then, one can use the new RFile to access the file/memory/whatever that the
// original RFile is attached to.  When Close() is called, the synchronization is finsished
// by updating the original RFile with the state from this.
// Danger:  Do not access the original RFile between Open(RFile*)/Close() pairs!
//
//////////////////////////////////////////////////////////////////////////////
int16_t RFile::Open(		// Returns 0 on success.
	RFile* pfile)			// RFile to open.
	{
   int16_t sResult	= SUCCESS;

	// Synchronize.
	m_fs				= pfile->m_fs;
	m_pucFile		= pfile->m_pucFile;
	m_sOwnMem		= pfile->m_sOwnMem;
	m_pucCur			= pfile->m_pucCur;
	m_lSize			= pfile->m_lSize;
	m_lGrowSize		= pfile->m_lGrowSize;
					
	m_sMemError		= pfile->m_sMemError;
	m_endian			= pfile->m_endian;
	m_flags			= pfile->m_flags;

	// Remember who to Unsynch with.
	m_pfileSynch	= pfile;

   return sResult;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Change the endian format used to read/write the file.
// Returns nothing.
//
//////////////////////////////////////////////////////////////////////////////
void RFile::SetEndian(Endian endian)
	{	
	ASSERT(endian == BigEndian || endian == NeutralEndian || endian == LittleEndian);
	m_endian = endian;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Close a file successfully opened with Open().
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
int16_t RFile::Close(void)
	{
   int16_t sResult	= SUCCESS;	// Assume success.

	ASSERT(m_fs != nullptr || m_pucFile != nullptr);

	// If we are synchronizing with another RFile . . .
	if (m_pfileSynch != nullptr)
		{
		// De/Un/Resynch.
		m_pfileSynch->m_fs				= m_fs;
		m_pfileSynch->m_pucFile			= m_pucFile;
		m_pfileSynch->m_sOwnMem			= m_sOwnMem;
		m_pfileSynch->m_pucCur			= m_pucCur;
		m_pfileSynch->m_lSize			= m_lSize;
		m_pfileSynch->m_lGrowSize		= m_lGrowSize;
						
		m_pfileSynch->m_sMemError		= m_sMemError;
		m_pfileSynch->m_endian			= m_endian;
		m_pfileSynch->m_flags			= m_flags;

		// Clear ("Close").
		m_fs				= nullptr;
		m_pucFile		= nullptr;
		m_sOwnMem		= FALSE;
		m_pucCur			= nullptr;
		m_sMemError		= FALSE;

		m_pfileSynch	= nullptr;
		}
	else
		{
		int16_t sClose	= TRUE;
		
		// If hook defined . . .
		if (ms_hClose != nullptr)
			{
			// If not re-entered . . .
			if (m_sCloseSem == 0)
				{
				m_sCloseSem++;
				
				sClose = (*ms_hClose)(this, ms_lCloseUser);
				
				m_sCloseSem--;
				}
			}

		// If no hook or hook told us to perform business as usual . . .
		if (sClose == TRUE)
			{
			if (IsFile() == TRUE)
				{
				KEEPCONNECTEDANDUPDATELASTACCESS;
				if (fclose(m_fs) == 0)
					{
					// Success.
					m_fs	= nullptr;
					
					#ifdef ALLOW_RFILE_REOPEN
						// Remove from open list.
                  if (ms_listOpen.Remove(this) == SUCCESS)
							{
							}
						else
							{
							TRACE("Close(): Unable to remove this from open list.\n");
                     sResult = FAILURE - 2;
							}
					#endif // ALLLOW_RFILE_REOPEN
					}
				else
					{
					TRACE("Close(): Unable to close file.\n");
               sResult = FAILURE - 1;
					}
				}
			else
				{
				if (IsMemory() == TRUE)
					{
					// If we own the buffer . . .
					if (m_sOwnMem != FALSE)
						{
						// Make sure we still have a buffer (a resize could have failed).
						if (m_pucFile != nullptr)
							{
							// Be gone.
							free(m_pucFile);
							}
						}

					m_pucFile	= nullptr;
					m_sOwnMem	= FALSE;
					m_pucCur		= nullptr;
               m_lSize		= 0;
               m_lGrowSize	= 0;
					m_sMemError	= 0;
					}
				else
					{
					TRACE("Close(): Unable to close unopened file.\n");
               sResult = FAILURE;
					}
				}
			}
		}

   return sResult;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Reads sNum bytes from currently open file.
// Returns number of bytes successfully read.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Read(void* pData, size_t lNum)
	{
	int32_t	lRes	= 0;	// Assume success.

	if (IsFile() == TRUE)
		{
      size_t	lToRead;
      size_t	lDidRead	= 1;
      size_t	lMaxRead	= ms_criticall ? MAX_CALLBACK_GRANULARITY_BYTES : lNum;
		do
			{
			lToRead	= MIN(lMaxRead, lNum);

			// If there is a CritiCall . . .
			if (ms_criticall != nullptr)
				{
				// Call it.
				(*ms_criticall)(lToRead);
				}

			KEEPCONNECTEDANDUPDATELASTACCESS;
			lDidRead = fread(pData, 1, lToRead, m_fs);

			if (lDidRead > 0)
				{
				lRes	+= lDidRead;
				lNum	-= lDidRead;
				pData = (uint8_t*)pData + lDidRead;
				}

			} while (lNum > 0 && lDidRead > 0);
		}
	else
		{
		if (IsMemory() == TRUE)
			{
			if (m_pucCur + lNum <= m_pucFile + m_lSize)
				{
				lRes = lNum;
				}
			else
				{
				lRes = (m_pucFile + m_lSize) - m_pucCur;
//				TRACE("Read(): Attempt to read passed end of memory file.\n");
				m_sMemError	= 1;
				}
			
			// "Read" data.
			memcpy(pData, m_pucCur, lRes);

			// Move file ptr.
			m_pucCur += lRes;
			}
		else
			{
			TRACE("Read(): File not open.\n");
         lRes = FAILURE;
			}
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Attempts to read lNum TYPE numbers from pfsIn or pu8In.
//
//////////////////////////////////////////////////////////////////////////////
template <
	class TYPE>			// Type for storing and overflow checking.
inline					// Speed.
int32_t ReadASCII(		// Returns number of complete TYPE items successfully read 
							// and stored.
	TYPE*		ptData,	// Out: Pointer to array of TYPE items for read data.
   size_t		lNum,		// In:  Number of TYPE items to read.
	FILE*		pfsIn,	// In:  File stream to use for input.
	double	dMax)		// In:  Maximum value for this type.
	{
	int32_t	lRes	= 0;	// Assume success.

	// Temp var to read into.
	double	dTemp;
	while (lNum--)
		{
		if (fscanf(pfsIn, " %le", &dTemp) == 1)
			{
			// Successfully read an item.
			
			// Check size . . .
			if (ABS(dTemp) < dMax)
				{
				// Within range.  Store.
				*ptData++	= (TYPE)dTemp;
				// Increment number stored.
				lRes++;
				}
			else
				{
				TRACE("ReadASCII(): Read %le which does not fit into item of size %u bits.\n",
					dTemp, sizeof(TYPE) * 8);
				break;
				}
			}
		else
			{
			TRACE("ReadASCII(): Failed to read numeric of size %u bits.\n",
				sizeof(TYPE) * 8);
			break;
			}
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Reads lNum uint8_t values from currently open file.
// Returns number of uint8_t values successfully written.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Read(uint8_t*	pu8Data, size_t lNum /*= 1*/)
	{
   int32_t	lRes	= 0;	// Assume success.

	if ((m_flags & Ascii) != 0)
		{
		// Read ASCII data.
		lRes = ReadASCII(pu8Data, lNum, m_fs, (double)UCHAR_MAX);
		}
	else
		{
		// Read Binary data.
		lRes	= Read8(pu8Data, lNum);
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Reads lNum int8_t values from currently open file.
// Returns number of int8_t values successfully written.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Read(int8_t*	ps8Data, size_t lNum /*= 1*/)
	{
   int32_t	lRes	= 0;	// Assume success.

	if ((m_flags & Ascii) != 0)
		{
		// Read ASCII data.
		lRes = ReadASCII(ps8Data, lNum, m_fs, (double)CHAR_MAX);
		}
	else
		{
		// Read Binary data.
		lRes	= Read8((uint8_t*)ps8Data, lNum);
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Reads sNum uint16_t values from currently open file.
// Returns number of uint16_t values successfully read.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Read(uint16_t* pu16Data, size_t lNum /*= 1*/)
	{
   int32_t	lRes	= 0;	// Assume success.

	if ((m_flags & Ascii) != 0)
		{
		// Read ASCII data.
		lRes = ReadASCII(pu16Data, lNum, m_fs, (double)USHRT_MAX);
		}
	else
		{
		// Read Binary data.
		lRes	= Read16(pu16Data, lNum);
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Reads lNum int16_t values from currently open file.
// Returns number of int16_t values successfully read.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Read(int16_t* ps16Data, size_t lNum /*= 1*/)
	{
   int32_t	lRes	= 0;	// Assume success.

	if ((m_flags & Ascii) != 0)
		{
		// Read ASCII data.
		lRes = ReadASCII(ps16Data, lNum, m_fs, (double)SHRT_MAX);
		}
	else
		{
		// Read Binary data.
		lRes	= Read16((uint16_t*)ps16Data, lNum);
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Reads lNum RPixel24 values from currently open file.
// Returns number of RPixel24 values successfully read.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Read(RPixel24* ppix24, size_t lNum /*= 1*/)
	{
   int32_t	lRes	= 0;	// Assume success.

	if ((m_flags & Ascii) != 0)
		{
		ASSERT(0 && "**Cannot read 24 bit types in ASCII**"); 
		// Read ASCII data.
//		lRes = ReadASCII(ppix24, lNum, m_fs, (double)(1 << 24));
		}
	else
		{
		// Read Binary data.
		lRes	= Read24(ppix24, lNum);
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Reads sNum uint32_t values from currently open file.
// Returns number of uint32_t values successfully read.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Read(uint32_t* pu32Data, size_t lNum /*= 1*/)
	{
   int32_t	lRes	= 0;	// Assume success.

	if ((m_flags & Ascii) != 0)
		{
		// Read ASCII data.
      lRes = ReadASCII(pu32Data, lNum, m_fs, (double)UINT32_MAX);
		}
	else
		{
		// Read Binary data.
		lRes	= Read32(pu32Data, lNum);
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Reads lNum int32_t values from currently open file.
// Returns number of int32_t values successfully read.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Read(int32_t* ps32Data, size_t lNum /*= 1*/)
	{
   int32_t	lRes	= 0;	// Assume success.

	if ((m_flags & Ascii) != 0)
		{
		// Read ASCII data.
      lRes = ReadASCII(ps32Data, lNum, m_fs, (double)INT32_MAX);
		}
	else
		{
		// Read Binary data.
		lRes	= Read32((uint32_t*)ps32Data, lNum);
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Reads lNum uint64_t values from currently open file.
// Returns number of uint64_t values successfully read.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Read(uint64_t* pu64Data, size_t lNum /*= 1*/)
	{
   int32_t	lRes	= 0;	// Assume success.

	if ((m_flags & Ascii) != 0)
		{
		ASSERT(0 && "**Cannot read 64 bit types other than double in ASCII**"); 
		}
	else
		{
		// Read Binary data.
		lRes	= Read64(pu64Data, lNum);
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Reads lNum int64_t values from currently open file.
// Returns number of int64_t values successfully read.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Read(int64_t* ps64Data, size_t lNum /*= 1*/)
	{
   int32_t	lRes	= 0;	// Assume success.

	if ((m_flags & Ascii) != 0)
		{
		ASSERT(0 && "**Cannot read 64 bit types other than double in ASCII**"); 
		}
	else
		{
		// Read Binary data.
		lRes	= Read64((uint64_t*)ps64Data, lNum);
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Reads lNum float values from currently open file.
// Returns number of float values successfully read.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Read(float* pfData, size_t lNum /*= 1*/)
	{
   int32_t	lRes	= 0;	// Assume success.

	if ((m_flags & Ascii) != 0)
		{
		// Read ASCII data.
		lRes = ReadASCII(pfData, lNum, m_fs, (double)FLT_MAX);
		}
	else
		{
		// Read Binary data.
		lRes	= Read32((uint32_t*)pfData, lNum);
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Reads lNum double values from currently open file.
// Returns number of double values successfully read.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Read(double* pdData, size_t lNum /*= 1*/)
	{
   int32_t	lRes	= 0;	// Assume success.

	if ((m_flags & Ascii) != 0)
		{
		// Read ASCII data.
		lRes = ReadASCII(pdData, lNum, m_fs, DBL_MAX);
		}
	else
		{
		// Read Binary data.
		lRes	= Read64((uint64_t*)pdData, lNum);
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
// NOTE that this function has very different functionality in each of the
// two Open() modes (Ascii & Binary).
//////////////////////////////////////////////////////////////////////////////
//
// ASCII MODE:
// Reads a quotes or white-space delimited string.  Quotes preceded by back- 
// slash are ignored and are stored as just quotes (without the backslash).  
// Strings are either whitespace or quotes delimited (e.g., a string beginning
// with quotes can only be ended by quotes, and a string beginning without
// quotes can only be ended by whitespace).
// Returns number of characters successfully stored (which may be less than
// the amount read), NOT including the nullptr terminator (like strlen()).
//
// BINARY MODE:
// Reads a nullptr terminated string.
// Returns number of characters successfully read,
// including the nullptr terminator (UNlike strlen()).
//
//
// pszString must point to a memory block sufficiently large
// enough to hold the string.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Read(char* pszString)
	{
	int32_t	lRes	= 0;	// Assume nothing.

	if (m_flags & Binary)
		{
		do
			{
			// Read one character.
			if (Read(pszString, 1) == 1)
				{
				}
			else
				{
				TRACE("Read(): Failed to read to nullptr terminator of string.\n");
				*pszString	= '\0';
				}

			lRes++;
			
			// Do this until we hit the nullptr or an error occurs.
			} while (*pszString++ != '\0');
		}
	else
		{
		// Skip whitespace.
		do
			{
			if (Read(pszString, 1) == 1)
				{
				}
			else
				{
				break;
				}

			} while (strchr(WHITE_SPACE, *pszString++) == nullptr);

		// Note whether started by qutoes.
		int16_t	sInQuotes	= FALSE;
		if (*pszString == '"')
			{
			sInQuotes	= TRUE;
			}

		// Store string.
		int16_t	sLastCharWasBackSlash	= FALSE;
		while (*pszString != '\0')
			{
			if (Read(pszString, 1) == 1)
				{
				// Increment count.
				lRes++;

				// If previous was backslash . . .
				if (sLastCharWasBackSlash != FALSE)
					{
					// If current is quotes . . .
					if (*pszString == '"')
						{
						// Overwrite backslash.
						*pszString-- = '"';
						// Deduct from count.
						lRes--;
						}
					}

				switch (*pszString++)
					{
					case '\\':
						sLastCharWasBackSlash	= TRUE;
						break;
					case '"':
						// If this is quotes delimited . . .
						if (sInQuotes != FALSE && sLastCharWasBackSlash == FALSE)
							{
							// Done.
							*pszString	= '\0';
							}
						// Intentional fall through to default case.
					default:
						// Clear last was backslash flag.
						sLastCharWasBackSlash	= FALSE;
						// If this is whitespace delimited . . .
						if (sInQuotes == FALSE)
							{
							// If this is whitespace . . .
							if (strchr(WHITE_SPACE, *pszString) != nullptr)
								{
								// Done.
								*pszString	= '\0';
								}
							}
						break;
					}
				}
			else
				{
				// Done.
				*pszString	= '\0';
				}
			}
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Writes lNum bytes from currently open file.
// Returns number of bytes successfully written.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Write(const void* pData, size_t lNum)
	{
	int32_t	lRes	= 0;	// Assume success.

	if (IsFile() == TRUE)
		{
      size_t	lToWrite;
		int32_t	lDidWrite	= 1;
      size_t	lMaxWrite	= ms_criticall ? MAX_CALLBACK_GRANULARITY_BYTES : lNum;
		do
			{
			lToWrite	= MIN(lMaxWrite, lNum);

			// If there is a CritiCall . . .
			if (ms_criticall != nullptr)
				{
				// Call it.
				(*ms_criticall)(lNum);
				}

			KEEPCONNECTEDANDUPDATELASTACCESS;
			lDidWrite = fwrite(pData, 1, lToWrite, m_fs);

			if (lDidWrite > 0)
				{
				lRes	+= lDidWrite;
				lNum	-= lDidWrite;
				pData = (uint8_t*)pData + lDidWrite;
				}

			} while (lDidWrite > 0 && lNum > 0);
		}
	else
		{
		if (IsMemory() == TRUE)
			{
			// If still within buffer . . .
			if (m_pucCur + lNum <= m_pucFile + m_lSize)
				{
				lRes = lNum;
				}
			else
				{
				// If we don't own the bufer . . .
				if (m_sOwnMem == FALSE)
					{
					// Unlike normal files, our file can't get bigger.
					// Our space is limited by the original value of m_lSize.
					lRes = (m_pucFile + m_lSize) - m_pucCur;
					TRACE("Write(): Attempt to write passed end of memory file.\n");
					m_sMemError	= 1;
					}
				else
					{
					// Attempt to enlarge the buffer by the max of m_lGrowSize or the
					// overrun amount.
					int32_t	lCurPos			= m_pucCur - m_pucFile;
					int32_t	lDistanceToEOF	= m_lSize - lCurPos;
               size_t	lNewSize			= m_lSize + MAX(m_lGrowSize, (lNum - lDistanceToEOF) );

					// Enlarge . . .
					uint8_t*	pucNewFile	= (uint8_t*)realloc(m_pucFile, lNewSize);
					// If successful . . .
					if (pucNewFile != nullptr)
						{
						// Set new buffer pointer.
						m_pucFile	= pucNewFile;
						// Set current position.
						m_pucCur		= pucNewFile + lCurPos;
						// Set new buffer size.
						m_lSize		= lNewSize;
						// Write amount requested.
						lRes			= lNum;
						}
					else
						{
						// Only write what we can.  Keep same buffer.
						lRes	= lDistanceToEOF;
						}
					}
				}
		
			// "Write" data.
			memcpy(m_pucCur, pData, lRes);

			// Move file ptr.
			m_pucCur += lRes;
			}
		else
			{
			TRACE("Write(): File not open.\n");
         lRes = FAILURE;
			}
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Attempts to write lNum TYPE numbers from pfsIn or pu8In.
//
//////////////////////////////////////////////////////////////////////////////
template <
	class TYPE>			// Type for storing and overflow checking.
inline					// Speed.
int32_t WriteASCII(		// Returns number of complete TYPE items successfully 
							// written.
	TYPE*		ptData,	// In:  Pointer to array of TYPE items for write data.
	int32_t		lNum,		// In:  Number of TYPE items to read.
	FILE*		pfsOut,	// In:  File stream to use for output.
   const char*		pszFrmt)	// In:  Output printf style format specifier.
	{
	int32_t	lRes	= 0;	// Assume success.

	while (lNum--)
		{
		if (fprintf(pfsOut, pszFrmt, *ptData++) > 0)
			{
			// Successfully wrote an item.
			
			// Increment number written.
			lRes++;
			}
		else
			{
			TRACE("WriteASCII(): Failed to write numeric of size %u bits.\n",
				sizeof(TYPE) * 8);
			break;
			}
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Writes lNum uint8_t values to currently open file.
// Returns number of uint8_t values successfully written.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Write(const uint8_t*	pu8Data, size_t lNum /*= 1*/)
	{
   int32_t	lRes	= 0;	// Assume success.

	if ((m_flags & Ascii) != 0)
		{
		// Write ASCII data.
		lRes = WriteASCII(pu8Data, lNum, m_fs, "%u ");
		}
	else
		{
		// Write Binary data.
		lRes	= Write8(pu8Data, lNum);
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Writes lNum int8_t values to currently open file.
// Returns number of int8_t values successfully written.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Write(const int8_t*	ps8Data, size_t lNum /*= 1*/)
	{
   int32_t	lRes	= 0;	// Assume success.

	if ((m_flags & Ascii) != 0)
		{
		// Write ASCII data.
		lRes = WriteASCII(ps8Data, lNum, m_fs, "%d ");
		}
	else
		{
		// Write Binary data.
		lRes	= Write8((uint8_t*)ps8Data, lNum);
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Writes sNum uint16_t values from currently open file.
// Returns number of uint16_t values successfully written.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Write(const uint16_t* pu16Data, size_t lNum /*= 1*/)
	{
   int32_t	lRes	= 0;	// Assume success.

	if ((m_flags & Ascii) != 0)
		{
		// Write ASCII data.
		lRes = WriteASCII(pu16Data, lNum, m_fs, "%u ");
		}
	else
		{
		// Write Binary data.
		lRes	= Write16(pu16Data, lNum);
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Writes lNum int16_t values to currently open file.
// Returns number of int16_t values successfully written.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Write(const int16_t* ps16Data, size_t lNum /*= 1*/)
	{
   int32_t	lRes	= 0;	// Assume success.

	if ((m_flags & Ascii) != 0)
		{
		// Write ASCII data.
		lRes = WriteASCII(ps16Data, lNum, m_fs, "%d ");
		}
	else
		{
		// Write Binary data.
		lRes	= Write16((uint16_t*)ps16Data, lNum);
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Writes lNum RPixel24 values to currently open file.
// Returns number of RPixel24 values successfully written.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Write(const RPixel24* ppix24, size_t lNum /*= 1*/)
	{
	int32_t	lRes	= 0;	// Assume success.

	if ((m_flags & Ascii) != 0)
		{
		ASSERT(0 && "**Cannot write 24 bit types in ASCII**"); 
		// Write ASCII data.
//		lRes = WriteASCII(ppix24, lNum, m_fs, "%06lX ");
		}
	else
		{
		// Write Binary data.
		lRes	= Write24(ppix24, lNum);
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Writes sNum uint32_t values from currently open file.
// Returns number of uint32_t values successfully written.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Write(const uint32_t* pu32Data, size_t lNum /*= 1*/)
	{
	int32_t	lRes	= 0;	// Assume success.

	if ((m_flags & Ascii) != 0)
		{
		// Write ASCII data.
		lRes = WriteASCII(pu32Data, lNum, m_fs, "%lu ");
		}
	else
		{
		// Write Binary data.
		lRes	= Write32(pu32Data, lNum);
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Writes lNum int32_t values to currently open file.
// Returns number of int32_t values successfully written.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Write(const int32_t* ps32Data, size_t lNum /*= 1*/)
	{
	int32_t	lRes	= 0;	// Assume success.

	if ((m_flags & Ascii) != 0)
		{
		// Write ASCII data.
		lRes = WriteASCII(ps32Data, lNum, m_fs, "%i ");
		}
	else
		{
		// Write Binary data.
		lRes	= Write32((uint32_t*)ps32Data, lNum);
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Writes lNum float values to the currently open file.
// Returns number of float values successfully written.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Write(const float* pfData, size_t lNum /*= 1*/)
	{
	int32_t	lRes	= 0;	// Assume success.

	if ((m_flags & Ascii) != 0)
		{
		// Write ASCII data.
		lRes = WriteASCII(pfData, lNum, m_fs, "%e ");
		}
	else
		{
		// Write Binary data.
		lRes	= Write32((uint32_t*)pfData, lNum);
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Writes lNum double values to the currently open file.
// Returns number of double values successfully written.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Write(const double* pdData, size_t lNum /*= 1*/)
	{
	int32_t	lRes	= 0;	// Assume success.

	if ((m_flags & Ascii) != 0)
		{
		// Write ASCII data.
		lRes = WriteASCII(pdData, lNum, m_fs, "%le ");
		}
	else
		{
		// Write Binary data.
		lRes	= Write64((uint64_t*)pdData, lNum);
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Writes lNum uint64_t values to currently open file.
// Returns number of uint64_t values successfully written.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Write(const uint64_t* pu64Data, size_t lNum /*= 1*/)
	{
	int32_t	lRes	= 0;	// Assume success.

	if ((m_flags & Ascii) != 0)
		{
		ASSERT(0 && "**Cannot write 64 bit types other than double in ASCII**"); 
		}
	else
		{
		// Write Binary data.
		lRes	= Write64(pu64Data, lNum);
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Writes lNum int64_t values to currently open file.
// Returns number of int64_t values successfully written.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Write(const int64_t* ps64Data, size_t lNum /*= 1*/)
	{
	int32_t	lRes	= 0;	// Assume success.

	if ((m_flags & Ascii) != 0)
		{
		ASSERT(0 && "**Cannot write 64 bit types other than double in ASCII**"); 
		}
	else
		{
		// Write Binary data.
		lRes	= Write64((uint64_t*)ps64Data, lNum);
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
// NOTE that this function has very different functionality in each of the
// two Open() modes (Ascii & Binary).
//////////////////////////////////////////////////////////////////////////////
//
// ASCII MODE:
// Writes a nullptr terminated string.  The string is surrounded by quotes in 
// the file.  Quotes in your string will be converted to \".
// Returns number of characters from your string successfully written (which 
// is less than the total amount written) (this value should be the same
// as that returned from strlen() on success).
//
// BINARY MODE:
// Writes a nullptr terminated string.
// Returns number of characters successfully written,
// including the nullptr terminator (UNlike strlen()).
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Write(const char* pszString)
	{
	int32_t	lRes	= 0;	// Assume nothing.
	
	if (m_flags & Binary)
		{
		lRes	= Write(pszString, strlen(pszString) + 1);
		}
	else
		{
		// Write leading quotes.
		if (Write("\"", 1) == 1)
			{
			while (*pszString != '\0')
				{
				switch (*pszString)
					{
					case '"':
						if (Write("\\", 1) == 1)
							{
							}
						break;
					}

				if (Write(pszString, 1) == 1)
					{
					lRes++;
					}
				else
					{
					break;
					}
				}
			
			// Write trailing quotes.
			Write("\"", 1);
			}
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Seeks within the file based on the supplied position argument
// { SEEK_SET, SEEK_CUR, SEEK_END }.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
int16_t RFile::Seek(ssize_t lPos, int32_t lOrigin)
   {
   int16_t sResult	= SUCCESS;	// Assume success.

	if (IsFile() == TRUE)
		{
		KEEPCONNECTEDANDUPDATELASTACCESS;
      sResult = fseek(m_fs, lPos, lOrigin);
		}
	else
		{
		if (IsMemory() == TRUE)
			{
			switch (lOrigin)
				{
				case SEEK_SET:
					// If w/i range . . .
               if (lPos >= 0 && size_t(lPos) <= m_lSize)
						{
						m_pucCur	= m_pucFile + lPos;
						}
					else
						{
						m_sMemError	= 1;
						TRACE("Seek(): Attempt to seek passed end or before beginning of mem file.\n");
                  sResult = FAILURE - 2;
						}
					break;
				case SEEK_CUR:
					// If w/i range . . .
					if (m_pucCur + lPos <= m_pucFile + m_lSize && m_pucCur + lPos >= m_pucFile)
						{
						m_pucCur	+= lPos;
						}
					else
						{
						m_sMemError	= 1;
						TRACE("Seek(): Attempt to seek passed end or before beginning of mem file.\n");
                  sResult = FAILURE - 2;
						}
					break;
				case SEEK_END:
               if (lPos <= 0 && size_t(0 - lPos) <= m_lSize)
						{
						m_pucCur = m_pucFile + m_lSize + lPos;
						}
					else
						{
						m_sMemError	= 1;
						TRACE("Seek(): Attempt to seek passed end or before beginning of mem file.\n");
                  sResult = FAILURE - 2;
						}
					break;
				default:
					TRACE("Seek(): Invalid origin flag provided.\n");
               sResult = FAILURE - 1;
					break;
				}
			}
		else
			{
			TRACE("Seek(): File not open.\n");
         sResult = FAILURE;
			}
		}

   return sResult;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Returns the current file position or FAILURE on error.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Tell(void)
	{
   int32_t lRes	= FAILURE;	// Assume error.
	if (IsFile() == TRUE)
		{
		KEEPCONNECTEDANDUPDATELASTACCESS;
		lRes = ftell(m_fs);
		}
	else
		{
		if (IsMemory() == TRUE)
			{
			lRes = m_pucCur - m_pucFile;
			}
		else
			{
			TRACE("Tell(): File not open.\n");
			}
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Returns the size of the file on success.  Negative on error.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::GetSize(void)
	{
	int32_t	lRes;

	// Remember the current position.
	int32_t	lPos	= Tell();
   if (lPos >= 0)
		{
		// Attempt to seek to the end . . .
      if (Seek(0, SEEK_END) == SUCCESS)
			{
			// Current pos is size of file.
			lRes = Tell();
			
			// Seek back.
         if (Seek(lPos, SEEK_SET) == SUCCESS)
				{
				// Success.
				}
			else
				{
				TRACE("GetSize(): Unable to seek back to start point.\n");
            lRes = FAILURE - 2;
				}
			}
		else
			{
			TRACE("GetSize(): Unable to seek to end of file.\n");
         lRes = FAILURE - 1;
			}
		}
	else
		{
		TRACE("GetSize(): Unable to tell position of file.\n");
      lRes = FAILURE;
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
// Internal Methods.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Reads in 8 bit data, swapped if necessary (BWAH HA).
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Read8(	// Returns number of 8 bit items read.
	uint8_t*	pu8,			// In:  8 bit data to read (swapping, if necessary).
   size_t lNum)			// In:  Number of 8 bit items to read.
	{
	int32_t	lRes	= 0;

	// Read data.
	lRes = Read((void*)pu8, lNum);

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Reads in 16 bit data, swapped if necessary.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Read16(	// Returns number of 16 bit items read.
	uint16_t*	pu16,			// In:  16 bit data to read (swapping, if necessary).
   size_t lNum)			// In:  Number of 16 bit items to read.
	{
	int32_t	lRes	= 0;

	// Read data.
	lRes = Read((void*)pu16, lNum * sizeof(uint16_t)) / sizeof(uint16_t);

	// If m_endian is consistent with this platform . . .
	if (ENDIAN_CONSISTENT)
		{
		// Done.
		}
	else
		{
		// Convert.
		uint8_t		u8_0;
		uint8_t*	pu8	= (uint8_t*)pu16;
      for (int32_t l = 0; l < lRes; l++, pu8 += sizeof(uint16_t))
			{
			// Store end.
			u8_0			= *(pu8 + 1);
			// Put beginning in end.
			*(pu8	+ 1)	= *(pu8 + 0);
			// Put end in beginning.
			*(pu8	+ 0)	= u8_0;
			}
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Reads in 24 bit data, swapped if necessary.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Read24(	// Returns number of 24 bit items read.
	RPixel24* ppix24,	// In:  24 bit data to read (swapping, if necessary).
   size_t lNum)			// In:  Number of 24 bit items to read.
	{
	int32_t	lRes	= 0;

	// Read data.
	lRes = Read((void*)ppix24, lNum * sizeof(RPixel24)) / sizeof(RPixel24);

	// If m_endian is consistent with this platform . . .
	if (ENDIAN_CONSISTENT)
		{
		// Done.
		}
	else
		{
		// Convert.
		uint8_t		u8_0;
		uint8_t*	pu8	= (uint8_t*)ppix24;
      for (int32_t l = 0; l < lRes; l++, pu8 += sizeof(RPixel24))
			{
			// Store end.
			u8_0			= *(pu8 + 2);
			// Put beginning in end.
			*(pu8	+ 2)	= *(pu8 + 0);
			// Put end in beginning.
			*(pu8	+ 0)	= u8_0;
			// Middle does not need to be swapped in odd sized types.
			}
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Reads in 32 bit data, swapped if necessary.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Read32(	// Returns number of 32 bit items read.
	uint32_t*	pu32,			// In:  32 bit data to read (swapping, if necessary).
   size_t	lNum)			// In:  Number of 32 bit items to read.
	{
	int32_t	lRes	= 0;

	// Read data.
	lRes = Read((void*)pu32, lNum * sizeof(uint32_t)) / sizeof(uint32_t);

	// If m_endian is consistent with this platform . . .
	if (ENDIAN_CONSISTENT)
		{
		// Done.
		}
	else
		{
		// Convert.
		uint8_t		u8_0;
		uint8_t		u8_1;
		uint8_t*	pu8	= (uint8_t*)pu32;
      for (int32_t l = 0; l < lRes; l++, pu8 += sizeof(uint32_t))
			{
			// Store end.
			u8_0			= *(pu8 + 3);
			// Store second to end.
			u8_1			= *(pu8 + 2);

			// Put beginning into end.
			*(pu8 + 3)	= *(pu8 + 0);
			// Put second to beginning into second to end.
			*(pu8	+ 2)	= *(pu8 + 1);
			
			// Put end in beginning.
			*(pu8 + 0)	= u8_0;
			// Put second to end into second to beginning.
			*(pu8 + 1)	= u8_1;
			}
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Reads in 64 bit data, swapped if necessary.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Read64(	// Returns number of 64 bit items read.
	uint64_t*	pu64,			// In:  64 bit data to read (swapping, if necessary).
   size_t	lNum)			// In:  Number of 64 bit items to read.
	{
	int32_t	lRes	= 0;

	// Read data.
	lRes = Read((void*)pu64, lNum * sizeof(uint64_t)) / sizeof(uint64_t);

	// If m_endian is consistent with this platform . . .
	if (ENDIAN_CONSISTENT)
		{
		// Done.
		}
	else
		{
		// Convert.
		uint8_t		u8Tmp;
		uint8_t*	pu8	= (uint8_t*)pu64;
      for (int32_t l = 0; l < lRes; l++, pu8 += sizeof(uint64_t))
			{
			u8Tmp			= *(pu8 + 0); 
			*(pu8 + 0)	= *(pu8 + 7);
			*(pu8 + 7)	= u8Tmp;

			u8Tmp			= *(pu8 + 1); 
			*(pu8 + 1)	= *(pu8 + 6);
			*(pu8 + 6)	= u8Tmp;

			u8Tmp			= *(pu8 + 2); 
			*(pu8 + 2)	= *(pu8 + 5);
			*(pu8 + 5)	= u8Tmp;

			u8Tmp			= *(pu8 + 3); 
			*(pu8 + 3)	= *(pu8 + 4);
			*(pu8 + 4)	= u8Tmp;
			}
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Writes out 8 bit data, swapped if necessary (BWAH HA).
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Write8(	// Returns number of 8 bit items written.
	const uint8_t*	pu8,			// In:  8 bit data to write (swapping, if necessary).
   size_t	lNum)			// In:  Number of 8 bit items to write.
	{
	int32_t	lRes	= 0;

	lRes = Write((void*)pu8, lNum);

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Writes out 16 bit data, swapped if necessary.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Write16(	// Returns number of 16 bit items written.
	const uint16_t*	pu16,				// In:  16 bit data to write (swapping, if necessary).
   size_t	lNum)				// In:  Number of 16 bit items to write.
	{
	int32_t	lRes	= 0;

	// If m_endian is consistent with this platform . . .
	if (ENDIAN_CONSISTENT)
		{
		// Write data.
		lRes = Write((void*)pu16, lNum * sizeof(uint16_t)) / sizeof(uint16_t);
		}
	else
		{
		// Convert.
		// Note that this could be sped up a bit more by using a variable to store
		// the current lSrc position before entering the inner loop and then subtract
		// that from lSrc after exiting the inner loop to determine the amount of 
		// items to write.
		// No template is used b/c it would be necessary to loop on sizeof(uint32_t)
		// which would negate much of the effort of this algorithm.
		uint8_t*	pu8Src	= (uint8_t*)pu16;		// Source data.
		uint8_t*	pu8Dst;							// Temp destination to be written after
													// swapped to.
      size_t lSrc;								// Number of source items swapped/written.
      size_t lDst;								// Number of items swapped on current
													// iteration.
		int32_t	lWritten	= 0;					// Number of items written on current
													// iteration.
      for (lSrc = 0; lSrc < lNum && lWritten >= 0; lRes += lWritten / sizeof(uint16_t))
			{
			pu8Dst	= ms_au8SwapBuf;
			for (
            lDst = 0;
				lSrc < lNum && lDst < sizeof(ms_au8SwapBuf); 
				lDst += sizeof(uint16_t), lSrc++, pu8Src += sizeof(uint16_t), pu8Dst += sizeof(uint16_t) )
				{
				*(pu8Dst	+ 1)	= *(pu8Src + 0);
				*(pu8Dst + 0)	= *(pu8Src + 1);
				}
		
			// Write data subportion.
			lWritten = Write((void*)ms_au8SwapBuf, lDst);
			}
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Writes out 24 bit data, swapped if necessary.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Write24(	// Returns number of 24 bit items written.
	const RPixel24*	ppix24,		// In:  24 bit data to write (swapping, if necessary).
   size_t	lNum)				// In:  Number of 24 bit items to write.
	{
	int32_t	lRes	= 0;

	// If m_endian is consistent with this platform . . .
	if (ENDIAN_CONSISTENT)
		{
		// Write data.
		lRes = Write((void*)ppix24, lNum * sizeof(RPixel24)) / sizeof(RPixel24);
		}
	else
		{
		// Convert.
		// Note that this could be sped up a bit more by using a variable to store
		// the current lSrc position before entering the inner loop and then subtract
		// that from lSrc after exiting the inner loop to determine the amount of 
		// items to write.
		// No template is used b/c it would be necessary to loop on sizeof(uint32_t)
		// which would negate much of the effort of this algorithm.
		uint8_t*	pu8Src	= (uint8_t*)ppix24;		// Source data.
		uint8_t*	pu8Dst;							// Temp destination to be written after
													// swapped to.
      size_t lSrc;								// Number of source items swapped/written.
      size_t lDst;								// Number of items swapped on current
													// iteration.
		int32_t	lWritten	= 0;					// Number of items written on current
													// iteration.
      for (lSrc = 0; lSrc < lNum && lWritten >= 0; lRes += lWritten / sizeof(RPixel24))
			{
			pu8Dst	= ms_au8SwapBuf;
			for (
            lDst = 0;
				lSrc < lNum && lDst < sizeof(ms_au8SwapBuf); 
				lDst += sizeof(RPixel24), lSrc++, pu8Src += sizeof(RPixel24), pu8Dst += sizeof(RPixel24) )
				{
				*(pu8Dst	+ 2)	= *(pu8Src + 0);
				*(pu8Dst + 1)	= *(pu8Src + 1);
				*(pu8Dst + 0)	= *(pu8Src + 2);
				}
		
			// Write data subportion.
			lWritten = Write((void*)ms_au8SwapBuf, lDst);
			}
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Writes out 32 bit data, swapped if necessary.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Write32(	// Returns number of 32 bit items written.
	const uint32_t*	pu32,				// In:  32 bit data to write (swapping, if necessary).
   size_t	lNum)				// In:  Number of 32 bit items to write.
	{
	int32_t	lRes	= 0;

	// If m_endian is consistent with this platform . . .
	if (ENDIAN_CONSISTENT)
		{
		// Write data.
		lRes = Write((void*)pu32, lNum * sizeof(uint32_t)) / sizeof(uint32_t);
		}
	else
		{
		// Convert.
		// Note that this could be sped up a bit more by using a variable to store
		// the current lSrc position before entering the inner loop and then subtract
		// that from lSrc after exiting the inner loop to determine the amount of 
		// items to write.
		// No template is used b/c it would be necessary to loop on sizeof(uint32_t)
		// which would negate much of the effort of this algorithm.
		uint8_t*	pu8Src	= (uint8_t*)pu32;		// Source data.
		uint8_t*	pu8Dst;							// Temp destination to be written after
													// swapped to.
      size_t lSrc;								// Number of source items swapped/written.
      size_t lDst;								// Number of items swapped on current
													// iteration.
		int32_t	lWritten	= 0;					// Number of items written on current
													// iteration.
      for (lSrc = 0; lSrc < lNum && lWritten >= 0; lRes += lWritten / sizeof(uint32_t))
			{
			pu8Dst	= ms_au8SwapBuf;
			for (
            lDst = 0;
				lSrc < lNum && lDst < sizeof(ms_au8SwapBuf); 
				lDst += sizeof(uint32_t), lSrc++, pu8Src += sizeof(uint32_t), pu8Dst += sizeof(uint32_t) )
				{
				*(pu8Dst	+ 3)	= *(pu8Src + 0);
				*(pu8Dst	+ 2)	= *(pu8Src + 1);
				*(pu8Dst + 1)	= *(pu8Src + 2);
				*(pu8Dst + 0)	= *(pu8Src + 3);
				}
		
			// Write data subportion.
			lWritten = Write((void*)ms_au8SwapBuf, lDst);
			}
		}

	return lRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Writes out 64 bit data, swapped if necessary.
//
//////////////////////////////////////////////////////////////////////////////
int32_t RFile::Write64(	// Returns number of 64 bit items written.
	const uint64_t*	pu64,				// In:  64 bit data to write (swapping, if necessary).
   size_t lNum)				// In:  Number of 64 bit items to write.
	{
	int32_t	lRes	= 0;

	// If m_endian is consistent with this platform . . .
	if (ENDIAN_CONSISTENT)
		{
		// Done.
		lRes	= Write((void*)pu64, lNum * sizeof(uint64_t)) / sizeof(uint64_t);
		}
	else
		{
		// Note that this could be sped up a bit more by using a variable to store
		// the current lSrc position before entering the inner loop and then subtract
		// that from lSrc after exiting the inner loop to determine the amount of 
		// items to write.
		// No template is used b/c it would be necessary to loop on sizeof(uint32_t)
		// which would negate much of the effort of this algorithm.
		uint8_t*	pu8Src	= (uint8_t*)pu64;		// Source data.
		uint8_t*	pu8Dst;							// Temp destination to be written after
													// swapped to.
      size_t	lSrc;								// Number of source items swapped/written.
      size_t	lDst;								// Number of items swapped on current
													// iteration.
		int32_t	lWritten	= 0;					// Number of items written on current
													// iteration.
      for (lSrc = 0; lSrc < lNum && lWritten >= 0; lRes += lWritten / sizeof(uint64_t))
			{
			pu8Dst	= ms_au8SwapBuf;
			for (
            lDst = 0;
				lSrc < lNum && lDst < sizeof(ms_au8SwapBuf); 
				lDst += sizeof(uint64_t), lSrc++, pu8Src += sizeof(uint64_t), pu8Dst += sizeof(uint64_t) )
				{
				*(pu8Dst	+ 0)	= *(pu8Src + 7);
				*(pu8Dst + 1)	= *(pu8Src + 6);
				*(pu8Dst	+ 2)	= *(pu8Src + 5);
				*(pu8Dst + 3)	= *(pu8Src + 4);
				*(pu8Dst	+ 4)	= *(pu8Src + 3);
				*(pu8Dst + 5)	= *(pu8Src + 2);
				*(pu8Dst	+ 6)	= *(pu8Src + 1);
				*(pu8Dst + 7)	= *(pu8Src + 0);
				}
		
			// Write data subportion.
			lWritten = Write((void*)ms_au8SwapBuf, lDst);
			}
		}

	return lRes;
	}

#ifdef ALLOW_RFILE_REOPEN
	//////////////////////////////////////////////////////////////////////////////
	//
	// Disconnects a disk file from the disk temporarily so that another
	// can use the FILE* that is made available.  Returns 0 on success.
	//
	//////////////////////////////////////////////////////////////////////////////
	int16_t RFile::Disconnect(void)
		{
      int16_t sResult	= SUCCESS;	// Assume success.
		
		// Must be disk file and must be connected.
		ASSERT(IsFile()			== TRUE);
		ASSERT(m_sDisconnected	== FALSE);

		// Close this file. BUT DON'T CLEAR m_fs!!!!  Needed for calls to IsFile(),
		// etc.
      if (fclose(m_fs) == SUCCESS)
			{
			// Successfully disconnected.
			}
		else
			{
			TRACE("Disconnect(): Unable to close file for disconnection from "
					"disk.\n");
         sResult = FAILURE - 1;
			}

      return sResult;
		}

	//////////////////////////////////////////////////////////////////////////////
	//
	// Reconnects a disk file that has been previously disconnected.
	// Does nothing if connected (i.e., if m_sDisconnected == FALSE).
	// Returns 0 on success.
	//
	//////////////////////////////////////////////////////////////////////////////
	int16_t RFile::Reconnect(void)
		{
      int16_t sResult	= SUCCESS;	// Assume success.

		// Must be disk file.
		ASSERT(IsFile()			== TRUE);
		
		// If disconnected . . .
		if (m_sDisconnected	== TRUE)
			{
			// Re open.
			m_fs = fopen(m_szFileName, m_szFlags);
			if (m_fs != nullptr)
				{
				// Reconnected.  Add to open list.
            if (ms_listOpen.Add(this) == SUCCESS)
					{
					// Success.
					}
				else
					{
					TRACE("Reconnect(): Unable to add file to open files list!\n");
               sResult = FAILURE - 1;
					}
				}
			else
				{
				TRACE("Reconnect(): UNABLE TO RECONNECT FILE!  A FILE MAY HAVE BEEN "
						"LOCKED BY ANOTHER USER/APP DURING DURATION OF DISCONNECT!\n");
            sResult = FAILURE;
				}
			}

      return sResult;
		}

	//////////////////////////////////////////////////////////////////////////////
	//
	// Disconnect the RFile attached to disk file that was accessed 
	// longest ago.
	// Returns 0 on success.
	// (static)
	//
	//////////////////////////////////////////////////////////////////////////////
	int16_t RFile::MakeStreamAvailable(void)
		{
      int16_t sResult	= SUCCESS;	// Assume success.

		// Find the open RFile attached to disk that was accessed longest ago.
		// All RFiles in the ms_listOpen list are attached to disk files.
		RFile*	pfileOld	= ms_listOpen.GetHead();
		RFile*	pfile		= ms_listOpen.GetNext();
		while (pfile != nullptr)
			{
			// If current was accessed longer ago than pfileOld . . .
			if (pfile->m_lLastAccess < pfileOld->m_lLastAccess)
				{
				pfileOld = pfile;
				}

			pfile = ms_listOpen.GetNext();
			}

		// If we got one . . .
		if (pfileOld != nullptr)
			{
			// Attempt to temporarily disconnect . . .
         if (pfileOld->Disconnect() == SUCCESS)
				{
				// Remove RFile from list.
            if (ms_listOpen.Remove(pfileOld) == SUCCESS)
					{
					}
				else
					{
					TRACE("MakeStreamAvailable(): Unable to remove RFile from list!  "
							"This could cause major problems!\n");
					// No error return value since we actually made on available.
					}
				}
			else
				{
				TRACE("MakeStreamAvailable(): Unable to disconnect RFile.\n");
            sResult = FAILURE - 1;
				}
			}
		else
			{
			TRACE("MakeStreamAvailable(): No open RFiles attached to disk.\n");
         sResult = FAILURE;
			}

      return sResult;
		}

#endif	// ALLOW_RFILE_REOPEN

//////////////////////////////////////////////////////////////////////////////
// Querries.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//	EOF
//////////////////////////////////////////////////////////////////////////////
