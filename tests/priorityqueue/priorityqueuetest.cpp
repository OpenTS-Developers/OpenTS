/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Holds the priority queue to its extraction order: what comes out for a given set of
// scores, and above all what comes out when several nodes carry the same score.
//
// The tie expectations are why this file exists. Random maps are built locally on every
// peer from a shared seed, and map generation seeds its spreads at exactly Score = 0.0f,
// so a reordering among equal scores would change the maps peers build from one build to
// the next. Every sequence below was recorded from the queue as it stood before its
// storage was rewritten, and every result crosses the API through Element_Of, so the same
// expectations read the same whether extraction hands back a pointer into the caller's
// nodes or a value.
//
// Needs no game data.

#include "priority.h"

#include <cstdio>
#include <cstring>
#include <optional>


namespace {

int Failures = 0;
int Checked = 0;


/// Stands in for a node the queue orders. It carries CellNode's contract rather than the
/// type itself, which would drag the map and COM headers in behind it: the queue orders on
/// Score alone and matches on Element alone, and every comparison widens to double exactly
/// as code/nodes.h does.
struct TestNode {
	int Element;
	float Score;

	bool operator==(TestNode const & other) const { return((double)Score == (double)other.Score); }
	bool operator!=(TestNode const & other) const { return((double)Score != (double)other.Score); }
	bool operator<(TestNode const & other) const { return((double)Score < (double)other.Score); }
	bool operator>(TestNode const & other) const { return((double)Score > (double)other.Score); }
	bool operator<=(TestNode const & other) const { return((double)Score <= (double)other.Score); }
	bool operator>=(TestNode const & other) const { return((double)Score >= (double)other.Score); }
};


/// Stands for the queue handing nothing back.
int const EMPTY = -1;


void Check(bool passed, char const * what)
{
	Checked++;
	if (!passed) {
		std::printf("FAILED %s\n", what);
		Failures++;
	}
}


/*
 * These three name the element behind whatever the queue handed back. Extraction and
 * Replace_Root return a pointer into the caller's nodes today and a value once the queue
 * holds its elements itself, and every expectation below is written against the element
 * number so that neither spelling reaches the cases.
 */
int Element_Of(TestNode const * node)
{
	return(node != NULL ? node->Element : EMPTY);
}


int Element_Of(TestNode const & node)
{
	return(node.Element);
}


int Element_Of(std::optional<TestNode> const & node)
{
	return(node ? node->Element : EMPTY);
}


template<typename Q>
int Pop(Q & queue)
{
	return(Element_Of(queue.Extract_Min()));
}


template<typename Q>
int Replace(Q & queue, TestNode & node)
{
	return(Element_Of(queue.Replace_Root(node)));
}


/// Fills the queue from a score list, numbering each node by its position in that list
/// counting from one. The caller's array has to outlive the queue, because the queue held
/// pointers into it before it held values.
template<typename Q>
void Fill(Q & queue, TestNode * nodes, float const * scores, int count)
{
	for (int index = 0; index < count; index++) {
		nodes[index].Element = index + 1;
		nodes[index].Score = scores[index];
		queue.Insert(nodes[index]);
	}
}


/// Empties the queue into a comma separated list of element numbers, which is the form
/// every expectation below is written in.
template<typename Q>
void Drain(Q & queue, char * out, int size)
{
	int used = 0;
	out[0] = '\0';

	for (;;) {
		int element = Pop(queue);
		if (element == EMPTY) {
			break;
		}

		int written = std::snprintf(out + used, size - used, used == 0 ? "%d" : ",%d", element);
		if (written <= 0 || written >= size - used) {
			break;
		}
		used += written;
	}
}


template<typename Q>
void Check_Drain(Q & queue, char const * expected, char const * what)
{
	char got[256];
	Drain(queue, got, sizeof(got));

	Checked++;
	if (std::strcmp(got, expected) != 0) {
		std::printf("FAILED %s\n    expected %s\n    got      %s\n", what, expected, got);
		Failures++;
	}
}


void Check_Ordering(void)
{
	{
		PriorityQueueClass<TestNode> queue(64);
		TestNode nodes[8];
		float const scores[8] = {5.0f, 1.0f, 8.0f, 3.0f, 7.0f, 2.0f, 6.0f, 4.0f};
		Fill(queue, nodes, scores, 8);

		Check(queue.Count() == 8, "the queue counts what went in");
		Check_Drain(queue, "2,6,4,8,1,7,5,3", "a shuffled set comes out in score order");
	}

	{
		PriorityQueueClass<TestNode> queue(64);
		TestNode nodes[5];
		float const scores[5] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
		Fill(queue, nodes, scores, 5);

		Check_Drain(queue, "1,2,3,4,5", "an ascending set comes out unchanged");
	}

	{
		PriorityQueueClass<TestNode> queue(64);
		TestNode nodes[5];
		float const scores[5] = {5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
		Fill(queue, nodes, scores, 5);

		Check_Drain(queue, "5,4,3,2,1", "a descending set comes out reversed");
	}

	{
		PriorityQueueClass<TestNode> queue(64);
		TestNode node;
		node.Element = 1;
		node.Score = 0.0f;
		queue.Insert(node);

		Check_Drain(queue, "1", "a queue holding one node hands it back");
	}

	{
		PriorityQueueClass<TestNode> queue(64);

		Check(queue.Count() == 0, "a fresh queue counts nothing");
		Check(Pop(queue) == EMPTY, "an empty queue hands nothing back");
		Check(Pop(queue) == EMPTY, "an empty queue hands nothing back twice running");
	}
}


void Check_Ties(void)
{
	{
		PriorityQueueClass<TestNode> queue(64);
		TestNode nodes[8];
		float const scores[8] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
		Fill(queue, nodes, scores, 8);

		Check_Drain(queue, "1,8,7,6,5,4,3,2", "eight nodes at one score come out in a fixed order");
	}

	{
		PriorityQueueClass<TestNode> queue(64);
		TestNode nodes[8];
		float const scores[8] = {5.0f, 1.0f, 5.0f, 3.0f, 1.0f, 9.0f, 3.0f, 1.0f};
		Fill(queue, nodes, scores, 8);

		Check_Drain(queue, "2,5,8,4,7,1,3,6", "nodes sharing a score come out in a fixed order");
	}

	{
		PriorityQueueClass<TestNode> queue(64);
		TestNode nodes[4];
		float const scores[4] = {0.0f, -0.0f, 0.0f, -0.0f};
		Fill(queue, nodes, scores, 4);

		Check_Drain(queue, "1,4,3,2", "a negative zero ties with a positive one");
	}

	{
		PriorityQueueClass<TestNode> queue(64);
		TestNode nodes[8];
		float const scores[8] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

		Fill(queue, nodes, scores, 8);
		queue.Clear();
		Fill(queue, nodes, scores, 8);

		Check_Drain(queue, "1,8,7,6,5,4,3,2", "a cleared queue ties the same way a fresh one does");
	}
}


void Check_Replace_Root(void)
{
	{
		PriorityQueueClass<TestNode> queue(64);
		TestNode offered;
		offered.Element = 9;
		offered.Score = 4.0f;

		Check(Replace(queue, offered) == 9, "an empty queue hands the offered node straight back");
		Check(queue.Count() == 0, "an empty queue stays empty");
	}

	{
		PriorityQueueClass<TestNode> queue(64);
		TestNode nodes[4];
		float const scores[4] = {5.0f, 7.0f, 6.0f, 8.0f};
		Fill(queue, nodes, scores, 4);

		TestNode offered;
		offered.Element = 9;
		offered.Score = 2.0f;

		Check(Replace(queue, offered) == 9, "a node below the root comes straight back");
		Check(queue.Count() == 4, "a node below the root does not enter");
		Check_Drain(queue, "1,3,2,4", "the queue is untouched by a node below the root");
	}

	{
		PriorityQueueClass<TestNode> queue(64);
		TestNode nodes[4];
		float const scores[4] = {5.0f, 7.0f, 6.0f, 8.0f};
		Fill(queue, nodes, scores, 4);

		TestNode offered;
		offered.Element = 9;
		offered.Score = 6.5f;

		Check(Replace(queue, offered) == 1, "a node above the root turns the root out");
		Check(queue.Count() == 4, "replacing the root leaves the count alone");
		Check_Drain(queue, "3,9,2,4", "the offered node takes its place in order");
	}

	{
		PriorityQueueClass<TestNode> queue(64);
		TestNode nodes[4];
		float const scores[4] = {5.0f, 7.0f, 6.0f, 8.0f};
		Fill(queue, nodes, scores, 4);

		TestNode offered;
		offered.Element = 9;
		offered.Score = 5.0f;

		Check(Replace(queue, offered) == 1, "a node level with the root turns the root out");
		Check_Drain(queue, "9,3,2,4", "a node level with the root takes its place");
	}
}


void Check_Remove_Matching(void)
{
	float const scores[7] = {5.0f, 7.0f, 6.0f, 9.0f, 8.0f, 10.0f, 11.0f};

	{
		PriorityQueueClass<TestNode> queue(64);
		TestNode nodes[7];
		Fill(queue, nodes, scores, 7);

		TestNode wanted;
		wanted.Element = 7;
		wanted.Score = 0.0f;

		Check(queue.Remove_Matching(wanted) == true, "a node in the last slot is found");
		Check(queue.Count() == 6, "removing a node drops the count");
		Check_Drain(queue, "1,3,2,5,4,6", "the rest come out in score order");
	}

	{
		PriorityQueueClass<TestNode> queue(64);
		TestNode nodes[7];
		Fill(queue, nodes, scores, 7);

		TestNode wanted;
		wanted.Element = 1;
		wanted.Score = 0.0f;

		Check(queue.Remove_Matching(wanted) == true, "the node at the root is found");
		Check_Drain(queue, "3,2,5,4,6,7", "the queue re-forms around a removed root");
	}

	{
		PriorityQueueClass<TestNode> queue(64);
		TestNode nodes[7];
		Fill(queue, nodes, scores, 7);

		TestNode wanted;
		wanted.Element = 3;
		wanted.Score = 0.0f;

		Check(queue.Remove_Matching(wanted) == true, "a node in a middle slot is found");
		Check_Drain(queue, "1,2,5,4,6,7", "the queue re-forms around a removed middle node");
	}

	{
		PriorityQueueClass<TestNode> queue(64);
		TestNode nodes[7];
		Fill(queue, nodes, scores, 7);

		TestNode wanted;
		wanted.Element = 2;
		wanted.Score = 0.0f;

		Check(queue.Remove_Matching(wanted) == true, "a node with a lighter parent is found");
		Check_Drain(queue, "1,3,5,4,6,7", "the queue re-forms around it");
	}

	{
		PriorityQueueClass<TestNode> queue(64);
		TestNode nodes[7];
		Fill(queue, nodes, scores, 7);

		TestNode wanted;
		wanted.Element = 99;
		wanted.Score = 0.0f;

		Check(queue.Remove_Matching(wanted) == false, "a node that is not held is not found");
		Check(queue.Count() == 7, "a search that finds nothing removes nothing");
		Check_Drain(queue, "1,3,2,5,4,6,7", "a search that finds nothing leaves the order alone");
	}

	{
		PriorityQueueClass<TestNode> queue(64);
		TestNode nodes[4];
		float const shared[4] = {5.0f, 7.0f, 6.0f, 9.0f};

		for (int index = 0; index < 4; index++) {
			nodes[index].Element = (index == 1 || index == 3) ? 2 : index + 1;
			nodes[index].Score = shared[index];
			queue.Insert(nodes[index]);
		}

		TestNode wanted;
		wanted.Element = 2;
		wanted.Score = 0.0f;

		Check(queue.Remove_Matching(wanted) == true, "one of two nodes sharing an element is found");
		Check_Drain(queue, "1,3,2", "the queue walks its slots rather than the order they went in");
	}
}


void Check_Clear(void)
{
	PriorityQueueClass<TestNode> queue(64);
	TestNode nodes[6];
	float const scores[6] = {4.0f, 2.0f, 6.0f, 1.0f, 5.0f, 3.0f};

	Fill(queue, nodes, scores, 6);
	queue.Clear();

	Check(queue.Count() == 0, "a cleared queue counts nothing");
	Check(Pop(queue) == EMPTY, "a cleared queue hands nothing back");

	Fill(queue, nodes, scores, 6);
	Check_Drain(queue, "4,2,6,1,5,3", "a refilled queue orders as a fresh one does");
}


void Check_Capacity(void)
{
	// The size a queue is built with is only how much room it takes up front.
	PriorityQueueClass<TestNode> queue(8);
	TestNode nodes[10];
	float const scores[10] = {9.0f, 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f};
	Fill(queue, nodes, scores, 10);

	Check(queue.Count() == 10, "a queue built with eight slots takes ten nodes");
	Check_Drain(queue, "10,9,8,7,6,5,4,3,2,1", "every node it took comes out in score order");
}

} // namespace


int main(void)
{
	Check_Ordering();
	Check_Ties();
	Check_Replace_Root();
	Check_Remove_Matching();
	Check_Clear();
	Check_Capacity();

	std::printf("%-52s %s\n", "Priority queue extraction order",
		Failures == 0 ? "ok" : "FAILED");
	std::printf("checked %d cases, %d mismatches\n", Checked, Failures);

	return(Failures == 0 ? 0 : 1);
}
