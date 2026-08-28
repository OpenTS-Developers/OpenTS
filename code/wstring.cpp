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

/****************************************************************************\
*        C O N F I D E N T I A L --- W E S T W O O D   S T U D I O S         *
******************************************************************************
Project Name: Carpenter  (The RedAlert ladder creator)
File Name   : string.cpp
Author      : Neal Kettler
Start Date  : June 1, 1997
Last Update : June 17, 1997

A fairly typical string class.  This string class always copies any input
string to it's own memory (for assignment or construction).
\***************************************************************************/

#include "always.h"

#include "wstring.h"

#include "win.h" /// needed for NULL, TRUE and FALSE

#include <cctype>
#include <cstring>


/// <summary>
/// Constructs an empty string.
/// </summary>
Wstring::Wstring(void) :str(NULL)
{
}


/// <summary>
/// Constructs a string from a block of text.
/// The text is copied into memory this string owns, so the source may be discarded
/// once construction is complete.
/// </summary>
Wstring::Wstring(IN char const* string) :str(NULL)
{
	set(string);
}


/// <summary>
/// Constructs a copy of another string.
/// The text is duplicated, so the two strings share no memory afterwards.
/// </summary>
Wstring::Wstring(IN const Wstring& other) :str(NULL)
{
	if (other.str != NULL)
	{
		str = new char[strlen(other.str) + 1];
		strcpy(str, other.str);
	}
}


/// <summary>
/// Destroys the string and frees the text it owns.
/// </summary>
Wstring::~Wstring(void)
{
	clear();
}


/// <summary>
/// Compares this string against a block of text for equality.
/// </summary>
/// <returns>bool; Does the text match this string?</returns>
/// <remarks>Unless both are NULL, this string and the text supplied must each have been
/// set.</remarks>
bool Wstring::operator==(IN char const* other) const
{
	if ((str == NULL) && (other == NULL))
		return(TRUE);
	if (strcmp(str, other) != 0)
		return(FALSE);
	else
		return(TRUE);
}


/// <summary>
/// Compares this string against another for equality.
/// Two strings that have never been set count as equal, and a string that has been set
/// never matches one that has not.
/// </summary>
/// <returns>bool; Are the two strings the same?</returns>
bool Wstring::operator==(IN Wstring const& other) const
{
	if ((str == NULL) && (other.str == NULL))
		return(TRUE);

	if ((str == NULL) || (other.str == NULL))
		return(FALSE);

	if (strcmp(str, other.str) != 0)
		return(FALSE);
	else
		return(TRUE);
}


/// <summary>
/// Compares this string against a block of text for inequality.
/// </summary>
/// <returns>bool; Does the text differ from this string?</returns>
/// <remarks>This string and the text supplied must each have been set; neither may be
/// NULL.</remarks>
bool Wstring::operator!=(IN char const* other) const
{
	if (strcmp(str, other) != 0)
		return(TRUE);
	else
		return(FALSE);
}


/// <summary>
/// Compares this string against another for inequality.
/// Two strings that have never been set count as equal, and a string that has been set
/// always differs from one that has not.
/// </summary>
/// <returns>bool; Do the two strings differ?</returns>
bool Wstring::operator!=(IN Wstring const& other) const
{
	if ((str == NULL) && (other.str == NULL))
		return(FALSE);

	if ((str == NULL) || (other.str == NULL))
		return(TRUE);

	if (strcmp(str, other.str) != 0)
		return(TRUE);
	else
		return(FALSE);
}


/// <summary>
/// Assigns a block of text to this string.
/// The text is copied, so the source may be discarded afterwards.
/// </summary>
/// <returns>Returns with a reference to this string.</returns>
Wstring& Wstring::operator=(char const* other)
{
	set(other);
	return(*this);
}


/// <summary>
/// Assigns another string to this one.
/// The text is copied, so the two strings share no memory afterwards.
/// </summary>
/// <returns>Returns with a reference to this string.</returns>
Wstring& Wstring::operator=(IN Wstring const& other)
{
	if (*this == other)
		return(*this);

	set(other.get());
	return(*this);
}


/// <summary>
/// Appends a block of text onto the end of the string.
/// A NULL source is quietly ignored rather than treated as an error.
/// </summary>
/// <param name="s">The text to append.</param>
char Wstring::cat(IN char const* s)
{
	char* oldStr;
	unsigned int   len;

	if (s == NULL)   // it's OK to cat nothing
		return(TRUE);

	// Save the contents of the string.
	oldStr = str;

	// Determine the length of the resultant string.
	len = strlen(s) + 1;
	if (oldStr)
		len += strlen(oldStr);

	// Allocate memory for the new string.
	str = new char[(len * sizeof(char))];

	// Copy the contents of the old string and concatenate the passed
	// string.
	if (oldStr)   strcpy(str, oldStr);
	else         str[0] = 0;

	strcat(str, s);

	// delete the old string.
	if (oldStr)
		delete[](oldStr);

	return(TRUE);
}


/// <summary>
/// Appends a fixed number of characters onto the end of the string.
/// Use this routine when the source is not necessarily terminated, such as a run of
/// characters carved out of a larger buffer.
/// </summary>
/// <param name="size">The number of characters to take from the source.</param>
/// <param name="s">The characters to append.</param>
char Wstring::cat(unsigned int size, IN char const* s)
{
	char* oldStr;
	unsigned int   len;

	// Save the contents of the string.
	oldStr = str;

	// Determine the length of the resultant string.
	len = size + 1;
	if (oldStr)
		len += strlen(oldStr);

	// Allocate memory for the new string.
	str = new char[(len * sizeof(char))];

	// Copy the contents of the old string and concatenate the passed
	// string.
	if (oldStr)
		strcpy(str, oldStr);
	else
		str[0] = 0;

	strncat(str, s, size);

	// delete the old string.
	if (oldStr)
		delete[](oldStr);

	return(TRUE);
}


/// <summary>
/// Appends another string onto the end of this one.
/// </summary>
char Wstring::cat(IN Wstring const& other)
{
	return(cat(other.get()));
}


/// <summary>
/// Appends a block of text onto the end of this string.
/// </summary>
/// <returns>Returns with a reference to this string, so that appends may be chained.</returns>
Wstring& Wstring::operator+=(IN char const* string)
{
	cat(string);
	return(*this);
}


/// <summary>
/// Appends another string onto the end of this one.
/// </summary>
/// <returns>Returns with a reference to this string, so that appends may be chained.</returns>
Wstring& Wstring::operator+=(IN Wstring const& other)
{
	cat(other.get());
	return(*this);
}


/// <summary>
/// Joins this string and a block of text into a new string.
/// Neither operand is disturbed.
/// </summary>
/// <returns>Returns with a new string holding this string followed by the text.</returns>
Wstring Wstring::operator+(IN char const* string) const
{
	Wstring temp = *this;
	temp.cat(string);
	return(temp);
}


/// <summary>
/// Joins two strings into a new string.
/// Neither operand is disturbed.
/// </summary>
/// <returns>Returns with a new string holding this string followed by the other.</returns>
Wstring Wstring::operator+(IN Wstring const& s) const
{
	Wstring temp = *this;
	temp.cat(s);
	return(temp);
}


//
// This function deletes 'count' characters indexed by `pos' from the Wstring.
// If `pos'+'count' is > the length of the array, the last 'count' characters
// of the string are removed.  If an error occurs, FALSE is returned.
// Otherwise, TRUE is returned.  Note: count has a default value of 1.
//
//
char Wstring::remove(int pos, int count)
{
	char* s;
	int   len;


	len = (int)strlen(str);

	if (pos + count > len)
		pos = len - count;
	if (pos < 0)
	{
		count += pos;    // If they remove before 0, ignore up till beginning
		pos = 0;
	}
	if (count <= 0)
		return(FALSE);

	s = new char[len - count + 1];

	///////DBGMSG("Wstring::remove  POS: "<<pos<<"  LEN: "<<len);

	// put nulls on both ends of substring to be removed
	str[pos] = 0;
	str[pos + count - 1] = 0;

	strcpy(s, str);
	strcat(s, &(str[pos + count]));
	delete[](str);
	str = s;

	return(TRUE);
}


// Remove all instances of a char from the string
char Wstring::removeChar(char c)
{
	int     len = 0;
	char* cptr = NULL;
	char    removed = FALSE;

	if (str == NULL)
		return(FALSE);

	len = strlen(str);
	while ((cptr = strchr(str, c)) != NULL)
	{
		memmove(cptr, cptr + 1, len - 1 - ((int)(cptr - str)));
		len--;
		str[len] = 0;
		removed = TRUE;
	}
	if (removed)
	{
		char* newStr = new char[strlen(str) + 1];
		strcpy(newStr, str);
		delete[](str);
		str = newStr;
	}
	return(removed);
}


/// <summary>
/// Removes all white space from the string.
/// Both spaces and tabs are stripped out, wherever in the string they occur.
/// </summary>
void Wstring::removeSpaces(void)
{
	removeChar(' ');
	removeChar('\t');
}


/// <summary>
/// Frees the text and leaves the string empty.
/// </summary>
void Wstring::clear(void)
{
	if (str)
		delete[](str);
	str = NULL;
}


/// <summary>
/// Resizes the string to an empty buffer of the size requested.
/// Any previous contents are discarded. Use this routine when the string is about to be
/// filled in by other code writing through the raw text pointer.
/// </summary>
/// <param name="size">The number of bytes to reserve, terminator included.</param>
void Wstring::setSize(int size)
{
	clear();
	if (size < 0)
		return;
	str = new char[size];
	memset(str, 0, size);
}


/// <summary>
/// Copies the string into a fixed width field.
/// This routine is used to lay text out in columns -- the destination is padded out to
/// the requested width with spaces.
/// </summary>
/// <param name="dest">The buffer to copy the text into.</param>
/// <param name="len">The width of the field to fill.</param>
/// <remarks>Be sure the destination buffer is big enough for the field width plus a
/// terminator.</remarks>
void Wstring::cellCopy(char* dest, unsigned int len) const
{
	unsigned int i;

	strncpy(dest, str, len);
	for (i = (unsigned int)strlen(str); i < len; i++)
		dest[i] = ' ';
	dest[len] = 0;
}


/// <summary>
/// Fetches the raw text of the string.
/// Use this routine to hand the string to the C library, or to any other code that deals
/// in plain character pointers.
/// </summary>
/// <returns>Returns with a pointer to the text. A string that was never set yields an
/// empty string rather than NULL.</returns>
char* Wstring::get(void) const
{
	if (!str)
		return((char *)"");
	return(str);
}


/// <summary>
/// Fetches a single character out of the string.
/// </summary>
/// <param name="index">The position of the character to fetch.</param>
/// <returns>Returns with the character found, or zero if the position lies past the end
/// of the string.</returns>
char Wstring::get(unsigned int index)
{
	if (index < strlen(str))
		return(str[index]);
	return(0);
}


/// <summary>
/// Fetches the length of the string.
/// </summary>
/// <returns>Returns with the number of characters in the string.</returns>
unsigned int Wstring::length(void) const
{
	if (str == NULL)
		return(0);
	return((unsigned int)strlen(str));
}


// Insert at given position and shift old stuff to right
char Wstring::insert(char const* instring, unsigned int pos)
{
	if (str == NULL)
		return(set(instring));
	if (pos > strlen(str))
		pos = strlen(str);
	char* newstr = new char[strlen(str) + strlen(instring) + 1];
	memset(newstr, 0, strlen(str) + strlen(instring) + 1);
	strcpy(newstr, str);
	// move the old data out of the way
	int bytes = strlen(str) + 1 - pos;
	memmove(newstr + pos + strlen(instring), newstr + pos, bytes);
	// move the new data into place
	memmove(newstr + pos, instring, strlen(instring));
	delete[](str);
	str = newstr;
	return(TRUE);
}


// This function inserts the character specified by `k' into the string at the
// position indexed by `pos'.  If `pos' is >= the length of the string, it is
// appended to the string.  If an error occurs, FALSE is returned.  Otherwise,
// TRUE is returned.
char Wstring::insert(char k, unsigned int pos)
{
	char* s;
	unsigned int   len;
	char     c[2];

	if (!str)
	{
		c[0] = k;
		c[1] = 0;
		return(set(c));
	}

	len = (unsigned int)strlen(str);

	if (pos > len)
		pos = len;

	s = (char*)new char[(len + 2)];

	c[0] = str[pos];
	str[pos] = 0;
	c[1] = 0;
	strcpy(s, str);
	str[pos] = c[0];
	c[0] = k;
	strcat(s, c);
	strcat(s, &(str[pos]));
	delete[](str);
	str = s;

	return(TRUE);
}


// This function replaces any occurences of the string pointed to by
// `replaceThis' with the string pointed to by `withThis'.  If an error
// occurs, FALSE is returned.  Otherwise, TRUE is returned.
char Wstring::replace(char const* replaceThis, char const* withThis)
{
	Wstring  dest;
	char* foundStr, * src;
	unsigned int   len;

	src = get();
	while (src && src[0])
	{
		foundStr = strstr(src, replaceThis);
		if (foundStr)
		{
			len = (unsigned int)(foundStr - src);
			if (len)
			{
				if (!dest.cat(len, src))
					return(FALSE);
			}
			if (!dest.cat(withThis))
				return(FALSE);
			src = foundStr + strlen(replaceThis);
		}
		else
		{
			if (!dest.cat(src))
				return(FALSE);

			src = NULL;
		}
	}
	return(set(dest.get()));
}


/// <summary>
/// Sets the string to a copy of the text supplied.
/// The string always owns its own memory, so the source may be discarded once this
/// routine returns. Any previous contents are freed.
/// </summary>
/// <param name="s">The text to copy.</param>
char Wstring::set(IN char const* s)
{
	unsigned int len;

	clear();

	len = (unsigned int)strlen(s) + 1;

	str = new char[len];
	strcpy(str, s);

	return(TRUE);
}


/// <summary>
/// Sets a single character within the string.
/// </summary>
/// <param name="c">The character to store.</param>
/// <param name="index">The position within the string to store it at.</param>
/// <returns>Returns true if the character was stored, false if the position lies past the
/// end of the string.</returns>
char Wstring::set(char c, unsigned int index)
{
	if (index >= (unsigned int)strlen(str))
		return(FALSE);

	str[index] = c;

	return(TRUE);
}


/// <summary>
/// Sets the string from a fixed number of characters.
/// Use this routine when the source is not necessarily terminated, such as a run of
/// characters carved out of a larger buffer. The copy is always terminated.
/// </summary>
/// <param name="size">The number of characters to copy from the source.</param>
/// <param name="string">The characters to copy.</param>
char Wstring::set(unsigned int size, IN char const* string)
{
	unsigned int len;

	clear();
	len = size + 1;

	str = new char[len];

	// Copy the bytes in the string, and NULL-terminate it.
	strncpy(str, string, size);
	str[size] = 0;

	return(TRUE);
}


// This function converts all alphabetical characters in the string to lower
// case.
void Wstring::toLower(void)
{
	unsigned int i;

	for (i = 0; i < length(); i++)
	{
		if ((str[i] >= 'A') && (str[i] <= 'Z'))
			str[i] = tolower(str[i]);
	}
}


// This function converts all alphabetical characters in the string to upper
// case.
void Wstring::toUpper(void)
{
	unsigned int i;

	for (i = 0; i < length(); i++)
	{
		if ((str[i] >= 'a') && (str[i] <= 'z'))
			str[i] = toupper(str[i]);
	}
}


//  This function truncates the string so its length will match the specified
// `len'.  If an error occurs, FALSE is returned.  Otherwise, TRUE is returned.
char Wstring::truncate(unsigned int len)
{
	Wstring tmp;
	if (!tmp.set(len, get()) || !set(tmp.get()))
		return(FALSE);
	return(TRUE);
}


// Truncate the string after the character 'c' (gets rid of 'c' as well)
//   Do nothing if 'c' isn't in the string
char Wstring::truncate(char c)
{
	int  len;

	if (str == NULL)
		return(FALSE);

	char* cptr = strchr(str, c);
	if (cptr == NULL)
		return(FALSE);
	len = (int)(cptr - str);
	truncate((unsigned int)len);
	return(TRUE);
}


// Get a token from this string that's seperated by one or more
//  chars from the 'delim' string , start at offset & return offset
int Wstring::getToken(int offset, char const* delim, Wstring& out) const
{
	int i;
	int start;
	int stop;

	if (offset < 0)
		return(-1);

	for (i = offset; i < (int)length(); i++) {
		if (strchr(delim, str[i]) == NULL)
			break;
	}
	if (i >= (int)length())
		return(-1);
	start = i;

	for (; i < (int)length(); i++) {
		if (strchr(delim, str[i]) != NULL)
			break;
	}
	stop = i - 1;
	out.set(str + start);
	out.truncate((unsigned int)stop - start + 1);
	return(stop + 1);
}


// Get the first line of text after offset.  Lines are terminated by '\r\n' or '\n'
int Wstring::getLine(int offset, Wstring& out) const
{
	int i;
	int start;
	int stop;

	start = i = offset;
	if (start >= (int)length())
		return(-1);

	for (; i < (int)length(); i++) {
		if (strchr("\r\n", str[i]) != NULL)
			break;
	}
	stop = i;
	if ((str[stop] == '\r') && (str[stop + 1] == '\n'))
		stop++;

	out.set(str + start);
	out.truncate((unsigned int)stop - start + 1);
	return(stop + 1);
}
