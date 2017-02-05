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
///////////////////////////////////////////////////////////////////////////////
//
//	task.cpp
// 
// History:
//		06/14/95 JMI	Started.
//
//		10/30/96	JMI	Changed:
//							Old label:		New label:
//							=========		=========
//							CTask				RTask
//							TASK_FUNC		TaskFunc
//							TASK_TIMEFUNC	TimeFunc
//							m_tf				m_fnTask
//							CList				RList
//
//////////////////////////////////////////////////////////////////////////////
//
//	Calls tasks stored in its list of tasks based on a time interval.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef TIME_TRAVEL_IS_REAL
#error I AM ARCHAIC - DO NOT USE
#endif

// Blue //////////////////////////////////////////////////////////////////////
#include "Blue.h"

#ifdef PATHS_IN_INCLUDES
	// Green //////////////////////////////////////////////////////////////////
	#include "GREEN/Task/task.h"
#else
	// Green //////////////////////////////////////////////////////////////////
	#include "task.h"
#endif // PATHS_IN_INCLUDES


//////////////////////////////////////////////////////////////////////////////
// Module typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) variables.
//////////////////////////////////////////////////////////////////////////////

////////////////// Instantiate Static members ////////////////////////////////
RList<RTask>	RTask::ms_listActive;			// List of tasks to be called.

//////////////////////////////////////////////////////////////////////////////
///////////////////////// Allocation /////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
		
//////////////////////////////////////////////////////////////////////////////
//
// Default constructor.
//
//////////////////////////////////////////////////////////////////////////////
RTask::RTask(void)
	{
	Reset();
	}

//////////////////////////////////////////////////////////////////////////////
//
// Special constructor that passes parms on to Init().
//
//////////////////////////////////////////////////////////////////////////////
RTask::RTask(TaskFunc tf, uint32_t ulUser)
	{
	Reset();

	Init(tf, ulUser);
	}

//////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
//////////////////////////////////////////////////////////////////////////////
RTask::~RTask(void)
	{
	Kill();
	}

//////////////////////////////////////////////////////////////////////////////
///////////////////////////// Querries ///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
///////////////////////////// Methods ////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Initialize task info.
//
// Returns nothing.
//
//////////////////////////////////////////////////////////////////////////////
void RTask::Init(TaskFunc tf, uint32_t ulUser)
	{
	ASSERT(tf != NULL);

	m_fnTask		= tf;
	m_ulUser	= ulUser;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Kill task info.
//
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
int16_t RTask::Kill(void)
	{
	int16_t sRes = 0;	// Assume success.
	
	// Attempt to stop the task . . .
	if (Suspend() == 0)
		{
		// Clear the members.
		Reset();
		}
	else
		{
		TRACE("Kill(): Suspend() failed.\n");
		sRes = -1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Start this task.
//
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
int16_t RTask::Start(void)
	{
	int16_t sRes = 0;	// Assume success.

	if (m_sActive == FALSE)
		{
		// Attempt to add to list . . .
		if (ms_listActive.Add(this) == 0)
			{
			// Set active flag.  So we don't have to traverse the list later just
			// to determine whether or not this is active.
			m_sActive = TRUE;
			
			// Set the next call time.
			m_lNextExpiration		= GetTime() + m_lInterval;
			
			// If an error has occurred at this point . . .
			if (sRes != 0)
				{
				Suspend();
				}
			}
		else
			{
			TRACE("Start(): Unable to add this task to active list.\n");
			sRes = -1;
			}
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Suspend this task (can be restarted after this is call).
//
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////////////
int16_t RTask::Suspend(void)
	{
	int16_t sRes = 0;	// Assume success.

	if (m_sActive == TRUE)
		{
		// Attempt to remove from list . . .
		if (ms_listActive.Remove(this) == 0)
			{
			// Clear active flag.
			m_sActive = FALSE;
			}
		else
			{
			TRACE("Suspend(): Unable to remove this task from active list.\n");
			sRes = -1;
			}
		}

	return sRes;
	}
		
//////////////////////////////////////////////////////////////////////////////
///////////////////////////// Internal Methods ///////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Initialize instantiable members.
//
// Returns nothing.
//
//////////////////////////////////////////////////////////////////////////////
void RTask::Reset(void)
	{
	// Clear instantiable members.
	m_fnTask			= NULL;
	m_ulUser		= 0L;

	m_sActive	= FALSE;

	m_fnTime		= NULL;
	}

//////////////////////////////////////////////////////////////////////////////
///////////////////////////// Static functions ///////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Critical call for all instances (static).  Checks each RTask's next
// execution time against the current time and, if the current time is greater
// or equal to the next time, calls the task.
//
// Returns nothing.
//
//////////////////////////////////////////////////////////////////////////////
void RTask::Do(void)
	{
	int32_t	lCurTime;

	// Go through each node of the list checking its next execution time
	// against the current.
	PTASK	ptask	= ms_listActive.GetHead();

	while (ptask != NULL)
		{
		// Get time for ptask.
		lCurTime = ptask->GetTime();
	
		// If time has expired  . . .
		if (lCurTime >= ptask->m_lNextExpiration)
			{
			ASSERT(ptask->m_fnTask != NULL);

			// Call task.
			(*(ptask->m_fnTask))(ptask->m_ulUser);
			// Set next expiration time.
			ptask->m_lNextExpiration = lCurTime + ptask->m_lInterval;
			}

		// Get the next task.
		ptask	= ms_listActive.GetNext();
		}
	}


//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
