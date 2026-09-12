/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2025 Electronic Arts Inc.
 * Copyright 2026 OpenTS contributors
 *
 * Contains material derived from Electronic Arts source code.
 * Modified by OpenTS contributors, 2026.
 * EA's GPLv3 Section 7 additional terms and supplemental warranty
 * disclaimers apply; see LICENSE.md.
 ******************************************************************************/

#pragma once

#include "win.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <math.h>
#include <new>


template <class T>
class ArrayList
{
public:
	ArrayList(void);
	ArrayList(ArrayList<T>& other);
	~ArrayList(void);

	void			clear(void);
	void			setEmpty(void) { Entries_ = 0; }

	char			add(T& node, int pos);
	char			addTail(T& node);
	char			addHead(T& node);
	char			addSortedAsc(T& node);
	char			addSortedDes(T& node);

	char			remove(T& node, int pos);
	char			remove(int pos);
	char			removeHead(T& node);
	char			removeTail(T& node);

	char			replace(T& node, int pos);

	char			get(T& node, int pos) const;
	char			getHead(T& node) const;
	char			getTail(T& node) const;

	char			getPointer(T** node, int pos) const;

	int				length(void) const;

	char			setSize(int newsize, T& filler);

	//void			print(FILE * out);

	ArrayList<T>&	operator=(ArrayList<T>& other);

private:
	int				_sortedLookup(T& target, int ascending);
	int				Entries_;
	int				Slots_;

	T *				Vector_;

	enum
	{
		INITIAL_SIZE = 10
	};

	char			growVector(void);
	char			shrinkVector(void);
};

template <class T>
ArrayList<T>::ArrayList(void)
{
	Entries_ = 0;
	Slots_ = 0;
	Vector_ = NULL;
}

template <class T>
ArrayList<T>::ArrayList(ArrayList<T>& other)
{
	Entries_ = 0;
	Slots_ = 0;
	Vector_ = NULL;
	(*this) = other;
}

template <class T>
ArrayList<T>::~ArrayList(void)
{
	clear();

	delete[]((unsigned char*)Vector_);

}

template <class T>
ArrayList<T>& ArrayList<T>::operator=(ArrayList<T>& other)
{
	T node;
	clear();
	for (int i = 0; i < other.length(); i++)
	{
		other.get(node, i);
		addTail(node);
	}
	return(*this);
}

template <class T>
void ArrayList<T>::clear(void)
{
	for (int i = 0; i < Entries_; i++)
	{
		(Vector_ + i)->~T();

	}
	Entries_ = 0;
}

template <class T>
char ArrayList<T>::setSize(int newsize, T& filler)
{
	int oldEntries = Entries_;
	Entries_ = newsize;

	if (newsize < 0)
		return(false);

	while (newsize > Slots_)
		growVector();

	for (int i = oldEntries; i < Entries_; i++)
	{

		new((void*)(Vector_ + i)) T(filler);
	}

	if ((Entries_ * 3) <= Slots_)
		shrinkVector();

	return(true);
}

template <class T>
char ArrayList<T>::add(T& node, int pos)
{
	if (pos > Entries_)
		pos = Entries_;
	if (pos >= Slots_)
		growVector();
	if (Entries_ >= Slots_)
		growVector();

	if (pos < Entries_)
		memmove(Vector_ + pos + 1, Vector_ + pos, sizeof(T) * (Entries_ - pos));

	new((void*)(Vector_ + pos)) T((T&)node);
	Entries_++;
	return(TRUE);
}

template <class T>
char ArrayList<T>::addHead(T& node)
{
	return(add(node, 0));
}

template <class T>
char ArrayList<T>::addTail(T& node)
{
	return(add(node, length()));
}

template <class T>
char ArrayList<T>::addSortedAsc(T& node)
{
	int pos = _sortedLookup(node, 1);
	return(add(node, pos));
}

template <class T>
char ArrayList<T>::addSortedDes(T& node)
{
	int pos = _sortedLookup(node, 0);
	return(add(node, pos));
}

template <class T>
int ArrayList<T>::_sortedLookup(T& target, int ascending)
{
	int	low, mid, high;
	T * lowtarget;
	T * hightarget;
	T * midtarget;

	if (Entries_ == 0)
		return(0);

	low = 0;
	high = Entries_ - 1;
	while (1)
	{
		assert(low <= high);
		mid = low + (int)(floor(((double)high - (double)low) / (double)2));

		getPointer(&lowtarget, low);
		getPointer(&hightarget, high);
		getPointer(&midtarget, mid);

		if (*midtarget == target)  return(mid);

		if (high == low)
		{
			if (ascending)
			{
				if (target <= *lowtarget)
					return(low);
				else
					return(low + 1);
			}
			else
			{
				if (target <= *lowtarget)
					return(low + 1);
				else
					return(low);
			}
		}

		if ((high - low) == 1)
		{
			if (ascending)
			{
				if (target <= *lowtarget)
					return(low);
				else if (target <= *hightarget)
					return(high);
				else
					return(high + 1);
			}
			else
			{
				if (target <= *hightarget)
					return(high + 1);
				else if (target <= *lowtarget)
					return(high);
				else
					return(low);
			}
		}

		if (ascending)
		{
			if (target < *midtarget)
				high = mid;
			else
				low = mid;
		}
		else
		{
			if (target < *midtarget)
				low = mid;
			else
				high = mid;
		}
	}
}

template <class T>
char ArrayList<T>::replace(T& node, int pos)
{
	if (Entries_ == 0)
		return(FALSE);
	if (pos < 0)
		pos = 0;
	if (pos >= Entries_)
		pos = Entries_ - 1;

	(Vector_ + pos)->~T();

	new((void*)(Vector_ + pos)) T(node);

	return(TRUE);
}

template <class T>
char ArrayList<T>::remove(int pos)
{
	if (Entries_ == 0)
		return(FALSE);
	if (pos < 0)
		pos = 0;
	if (pos >= Entries_)
		pos = Entries_ - 1;

	(Vector_ + pos)->~T();

	memmove(Vector_ + pos, Vector_ + pos + 1, sizeof(T) * (Entries_ - pos - 1));

	Entries_--;

	if ((Entries_ * 3) <= Slots_)
		shrinkVector();

	return(TRUE);
}

template <class T>
char ArrayList<T>::remove(T& node, int pos)
{
	char retval;
	retval = get(node, pos);
	if (retval == FALSE)
		return(FALSE);
	return(remove(pos));
}

template <class T>
char ArrayList<T>::removeHead(T& node)
{
	return(remove(node, 0));
}

template <class T>
char ArrayList<T>::removeTail(T& node)
{
	return(remove(node, Entries_ - 1));
}

template <class T>
char ArrayList<T>::getPointer(T** node, int pos) const
{
	if ((pos < 0) || (pos >= Entries_))
		return(FALSE);
	*node = &(Vector_[pos]);
	return(TRUE);
}

template <class T>
char ArrayList<T>::get(T& node, int pos) const
{
	if ((pos < 0) || (pos >= Entries_))
		return(FALSE);
	node = Vector_[pos];
	return(TRUE);
}

template <class T>
char ArrayList<T>::getHead(T& node) const
{
	return(get(node, 0));
}

template <class T>
char ArrayList<T>::getTail(T& node) const
{
	return(get(node, Entries_ - 1));
}

/// Not in TS
#if 0
template <class T>
void ArrayList<T>::print(FILE * out)
{
	fprintf(out, "--------------------\n");

	fprintf(out, "Entries: %d  Slots:  %d  sizeof(T): %d\n", Entries_, Slots_,
		sizeof(T));
	fprintf(out, "--------------------\n");
}
#endif

template <class T>
int ArrayList<T>::length(void) const
{
	return(Entries_);
}

template <class T>
char ArrayList<T>::growVector(void)
{
	if (Entries_ < Slots_)
		return(FALSE);

	int   newSlots = Entries_ * 2;
	if (newSlots < INITIAL_SIZE)
		newSlots = INITIAL_SIZE;

	T * newVector = (T*)(new unsigned char[newSlots * sizeof(T)]);
	memset(newVector, 0, newSlots * sizeof(T));

	if (Vector_ != NULL)
		memcpy(newVector, Vector_, Entries_ * sizeof(T));

	delete[]((unsigned char*)Vector_);

	Vector_ = newVector;
	Slots_ = newSlots;

	return(TRUE);
}

template <class T>
char ArrayList<T>::shrinkVector(void)
{

	if ((Entries_ * 3) > Slots_)
		return(FALSE);

	int   newSlots = Slots_ / 2;
	if (newSlots < INITIAL_SIZE)
		newSlots = INITIAL_SIZE;

	if (newSlots >= Slots_)
		return(FALSE);

	T * newVector = (T*)(new unsigned char[newSlots * sizeof(T)]);

	if (Vector_ != NULL)
		memcpy(newVector, Vector_, Entries_ * sizeof(T));

	delete[]((unsigned char*)Vector_);

	Vector_ = newVector;
	Slots_ = newSlots;

	return(TRUE);
}
