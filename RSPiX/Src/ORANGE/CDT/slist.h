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
// slist.h
// 
// History:
//		09/25/95 JMI	Started.
//
//		06/14/96	JMI	Added user-definable compare function.
//
//		07/31/96 MJR	Removed partial path from "blue\blue.h"
//
//		10/09/96	JMI	CSList() constructor now has option to skip initialization
//							and automatic dealloaction.
//
//		10/30/96	JMI	Changed:
//							Old label:		New label:
//							=========		=========
//							CSList			RSList
//
//////////////////////////////////////////////////////////////////////////////
//
// This module provides dynamic linked list services with a sorted key.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef H_SLIST
#define H_SLIST

#include <cstring>
#include <BLUE/Blue.h>

template <class T, class K> class RSList
	{
	protected:	// Internal types.
		typedef T*	LISTDATA;
		
		typedef K	SORTKEY, *PSORTKEY; 

		typedef struct	tagNODE
			{
			LISTDATA	ldData;
			PSORTKEY	psk;
			tagNODE*	pNext;
			tagNODE*	pPrev;
			} NODE, *PNODE, *NODESLIST;

		typedef short (*CMPCALL)(T* t1, T* t2);

	public:
		// Reposition all.
		void Resort(void)
			{
			PNODE	pnCur	= m_pnHead;	// The current insertion position.
			PNODE	pnMin	= m_pnHead;	// The minimum from insertion position on.
			PNODE pnTmp;				// The index.

			while (pnCur != nullptr)
				{
				pnTmp = pnCur->pNext;
				pnMin = pnCur;

				// Look for smallest.
				while (pnTmp != nullptr)
					{
					if (Compare(pnTmp, pnMin) < 0)
						{
						pnMin = pnTmp;
						}

					pnTmp = pnTmp->pNext;
					}
				
				// If pnMin is not pnCur . . .
				if (pnMin != pnCur)
					{
					// Disconnect min.
					pnMin->pPrev->pNext = pnMin->pNext;

					if (pnMin->pNext != nullptr)
						pnMin->pNext->pPrev = pnMin->pPrev;
					else	// pnMin was the tail.
						m_pnTail = pnMin->pPrev;

					// Connect min.
					if (pnCur->pPrev != nullptr)
						pnCur->pPrev->pNext = pnMin;
					
					pnMin->pPrev = pnCur->pPrev;
					pnCur->pPrev = pnMin;
					pnMin->pNext = pnCur;

					// If the current was the head . . .
					if (pnCur == m_pnHead)
						m_pnHead = pnMin;
					}

				pnCur = pnMin->pNext;
				}
			}

		// Reposition ld.
		// Returns 0 on success.
		int16_t Reposition(LISTDATA ld)
			{
			int16_t sResult = SUCCESS;	// Assume success.

			// Find node.
			PNODE pn = Find(ld);
			// If found . . .
			if (pn != nullptr)
				{
				// Remove from links.
				if (pn->pPrev != nullptr)
					pn->pPrev->pNext = pn->pNext;
				
				if (pn->pNext != nullptr)
					pn->pNext->pPrev = pn->pPrev;

				if (pn == m_pnHead)
					m_pnHead	= pn->pNext;

				if (pn == m_pnTail)
					m_pnTail = pn->pPrev;

				// Place node.
				Place(pn);
				}
			else
				{
				TRACE("Reposition(): Node not found.\n");
				sResult = FAILURE;
				}

			return sResult;
			}

		// Insert pnNew 
		// Returns 0 on success.
		int16_t Insert(LISTDATA ldNew, PSORTKEY psk	= nullptr)
			{
			int16_t sResult = SUCCESS; // Assume success.
			// Allocate new node.
			PNODE pnNew = new NODE;
			// If successful . . .
			if (pnNew != nullptr)
				{
				// Point to supplied data.
				pnNew->ldData	= ldNew;
				// Copy sort key.
				pnNew->psk		= psk;
				// Place node.
				Place(pnNew);
				}
			else
				{
				sResult = FAILURE;
				TRACE("RSList::Insert():  Unable to allocate new node.\n");
				}

			return sResult;
			}

		// Remove a node from the list.  If no node specified, remove current
		int16_t Remove(LISTDATA ldRem = nullptr)
			{
			int16_t sResult = SUCCESS; // Assume success.
			// Make sure the list is not empty
			if (IsEmpty() == FALSE)
				{
				PNODE pn;
				// If node provided . . .
				if (ldRem)
					{
					// Find node to remove.
					pn = Find(ldRem);
					}
				else
					{
					// Use current node.
					pn = m_pnCurrent;
					}

				// If we have a valid node to remove . . .
				if (pn != nullptr)
					{
					// If there is a node previous to pn . . .
					if (pn->pPrev != nullptr)
						{
						// Make pn's previous' next point to pn's next.
						pn->pPrev->pNext = pn->pNext;
						}
					else
						{
						// pn is the head...make pn's next the head.
						m_pnHead = pn->pNext;
						}
			
					// If there is a node after pn . . .
					if (pn->pNext != nullptr)
						{
						// Make pn's next's previous point to pn's previous.
						pn->pNext->pPrev = pn->pPrev;
						}
					else
						{
						// pn is the tail...make pn's previous the tail.
						m_pnTail = pn->pPrev;
						}
			
					// Update current info.
					m_pnPrev		= pn->pPrev;
					m_pnCurrent = nullptr;     
					m_pnNext		= pn->pNext;
			
					delete pn;
					}
				else
					{
					sResult = FAILURE * 2;
					TRACE("RSList::Remove():  Unable to find supplied node or no current node.\n");
					}
				}
			else
				{
				sResult = FAILURE;
				TRACE("RSList::Remove():  The list is empty.\n");
				}

			return sResult;
			}  

		// Get Head
		LISTDATA GetHead() 
			{
			m_pnCurrent = m_pnHead;
			if (m_pnCurrent)
				{
				m_pnPrev = m_pnCurrent->pPrev; 
				m_pnNext = m_pnCurrent->pNext;
				}
			return (m_pnCurrent ? m_pnCurrent->ldData : nullptr);
			}

		// Get Tail
		LISTDATA GetTail() 
			{
			m_pnCurrent = m_pnTail;
			if (m_pnCurrent)
				{
				m_pnPrev = m_pnCurrent->pPrev; 
				m_pnNext = m_pnCurrent->pNext;
				}
			return (m_pnCurrent ? m_pnCurrent->ldData : nullptr);
			}

		// Get node following last GetX
		LISTDATA GetNext() 
			{
			m_pnCurrent = m_pnNext;
			if (m_pnCurrent)
				{
				m_pnPrev = m_pnCurrent->pPrev; 
				m_pnNext = m_pnCurrent->pNext;
				}
			return (m_pnCurrent ? m_pnCurrent->ldData : nullptr);
			}

		// Get node following ldData
		LISTDATA GetNext(LISTDATA ldData)
			{
			LISTDATA lnRes = nullptr; // Assume failure.
			// Make sure the list is not empty
			if (IsEmpty() == FALSE)
				{
				// Attempt to find node.
				PNODE pn = Find(ldData);
				// If node found . . .
				if (pn != nullptr)
					{
					// Make global previous node found.
					m_pnPrev		= pn;
					// Make global current node's next.
					m_pnCurrent = pn->pNext;
					// If new current exists . . .
					if (m_pnCurrent != nullptr)
						{
						// Make global next current's next.
						m_pnNext	= m_pnCurrent->pNext;       
		      		// Return current's data.
						lnRes = m_pnCurrent->ldData;
						}
					else
						{
						// There is no next to supplied node.
						}
					}
				else
					{
					TRACE("RSList::GetNext(): Unable to find supplied node.\n");
					}
				}
			else
				{
				TRACE("RSList::GetNext():  The list is empty.\n");
				}

			return lnRes;
			}

		// Get node preceding last GetX
		LISTDATA GetPrev() 
			{
			m_pnCurrent = m_pnPrev;
			if (m_pnCurrent)
				{
				m_pnPrev = m_pnCurrent->pPrev; 
				m_pnNext = m_pnCurrent->pNext;
				}
			return (m_pnCurrent ? m_pnCurrent->ldData : nullptr);
			}

		// Get node preceding ldData
		LISTDATA GetPrev(LISTDATA ldData)
			{
			LISTDATA lnRes = nullptr; // Assume failure.
			// Make sure the list is not empty
			if (IsEmpty() == FALSE)
				{
				// Attempt to find node.
				PNODE pn = Find(ldData);
				// If node found . . .
				if (pn != nullptr)
					{
					// Make global next found node.
					m_pnNext		= pn;
					// Make global current node's previous.
					m_pnCurrent = pn->pPrev;
					// If new current exists . . .
					if (m_pnCurrent)
						{
						// Make global previous current's previous.
						m_pnPrev	= m_pnCurrent->pPrev;       
		      		// Return current's data.
						lnRes = m_pnCurrent->ldData;
						}
					else
						{
						// There is no previous to supplied node.
						}
					}
				else
					{
					TRACE("RSList::GetPrev():  Unable to find supplied node.\n");
					}
				}
			else
				{
				TRACE("RSList::GetPrev():  The list is empty.\n");
				}

			return lnRes;
			}

		// Get node from last GetX/Insert/etc.
		LISTDATA GetCurrent() 
			{
			return (m_pnCurrent ? m_pnCurrent->ldData : nullptr);
			}

		int16_t IsEmpty()
		{ return (int16_t)(m_pnHead == nullptr); }

		// Find the item with the key value skFind for its key or the
		// item that would follow, if that key does not exist.
		T*	FindItem(SORTKEY skFind)
			{
			NODE	nFind;
			memset(&nFind, 0, sizeof(nFind));
			T		t;
			memset(&t, 0, sizeof(t));
			
			nFind.ldData	= &t;
			nFind.psk		= &skFind;
	
			PNODE	pn = FindKey(&nFind);

			return (pn != nullptr ? pn->ldData : nullptr);
			}

		// Find the node with the value LISTDATA.
		PNODE Find(LISTDATA ldFind)
			{
			PNODE pn;
	
			// Start at head.
			pn = m_pnHead;
			while (pn != nullptr && pn->ldData != ldFind)
				{
				pn = pn->pNext;
				}                             

			return pn;               
			}

		// Find the node with the value *pnFind->psk for its key or the
		// node that would follow, if that key does not exist.
		PNODE FindKey(PNODE pnFind)
			{
			PNODE pn		= m_pnHead;

			// Search forward.
			while (pn != nullptr && Compare(pn, pnFind) < 0)
				{
				pn = pn->pNext;
				}

			return pn;               
			}

		void Reset(void)
			{ Free(); }

		// Compares two keys (uses user function if provided).
		// Returns negative	if *pn1 < *pn2.
		// Returns 0			if *pn1 == *pn2.
		// Returns positive	if *pn1 > *pn2.
		int16_t Compare(PNODE pn1, PNODE pn2)
			{
			// If user function provided . . .
			if (m_fncmp != nullptr)
				{
				return (*m_fncmp)(pn1->ldData, pn2->ldData);
				}

			ASSERT(pn1->psk != nullptr);
			ASSERT(pn2->psk != nullptr);

			return	((*pn1->psk) < (*pn2->psk))	? -1	:
						((*pn1->psk) > (*pn2->psk))	?  1	: 0;
			}

		// Set or clear the compare function used to compare nodes.
		void SetCompareFunc(CMPCALL cc)
			{ m_fncmp	= cc; }

	public:
		RSList(
			int16_t sInitialize	= TRUE)	// If this flag is FALSE, no initialization
												// or freeing will be done.  It will be the
												// user's responsibility!
			{
			if (sInitialize != FALSE)
				{
				m_pnHead    = nullptr;  
				m_pnPrev		= nullptr;
				m_pnCurrent = nullptr;
				m_pnNext		= nullptr;
				m_pnTail    = nullptr;
				m_fncmp		= nullptr;
				}

			m_sInitialize	= sInitialize;
			}

		~RSList()
			{
			if (m_sInitialize != FALSE)
				{
				// Free all nodes.
				Free();
				}
			}

	protected:
		
		// Place pnPlace in the list in the proper sorted position.
		// Cannot fail.
		void Place(PNODE pnPlace)
			{
			// Find the node to insert before.
			PNODE pnBefore = FindKey(pnPlace);
			// Move to last of this key.
			while (pnBefore != nullptr && Compare(pnBefore, pnPlace) == SUCCESS)
				{
				pnBefore	= pnBefore->pNext;
				}

			// If node found . . .
			if (pnBefore != nullptr)
				{
				// Point previous to node to insert before's previous.
				pnPlace->pPrev = pnBefore->pPrev;
				// If there is a previous . . .
				if (pnPlace->pPrev != nullptr)
					{
					// Have previous' next point to new.
					pnPlace->pPrev->pNext = pnPlace;
					}
				else
					{
					// Make new the head.
					m_pnHead = pnPlace;
					}
				// Point new's next to node to insert before.
				pnPlace->pNext = pnBefore;
				// Point before's previous to new.
				pnBefore->pPrev = pnPlace;
				}
			else
				{
				// Make new tail.
				pnPlace->pNext = nullptr;

				// If a tail exists . . .
				if (m_pnTail != nullptr)
					{
					// Make the tail's next point to the new.
					m_pnTail->pNext = pnPlace;
					// Make the new's previous point to the tail.
					pnPlace->pPrev    = m_pnTail;
					}
				else
					{
					// Make the new's previous nullptr.
					pnPlace->pPrev	= nullptr;
					// Make the head the tail, since, if no tail exists,
					// there is no head.
					m_pnHead			= pnPlace;
					}                 
				// Make new node the tail.
				m_pnTail			= pnPlace;
				}

			// Update current info.
			m_pnPrev		= pnPlace->pPrev;
			m_pnCurrent = pnPlace;       
			m_pnNext		= pnPlace->pNext;
			}

		// Free the entire list.
		void Free(void)
			{
			PNODE pn, pnTemp;

			// Start at the head.
			pn = m_pnHead;
			while (pn)
				{
				pnTemp = pn;
				pn = pn->pNext;

				delete pnTemp;  
				}                                            
	
			// Clear all node pointers.
			m_pnHead    = nullptr;  
			m_pnPrev		= nullptr;
			m_pnCurrent = nullptr;
			m_pnNext		= nullptr;
			m_pnTail    = nullptr;
			}

		PNODE		m_pnHead;
		PNODE		m_pnPrev;
		PNODE		m_pnCurrent;
		PNODE		m_pnNext;
		PNODE		m_pnTail;
		CMPCALL	m_fncmp;
		int16_t		m_sInitialize;		// TRUE if this item should handle intializing
											// and freeing the list and members.
	};
   
#endif // H_SLIST
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
