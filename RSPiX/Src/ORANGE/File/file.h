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
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
//
//////////////////////////////////////////////////////////////////////////////
//
// File.h
// 
// History:
//		07/29/95 JMI	Started.
//
//		01/22/97	JMI	Added NeutralEndian which never byte swaps and additional
//							overload for Open() to open memfiles that grow when writes
//							exceed their bounds.
//
//		01/28/97	JMI	Added yet another Open() overload that allows one to open
//							an existing FILE* stream with an RFile.
//
//		01/28/97	JMI	Added yet another Open() overload that allows one to open
//							an existing FILE* stream with an RFile.
//							And yet another Open() which opens from another RFile
//							that is already attached to disk or memory.  This is use-
//							ful for descended classes such as RIff (so they can sub-
//							class an existing RFile).
//
//		02/04/97	MJR/JMI	Added RFileEZSave() and RFileEZLoad() that are meant
//							to replace the myclass.Load(char*) and 
//							myclass.Save(char*) that has been written so far for 
//							nearly every class that loads and saves.  These new funcs
//							are essentially macros that will allocate an RFile, open
//							it with the specified filename, permissions, and endian 
//							format, call the specified class's Load or Save, check
//							for error, close the RFile, and deallocate the RFile.
//
//		02/05/97	JMI	Protected Read and Write for void* (see comments there
//							for details).  Consequently, I had to add an overload
//							for char* that was being caught by the void* version
//							before.
//							Also, added Read and Writes for RPixel32.
//
//		02/10/97	JMI	Added rspAnyLoad() and rspAnySave().  There are two types
//							of implementations for these functions.  One is the 
//							explicit overload where someone defines an rspAnyLoad/Save
//							specific to the overloaded type and the other is implicit
//							in that there is a template function that catches all the
//							ones that don't have an explicit overload.
//							Also, added explicit overloads for most RFile supported
//							types.
//
//		02/10/97	JMI	RFILE_INSTANTIATE_ANYLOAD and SAVE had a bug where they
//							returned the number of items read or written instead of
//							0 indicating success or non-zero indicating failure.
//
//		06/28/97 MJR	Metrowerks compiler has a problem with explicit functions
//							being created AFTER templates of the same name are
//							defined.  I simply rearranged the order so it works.
//
//		10/06/99	JMI	rspEZSave() was not deleting its RFile when done.
//
//////////////////////////////////////////////////////////////////////////////
//
// See CPP comment for details.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef FILE_H
#define FILE_H

//////////////////////////////////////////////////////////////////////////////
// Headers.
//////////////////////////////////////////////////////////////////////////////
#include <cstdio>

#include <BLUE/Blue.h>

#ifdef ALLOW_RFILE_REOPEN
# include <ORANGE/CDT/List.h>
#endif // ALLOW_RFILE_REOPEN

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////

#ifdef ALLOW_RFILE_REOPEN
	#define KEEPCONNECTEDANDUPDATELASTACCESS	Reconnect(), m_lLastAccess = rspGetMilliseconds()
#else	// ALLOW_RFILE_REOPEN
	#define KEEPCONNECTEDANDUPDATELASTACCESS
#endif // ALLOW_RFILE_REOPEN

#define MAX_MODE_LEN		6
#define MAX_NAME_LEN		512

// Size of buffer used to byte swap by Write().  It'd be safe to make sure
// this number is a multiple of 8 (our largest base type is 8 bytes).
#define RFILE_SWAP_SIZE	1024

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////

// Forward declared for typedef'd protos.
class RFile;

//////////////////////////////////////////////////////////////////////////////
class RFile
	{
	public:	// Typedefs & Enums.
		// Use for flags field of Open():
		typedef enum
			{
			// No flags.
			NoFlags	= 0x0000,
			// Binary mode.  All numbers written to file as numbers.
			Binary	= 0x0001,
			// ASCII mode.  All numbers written to file as text.
			Ascii		= 0x0002
			} Flags;

		// Use for endian field of Open and SetEndian calls.
		typedef enum
			{
			BigEndian,			// Big endian (byte swaps on little endian systems).
			NeutralEndian,		// Neutral endian (Never byte swaps).
			LittleEndian		// Little endian (byte swaps on big endian systems).
			} Endian;

		typedef short (*OpenHook)(	// Returns 0 to bypass default Open's 
											// functionality.  Non-zero for normal ops.
			RFile* pfile,				// Pointer to RFile being opened.
			const char* pszFileName,		// File path and name to open.
			const char* pszFlags,			// Open flags ala fopen.
			Endian endian,				// Endian nature of file.
         uintptr_t lUser);				// User value.

		typedef short (*CloseHook)(	// Returns 0 to bypass default Close's
												// functionality.
			RFile* pfile,					// Pointer to RFile being closed.
         uintptr_t lUser);					// User value.

		// This is the type of the function that is called before every disk
		// read or write operation to let the user know of (and, perhaps, act
		// upon) delays that may occur.
		typedef void (*CritiCall)(	// Returns nothing.
         size_t	lBytes);				// Bytes about to be read or written.

	public:	// Construction/Destruction.
		// Default constructor.
		RFile(void);
		// Destructor.
		~RFile(void);

//////////////////////////////////////////////////////////////////////////////

	public:	// Methods.
		////////////////////////////////////////////////////////////////////////
		// Open, Close, and operation flags.
		////////////////////////////////////////////////////////////////////////

		// Open file pszFileName with fopen flags pszFlags and endian format 
		// endian { RFile::Big, RFile::Little }.
		// Returns 0 on success.
		int16_t Open(					// Returns 0 on success.
			const char* pszFileName,	// Filename to open.
			const char* pszFlags,		// fopen flags to use for opening.
			Endian endian,			// { RFile::BigEndian | RFile::LittleEndian | RFile::NeutralEndian }.
			Flags flags	=			// See comments in Typedefs & Enums section 
				Binary);				// above.

		// Open memory pFile of size lSize and endian format endian.
		// Size and location of memory will not be affected by RFile.
		// { RFile::Big, RFile::Little }.
		// Returns 0 on success.
		int16_t Open(					// Returns 0 on success.
			void* pFile,			// Pointer to memory to open.
         size_t lSize,				// Size of *pFile in bytes.
			Endian endian);		// { RFile::BigEndian | RFile::LittleEndian | RFile::NeutralEndian }.

		// Open memory pFile of size lSize and endian format endian.
		// RFile may size and or relocate the memory in order to expand the memory file.
		// Deallocates on Close().
		// { RFile::Big, RFile::Little }.
		// Returns 0 on success.
		int16_t Open(					// Returns 0 on success.
         size_t	lSize,			// Size in bytes to begin with.
         size_t	lGrowSize,		// Min amount in bytes to grow memory file when written passed end.
										// Note: The larger of lGrowSize and the amount overwritten will
										// be allocated in the case of an overrun.
			Endian endian);		// { RFile::BigEndian | RFile::LittleEndian | RFile::NeutralEndian }.

		// Open an existing FILE* stream.
		// Once a FILE* is opened, you can use this class's Close() instead of fclose(),
		// if that is more convenient.
		int16_t Open(					// Returns 0 on success.
			FILE*	fs,				// FILE* stream to open.
			Endian endian,			// { RFile::BigEndian | RFile::LittleEndian | RFile::NeutralEndian }.
			Flags flags	=			// See comments in Typedefs & Enums section 
				Binary);				// above.

		// Open an existing RFile.
		// "Ack!" you say ... and I agree.
		// This basically begins what I like to think of as a synchronization between two RFiles.
		// This RFile snags the current state (basically copies the members) from the specified
		// RFile.  Then, one can use the new RFile to access the file/memory/whatever that the
		// original RFile is attached to.  When Close() is called, the synchronization is finsished
		// by updating the original RFile with the state from this.
		// Danger:  Do not access the original RFile between Open(RFile*)/Close() pairs!
		int16_t Open(					// Returns 0 on success.
			RFile* pfile);			// RFile to open.

		// Sets an Open(char*...) hook.  NOTE: You CAN call any RFile Open from
		// within an Open hook.  The Open hook should return 0 to bypass RFile's
		// functionality.
      static void SetOpenHook(OpenHook hook, uintptr_t lUser)
			{ ms_hOpen = hook; ms_lOpenUser = lUser; }

		// Change the endian format used to read/write the file.
		// Returns nothing.
		void SetEndian(Endian endian);

		// Set the size of the buffer used to read from the disk.
		// This function only applies to files open from the disk.
		// Returns 0 on success.
		int16_t SetBufferSize(size_t stBufSize)
			{
			if (IsFile() == TRUE)
				{
				KEEPCONNECTEDANDUPDATELASTACCESS;
				return (int16_t)setvbuf(m_fs, nullptr, _IOFBF, stBufSize);
				}
			else
				if (IsMemory() == TRUE)
					return SUCCESS;
				else
               return FAILURE;
			}

		// Close a file successfully opened with Open().
		// Returns 0 on success.
		int16_t Close(void);

		// Sets a Close() hook.  NOTE: You CAN call any RFile Close from
		// within a Close hook.  The Close hook should return 0 to bypass RFile's
		// functionality.
      static void SetCloseHook(CloseHook hook, uintptr_t lUser)
			{ ms_hClose = hook; ms_lCloseUser = lUser; }

		////////////////////////////////////////////////////////////////////////
		// Position.
		////////////////////////////////////////////////////////////////////////

		// Seeks within the file based on the supplied position argument
		// { SEEK_SET, SEEK_CUR, SEEK_END }.
		// Returns 0 on success.
      int16_t Seek(ssize_t lPos, int32_t lOrigin);

		// Returns the current file position or -1 on error.
      int32_t Tell(void);

		////////////////////////////////////////////////////////////////////////
		// Read.
		////////////////////////////////////////////////////////////////////////

	protected:
		// NOTE:  THIS FUNCTION IS PROTECTED FOR AN IMPORTANT REASON:
		// This function was a catch all for any unsupported type.  The dis-
		// advantage was that, if you had a type that needed to be swapped
		// but was not supported, this API would happily accept it, but not
		// swap it.  This could cause hard to detect problems.
		// If you are reading or writing something that does not require 
      // swapping, cast it as uint8_t* or int8_t*.
		// Reads lNum bytes from currently open file.
		// Returns number of bytes successfully read.
      int32_t Read(void* pData, size_t lNum);

   public:
		// Reads lNum char values from currently open file.
		// Returns number of char values successfully read.
      int32_t Read(char* pcData, size_t lNum)
			{ return Read((void*)pcData, lNum); }

		// Reads lNum uint8_t values from currently open file.
		// Returns number of uint8_t values successfully read.
      int32_t Read(uint8_t*	pu8Data, size_t lNum = 1);

      // Reads lNum int8_t values from currently open file.
      // Returns number of int8_t values successfully read.
      int32_t Read(int8_t*	ps8Data, size_t lNum = 1);

      // Reads lNum uint16_t values from currently open file.
      // Returns number of uint16_t values successfully read.
      int32_t Read(uint16_t* pu16Data, size_t lNum = 1);
		
      // Reads lNum int16_t values from currently open file.
      // Returns number of int16_t values successfully read.
      int32_t Read(int16_t* ps16Data, size_t lNum = 1);

		// Reads lNum RPixel24 values from currently open file.
		// Returns number of RPixel24 values successfully read.
      int32_t Read(RPixel24* ppix24, size_t lNum = 1);

      // Reads lNum uint32_t values from currently open file.
      // Returns number of uint32_t values successfully read.
      int32_t Read(uint32_t* pu32Data, size_t lNum = 1);

      // Reads lNum int32_t values from currently open file.
      // Returns number of int32_t values successfully read.
      int32_t Read(int32_t* ps32Data, size_t lNum = 1);

		// Reads lNum RPixel32 values from currently open file.
		// Returns number of RPixel32 values successfully read.
      int32_t Read(RPixel32* ppix32Data, size_t lNum = 1)
         { return Read((uint32_t*)ppix32Data, lNum); }

      // Reads lNum uint64_t values from currently open file.
      // Returns number of uint64_t values successfully read.
      int32_t Read(uint64_t* pu64Data, size_t lNum = 1);

      // Reads lNum int64_t values from currently open file.
      // Returns number of int64_t values successfully read.
      int32_t Read(int64_t* ps64Data, size_t lNum = 1);

		// Reads lNum float values from currently open file.
		// Returns number of float values successfully read.
      int32_t Read(float* pfData, size_t lNum = 1);

		// Reads lNum double values from currently open file.
		// Returns number of double values successfully read.
      int32_t Read(double* pdData, size_t lNum = 1);

		// Reads a nullptr terminated string.  The '\0' must be
		// actually in the file to denote the end of the string.
		// pszString must point to a memory block sufficiently large
		// enough to hold the string.
		// Returns number of characters successfully read,
		// including the nullptr terminator (unlike strlen()).
		int32_t Read(char* pszString);

		////////////////////////////////////////////////////////////////////////
		// Write.
		////////////////////////////////////////////////////////////////////////

	protected:
		// NOTE:  THIS FUNCTION IS PROTECTED FOR AN IMPORTANT REASON:
		// This function was a catch all for any unsupported type.  The dis-
		// advantage was that, if you had a type that needed to be swapped
		// but was not supported, this API would happily accept it, but not
		// swap it.  This could cause hard to detect problems.
		// If you are reading or writing something that does not require 
      // swapping, cast it as uint8_t* or int8_t*.
		// Writes lNum bytes from currently open file.
		// Returns number of bytes successfully written.
      int32_t Write(const void* pData, size_t lNum);

	public:
		// Writes lNum char values to currently open file.
		// Returns number of uint8_t values successfully written.
      int32_t Write(const char*	pcData, size_t lNum)
			{ return Write((void*)pcData, lNum); }

		// Writes lNum uint8_t values to currently open file.
		// Returns number of uint8_t values successfully written.
      int32_t Write(const uint8_t*	pu8Data, size_t lNum = 1);

      // Writes lNum int8_t values to currently open file.
      // Returns number of int8_t values successfully written.
      int32_t Write(const int8_t*	ps8Data, size_t lNum = 1);

      // Writes lNum uint16_t values to currently open file.
      // Returns number of uint16_t values successfully written.
      int32_t Write(const uint16_t* pu16Data, size_t lNum = 1);

      // Writes lNum int16_t values to currently open file.
      // Returns number of int16_t values successfully written.
      int32_t Write(const int16_t* ps16Data, size_t lNum = 1);

		// Writes lNum RPixel24 values to currently open file.
		// Returns number of RPixel24 values successfully written.
      int32_t Write(const RPixel24* ppix24, size_t lNum = 1);

      // Writes lNum uint32_t values to currently open file.
      // Returns number of uint32_t values successfully written.
      int32_t Write(const uint32_t* pu32Data, size_t lNum = 1);

      // Writes lNum int32_t values to currently open file.
      // Returns number of int32_t values successfully written.
      int32_t Write(const int32_t* ps32Data, size_t lNum = 1);

		// Writes lNum RPixel32 values to currently open file.
		// Returns number of RPixel32 values successfully written.
      int32_t Write(const RPixel32* ppix32Data, size_t lNum = 1)
         { return Write((uint32_t*)ppix32Data, lNum); }

      // Writes lNum uint64_t values to currently open file.
      // Returns number of uint64_t values successfully written.
      int32_t Write(const uint64_t* pu64Data, size_t lNum = 1);

      // Writes lNum int64_t values to currently open file.
      // Returns number of int64_t values successfully written.
      int32_t Write(const int64_t* ps64Data, size_t lNum = 1);

		// Writes lNum float values to the currently open file.
		// Returns number of float values successfully written.
      int32_t Write(const float* pfData, size_t lNum = 1);

		// Writes lNum double values to the currently open file.
		// Returns number of double values successfully written.
      int32_t Write(const double* pdData, size_t lNum = 1);

		// Writes a nullptr terminated string.  The '\0' must be
		// actually written to file to denote the end of the string.
		// Returns number of characters successfully written,
		// including the nullptr terminator (unlike strlen()).
		int32_t Write(const char* pszString);

		////////////////////////////////////////////////////////////////////////
		// These functions write one value when passed by value.
		// This is useful for writing literals, variables that should be written
		// by other than their type, and for variables of types that are not
		// pointer overloaded by other Write(type*, long) RFile functions.
		////////////////////////////////////////////////////////////////////////

		// Writes one char.
		int32_t Write(char cVal)
			{ return Write(&cVal, 1); }

		// Writes one uint8_t.
		int32_t Write(uint8_t u8Val)
			{ return Write(&u8Val); }

      // Writes one int8_t.
      int32_t Write(int8_t s8Val)
			{ return Write(&s8Val); }

      // Writes one uint16_t.
      int32_t Write(uint16_t u16Val)
			{ return Write(&u16Val); }

      // Writes one int16_t.
      int32_t Write(int16_t s16Val)
			{ return Write(&s16Val); }

		// Writes one RPixel24.
		int32_t Write(RPixel24 pix24Val)
			{ return Write(&pix24Val); }

      // Writes one uint32_t.
      int32_t Write(uint32_t u32Val)
			{ return Write(&u32Val); }

      // Writes one int32_t.
      int32_t Write(int32_t s32Val)
			{ return Write(&s32Val); }

		// Writes one RPixel32.
		int32_t Write(RPixel32 pix32Val)
			{ return Write(&pix32Val); }

		// Writes one float.
		int32_t Write(float fVal)
			{ return Write(&fVal); }

      // Writes one uint64_t.
      int32_t Write(uint64_t u64Val)
			{ return Write(&u64Val); }

      // Writes one int64_t.
      int32_t Write(int64_t s64Val)
			{ return Write(&s64Val); }

		// Writes one double.
		int32_t Write(double dVal)
			{ return Write(&dVal); }

//////////////////////////////////////////////////////////////////////////////

	public:	// Querries.
		////////////////////////////////////////////////////////////////////////
		// Querries.
		////////////////////////////////////////////////////////////////////////

		// Returns TRUE, if open, FALSE if closed.
		int16_t IsOpen(void)	{ return (m_fs == nullptr && m_pucFile == nullptr ? FALSE : TRUE); }

		// Returns TRUE, if connected to a non-memory file.
		int16_t IsFile(void)	{ return (m_fs == nullptr ? FALSE : TRUE); }

		// Returns TRUE, if connected to a memory file.
		int16_t IsMemory(void)	{ return (m_pucFile == nullptr ? FALSE : TRUE); }

		// Returns FALSE if no error has occurred on this stream; TRUE if an
		// error has occurred.
		int16_t Error(void) 
			{ 
			if (IsFile() == TRUE)
				{
				KEEPCONNECTEDANDUPDATELASTACCESS;
				return (ferror(m_fs) == 0 ? FALSE : TRUE);
				}
			else
				if (IsMemory() == TRUE)
					return m_sMemError;
				else
					return TRUE;
			}

		// Clears any error that has occurred on this stream.
		void ClearError(void) 
			{ 
			if (IsFile() == TRUE)
				{
				KEEPCONNECTEDANDUPDATELASTACCESS;
				clearerr(m_fs);
				}
			else
				if (IsMemory() == TRUE)
					m_sMemError	= FALSE;
			}

		// Returns TRUE if end of file has been reached, FALSE otherwise.
		// For disk files, does not return TRUE until the first i/o operation
		// fails due to EOF.
		int16_t IsEOF(void)
			{
			if (IsFile() == TRUE)
				{
				KEEPCONNECTEDANDUPDATELASTACCESS;
				return (feof(m_fs) == 0 ? FALSE : TRUE);
				}
			else
				if (IsMemory() == TRUE)
					return (m_pucCur == m_pucFile + m_lSize);
				else
					return TRUE;
			}

		// Returns the size of the file on success.  Negative on error.
		int32_t GetSize(void);

		// Returns the endian setting for this object.
		int16_t GetEndian(void)
			{ return m_endian; }

		// Returns the memory ptr if this is a memory file; nullptr, otherwise.
		// NOTE:  This buffer may move after any Write() call that exceeds the
		// existing buffer size.
		uint8_t* GetMemory(void)
			{ return m_pucFile; }

//////////////////////////////////////////////////////////////////////////////
	protected:	// Functions for my friends.

		// Sets the instantiable user value.
      void SetUserVal(uintptr_t lUser)
			{ m_lUser = lUser; }

		// Gets the instantiable user value.
      uintptr_t GetUserVal(void)
			{ return m_lUser; }

//////////////////////////////////////////////////////////////////////////////

	protected:	// Internal functions.

/////////// Binary Reads /////////////////////////////////////////////////////

		// Reads in 8 bit data, swapped if necessary (BWAH HA).
		int32_t Read8(		// Returns number of 8 bit items read.
			uint8_t*	pu8,		// In:  8 bit data to read (swapping, if necessary).
         size_t lNum);	// In:  Number of 8 bit items to read.

		// Reads in 16 bit data, swapped if necessary.
		int32_t Read16(		// Returns number of 16 bit items read.
         uint16_t*	pu16,		// In:  16 bit data to read (swapping, if necessary).
         size_t lNum);	// In:  Number of 16 bit items to read.

		// Reads in 24 bit data, swapped if necessary.
		int32_t Read24(			// Returns number of 24 bit items read.
			RPixel24* ppix24,	// In:  24 bit data to read (swapping, if necessary).
         size_t lNum);		// In:  Number of 24 bit items to read.

		// Reads in 32 bit data, swapped if necessary.
		int32_t Read32(		// Returns number of 32 bit items read.
         uint32_t*	pu32,		// In:  32 bit data to read (swapping, if necessary).
         size_t lNum);	// In:  Number of 32 bit items to read.

		// Reads in 64 bit data, swapped if necessary.
		int32_t Read64(		// Returns number of 64 bit items read.
         uint64_t*	pu64,		// In:  64 bit data to read (swapping, if necessary).
         size_t lNum);	// In:  Number of 64 bit items to read.

/////////// Binary Writes ////////////////////////////////////////////////////

		// Writes out 8 bit data, swapped if necessary (BWAH HA).
		int32_t Write8(		// Returns number of 8 bit items written.
			const uint8_t*	pu8,		// In:  8 bit data to write (swapping, if necessary).
         size_t lNum);	// In:  Number of 8 bit items to write.

		// Writes out 16 bit data, swapped if necessary.
		int32_t Write16(		// Returns number of 16 bit items written.
         const uint16_t*	pu16,		// In:  16 bit data to write (swapping, if necessary).
         size_t lNum);	// In:  Number of 16 bit items to write.

		// Writes out 24 bit data, swapped if necessary.
		int32_t Write24(			// Returns number of 24 bit items written.
			const RPixel24* ppix24,	// In:  24 bit data to write (swapping, if necessary).
         size_t lNum);		// In:  Number of 24 bit items to write.

		// Writes out 32 bit data, swapped if necessary.
		int32_t Write32(		// Returns number of 32 bit items written.
         const uint32_t*	pu32,		// In:  32 bit data to write (swapping, if necessary).
         size_t lNum);	// In:  Number of 32 bit items to write.

		// Writes out 64 bit data, swapped if necessary.
		int32_t Write64(		// Returns number of 64 bit items written.
         const uint64_t*	pu64,		// In:  64 bit data to write (swapping, if necessary).
         size_t lNum);	// In:  Number of 64 bit items to write.

		#ifdef ALLOW_RFILE_REOPEN
			// Disconnects this RFile from the disk temporarily so that another
			// can use the FILE* that is made available.  Returns 0 on success.
			int16_t Disconnect(void);

			// Reconnects a disk file that has been previously disconnected.
			// Does nothing if connected (i.e., if m_sDisconnected == FALSE).
			// Returns 0 on success.
			int16_t Reconnect(void);

			// Disconnect the RFile attached to disk file that was accessed 
			// longest ago.
			// Returns 0 on success.
			static int16_t MakeStreamAvailable(void);
		#endif	// ALLOW_RFILE_REOPEN

//////////////////////////////////////////////////////////////////////////////
		friend class RRes;
//////////////////////////////////////////////////////////////////////////////

	public:	// Member variables.
		FILE*			m_fs;						// Stdio file stream ptr.
		static CritiCall	ms_criticall;	// Called on every read and write with
													// the amount that is about to be
													// processed.

	protected:	// Member variables.
		uint8_t*		m_pucFile;			// Memory file ptr.
		int16_t			m_sOwnMem;			// TRUE, if RFile allocated m_pucFile.
		uint8_t*		m_pucCur;			// Current position in memory file.
      size_t			m_lSize;				// Size of memory file (in bytes).
      size_t			m_lGrowSize;		// Amount to grow memfile when buffer
												// overwritten.
		int16_t			m_sMemError;		// TRUE if memory file access functions
												// caused an error.
		Endian		m_endian;			// Endian type.
		Flags			m_flags;				// Flags.  See comments in Typedefs & 
												// Enums section above.

		RFile*		m_pfileSynch;		// RFile this RFile is synchronized with.
												// Used on Close() to resynch.

		static uint8_t	ms_au8SwapBuf[RFILE_SWAP_SIZE];	// Used to byte swap by Write().

		// Hook stuff.
      uintptr_t							m_lUser;			// Instantiable hook value.
		static OpenHook			ms_hOpen;		// Hook for calls to Open(char*...).
      static uintptr_t					ms_lOpenUser;	// User value passed to m_hOpen.
		int16_t							m_sOpenSem;		// Semaphore to block recursion greater
															// than 1.
		static CloseHook			ms_hClose;		// Hook for calls to Close().
      static uintptr_t					ms_lCloseUser;	// User value passed to m_hClose.
		int16_t							m_sCloseSem;	// Semaphore to block recursion greater
															// than 1.
	#ifdef ALLOW_RFILE_REOPEN
		// Reopen stuff.
		static RList <RFile>	ms_listOpen;				// List of open RFiles.
		int32_t			m_lLastAccess;							// Time of last access.
		char			m_szFlags[MAX_MODE_LEN + 1];		// Last Open's flags.
		int16_t			m_sDisconnected;						// TRUE if file has been 
																	// diskonnected.
		char			m_szFileName[MAX_NAME_LEN + 1];	// Filename for reopening.
	#endif // ALLOW_RFILE_REOPEN
	};


////////////////////////////////////////////////////////////////////////////////
// This template function offers an easy way to use any object's Load()
// function.  Most RSPiX objects have a Load() function that expects an RFile*
// as its only parameter.  This function opens the file, calls the object's
// Load(), and closes the file, all with full error checking.  Note that the
// file mode defaults to "rb" and the endian mode defaults to LittleEndian.
////////////////////////////////////////////////////////////////////////////////
#define	rspEZLoad(a,b) RFileEZLoad(a,b,"rb",RFile::LittleEndian)
#define	rspEZSave(a,b) RFileEZSave(a,b,"wb",RFile::LittleEndian)

#ifdef _DEBUG
	#define RFileEZLoad(a,b,c,d)	RFileEZLoadDebug(__FILE__, __LINE__, a, b, c, d)
#endif

template <class ClassType>
#ifdef _DEBUG
int16_t RFileEZLoadDebug(
   const char* FILE_MACRO,
	int32_t LINE_MACRO,
#else
int16_t RFileEZLoad(
#endif
	ClassType* pObject,
   const char* pszName,
   const char* pszMode,
	RFile::Endian endian)
	{
   int16_t sResult = SUCCESS;

	// Create RFile object
	RFile* pFile = new RFile;
   if (pFile != nullptr)
		{

		// Open the file
		sResult = pFile->Open(pszName, pszMode, endian);
		if (sResult == SUCCESS)
			{

			// Call the specified object's Load()
			sResult = pObject->Load(pFile);

			// If no error was returned then check for file I/O errors.  This
			// may be redundant if the object's Load() already checked for file
			// errors, but if it didn't then this becomes very important.
			if ((sResult == SUCCESS) && pFile->Error())
				{
				sResult = FAILURE;
				#ifdef _DEBUG
					STRACE("%s(%i):RFileEZLoad(): File I/O Error!\n", FILE_MACRO, LINE_MACRO);
				#endif
				}

			// Close the file
			pFile->Close();
			}
		else
			{
			sResult = FAILURE;
			#ifdef _DEBUG
				STRACE("%s(%i):RFileEZLoad(): Couldn't open file '%s'!\n", FILE_MACRO, LINE_MACRO, pszName);
			#endif
			}
		
		// Delete file object
		delete pFile;
		}
	else
		{
		sResult = FAILURE;
		#ifdef _DEBUG
			STRACE("%s(%i):RFileEZLoad(): Couldn't create RFile object!\n", FILE_MACRO, LINE_MACRO);
		#endif
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// This template function offers an easy way to use any object's Save()
// function.  Most RSPiX objects have a Save() function that expects an RFile*
// as its only parameter.  This function opens the file, calls the object's
// Save(), and closes the file, all with full error checking.  Note that the
// file mode defaults to "wb" and the endian mode defaults to LittleEndian.
////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
	#define RFileEZSave(a,b,c,d)	RFileEZSaveDebug(__FILE__, __LINE__, a, b, c, d)
#endif

template <class ClassType>
#ifdef _DEBUG
int16_t RFileEZSaveDebug(
   const char* FILE_MACRO,
 	int32_t LINE_MACRO,
#else
int16_t RFileEZSave(
#endif
	ClassType* pObject,
   const char* pszName,
   const char* pszMode,
	RFile::Endian endian)
	{
   int16_t sResult = SUCCESS;

	// Create RFile object
	RFile* pFile = new RFile;
   if (pFile != nullptr)
		{

		// Open the file
		sResult = pFile->Open(pszName, pszMode, endian);
		if (sResult == SUCCESS)
			{

			// Call the specified object's Save()
			sResult = pObject->Save(pFile);

			// If no error was returned then check for file I/O errors.  This
			// may be redundant if the object's Save() already checked for file
			// errors, but if it didn't then this becomes very important.
			if ((sResult == SUCCESS) && pFile->Error())
				{
				sResult = FAILURE;
				#ifdef _DEBUG
					STRACE("%s(%i):RFileEZSave(): File I/O Error!\n", FILE_MACRO, LINE_MACRO);
				#endif
				}

			// Close the file
			pFile->Close();
			}
		else
			{
			sResult = FAILURE;
			#ifdef _DEBUG
				STRACE("%s(%i):RFileEZSave(): Couldn't open file '%s'!\n", FILE_MACRO, LINE_MACRO, pszName);
			#endif
			}

		delete pFile;
		pFile	= 0;
		}
	else
		{
		sResult = FAILURE;
		#ifdef _DEBUG
			STRACE("%s(%i):RFileEZSave(): Couldn't create RFile object!\n", FILE_MACRO, LINE_MACRO);
		#endif
		}

	return sResult;
	}


////////// rspAnyLoad() and rspAnySave() /////////////////////////////////////
// There are two types of implementations for these functions.  One is the 
//	explicit overload where someone defines an rspAnyLoad/Save specific to the
// overloaded type and the other is implicit in that there is a template 
// function that catches all the ones that don't have an explicit overload.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Explicit overloads for rspAnyLoad and rspAnySave.
//
// NOTICE: Metrowerks compiler has a problem with these explicit functions
// being created AFTER the templates for them are defined.  The solution is
// simply to define these BEFORE the templates.
//////////////////////////////////////////////////////////////////////////////

// This macro makes it easy to create explicit versions of rspAnyLoad() for
// basic types that are supported by RFile.  Could be enhanced by allowing
// caller to specify how to cast the value before calling RFile::Read().
#define RFILE_INSTANTIATE_ANYLOAD(type)	\
inline																							\
short rspAnyLoad(		/* Returns 0 on success.*/										\
	type*		ptype,	/* Ptr to type to load.*/										\
	RFile*	pfile)	/* Open RFile to load from.  Must have read access.*/	\
	{																								\
	return (pfile->Read(ptype) == 1) ? 0 : 1;											\
	}																								

// This macro makes it easy to create explicit versions of rspAnySave() for
// basic types that are supported by RFile.  Could be enhanced by allowing
// caller to specify how to cast the value before calling RFile::Write().
#define RFILE_INSTANTIATE_ANYSAVE(type)	\
inline																							\
short rspAnySave(		/* Returns 0 on success.*/										\
	type*		ptype,	/* Ptr to float to load.*/										\
	RFile*	pfile)	/* Open RFile to save to.  Must have write access.*/	\
	{																								\
	return (pfile->Write(ptype) == 1) ? 0 : 1;										\
	}																								

RFILE_INSTANTIATE_ANYLOAD(uint8_t)
RFILE_INSTANTIATE_ANYSAVE(uint8_t)
RFILE_INSTANTIATE_ANYLOAD(int8_t)
RFILE_INSTANTIATE_ANYSAVE(int8_t)
RFILE_INSTANTIATE_ANYLOAD(uint16_t)
RFILE_INSTANTIATE_ANYSAVE(uint16_t)
RFILE_INSTANTIATE_ANYLOAD(int16_t)
RFILE_INSTANTIATE_ANYSAVE(int16_t)
RFILE_INSTANTIATE_ANYLOAD(RPixel24)
RFILE_INSTANTIATE_ANYSAVE(RPixel24)
RFILE_INSTANTIATE_ANYLOAD(uint32_t)
RFILE_INSTANTIATE_ANYSAVE(uint32_t)
RFILE_INSTANTIATE_ANYLOAD(int32_t)
RFILE_INSTANTIATE_ANYSAVE(int32_t)
RFILE_INSTANTIATE_ANYLOAD(RPixel32)
RFILE_INSTANTIATE_ANYSAVE(RPixel32)
RFILE_INSTANTIATE_ANYLOAD(float)
RFILE_INSTANTIATE_ANYSAVE(float)
RFILE_INSTANTIATE_ANYLOAD(uint64_t)
RFILE_INSTANTIATE_ANYSAVE(uint64_t)
RFILE_INSTANTIATE_ANYLOAD(int64_t)
RFILE_INSTANTIATE_ANYSAVE(int64_t)
RFILE_INSTANTIATE_ANYLOAD(double)
RFILE_INSTANTIATE_ANYSAVE(double)


//////////////////////////////////////////////////////////////////////////////
// Loads any type that does not have a specific overload.  This implicit one
// only works for types that have their own Load(RFile*) member function.
//////////////////////////////////////////////////////////////////////////////
template <class obj>
int16_t rspAnyLoad(		// Returns 0 on success.
	obj*		pobj,		// Ptr to object to load.
	RFile*	pfile)	// Open RFile.  Must have read access.
	{
	return pobj->Load(pfile);
	}

//////////////////////////////////////////////////////////////////////////////
// Saves any type that does not have a specific overload.  This implicit one 
// only works for types that have their own Save(RFile*) member function.
//////////////////////////////////////////////////////////////////////////////
template <class obj>
int16_t rspAnySave(		// Returns 0 on success.
	obj*		pobj,		// Ptr to object to save.
	RFile*	pfile)	// Open RFile.  Must have write access.
	{
	return pobj->Save(pfile);
	}

// Find the correct file. This will favor the prefpath
//  for reading, and always use the prefpath for writing.
//  On systems without a prefpath, it just returns pszName, otherwise
//  it will find the right file name and return it in platform-
//  specific notation.
//  This may return a static buffer, and as such is not thread safe.
extern const char *FindCorrectFile(const char *pszName, const char *pszMode);

#endif // FILE_H
//////////////////////////////////////////////////////////////////////////////
//	EOF
//////////////////////////////////////////////////////////////////////////////
