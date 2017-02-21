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
// log.cpp
// Project: Nostril (aka Postal)
//
// This module with a mechanism to log network traffic created by the game
//
// History:
//		12/05/97 AJC	Started.
//
//		12/23/97 SPA	Moved from Play.cpp to seperate file
////////////////////////////////////////////////////////////////////////////////
#include <RSPiX.h>
#include "game.h"
#include "net.h"
#include "Log.h"

#if !defined(MULTIPLAYER_REMOVED)
#include "netmsgr.h"
#endif // !defined(MULTIPLAYER_REMOVED)


////////////////////////////////////////////////////////////////////////////////
// OpenLogFile()
//			Open a file for logging
//		global variables used:		g_GameSettings
////////////////////////////////////////////////////////////////////////////////
int16_t OpenLogFile()
	{
   int16_t sResult = SUCCESS; // Assume success
	if (g_GameSettings.m_bLogNetTime)
		{
			if (!g_GameSettings.m_rfNetSyncLog.IsOpen())
			{
#if BYTE_ORDER == BIG_ENDIAN
			if (g_GameSettings.m_rfNetSyncLog.Open(g_GameSettings.m_szNetSyncLogFile, 
            "wt+", RFile::BigEndian) != SUCCESS)
#elif BYTE_ORDER == LITTLE_ENDIAN
           if (g_GameSettings.m_rfNetSyncLog.Open(g_GameSettings.m_szNetSyncLogFile,
            "wt+", RFile::LittleEndian) != SUCCESS)
#else
# error NOT IMPLEMENTED
#endif
				{
				sResult = 1;
				TRACE("Play: Cannot open network syn log file\n");
				}			
			}
		}
	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// CloseLogFile()
//			Close a file for logging
//		global variables used:		g_GameSettings
////////////////////////////////////////////////////////////////////////////////
int16_t CloseLogFile()
	{
   int16_t sResult = SUCCESS; // Assume success
	if (g_GameSettings.m_bLogNetTime)
		{
      if ((g_GameSettings.m_rfNetSyncLog.Close()) != SUCCESS)
			{
			sResult = 1;
			TRACE("Play: Failed to close the network syn log file\n");
			}
		}
	return sResult;
	}

#if !defined(MULTIPLAYER_REMOVED)

////////////////////////////////////////////////////////////////////////////////
// WriteTimeStamp()
//			Write the network time log
//		global variables used:		g_GameSettings
////////////////////////////////////////////////////////////////////////////////
int16_t WriteTimeStamp(const char *pszCaller,						// Name of calling routine
                     const char *pszCalleeName,					// Name of player being sent or sending
							uint8_t ucMsgType,			// Message type
							Net::SEQ seqStart,					// Beginning sequent sent/received
							int32_t sNum,								// Number of seq's sent/received
							char bReceived,							// a received or a sent message? TRUE if received
							uint16_t u16PackageID/*=0*/)				// Uniquely identifiable package id																		//		True if receiving, false if sending
	{	
   int16_t sResult = SUCCESS;
   const char *szCallerMsg;
	char szTime[256]; 
	char szSeq[256];
	char szNum[256];
   milliseconds_t lTime = rspGetMilliseconds();

  if (ucMsgType == NetMsg::START_REALM && bReceived)
  {
    g_GameSettings.m_lStartRealmTime = lTime;
    sResult = snprintf(szTime, sizeof(szTime), "%u", lTime) < 0 ? FAILURE : SUCCESS;
    //_ltoa(lTime, szTime, 10);
  }
  sResult = snprintf(szTime, sizeof(szTime), "%u", lTime - g_GameSettings.m_lStartRealmTime) < 0 ? FAILURE : SUCCESS;
  //_ltoa(lTime - g_GameSettings.m_lStartRealmTime, szTime, 10);


#if BYTE_ORDER == BIG_ENDIAN
	RFile::Endian endian = RFile::BigEndian;
#elif BYTE_ORDER == LITTLE_ENDIAN
   RFile::Endian endian = RFile::LittleEndian;
#else
#error NOT IMPLEMENTED
#endif

	szCallerMsg = 0;

	// For convenience
	RFile *prfLog = &(g_GameSettings.m_rfNetSyncLog);

	// Log file should be open, if not, open it
	if (!prfLog->IsOpen())
		{
      if ((prfLog->Open(g_GameSettings.m_szNetSyncLogFile, "wt+", endian)) != SUCCESS)
			{
			TRACE("WriteTimeStamp: Failed to open network time stamp log file\n");
			sResult = FAILURE;
			}
		}

	// Write name of player calling
	prfLog->Write(g_GameSettings.m_szPlayerName);

	// Write receiving or sending
	if (bReceived)
		prfLog->Write(" Received ");
	else
		prfLog->Write(" Sent     ");

	// Write name of person who will be receiving or has sent the message
	if (pszCalleeName != nullptr)
		prfLog->Write(pszCalleeName);
	else
		prfLog->Write("Server");
	prfLog->Write(" ");

	// Write package ID
	char szPackageID[256];
   snprintf(szPackageID, sizeof(szPackageID), "%u", u16PackageID);
   //ltoa((int32_t)u16PackageID, szPackageID, 10);
	prfLog->Write(szPackageID);
	prfLog->Write(" ");

	// Write time of logging
	prfLog->Write(szTime);
	prfLog->Write(" ");

   // Write starting sequence sent/received
   snprintf(szSeq, sizeof(szSeq), "%u", seqStart);
   //itoa(seqStart, szSeq, 10);
	prfLog->Write(szSeq);
	prfLog->Write(" ");

	// Write number of sequences sent/received
   snprintf(szNum, sizeof(szNum), "%u", sNum);
   //ltoa(sNum, szNum, 10);
	prfLog->Write(szNum);
	prfLog->Write(" ");

	// Write type of message
	switch (ucMsgType)
		{
		case NetMsg::NOTHING:
			szCallerMsg = "NOTHING";
			break;
		case NetMsg::STAT:
			szCallerMsg = "STAT";
			break;
		case NetMsg::ERR:
			szCallerMsg = "ERR";
			break;
		case NetMsg::LOGIN:
			szCallerMsg = "LOGIN";
			break;
		case NetMsg::LOGIN_ACCEPT:
			szCallerMsg = "LOGIN_ACCEPT";
			break;
		case NetMsg::LOGIN_DENY:
			szCallerMsg = "LOGIN_DENY";
			break;
		case NetMsg::LOGOUT:
			szCallerMsg = "LOGOUT";
			break;
		case NetMsg::JOIN_REQ:
			szCallerMsg = "JOIN_REQ";
			break;
		case NetMsg::JOIN_ACCEPT:
			szCallerMsg = "JOIN_ACCEPT";
			break;
		case NetMsg::JOIN_DENY:
			szCallerMsg = "JOIN_DENY";
			break;
		case NetMsg::JOINED:
			szCallerMsg = "JOINED";
			break;
		case NetMsg::CHANGE_REQ:
			szCallerMsg = "CHANGE_REQ";
			break;
		case NetMsg::CHANGED:
			szCallerMsg = "CHANGED";
			break;
		case NetMsg::DROP_REQ:
			szCallerMsg = "DROP_REQ";
			break;
		case NetMsg::DROPPED:
			szCallerMsg = "DROP_REQ";
			break;
		case NetMsg::DROP_ACK:
			szCallerMsg = "DROP_ACK";
			break;
		case NetMsg::INPUT_REQ:
			szCallerMsg = "INPUT_REQ";
			break;
		case NetMsg::INPUT_DATA:
			szCallerMsg = "INPUT_DATA";
			break;
		case NetMsg::INPUT_MARK:
			szCallerMsg = "INPUT_MARK";
			break;
		case NetMsg::CHAT_REQ:
			szCallerMsg = "CHAT_REQ";
			break;
		case NetMsg::CHAT:
			szCallerMsg = "CHAT";
			break;
		case NetMsg::SETUP_GAME:
			szCallerMsg = "SETUP_GAME";
			break;
		case NetMsg::START_GAME:
			szCallerMsg = "START_GAME";
			break;
		case NetMsg::ABORT_GAME:
			szCallerMsg = "ABORT_GAME";
			break;
		case NetMsg::READY_REALM:
			szCallerMsg = "READY_REALM";
			break;
		case NetMsg::BAD_REALM:
			szCallerMsg = "BAD_REALM";
			break;
		case NetMsg::START_REALM:
			szCallerMsg = "START_REALM";
			break;
		case NetMsg::HALT_REALM:
			szCallerMsg = "HALT_REALM";
			break;
		case NetMsg::NEXT_REALM:
			szCallerMsg = "NEXT_REALM";
			break;
		case NetMsg::PROGRESS_REALM:
			szCallerMsg = "PROGRESS_REALM";
			break;
		case NetMsg::PROCEED:
			szCallerMsg = "PROCEED";
			break;
		case NetMsg::PING:
			szCallerMsg = "PING";
			break;
		case NetMsg::RAND:
			szCallerMsg = "RAND";
			break;
		default:
			szCallerMsg = "INVALID";
		}

	// Write the message type
	prfLog->Write(szCallerMsg);
	prfLog->Write(" ");

	// Write the calling routine
	prfLog->Write(pszCaller);

	prfLog->Write("\n");

	sResult = prfLog->Error();

	return sResult;
	}
/*** 12/5/97 AJC ***/
/*** 12/7/97 AJC ***/
////////////////////////////////////////////////////////////////////////////////
// WriteInputData()
//			Write the network input data to network sync log
//		global variables used:		g_GameSettings
////////////////////////////////////////////////////////////////////////////////
int16_t WriteInputData(uint32_t *input)
	{
   int16_t sResult = SUCCESS;
	char szInput[256]; 

	// For convenience
	RFile *prfLog = &(g_GameSettings.m_rfNetSyncLog);
	
   snprintf(szInput, sizeof(szInput), "%x", *input);
   //ltoa(*input, szInput, 16);

	prfLog->Write(szInput);
	prfLog->Write("\n");

	sResult = prfLog->Error();

	return sResult;
	}

/*** 12/7/97 AJC ***/
#endif // !defined(MULTIPLAYER_REMOVED)

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
