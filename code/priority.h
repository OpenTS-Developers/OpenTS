/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "dbgprint.h"

#include <cassert>
#include <climits>
#include <cstdint>


#define PARENT(index) (index >> 1)
#define LEFT_CHILD(index) ((index << 1))
#define RIGHT_CHILD(index) ((index << 1) + 1)
#define SWAP(left, right) \
{ \
	T * temp = Heap[left]; \
	Heap[left] = Heap[right]; \
	Heap[right] = temp; \
}


template<typename T>
class PriorityQueueClass
{
		friend class AStarClass;
	public:
		PriorityQueueClass(int size);
		~PriorityQueueClass(void);

		void Clear(void);

		bool Insert(T & node);
		T * Extract_Min(void);
		T * Replace_Root(T & node);
		bool Remove_Matching(T const & item);
		void Heapify(int index);

		int Count(void) const { return(ActiveCount); }

		/*
		 * Carries the queue to or from a save game. The heap holds pointers into the
		 * caller's node array, so each slot travels as its index into that array and the
		 * array has to be handed back in on the way in.
		 */
		template<typename S>
		void Serialize(S & stream, T * nodes);

	private:
		/*
		 * This is the number of nodes currently in the queue, which is also the index of the
		 * last of them.
		 */
		int ActiveCount;

		/*
		 * This is the number of slots the queue was created with. An insert that would run
		 * past the last slot is refused, since the queue never grows.
		 */
		int Size;

		/*
		 * This points to the array of node pointers that makes up the heap. Slot zero is not
		 * used -- the lowest scoring node sits at slot one, and the children of any node lie
		 * at twice its index.
		 */
		T ** Heap;

		/// Unused
		uintptr_t MaxNodePointer;
		uintptr_t MinNodePointer;
};


template<typename T>
PriorityQueueClass<T>::PriorityQueueClass(int size)
{
	MaxNodePointer = 0;
	MinNodePointer = UINTPTR_MAX;
	ActiveCount = 0;
	Size = size;
	Heap = new T * [size + 1]();
	for (int index = 0; index <= Size; index++) {
		Heap[index] = NULL;
	}
}


template<typename T>
PriorityQueueClass<T>::~PriorityQueueClass(void)
{
	delete[] Heap;
}


template<typename T>
void PriorityQueueClass<T>::Clear(void)
{
	for (int index = 0; index <= ActiveCount; index++) {
		Heap[index] = NULL;
	}
	ActiveCount = 0;
}


template<typename T>
inline bool PriorityQueueClass<T>::Insert(T & node)
{
	unsigned index = ActiveCount + 1;
	unsigned parent_index = PARENT(index);
	float score = node.Score;

	if (index >= (unsigned)Size) {
		return(false);
	}

	while (index > 1) {
		if (Heap[parent_index]->Score <= score) {
			break;
		}
		Heap[index] = Heap[parent_index];
		index = parent_index;
		parent_index = PARENT(index);
	}

	Heap[index] = &node;
	ActiveCount++;

	if ((uintptr_t)&node > MaxNodePointer) {
		MaxNodePointer = (uintptr_t)&node;
	}

	if ((uintptr_t)&node < MinNodePointer) {
		MinNodePointer = (uintptr_t)&node;
	}

	return(true);
}


template <typename T>
inline T * PriorityQueueClass <T>::Extract_Min(void)
{
	if (ActiveCount == 0) {
		return(NULL);
	}

	T * min = Heap[1];
	Heap[1] = Heap[ActiveCount];
	Heap[ActiveCount] = NULL;
	ActiveCount--;

	Heapify(1);

	return(min);
}


template <typename T>
inline T * PriorityQueueClass<T>::Replace_Root(T & node)
{
	if (ActiveCount == 0) {
		return(&node);
	}

	T * old_root = Heap[1];

	if (node < *old_root) {
		return(&node);
	}

	Heap[1] = &node;

	Heapify(1);

	return(old_root);
}


template <typename T>
inline bool PriorityQueueClass <T>::Remove_Matching(T const & item)
{
	for (int index = 1; index <= ActiveCount; index++) {
		if (Heap[index]->Element == item.Element) {
			if (index == ActiveCount) {
				ActiveCount--;
			} else {
				T * last = Heap[ActiveCount];
				float last_score = last->Score;
				int parent = PARENT(index);
				ActiveCount--;
				if (index != 1 && Heap[parent]->Score >= last_score) {
					while ((unsigned)index > 1) {
						if (Heap[parent]->Score <= last_score) break;
						Heap[index] = Heap[parent];
						index = parent;
						parent = PARENT(index);
					}
					Heap[index] = last;
				} else {
					Heap[index] = last;
					Heapify(index);
				}
			}

			DebugString("Exiting Remove_Matching\n");
			return(true);
		}
	}

	DebugString("Exiting Remove_Matching\n");
	return(false);
}


template <typename T>
inline void PriorityQueueClass<T>::Heapify(int index)
{
	int smallest = index;

	int left = LEFT_CHILD(index);
	int right = RIGHT_CHILD(index);

	smallest = left <= Count() && *Heap[left] < *Heap[index] ? left : index;
	smallest = right <= Count() && *Heap[right] < *Heap[smallest] ? right : smallest;

	while (smallest != index) {

		SWAP(index, smallest)

		index = smallest;

		left = LEFT_CHILD(index);
		right = RIGHT_CHILD(index);

		smallest = left <= Count() && *Heap[left] < *Heap[index] ? left : index;
		smallest = right <= Count() && *Heap[right] < *Heap[smallest] ? right : smallest;
	}
}


template<typename T>
template<typename S>
void PriorityQueueClass<T>::Serialize(S & stream, T * nodes)
{
	int count = ActiveCount;
	int size = Size;

	stream.Serialize(count);
	stream.Serialize(size);

	/*
	 * The heap was sized when the queue was built and never grows, so a save describing
	 * a different one, or more nodes than fit, cannot be read into this queue.
	 */
	if (stream.Is_Loading()) {
		if (size != Size || count < 0 || count > Size) {
			stream.Fail();
			return;
		}
		ActiveCount = count;
	}

	for (int slot = 0; slot < Size; slot++) {
		int index = stream.Is_Saving() ? (int)(Heap[slot] - nodes) : 0;
		stream.Serialize(index);

		if (stream.Is_Loading()) {
			Heap[slot] = &nodes[index];

			if ((uintptr_t)Heap[slot] > MaxNodePointer) {
				MaxNodePointer = (uintptr_t)Heap[slot];
			}
			if ((uintptr_t)Heap[slot] < MinNodePointer) {
				MinNodePointer = (uintptr_t)Heap[slot];
			}
		}
	}
}

#undef PARENT
#undef LEFT_CHILD
#undef RIGHT_CHILD
#undef SWAP
