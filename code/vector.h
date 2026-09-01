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

/* $Header: /CounterStrike/VECTOR.H 1     3/03/97 10:26a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : VECTOR.H                                                     *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 02/19/95                                                     *
 *                                                                                             *
 *                  Last Update : March 13, 1995 [JLB]                                         *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   VectorClass<T>::VectorClass -- Constructor for vector class.                              *
 *   VectorClass<T>::~VectorClass -- Default destructor for vector class.                      *
 *   VectorClass<T>::VectorClass -- Copy constructor for vector object.                        *
 *   VectorClass<T>::operator = -- The assignment operator.                                    *
 *   VectorClass<T>::operator == -- Equality operator for vector objects.                      *
 *   VectorClass<T>::Clear -- Frees and clears the vector.                                     *
 *   VectorClass<T>::Resize -- Changes the size of the vector.                                 *
 *   DynamicVectorClass<T>::DynamicVectorClass -- Constructor for dynamic vector.              *
 *   DynamicVectorClass<T>::Resize -- Changes the size of a dynamic vector.                    *
 *   DynamicVectorClass<T>::Add -- Add an element to the vector.                               *
 *   DynamicVectorClass<T>::Delete -- Remove the specified object from the vector.             *
 *   DynamicVectorClass<T>::Delete -- Deletes the specified index from the vector.             *
 *   VectorClass<T>::ID -- Pointer based conversion to index number.                           *
 *   VectorClass<T>::ID -- Finds object ID based on value.                                     *
 *   DynamicVectorClass<T>::ID -- Find matching value in the dynamic vector.                   *
 *   DynamicVectorClass<T>::Uninitialized_Add -- Add an empty place to the vector.             *
 *   DynamicVectorClass<T>::Insert -- insert an object at the desired index                    *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <ranges>
#include <source_location>
#include <span>
#include <type_traits>
#include <utility>


/**************************************************************************
**	This is a general purpose vector class. A vector is defined by this
**	class, as an array of arbitrary objects where the array can be dynamically
**	sized. Because is deals with arbitrary object types, it can handle everything.
**	As a result of this, it is not terribly efficient for integral objects (such
**	as char or int). It will function correctly, but the copy constructor
**	could be highly optimized if the integral type were known.
**	This efficiency can be implemented by deriving an integral vector template
**	from this one in order to supply more efficient routines.
*/
template<class T>
class VectorClass
{
	public:
		using value_type = T;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using reference = T &;
		using const_reference = T const &;
		using iterator = T *;
		using const_iterator = T const *;

		VectorClass(int size=0);
		VectorClass(VectorClass<T> const &);		// Copy constructor.
		VectorClass(VectorClass<T> &&) noexcept;	// Move constructor.
		VectorClass(std::initializer_list<T> list);
		~VectorClass(void);

		T & operator[](int index) {assert((unsigned)index < (unsigned)VectorMax); return(Vector[index]);};
		T const & operator[](int index) const {assert((unsigned)index < (unsigned)VectorMax); return(Vector[index]);};

		VectorClass<T> & operator =(VectorClass<T> const &); // Assignment operator.
		VectorClass<T> & operator =(VectorClass<T> &&) noexcept;

		bool Resize(int newsize);
		void Clear(void);
		[[nodiscard]] int Length(void) const noexcept {return(VectorMax);};
		[[nodiscard]] int ID(T const * ptr) const;	// Pointer based identification.
		[[nodiscard]] int ID(T const & ptr) const;	// Value based identification.

		[[nodiscard]] T * begin(void) noexcept {return(Vector);};
		[[nodiscard]] T * end(void) noexcept {return(Vector + VectorMax);};
		[[nodiscard]] T const * begin(void) const noexcept {return(Vector);};
		[[nodiscard]] T const * end(void) const noexcept {return(Vector + VectorMax);};

		operator std::span<T>(void) noexcept {return(std::span<T>(Vector, (std::size_t)VectorMax));};
		operator std::span<T const>(void) const noexcept {return(std::span<T const>(Vector, (std::size_t)VectorMax));};

		/*
		 * Carries the vector to or from a save game, its length first and then the
		 * elements. Loading sizes it before any element registers the slot it occupies.
		 */
		template<typename S>
		void Serialize(S & stream, std::source_location const & where = std::source_location::current())
		{
			int count = VectorMax;
			stream.Serialize(count);

			if (stream.Is_Loading()) {
				if (count < 0) {
					stream.Fail();
					return;
				}
				Clear();
				if (count > 0) {
					Resize(count);
				}
			}

			if constexpr (std::is_arithmetic_v<T> || std::is_enum_v<T>) {
				if (count > 0) {
					stream.Serialize_Bytes(Vector, (int)(sizeof(T) * count));
				}
			} else {
				for (int index = 0; index < count; index++) {
					// Elements report against the member's call site, not against this loop.
					if constexpr (requires { stream.Serialize(Vector[index], where); }) {
						stream.Serialize(Vector[index], where);
					} else {
						stream.Serialize(Vector[index]);
					}
				}
			}
		}

	protected:

		/*
		**	This is a pointer to the allocated vector array of elements.
		*/
		T * Vector;

		/*
		**	This is the maximum number of elements allowed in this vector.
		*/
		int VectorMax;
};


/***********************************************************************************************
 * VectorClass<T>::VectorClass -- Constructor for vector class.                                *
 *                                                                                             *
 *    This constructor for the vector class is passed the initial size of the vector. The      *
 *    vector is allocated out of free store (with the "new" operator).                         *
 *                                                                                             *
 * INPUT:   size  -- The number of elements to initialize this vector to.                      *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
VectorClass<T>::VectorClass(int size) :
	Vector(nullptr),
	VectorMax(size)
{
	/*
	**	Allocate the vector. The default constructor will be called for every
	**	object in this vector.
	*/
	if (size > 0) {
		Vector = new T[size];
	} else {
		VectorMax = 0;
	}
}


/***********************************************************************************************
 * VectorClass<T>::~VectorClass -- Default destructor for vector class.                        *
 *                                                                                             *
 *    This is the default destructor for the vector class. It will deallocate any memory       *
 *    that it may have allocated.                                                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
VectorClass<T>::~VectorClass(void)
{
	Clear();
}


/***********************************************************************************************
 * VectorClass<T>::VectorClass -- Copy constructor for vector object.                          *
 *                                                                                             *
 *    This is the copy constructor for the vector class. It will duplicate the provided        *
 *    vector into the new vector being created.                                                *
 *                                                                                             *
 * INPUT:   vector   -- Reference to the vector to use as a copy.                              *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
VectorClass<T>::VectorClass(VectorClass<T> const & vector) :
	Vector(nullptr),
	VectorMax(0)
{
	*this = vector;
}


/// <summary>
/// Takes over the storage of another vector, leaving that vector empty.
/// </summary>
/// <param name="vector">The vector to take the elements from.</param>
template<class T>
VectorClass<T>::VectorClass(VectorClass<T> && vector) noexcept :
	Vector(vector.Vector),
	VectorMax(vector.VectorMax)
{
	vector.Vector = nullptr;
	vector.VectorMax = 0;
}


/// <summary>
/// Constructs a vector holding a copy of every element in the list.
/// </summary>
/// <param name="list">The elements to fill the vector with.</param>
template<class T>
VectorClass<T>::VectorClass(std::initializer_list<T> list) :
	Vector(nullptr),
	VectorMax((int)list.size())
{
	if (VectorMax > 0) {
		Vector = new T[VectorMax];
		std::copy(list.begin(), list.end(), Vector);
	}
}


/***********************************************************************************************
 * VectorClass<T>::operator = -- The assignment operator.                                      *
 *                                                                                             *
 *    This the the assignment operator for vector objects. It will alter the existing lvalue   *
 *    vector to duplicate the rvalue one.                                                      *
 *                                                                                             *
 * INPUT:   vector   -- The rvalue vector to copy into the lvalue one.                         *
 *                                                                                             *
 * OUTPUT:  Returns with reference to the newly copied vector.                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
VectorClass<T> & VectorClass<T>::operator =(VectorClass<T> const & vector)
{
	if (this != &vector) {

		// The replacement storage is built before this vector is disturbed, so a failed
		// allocation leaves the original elements intact.
		int newmax = vector.Length();
		T * newptr = nullptr;
		if (newmax > 0) {
			newptr = new T[newmax];
			for (int index = 0; index < newmax; index++) {
				newptr[index] = vector[index];
			}
		}

		Clear();
		Vector = newptr;
		VectorMax = newmax;
	}
	return(*this);
}


/// <summary>
/// Takes over the storage of another vector, leaving that vector empty.
/// </summary>
/// <param name="vector">The vector to take the elements from.</param>
/// <returns>Returns with a reference to this vector.</returns>
template<class T>
VectorClass<T> & VectorClass<T>::operator =(VectorClass<T> && vector) noexcept
{
	if (this != &vector) {
		Clear();
		Vector = vector.Vector;
		VectorMax = vector.VectorMax;
		vector.Vector = nullptr;
		vector.VectorMax = 0;
	}
	return(*this);
}


/***********************************************************************************************
 * VectorClass<T>::ID -- Pointer based conversion to index number.                             *
 *                                                                                             *
 *    Use this routine to convert a pointer to an element in the vector back into the index    *
 *    number of that object. This routine ONLY works with actual pointers to object within     *
 *    the vector. For "equivalent" object index number (such as with similar integral values)  *
 *    then use the "by value" index number ID function.                                        *
 *                                                                                             *
 * INPUT:   pointer  -- Pointer to an actual object in the vector.                             *
 *                                                                                             *
 * OUTPUT:  Returns with the index number for the object pointed to by the parameter.          *
 *                                                                                             *
 * WARNINGS:   This routine is only valid for actual pointers to object that exist within      *
 *             the vector. All other object pointers will yield undefined results.             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/13/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
inline int VectorClass<T>::ID(T const * ptr) const
{
	return((int)(ptr - Vector));
}


/***********************************************************************************************
 * VectorClass<T>::ID -- Finds object ID based on value.                                       *
 *                                                                                             *
 *    Use this routine to find the index value of an object with equivalent value in the       *
 *    vector. Typical use of this would be for integral types.                                 *
 *                                                                                             *
 * INPUT:   object   -- Reference to the object that is to be looked up in the vector.         *
 *                                                                                             *
 * OUTPUT:  Returns with the index number of the object that is equivalent to the one          *
 *          specified. If no matching value could be found then -1 is returned.                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/13/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
int VectorClass<T>::ID(T const & object) const
{
	T const * found = std::find(begin(), end(), object);
	if (found != end()) {
		return((int)(found - Vector));
	}
	return(-1);
}


/***********************************************************************************************
 * VectorClass<T>::Clear -- Frees and clears the vector.                                       *
 *                                                                                             *
 *    Use this routine to reset the vector to an empty (non-allocated) state. A vector will    *
 *    free all allocated memory when this routine is called. In order for the vector to be     *
 *    useful after this point, the Resize function must be called to give it element space.    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
void VectorClass<T>::Clear(void)
{
	if (Vector != nullptr) {
		delete[] Vector;
	}
	Vector = nullptr;
	VectorMax = 0;
}


/***********************************************************************************************
 * VectorClass<T>::Resize -- Changes the size of the vector.                                   *
 *                                                                                             *
 *    This routine is used to change the size (usually to increase) the size of a vector. This *
 *    is the only way to increase the vector's working room (number of elements).              *
 *                                                                                             *
 * INPUT:   newsize  -- The desired size of the vector.                                        *
 *                                                                                             *
 * OUTPUT:  bool; Was the array resized successfully?                                          *
 *                                                                                             *
 * WARNINGS:   Failure to succeed could be the result of running out of memory.                *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
bool VectorClass<T>::Resize(int newsize)
{
	if (newsize > 0) {

		/*
		**	Allocate a new vector of the size specified. The default constructor
		**	will be called for every object in this vector.
		*/
		// This happens before anything is released, so a failed allocation leaves
		// this vector as it was.
		T * newptr = new T[newsize];

		/*
		**	Copy as much of the old vector into the new vector as possible. This
		**	presumes that there is a functional assignment operator for each
		**	of the objects in the vector.
		*/
		if (Vector != nullptr) {
			int copycount = (newsize < VectorMax) ? newsize : VectorMax;
			for (int index = 0; index < copycount; index++) {
				newptr[index] = Vector[index];
			}

			/*
			**	Delete the old vector. This might cause the destructors to be called
			**	for all of the old elements. This makes the implementation of suitable
			**	assignment operator very important. The default assignment operator will
			**	only work for the simplest of objects.
			*/
			delete[] Vector;
		}

		/*
		**	Assign the new vector data to this class.
		*/
		Vector = newptr;
		VectorMax = newsize;

	} else {

		/*
		**	Resizing to zero is the same as clearing the vector.
		*/
		Clear();
	}
	return(true);
}



/**************************************************************************
**	This derivative vector class adds the concept of adding and deleting
**	objects. The objects are packed to the beginning of the vector array.
**	If this is instantiated for a class object, then the assignment operator
**	and the equality operator must be supported. If the vector allocates its
**	own memory, then the vector can grow if it runs out of room adding items.
**	The growth rate is controlled by setting the growth step rate. A growth
**	step rate of zero disallows growing.
*/
template<class T>
class DynamicVectorClass : public VectorClass<T>
{
		typedef VectorClass<T> BASECLASS;
	public:
		using BASECLASS::Length;

		DynamicVectorClass(int size=0);
		DynamicVectorClass(std::initializer_list<T> list);

		DynamicVectorClass(DynamicVectorClass<T> const &) = default;
		DynamicVectorClass<T> & operator =(DynamicVectorClass<T> const &) = default;
		DynamicVectorClass(DynamicVectorClass<T> &&) noexcept;
		DynamicVectorClass<T> & operator =(DynamicVectorClass<T> &&) noexcept;

		// Change maximum size of vector.
		bool Resize(int newsize);

		// Resets and frees the vector array.
		void Clear(void) {ActiveCount = 0;BASECLASS::Clear();};

		// Fetch number of "allocated" vector objects.
		[[nodiscard]] int Count(void) const noexcept {return(ActiveCount);};

		/*
		 * Carries the active elements to or from a save game, the count first and then the
		 * elements. Loading sizes it before any element registers the slot it occupies.
		 */
		template<typename S>
		void Serialize(S & stream, std::source_location const & where = std::source_location::current())
		{
			int count = ActiveCount;
			stream.Serialize(count);

			if (stream.Is_Loading()) {
				if (count < 0) {
					stream.Fail();
					return;
				}
				Clear();
				if (count > 0) {
					Resize(count);
				}
				ActiveCount = count;
			}

			if constexpr (std::is_arithmetic_v<T> || std::is_enum_v<T>) {
				if (count > 0) {
					stream.Serialize_Bytes(&(*this)[0], (int)(sizeof(T) * count));
				}
			} else {
				for (int index = 0; index < count; index++) {
					// Elements report against the member's call site, not against this loop.
					if constexpr (requires { stream.Serialize((*this)[index], where); }) {
						stream.Serialize((*this)[index], where);
					} else {
						stream.Serialize((*this)[index]);
					}
				}
			}
		}

		// Add object to vector (growing as necessary).
		bool Add(T const & object);
		bool Add_Head(T const & object);
		bool Insert(int index, T const & object);
		bool Insert_After(int index, T const & object);

		// Delete object just like this from vector.
		bool Delete(T const & object);

		// Delete object at this vector index.
		bool Delete_Index(int index);

		// Deletes all objects in the vector.
		void Delete_All(void);

		// Set amount that vector grows by.
		int Set_Growth_Step(int step) {return(GrowthStep = step);};

		// Fetch current growth step rate.
		[[nodiscard]] int Growth_Step(void) const noexcept {return(GrowthStep);};

		[[nodiscard]] int ID(T const * ptr) const {return(BASECLASS::ID(ptr));};
		[[nodiscard]] int ID(T const & ptr) const;

		[[nodiscard]] T * end(void) noexcept {return(BASECLASS::begin() + ActiveCount);};
		[[nodiscard]] T const * end(void) const noexcept {return(BASECLASS::begin() + ActiveCount);};

		operator std::span<T>(void) noexcept {return(std::span<T>(BASECLASS::begin(), (std::size_t)ActiveCount));};
		operator std::span<T const>(void) const noexcept {return(std::span<T const>(BASECLASS::begin(), (std::size_t)ActiveCount));};

	protected:

		bool Grow(void);

		using BASECLASS::VectorMax;

		/*
		**	This is a count of the number of active objects in this
		**	vector. The memory array often times is bigger than this
		**	value.
		*/
		int ActiveCount;

		/*
		**	If there is insufficient room in the vector array for a new
		**	object to be added, then the vector will grow by the number
		**	of objects specified by this value. This is controlled by
		**	the Set_Growth_Step() function.
		*/
		int GrowthStep;
};


/***********************************************************************************************
 * DynamicVectorClass<T>::DynamicVectorClass -- Constructor for dynamic vector.                *
 *                                                                                             *
 *    This is the normal constructor for the dynamic vector class. It is similar to the normal *
 *    vector class constructor. The vector is initialized to contain the number of elements    *
 *    specified in the "size" parameter. The memory is allocated from free store.              *
 *                                                                                             *
 * INPUT:   size  -- The maximum number of objects allowed in this vector.                     *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
DynamicVectorClass<T>::DynamicVectorClass(int size)
	: BASECLASS(size)
{
	GrowthStep = 10;
	ActiveCount = 0;
}


/// <summary>
/// Takes over the storage of another vector, leaving that vector empty.
/// </summary>
/// <param name="that">The vector to take the elements from.</param>
template<class T>
DynamicVectorClass<T>::DynamicVectorClass(DynamicVectorClass<T> && that) noexcept
	: BASECLASS(std::move(that))
{
	ActiveCount = that.ActiveCount;
	GrowthStep = that.GrowthStep;
	that.ActiveCount = 0;
}


/// <summary>
/// Takes over the storage of another vector, leaving that vector empty.
/// </summary>
/// <param name="that">The vector to take the elements from.</param>
/// <returns>Returns with a reference to this vector.</returns>
template<class T>
DynamicVectorClass<T> & DynamicVectorClass<T>::operator =(DynamicVectorClass<T> && that) noexcept
{
	if (this != &that) {
		BASECLASS::operator =(std::move(that));
		ActiveCount = that.ActiveCount;
		GrowthStep = that.GrowthStep;
		that.ActiveCount = 0;
	}
	return(*this);
}


/// <summary>
/// Constructs a vector holding a copy of every element in the list.
/// </summary>
/// <param name="list">The elements to fill the vector with.</param>
/// <remarks>A braced single integer supplies one element rather than a size; construct
/// with parentheses to reserve room instead.</remarks>
template<class T>
DynamicVectorClass<T>::DynamicVectorClass(std::initializer_list<T> list)
	: BASECLASS(list)
{
	GrowthStep = 10;
	ActiveCount = Length();
}


/// <summary>
/// Makes room for one more object, expanding the vector by its growth step if needed.
/// </summary>
/// <returns>bool; Is there room for another object?</returns>
/// <remarks>Growing is refused for a vector whose growth step is zero.</remarks>
template<class T>
bool DynamicVectorClass<T>::Grow(void)
{
	if (ActiveCount < Length()) {
		return(true);
	}
	if (GrowthStep > 0) {
		return(Resize(Length() + GrowthStep));
	}
	return(false);
}


/***********************************************************************************************
 * DynamicVectorClass<T>::Resize -- Changes the size of a dynamic vector.                      *
 *                                                                                             *
 *    Use this routine to change the size of the vector. The size changed is the maximum       *
 *    number of allocated objects within this vector. The memory will be allocated out of      *
 *    free store.                                                                              *
 *                                                                                             *
 * INPUT:   newsize  -- The desired maximum size of this vector.                               *
 *                                                                                             *
 * OUTPUT:  bool; Was vector successfully resized according to specifications?                 *
 *                                                                                             *
 * WARNINGS:   Failure to resize the vector could be the result of lack of free store.         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
bool DynamicVectorClass<T>::Resize(int newsize)
{
	if (BASECLASS::Resize(newsize)) {
		if (Length() < ActiveCount) ActiveCount = Length();
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * DynamicVectorClass<T>::ID -- Find matching value in the dynamic vector.                     *
 *                                                                                             *
 *    Use this routine to find a matching object (by value) in the vector. Unlike the base     *
 *    class ID function of similar name, this one restricts the scan to the current number     *
 *    of valid objects.                                                                        *
 *                                                                                             *
 * INPUT:   object   -- A reference to the object that a match is to be found in the           *
 *                      vector.                                                                *
 *                                                                                             *
 * OUTPUT:  Returns with the index number of the object that is equivalent to the one          *
 *          specified. If no equivalent object could be found then -1 is returned.             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/13/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
int DynamicVectorClass<T>::ID(T const & object) const
{
	T const * first = BASECLASS::begin();
	T const * found = std::find(first, first + ActiveCount, object);
	if (found != first + ActiveCount) {
		return((int)(found - first));
	}
	return(-1);
}


/***********************************************************************************************
 * DynamicVectorClass<T>::Add -- Add an element to the vector.                                 *
 *                                                                                             *
 *    Use this routine to add an element to the vector. The vector will automatically be       *
 *    resized to accomodate the new element IF the vector was allocated previously and the     *
 *    growth rate is not zero.                                                                 *
 *                                                                                             *
 * INPUT:   object   -- Reference to the object that will be added to the vector.              *
 *                                                                                             *
 * OUTPUT:  bool; Was the object added successfully? If so, the object is added to the end     *
 *                of the vector.                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
bool DynamicVectorClass<T>::Add(T const & object)
{
	if (!Grow()) {
		return(false);
	}

	/*
	**	There is room for the new object now. Add it to the end of the object vector.
	*/
	(*this)[ActiveCount++] = object;
	return(true);
}


/***********************************************************************************************
 * DynamicVectorClass<T>::Add_Head -- Adds element to head of the list.                        *
 *                                                                                             *
 *    This routine will add the specified element to the head of the vector. If necessary,     *
 *    the vector will be expanded accordingly.                                                 *
 *                                                                                             *
 * INPUT:   object   -- Reference to the object to add to the head of this vector.             *
 *                                                                                             *
 * OUTPUT:  bool; Was the object added without error?                                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
bool DynamicVectorClass<T>::Add_Head(T const & object)
{
	return(Insert(0, object));
}


/***********************************************************************************************
 * DynamicVectorClass<T>::Insert -- insert an object at the desired index                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/27/99    GTH : Created.                                                                 *
 *=============================================================================================*/
template<class T>
bool DynamicVectorClass<T>::Insert(int index, T const & object)
{
	if (index < 0) return(false);
	if (index > ActiveCount) return(false);

	if (!Grow()) {
		return(false);
	}

	/*
	**	There is room for the new object now. Add it at the desired position.
	*/
	if (index < ActiveCount) {
		if constexpr (std::is_trivially_copyable_v<T>) {
			memmove(&(*this)[index + 1], &(*this)[index], (ActiveCount - index) * sizeof(T));
		} else {
			for (int i = ActiveCount; i > index; i--) {
				(*this)[i] = (*this)[i - 1];
			}
		}
	}
	(*this)[index] = object;
	ActiveCount++;
	return(true);
}


/// <summary>
/// Inserts an object after the specified index.
/// </summary>
/// <param name="index">The index to insert after. An index of -1 places the object at
/// the head of the vector.</param>
/// <param name="object">The object to insert.</param>
/// <returns>bool; Was the object inserted?</returns>
template<class T>
bool DynamicVectorClass<T>::Insert_After(int index, T const & object)
{
	return(Insert(index + 1, object));
}


/***********************************************************************************************
 * DynamicVectorClass<T>::Delete -- Remove the specified object from the vector.               *
 *                                                                                             *
 *    This routine will delete the object referenced from the vector. All objects in the       *
 *    vector that follow the one deleted will be moved "down" to fill the hole.                *
 *                                                                                             *
 * INPUT:   object   -- Reference to the object in this vector that is to be deleted.          *
 *                                                                                             *
 * OUTPUT:  bool; Was the object deleted successfully? This should always be true.             *
 *                                                                                             *
 * WARNINGS:   Do no pass a reference to an object that is NOT part of this vector. The        *
 *             results of this are undefined and probably catastrophic.                        *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
bool DynamicVectorClass<T>::Delete(T const & object)
{
	int id = ID(object);
	if (id != -1) {
		return(Delete_Index(id));
	}
	return(false);
}


/***********************************************************************************************
 * DynamicVectorClass<T>::Delete -- Deletes the specified index from the vector.               *
 *                                                                                             *
 *    Use this routine to delete the object at the specified index from the objects in the     *
 *    vector. This routine will move all the remaining objects "down" in order to fill the     *
 *    hole.                                                                                    *
 *                                                                                             *
 * INPUT:   index -- The index number of the object in the vector that is to be deleted.       *
 *                                                                                             *
 * OUTPUT:  bool; Was the object index deleted successfully? Failure might mean that the index *
 *                specified was out of bounds.                                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
bool DynamicVectorClass<T>::Delete_Index(int index)
{
	if (index >= 0 && index < ActiveCount) {
		ActiveCount--;

		/*
		**	If there are any objects past the index that was deleted, copy those
		**	objects down in order to fill the hole.
		*/
		if (index < ActiveCount) {
			if constexpr (std::is_trivially_copyable_v<T>) {
				memmove(&(*this)[index], &(*this)[index + 1], (ActiveCount - index) * sizeof(T));
			} else {
				for (int i = index; i < ActiveCount; i++) {
					(*this)[i] = (*this)[i+1];
				}
			}
		}
		return(true);
	}
	return(false);
}


template<class T>
void DynamicVectorClass<T>::Delete_All(void)
{
	int len = VectorMax;
	Clear();		// Forces destructor call on each object.
	Resize(len);
}


// The save/load and sorting code takes pointers into the element storage and walks them.
static_assert(std::ranges::contiguous_range<VectorClass<int>>);
static_assert(std::ranges::contiguous_range<DynamicVectorClass<int>>);


