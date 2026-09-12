/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <cstddef>
#include <optional>
#include <utility>
#include <vector>


/*
 * A binary min heap ordered by the Score each element carries.
 *
 * The sift is spelled out here because the order equal scores come out in is part of the
 * simulation, and the standard heap algorithms may settle such a tie either way.
 * tests/priorityqueue holds that order.
 */
template<typename T>
class PriorityQueueClass
{
	public:
		// The size is how much room to take up front, not a limit on what the queue holds.
		explicit PriorityQueueClass(int size = 0);

		void Clear(void) { Heap.clear(); }
		void Reserve(int size) { if (size > 0) { Heap.reserve((std::size_t)size); } }
		int Count(void) const { return((int)Heap.size()); }

		void Insert(T node);
		std::optional<T> Extract_Min(void);
		T Replace_Root(T node);
		bool Remove_Matching(T const & item);

		/*
		 * Carries the queue to or from a save game as its length followed by its elements,
		 * so the element type has to describe its own members to the stream. The elements
		 * travel in heap order rather than score order, because draining the queue to sort
		 * them would build a different heap and reorder the ties within it.
		 */
		template<typename S>
		void Serialize(S & stream) { stream.Serialize(Heap); }

	private:
		void Heapify(std::size_t index);

		static std::size_t Parent(std::size_t index) { return((index - 1) / 2); }
		static std::size_t Left_Child(std::size_t index) { return((2 * index) + 1); }
		static std::size_t Right_Child(std::size_t index) { return((2 * index) + 2); }

		/*
		 * The lowest scoring element sits at slot zero, and the children of the element at
		 * any slot lie at twice its index plus one and plus two.
		 */
		std::vector<T> Heap;
};


template<typename T>
PriorityQueueClass<T>::PriorityQueueClass(int size)
{
	Reserve(size);
}


template<typename T>
inline void PriorityQueueClass<T>::Insert(T node)
{
	float score = node.Score;
	std::size_t index = Heap.size();

	Heap.emplace_back();

	while (index > 0) {
		std::size_t parent_index = Parent(index);
		if (Heap[parent_index].Score <= score) {
			break;
		}
		Heap[index] = Heap[parent_index];
		index = parent_index;
	}

	Heap[index] = node;
}


template<typename T>
inline std::optional<T> PriorityQueueClass<T>::Extract_Min(void)
{
	if (Heap.empty()) {
		return(std::nullopt);
	}

	T min = Heap.front();

	Heap.front() = Heap.back();
	Heap.pop_back();

	Heapify(0);

	return(min);
}


template<typename T>
inline T PriorityQueueClass<T>::Replace_Root(T node)
{
	if (Heap.empty()) {
		return(node);
	}

	if (node < Heap.front()) {
		return(node);
	}

	T old_root = Heap.front();
	Heap.front() = node;

	Heapify(0);

	return(old_root);
}


template<typename T>
inline bool PriorityQueueClass<T>::Remove_Matching(T const & item)
{
	for (std::size_t index = 0; index < Heap.size(); index++) {
		if (!(Heap[index].Element == item.Element)) {
			continue;
		}

		if (index + 1 == Heap.size()) {
			Heap.pop_back();
			return(true);
		}

		T last = Heap.back();
		float last_score = last.Score;
		Heap.pop_back();

		// A parent scoring exactly what the replacement scores leaves it where it stands,
		// never sifted back down. That is what the extraction order was built on.
		if (index != 0 && Heap[Parent(index)].Score >= last_score) {
			std::size_t hole = index;
			while (hole > 0) {
				std::size_t parent = Parent(hole);
				if (Heap[parent].Score <= last_score) {
					break;
				}
				Heap[hole] = Heap[parent];
				hole = parent;
			}
			Heap[hole] = last;
		} else {
			Heap[index] = last;
			Heapify(index);
		}

		return(true);
	}

	return(false);
}


template<typename T>
inline void PriorityQueueClass<T>::Heapify(std::size_t index)
{
	for (;;) {
		std::size_t left = Left_Child(index);
		std::size_t right = Right_Child(index);

		std::size_t smallest = left < Heap.size() && Heap[left] < Heap[index] ? left : index;
		smallest = right < Heap.size() && Heap[right] < Heap[smallest] ? right : smallest;

		if (smallest == index) {
			break;
		}

		std::swap(Heap[index], Heap[smallest]);
		index = smallest;
	}
}
