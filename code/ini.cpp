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

/* $Header: /CounterStrike/INI.CPP 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : INI.CPP                                                      *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : September 10, 1993                                           *
 *                                                                                             *
 *                  Last Update : November 2, 1996 [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   INIClass::Clear -- Clears out a section (or all sections) of the INI data.                *
 *   INIClass::Entry_Count -- Fetches the number of entries in a specified section.            *
 *   INIClass::Find_Entry -- Find specified entry within section.                              *
 *   INIClass::Find_Section -- Find the specified section within the INI data.                 *
 *   INIClass::Get_Bool -- Fetch a boolean value for the section and entry specified.          *
 *   INIClass::Get_Entry -- Get the entry identifier name given ordinal number and section name*
 *   INIClass::Get_Float -- Fetch a floating point number from the database.                   *
 *   INIClass::Get_Hex -- Fetches integer [hex format] from the section and entry specified.   *
 *   INIClass::Get_Int -- Fetch an integer entry from the specified section.                   *
 *   INIClass::Get_PKey -- Fetch a key from the ini database.                                  *
 *   INIClass::Get_String -- Fetch the value of a particular entry in a specified section.     *
 *   INIClass::Get_TextBlock -- Fetch a block of normal text.                                  *
 *   INIClass::Get_UUBlock -- Fetch an encoded block from the section specified.               *
 *   INIClass::INISection::Find_Entry -- Finds a specified entry and returns pointer to it.    *
 *   INIClass::Load -- Load INI data from the file specified.                                  *
 *   INIClass::Load -- Load the INI data from the data stream (straw).                         *
 *   INIClass::Put_Bool -- Store a boolean value into the INI database.                        *
 *   INIClass::Put_Float -- Store a floating point number to the database.                     *
 *   INIClass::Put_Hex -- Store an integer into the INI database, but use a hex format.        *
 *   INIClass::Put_Int -- Stores a signed integer into the INI data base.                      *
 *   INIClass::Put_PKey -- Stores the key to the INI database.                                 *
 *   INIClass::Put_String -- Output a string to the section and entry specified.               *
 *   INIClass::Put_TextBlock -- Stores a block of text into an INI section.                    *
 *   INIClass::Put_UUBlock -- Store a binary encoded data block into the INI database.         *
 *   INIClass::Save -- Save the ini data to the file specified.                                *
 *   INIClass::Save -- Saves the INI data to a pipe stream.                                    *
 *   INIClass::Section_Count -- Counts the number of sections in the INI data.                 *
 *   INIClass::Strip_Comments -- Strips comments of the specified text line.                   *
 *   INIClass::~INIClass -- Destructor for INI handler.                                        *
 *   INIClass::Put_Rect -- Store a rectangle  into the INI database.                           *
 *   INIClass::Get_Rect -- Retrieve a rectangle data from the database.                        *
 *   INIClass::Put_Point -- Store a point value to the database.                               *
 *   INIClass::Get_Point -- Fetch a point value from the INI database.                         *
 *   INIClass::Put_Point -- Stores a 3D point to the database.                                 *
 *   INIClass::Get_Point -- Fetch a 3D point from the database.                                *
 *   INIClass::Get_Point -- Fetch a 2D point from the INI database.                            *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "ini.h"

#include "b64pipe.h"
#include "b64straw.h"
#include "cstraw.h"
#include "dbgprint.h"
#include "pk.h"
#include "rect.h"
#include "trim.h"
#include "xpipe.h"
#include "xstraw.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>

INIClass::~INIClass(void) = default;


/// <summary>
/// Erases one entry, one section, or the whole database.
/// </summary>
/// <param name="section">The section to clear, or NULL to clear the whole database.</param>
/// <param name="entry">The entry within the section to clear, or NULL to clear the whole
/// section.</param>
/// <returns>bool; Always true, whether or not anything was found to clear.</returns>
bool INIClass::Clear(char const * section, char const * entry)
{
	if (section == NULL) {
		SectionIndex.clear();
		SectionList.clear();
		TailComment.clear();
		return(true);
	}

	INISection * secptr = Find_Section(section);
	if (secptr == NULL) {
		return(true);
	}

	if (entry != NULL) {
		INIEntry * entptr = secptr->Find_Entry(entry);
		if (entptr != NULL) {
			Remove_Entry(*secptr, *entptr);
		}
		return(true);
	}

	SectionIndex.erase(secptr->Section);
	std::erase_if(SectionList, [secptr](std::unique_ptr<INISection> const & held) {return(held.get() == secptr);});
	return(true);
}


/***********************************************************************************************
 * INIClass::Load -- Load INI data from the file specified.                                    *
 *                                                                                             *
 *    Use this routine to load the INI class with the data from the specified file.            *
 *                                                                                             *
 * INPUT:   file  -- Reference to the file that will be used to fill up this INI manager.      *
 *                                                                                             *
 * OUTPUT:  bool; Was the file loaded successfully?                                            *
 *                                                                                             *
 * WARNINGS:   This routine allocates memory.                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/02/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int INIClass::Load(FileClass & file, bool keepcomments)
{
	FileStraw fs(file);
	return(Load(fs, keepcomments, file.File_Name()));
}


/// <summary>
/// Reads one line of the file, dropping carriage returns and the newline that ends it.
/// </summary>
/// <returns>bool; Was a line read? The last line of a file is read whether or not a newline
/// ends it, so only the end of the file returns false.</returns>
static bool Read_Line(Straw & file, std::string & line)
{
	line.clear();

	for (;;) {
		char c;
		if (file.Get(&c, sizeof(c)) != sizeof(c)) {
			return(!line.empty());
		}
		if (c == '\n') {
			return(true);
		}
		if (c != '\r') {
			line.push_back(c);
		}
	}
}


int INIClass::Load(Straw & file, bool keepcomments)
{
	return(Load(file, keepcomments, NULL));
}


/// <summary>
/// Builds the database from the INI text of a straw, or merges the text into what the
/// database already holds.
/// </summary>
/// <remarks>
/// Every assignment is stored as Put_String stores one: a repeated section header continues
/// the section already read, and a repeated key takes the later value and moves to the end
/// of its section. A repeat within the file being read is logged; a later file overriding an
/// earlier one is not, since that is how the rules layers stack. Comments and layout are kept
/// only when the database was empty and keepcomments asks for them.
/// </remarks>
/// <param name="ffile">The straw supplying the INI text.</param>
/// <param name="keepcomments">Should comment lines and column layout be kept for Save?</param>
/// <param name="source">The file name to give in diagnostics, or NULL when there is none.</param>
/// <returns>bool; Was a section header found? A file of comments alone counts as loaded only
/// when its comments were kept.</returns>
int INIClass::Load(Straw & ffile, bool keepcomments, char const * source)
{
	bool const merge = !SectionList.empty();
	if (merge) {
		keepcomments = false;
	} else {
		TailComment.clear();
	}

	SourceNames.push_back(source != NULL ? source : "");
	LoadGeneration = (unsigned)(SourceNames.size() - 1);
	std::string const prefix = SourceNames.back().empty() ? std::string() : SourceNames.back() + " ";

	CacheStraw file;
	file.Get_From(ffile);

	std::string line;
	line.reserve(MAX_LINE_LENGTH);

	INICommentBlock pending;
	INISection * current = NULL;
	std::string currentname;
	bool sawsection = false;

	while (Read_Line(file, line)) {
		char * buffer = line.data();

		if (Is_A_Section(buffer)) {
			strtrim(buffer);
			char * close = strchr(buffer, ']');
			if (close != NULL) *close = '\0';
			char const * name = buffer + 1;

			sawsection = true;
			currentname = name;

			// Without comments a section is only created once it has an entry, so that an
			// empty section never exists.
			current = Find_Section(name);
			if (current != NULL) {
				if (current->Generation == LoadGeneration) {
					DebugString("INI: %s[%s] appears again; its keys merge into the earlier block.\n", prefix.c_str(), name);
				}
				if (keepcomments) {
					current->PrefixComment.insert(current->PrefixComment.end(), pending.begin(), pending.end());
				}
			} else if (keepcomments) {
				current = &Find_Or_Add_Section(name, std::move(pending));
			}
			if (current != NULL) {
				current->Generation = LoadGeneration;
			}
			pending.clear();
			continue;
		}

		if (!sawsection) {
			if (keepcomments) {
				pending.push_back(line);
			}
			continue;
		}

		int assign_col = 0;
		int value_col = 0;
		int comment_col = 0;
		std::optional<std::string> linecomment;

		if (keepcomments) {
			pending.push_back(line);
			char const * text = Scan_Line_For_Columns(buffer, assign_col, value_col, comment_col);
			if (text != NULL) {
				linecomment = text;
			}
		}

		/*
		**	Determine if this line is a comment or blank line. Throw it out if it is.
		*/
		// A thrown out line stays in the pending block, so that it is written back out when
		// comments are kept.
		Strip_Comments(buffer);
		if (buffer[0] == '\0' || buffer[0] == ';' || buffer[0] == '=') continue;

		/*
		**	The line isn't an obvious comment. Make sure that there is the "=" character
		**	at an appropriate spot.
		*/
		char * divider = strchr(buffer, '=');
		if (divider == NULL) continue;

		/*
		**	Split the line into entry and value sections. Be sure to catch the
		**	"=foobar" and "foobar=" cases. These lines are ignored.
		*/
		*divider++ = '\0';
		strtrim(buffer);
		if (buffer[0] == '\0') continue;

		strtrim(divider);
		if (divider[0] == '\0') continue;

		if (current == NULL) {
			current = &Find_Or_Add_Section(currentname);
			current->Generation = LoadGeneration;
		}

		INIEntry * existing = current->Find_Entry(buffer);
		if (existing != NULL && existing->Generation == LoadGeneration) {
			DebugString("INI: %s[%s] repeats %s; the later value wins and the key moves to the end.\n", prefix.c_str(), current->Section.c_str(), buffer);
		}

		INIEntry & entry = Store_Entry(*current, buffer, divider);
		entry.Generation = LoadGeneration;

		if (keepcomments) {
			pending.pop_back();
			entry.PrefixComment.insert(entry.PrefixComment.end(), pending.begin(), pending.end());
			entry.LineComment = std::move(linecomment);
			entry.AssignColumn = assign_col;
			entry.ValueColumn = value_col;
			entry.CommentColumn = comment_col;
		}
		pending.clear();
	}

	if (!sawsection) {
		if (keepcomments) {
			TailComment = std::move(pending);
			return(true);
		}
		return(false);
	}

	if (keepcomments) {
		TailComment = std::move(pending);
	}
	return(true);
}


/***********************************************************************************************
 * INIClass::Save -- Save the ini data to the file specified.                                  *
 *                                                                                             *
 *    Use this routine to save the ini data to the file specified. All existing data in the    *
 *    file, if it was present, is replaced.                                                    *
 *                                                                                             *
 * INPUT:   file  -- Reference to the file to write the INI data to.                           *
 *                                                                                             *
 * OUTPUT:  bool; Was the data written to the file?                                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/02/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int INIClass::Save(FileClass & file) const
{
	FilePipe fp(file);
	return(Save(fp));
}


/***********************************************************************************************
 * INIClass::Save -- Saves the INI data to a pipe stream.                                      *
 *                                                                                             *
 *    This routine will output the data of the INI file to a pipe stream.                      *
 *                                                                                             *
 * INPUT:   pipe  -- Reference to the pipe stream to pump the INI image to.                    *
 *                                                                                             *
 * OUTPUT:  Returns with the number of bytes output to the pipe.                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/02/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int INIClass::Save(Pipe & pipe) const
{
	int total = 0;
	char spacebuffer[MAX_LINE_LENGTH];

	memset(spacebuffer, ' ', sizeof(spacebuffer));
	spacebuffer[MAX_LINE_LENGTH - 1] = '\0';

	for (std::unique_ptr<INISection> const & secptr : SectionList) {

		if (total > 0 && secptr->PrefixComment.empty()) {
			total += pipe.Put("\r\n", strlen("\r\n"));
		}

		for (std::string const & comment : secptr->PrefixComment) {
			total += pipe.Put(comment.c_str(), (int)comment.size());
			total += pipe.Put("\r\n", strlen("\r\n"));
		}

		/*
		**	Output the section identifier.
		*/
		total += pipe.Put("[", 1);
		total += pipe.Put(secptr->Section.c_str(), (int)secptr->Section.size());
		total += pipe.Put("]", 1);
		total += pipe.Put("\r\n", strlen("\r\n"));

		/*
		**	Output all the entries and values in this section.
		*/
		for (std::unique_ptr<INIEntry> const & entryptr : secptr->EntryList) {

			for (std::string const & comment : entryptr->PrefixComment) {
				total += pipe.Put(comment.c_str(), (int)comment.size());
				total += pipe.Put("\r\n", strlen("\r\n"));
			}

			int entrylen = (int)entryptr->Entry.size();
			int valuelen = (int)entryptr->Value.size();
			int spacepad = entryptr->AssignColumn - (entrylen);
			int totalspacepad = 0;
			/// why half..
			spacepad = std::min(MAX_LINE_LENGTH / 2, spacepad);

			total += pipe.Put(entryptr->Entry.c_str(), entrylen);
			if (spacepad > 0) {
				total += pipe.Put(spacebuffer, spacepad);
				totalspacepad += spacepad;
			}

			total += pipe.Put("=", 1);

			spacepad = entryptr->ValueColumn - (entrylen + totalspacepad + 1);
			/// why half..
			spacepad = std::min(MAX_LINE_LENGTH / 2, spacepad);

			if (spacepad > 0) {
				total += pipe.Put(spacebuffer, spacepad);
				totalspacepad += spacepad;
			}

			total += pipe.Put(entryptr->Value.c_str(), valuelen);

			if (entryptr->LineComment.has_value()) {
				spacepad = entryptr->CommentColumn - (entrylen + valuelen + totalspacepad + 1);
				/// why half..
				spacepad = std::min(MAX_LINE_LENGTH / 2, spacepad);

				if (spacepad > 0) {
					total += pipe.Put(spacebuffer, spacepad);
				}

				total += pipe.Put(";", 1);
				total += pipe.Put(entryptr->LineComment->c_str(), (int)entryptr->LineComment->size());
			}

			/*
			**	After the last entry in this section, output an extra
			**	blank line for readability purposes.
			*/
			total += pipe.Put("\r\n", strlen("\r\n"));
		}
	}

	for (std::string const & comment : TailComment) {
		total += pipe.Put(comment.c_str(), (int)comment.size());
		total += pipe.Put("\r\n", strlen("\r\n"));
	}

	total += pipe.End();

	return(total);
}


/// <summary>
/// Finds a section by name. Names are compared byte for byte, so the search is case
/// sensitive.
/// </summary>
/// <param name="section">The section name to look for.</param>
/// <returns>The section, or NULL if the database holds no section of that name.</returns>
INIClass::INISection * INIClass::Find_Section(char const * section) const
{
	if (section == NULL) {
		return(NULL);
	}
	auto found = SectionIndex.find(section);
	return(found != SectionIndex.end() ? found->second : NULL);
}


/***********************************************************************************************
 * INIClass::Section_Count -- Counts the number of sections in the INI data.                   *
 *                                                                                             *
 *    This routine will scan through all the sections in the INI data and return a count       *
 *    of the number it found.                                                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of sections recorded in the INI data.                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/02/1996 JLB : Created.                                                                 *
 *   11/02/1996 JLB : Uses index manager.                                                      *
 *=============================================================================================*/
int INIClass::Section_Count(void) const
{
	return((int)SectionList.size());
}


/***********************************************************************************************
 * INIClass::Entry_Count -- Fetches the number of entries in a specified section.              *
 *                                                                                             *
 *    This routine will examine the section specified and return with the number of entries    *
 *    associated with it.                                                                      *
 *                                                                                             *
 * INPUT:   section  -- Pointer to the section that will be examined.                          *
 *                                                                                             *
 * OUTPUT:  Returns with the number entries in the specified section.                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/02/1996 JLB : Created.                                                                 *
 *   11/02/1996 JLB : Uses index manager.                                                      *
 *=============================================================================================*/
int INIClass::Entry_Count(char const * section) const
{
	INISection * secptr = Find_Section(section);
	if (secptr != NULL) {
		return((int)secptr->EntryList.size());
	}
	return(0);
}


/***********************************************************************************************
 * INIClass::Find_Entry -- Find specified entry within section.                                *
 *                                                                                             *
 *    This support routine will find the specified entry in the specified section. If found,   *
 *    a pointer to the entry control structure will be returned.                               *
 *                                                                                             *
 * INPUT:   section  -- Pointer to the section name to search under.                           *
 *                                                                                             *
 *          entry    -- Pointer to the entry name to search for.                               *
 *                                                                                             *
 * OUTPUT:  If the entry was found, then a pointer to the entry control structure will be      *
 *          returned. Otherwise, NULL will be returned.                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/02/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
INIClass::INIEntry * INIClass::Find_Entry(char const * section, char const * entry) const
{
	INISection * secptr = Find_Section(section);
	if (secptr != NULL) {
		return(secptr->Find_Entry(entry));
	}
	return(NULL);
}


/***********************************************************************************************
 * INIClass::Get_Entry -- Get the entry identifier name given ordinal number and section name. *
 *                                                                                             *
 *    This will return the identifier name for the entry under the section specified. The      *
 *    ordinal number specified is used to determine which entry to retrieve. The entry         *
 *    identifier is the text that appears to the left of the "=" character.                    *
 *                                                                                             *
 * INPUT:   section  -- The section to use.                                                    *
 *                                                                                             *
 *          index    -- The ordinal number to use when fetching an entry name.                 *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the entry name.                                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/02/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
char const * INIClass::Get_Entry(char const * section, int index) const
{
	INISection * secptr = Find_Section(section);

	if (secptr != NULL && index >= 0 && index < (int)secptr->EntryList.size()) {
		return(secptr->EntryList[index]->Entry.c_str());
	}
	return(NULL);
}


/***********************************************************************************************
 * INIClass::Put_UUBlock -- Store a binary encoded data block into the INI database.           *
 *                                                                                             *
 *    Use this routine to store an arbitrary length binary block of data into the INI database.*
 *    This routine will covert the data into displayable form and then break it into lines     *
 *    that are stored in sequence to the section. A section used to store data in this         *
 *    fashion can not be used for any other entries.                                           *
 *                                                                                             *
 * INPUT:   section  -- The section identifier to place the data into.                         *
 *                                                                                             *
 *          block    -- Pointer to the block of binary data to store.                          *
 *                                                                                             *
 *          len      -- The length of the binary data.                                         *
 *                                                                                             *
 * OUTPUT:  bool; Was the data stored to the database?                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool INIClass::Put_UUBlock(char const * section, void const * block, int len)
{
	if (section == NULL || block == NULL || len < 1) return(false);

	Clear(section);

	BufferStraw straw(block, len);
	Base64Straw bstraw(Base64Straw::ENCODE);
	bstraw.Get_From(straw);

	int counter = 1;

	for (;;) {
		char buffer[71];
		char sbuffer[32];

		int length = bstraw.Get(buffer, sizeof(buffer)-1);
		buffer[length] = '\0';
		if (length == 0) break;

		sprintf(sbuffer, "%d", counter);
		Put_String(section, sbuffer, buffer);
		counter++;
	}
	return(true);
}


/***********************************************************************************************
 * INIClass::Get_UUBlock -- Fetch an encoded block from the section specified.                 *
 *                                                                                             *
 *    This routine will take all the entries in the specified section and decompose them into  *
 *    a binary block of data that will be stored into the buffer specified. By using this      *
 *    routine [and the Put_UUBLock counterpart], arbitrary blocks of binary data may be        *
 *    stored in the INI file. A section processed by this routine can contain no other         *
 *    entries than those put there by a previous call to Put_UUBlock.                          *
 *                                                                                             *
 * INPUT:   section  -- The section name to process.                                           *
 *                                                                                             *
 *          block    -- Pointer to the buffer that will hold the retrieved data.               *
 *                                                                                             *
 *          len      -- The length of the buffer. The retrieved data will not fill past this   *
 *                      limit.                                                                 *
 *                                                                                             *
 * OUTPUT:  Returns with the number of bytes decoded into the buffer specified.                *
 *                                                                                             *
 * WARNINGS:   If the number of bytes retrieved exactly matches the length of the buffer       *
 *             specified, then you might have a condition of buffer "overflow".                *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/02/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int INIClass::Get_UUBlock(char const * section, void * block, int len) const
{
	if (section == NULL) return(0);

	Base64Pipe b64pipe(Base64Pipe::DECODE);
	BufferPipe bpipe(block, len);

	b64pipe.Put_To(&bpipe);

	int total = 0;
	int counter = Entry_Count(section);
	for (int index = 0; index < counter; index++) {
		char buffer[128];

		int length = Get_String(section, Get_Entry(section, index), "=", buffer, sizeof(buffer));
		int outcount = b64pipe.Put(buffer, length);
		total += outcount;
	}
	total += b64pipe.End();
	return(total);
}


/***********************************************************************************************
 * INIClass::Put_TextBlock -- Stores a block of text into an INI section.                      *
 *                                                                                             *
 *    This routine will take an arbitrarily long block of text and store it into the INI       *
 *    database. The text is broken up into lines and each line is then stored as a numbered    *
 *    entry in the specified section. A section used to store text in this way can not be used *
 *    to hold any other entries. The text is presumed to contain space characters scattered    *
 *    throughout it and that one space between words and sentences is natural.                 *
 *                                                                                             *
 * INPUT:   section  -- The section to place the text block into.                              *
 *                                                                                             *
 *          text     -- Pointer to a null terminated text string that holds the block of       *
 *                      text. The length can be arbitrary.                                     *
 *                                                                                             *
 * OUTPUT:  bool; Was the text block placed into the database?                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool INIClass::Put_TextBlock(char const * section, char const * text)
{
	if (section == NULL) return(false);

	Clear(section);

	int index = 1;
	while (text != NULL && *text != '\0') {

		char buffer[128];

		strncpy(buffer, text, 75);
		buffer[75] = '\0';

		char b[32];
		sprintf(b, "%d", index);

		/*
		**	Scan backward looking for a good break position.
		*/
		int count = strlen(buffer);
		if (count > 0) {
			if (count >= 75) {
				while (count) {
					char c = buffer[count];

					//if (isspace(c)) break;
					if (c != 0 && (unsigned char)c <= _CONTROL) break;
					count--;
				}

				if (count == 0) {
					break;
				} else {
					buffer[count] = '\0';
				}
			}

			strtrim(buffer);
			Put_String(section, b, buffer);
			index++;
			text = ((char  *)text) + count;
		} else {
			break;
		}
	}
	return(true);
}


/***********************************************************************************************
 * INIClass::Get_TextBlock -- Fetch a block of normal text.                                    *
 *                                                                                             *
 *    This will take all entries in the specified section and format them into a block of      *
 *    normalized text. That is, text with single spaces between each concatenated line. All    *
 *    entries in the specified section are processed by this routine. Use Put_TextBlock to     *
 *    build the entries in the section.                                                        *
 *                                                                                             *
 * INPUT:   section  -- The section name to process.                                           *
 *                                                                                             *
 *          buffer   -- Pointer to the buffer that will hold the complete text.                *
 *                                                                                             *
 *          len      -- The length of the buffer specified. The text will, at most, fill this  *
 *                      buffer with the last character being forced to null.                   *
 *                                                                                             *
 * OUTPUT:  Returns with the number of characters placed into the buffer. The trailing null    *
 *          is not counted.                                                                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/02/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int INIClass::Get_TextBlock(char const * section, char * buffer, int len) const
{
	if (len <= 0) return(0);

	buffer[0] = '\0';
	if (len <= 1) return(0);

	int elen = Entry_Count(section);
	int total = 0;
	for (int index = 0; index < elen; index++) {

		/*
		**	Add spacers between lines of fetched text.
		*/
		if (index > 0) {
			*buffer++ = ' ';
			len--;
			total++;
		}

		Get_String(section, Get_Entry(section, index), "", buffer, len);

		int partial = strlen(buffer);
		total += partial;
		buffer += partial;
		len -= partial;
		if (len <= 1) break;
	}
	return(total);
}


/***********************************************************************************************
 * INIClass::Put_Int -- Stores a signed integer into the INI data base.                        *
 *                                                                                             *
 *    Use this routine to store an integer value into the section and entry specified.         *
 *                                                                                             *
 * INPUT:   section  -- The identifier for the section that the entry will be placed in.       *
 *                                                                                             *
 *          entry    -- The entry identifier used for the integer number.                      *
 *                                                                                             *
 *          number   -- The integer number to store in the database.                           *
 *                                                                                             *
 *          format   -- The format to store the integer. The format is generally only a        *
 *                      cosmetic affect. The Get_Int operation will interpret the value the    *
 *                      same regardless of what format was used to store the integer.          *
 *                                                                                             *
 *                      0  : plain decimal digit                                               *
 *                      1  : hexadecimal digit (trailing "h")                                  *
 *                      2  : hexadecimal digit (leading "$")                                   *
 *                                                                                             *
 * OUTPUT:  bool; Was the number stored?                                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *   07/10/1996 JLB : Handles multiple integer formats.                                        *
 *=============================================================================================*/
bool INIClass::Put_Int(char const * section, char const * entry, int number, int format)
{
	char buffer[MAX_LINE_LENGTH];

	switch (format) {
		default:
		case 0:
			sprintf(buffer, "%d", number);
			break;

		case 1:
			sprintf(buffer, "%Xh", number);
			break;

		case 2:
			sprintf(buffer, "$%X", number);
			break;
	}
	return(Put_String(section, entry, buffer));
}


/***********************************************************************************************
 * INIClass::Get_Int -- Fetch an integer entry from the specified section.                     *
 *                                                                                             *
 *    This routine will fetch an integer value from the entry and section specified. If no     *
 *    entry could be found, then the default value will be returned instead.                   *
 *                                                                                             *
 * INPUT:   section  -- The section name to search under.                                      *
 *                                                                                             *
 *          entry    -- The entry name to search for.                                          *
 *                                                                                             *
 *          defvalue -- The default value to use if the specified entry could not be found.    *
 *                                                                                             *
 * OUTPUT:  Returns with the integer value specified in the INI database or else returns the   *
 *          default value.                                                                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/02/1996 JLB : Created.                                                                 *
 *   07/10/1996 JLB : Handles multiple integer formats.                                        *
 *=============================================================================================*/
int INIClass::Get_Int(char const * section, char const * entry, int defvalue) const
{
	/*
	**	Verify that the parameters are nominally correct.
	*/
	if (section == NULL || entry == NULL) return(defvalue);

	INIEntry * entryptr = Find_Entry(section, entry);
	if (entryptr != NULL) {
		char const * value = entryptr->Value.c_str();
		if (*value == '$') {
			sscanf(value, "$%x", &defvalue);
		} else {
			if (tolower(value[strlen(value)-1]) == 'h') {
				sscanf(value, "%xh", &defvalue);
			} else {
				defvalue = atoi(value);
			}
		}
	}
	return(defvalue);
}


/// <summary>
/// Fetches a class identifier from the specified section.
/// This routine will fetch the printable form of a class identifier from the entry and
/// section specified and convert it back into binary form. If the entry is missing or the
/// text is not a legal identifier, then the default value is returned instead.
/// </summary>
/// <param name="section">The section name to search under.</param>
/// <param name="entry">The entry name to search for.</param>
/// <param name="defvalue">The default identifier to use if the entry could not be found.</param>
/// <returns>Returns with the class identifier specified in the INI database or else returns
/// the default value.</returns>
CLSID const INIClass::Get_CLSID(char const * section, char const * entry, CLSID defvalue) const
{
	char buffer[128];

	if (Get_String(section, entry, "", buffer, sizeof(buffer))) {
		wchar_t olestr[128];
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, buffer, -1, olestr, ARRAY_SIZE(olestr));
		CLSID clsid;
		if (SUCCEEDED(CLSIDFromString(olestr, &clsid))) {
			return(clsid);
		}
	}
	return(defvalue);
}


/// <summary>
/// Stores a class identifier into the INI database.
/// This routine will convert the identifier into its printable brace and hyphen form before
/// storing it, so that the resulting entry stays readable and can be edited by hand.
/// </summary>
/// <param name="section">The identifier for the section that the entry will be placed in.</param>
/// <param name="entry">The entry identifier to tag to the class identifier specified.</param>
/// <param name="value">The class identifier to store.</param>
/// <returns>bool; Was the class identifier placed into the INI database?</returns>
bool INIClass::Put_CLSID(char const * section, char const * entry, CLSID const & value)
{
	char buffer[128];
	LPOLESTR olestr = NULL;

	StringFromCLSID(value, &olestr);
	if (WideCharToMultiByte(CP_ACP, 0, olestr, -1, buffer, sizeof(buffer), NULL, NULL) == 0) {
		/// BUG, return not used
		GetLastError();
	}
	SysFreeString(olestr);
	return(Put_String(section, entry, buffer));
}


/***********************************************************************************************
 * INIClass::Put_Rect -- Store a rectangle  into the INI database.                             *
 *                                                                                             *
 *    This routine will store the four values that constitute the specified rectangle into     *
 *    the database under the section and entry specified.                                      *
 *                                                                                             *
 * INPUT:   section  -- Name of the section to place the entry under.                          *
 *                                                                                             *
 *          entry    -- Name of the entry that the rectangle data will be stored to.           *
 *                                                                                             *
 *          value    -- The rectangle value to store.                                          *
 *                                                                                             *
 * OUTPUT:  bool; Was the rectangle data written to the database?                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/19/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool INIClass::Put_Rect(char const * section, char const * entry, Rect const & value)
{
	char buffer[64];

	sprintf(buffer, "%d,%d,%d,%d", value.X, value.Y, value.Width, value.Height);
	return(Put_String(section, entry, buffer));
}


/// <summary>
/// Fetches a rectangle from the INI database.
/// </summary>
/// <returns>Returns with the rectangle specified. If the entry is absent, or does not hold
/// four numbers, then the default value is returned.</returns>
Rect const INIClass::Get_Rect(char const * section, char const * entry, Rect const & defvalue) const
{
	int values[4];

	if (Read_Numbers(section, entry, values, 4) == INIReadResult::Parsed) {
		Rect retval = defvalue;
		retval.X = values[0];
		retval.Y = values[1];
		retval.Width = values[2];
		retval.Height = values[3];
		return(retval);
	}
	return(defvalue);
}


/***********************************************************************************************
 * INIClass::Put_Hex -- Store an integer into the INI database, but use a hex format.          *
 *                                                                                             *
 *    This routine is similar to the Put_Int routine, but the number is stored as a hexadecimal*
 *    number.                                                                                  *
 *                                                                                             *
 * INPUT:   section  -- The identifier for the section that the entry will be placed in.       *
 *                                                                                             *
 *          entry    -- The entry identifier to tag to the integer number specified.           *
 *                                                                                             *
 *          number   -- The number to assign the the specified entry and placed in the         *
 *                      specified section.                                                     *
 *                                                                                             *
 * OUTPUT:  bool; Was the number placed into the INI database?                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool INIClass::Put_Hex(char const * section, char const * entry, int number)
{
	char buffer[MAX_LINE_LENGTH];

	sprintf(buffer, "%X", number);
	return(Put_String(section, entry, buffer));
}


/***********************************************************************************************
 * INIClass::Get_Hex -- Fetches integer [hex format] from the section and entry specified.     *
 *                                                                                             *
 *    This routine will search under the section specified, looking for a matching entry. The  *
 *    value is interpreted as a hexadecimal number and then returned. If no entry could be     *
 *    found, then the default value is returned instead.                                       *
 *                                                                                             *
 * INPUT:   section  -- The section identifier to search under.                                *
 *                                                                                             *
 *          entry    -- The entry identifier to search for.                                    *
 *                                                                                             *
 *          defvalue -- The default value to use if the entry could not be located.            *
 *                                                                                             *
 * OUTPUT:  Returns with the integer value from the specified section and entry. If no entry   *
 *          could be found, then the default value will be returned instead.                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/02/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int INIClass::Get_Hex(char const * section, char const * entry, int defvalue) const
{
	/*
	**	Verify that the parameters are nominally correct.
	*/
	if (section == NULL || entry == NULL) return(defvalue);

	INIEntry * entryptr = Find_Entry(section, entry);
	if (entryptr != NULL) {
		sscanf(entryptr->Value.c_str(), "%x", &defvalue);
	}
	return(defvalue);
}


/// <summary>
/// Fetches a floating point number from the INI database.
/// A percent sign anywhere in the value divides the number by one hundred, so 50% reads as
/// one half.
/// </summary>
/// <returns>Returns with the number specified. If the entry is absent, or does not start
/// with a number, then the default value is returned.</returns>
double INIClass::Get_Float(char const * section, char const * entry, double defvalue) const
{
	INIEntry * entryptr = Find_Entry(section, entry);
	if (entryptr == NULL) {
		return(defvalue);
	}

	char const * text = entryptr->Value.c_str();
	char * end = NULL;
	float const val = strtof(text, &end);
	if (end == text) {
		char const * source = Source_Of(*entryptr);
		DebugString("INI: %s%s[%s] %s=%s is not a number; using the default.\n", source, *source ? " " : "", section, entry, text);
		return(defvalue);
	}

	double result = val;
	if (strchr(text, '%') != NULL) {
		result /= 100.0;
	}
	return(result);
}


/***********************************************************************************************
 * INIClass::Put_Float -- Store a floating point number to the database.                       *
 *                                                                                             *
 *    This routine will store a flaoting point number to the section and entry of the          *
 *    database.                                                                                *
 *                                                                                             *
 * INPUT:   section  -- The section to store the entry under.                                  *
 *                                                                                             *
 *          entry    -- The entry to store the floating point number to.                       *
 *                                                                                             *
 *          number   -- The floating point number to store.                                    *
 *                                                                                             *
 * OUTPUT:  bool; Was the floating point number stored without error?                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/31/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool INIClass::Put_Float(char const * section, char const * entry, double number)
{
	char buffer[MAX_LINE_LENGTH];

	sprintf(buffer, "%f", (float)number);
	return(Put_String(section, entry, buffer));
}


/// <summary>
/// Stores a value under the section and entry named, creating the section if it is absent.
/// An existing entry keeps its comments and layout but takes the new value and moves to the
/// end of its section. An empty or NULL value removes the entry instead.
/// </summary>
/// <returns>bool; Were a section and an entry named? Storing never fails otherwise.</returns>
bool INIClass::Put_String(char const * section, char const * entry, char const * string)
{
	if (section == NULL || entry == NULL) return(false);

	INISection & secref = Find_Or_Add_Section(section);

	if (string == NULL || *string == '\0') {
		INIEntry * existing = secref.Find_Entry(entry);
		if (existing != NULL) {
			Remove_Entry(secref, *existing);
		}
	} else {
		Store_Entry(secref, entry, string);
	}
	return(true);
}


/***********************************************************************************************
 * INIClass::Get_String -- Fetch the value of a particular entry in a specified section.       *
 *                                                                                             *
 *    This will retrieve the entire text to the right of the "=" character. The text is        *
 *    found by finding a matching entry in the section specified. If no matching entry could   *
 *    be found, then the default value will be stored in the output string buffer.             *
 *                                                                                             *
 * INPUT:   section  -- Pointer to the section name to search under.                           *
 *                                                                                             *
 *          entry    -- The entry identifier to search for.                                    *
 *                                                                                             *
 *          defvalue -- If no entry could be found, then this text will be returned.           *
 *                                                                                             *
 *          buffer   -- Output buffer to store the retrieved string into.                      *
 *                                                                                             *
 *          size     -- The size of the output buffer. The maximum string length that could    *
 *                      be retrieved will be one less than this length. This is due to the     *
 *                      forced trailing zero added to the end of the string.                   *
 *                                                                                             *
 * OUTPUT:  Returns with the length of the string retrieved.                                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/02/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int INIClass::Get_String(char const * section, char const * entry, char const * defvalue, char * buffer, int size) const
{
	/*
	**	Verify that the parameters are nominally legal.
	*/
	if (buffer == NULL || size < 2 || section == NULL || entry == NULL) return(0);

	/*
	**	Fetch the entry string if it is present. If not, then the normal default
	**	value will be used as the entry value.
	*/
	INIEntry * entryptr = Find_Entry(section, entry);
	if (entryptr != NULL) {
		defvalue = entryptr->Value.c_str();
		if (entryptr->Value.size() > (std::size_t)(size - 1) && !entryptr->TruncationReported) {
			entryptr->TruncationReported = true;
			char const * source = Source_Of(*entryptr);
			DebugString("INI: %s%s[%s] %s is %d characters but the reader holds %d; the value was cut short.\n", source, *source ? " " : "", section, entry, (int)entryptr->Value.size(), size - 1);
		}
	}

	/*
	**	Fill in the buffer with the entry value and return with the length of the string.
	*/
	if (defvalue == NULL) {
		buffer[0] = '\0';
		return(0);
	} else if (buffer == defvalue) {
		return(strlen(buffer));
	} else {
		strncpy(buffer, defvalue, size);
		buffer[size-1] = '\0';
		strtrim(buffer);
		return(strlen(buffer));
	}
}


/// <summary>
/// Fetches the value of an entry as a string.
/// </summary>
/// <returns>The value, trimmed of surrounding whitespace, or the default trimmed the same way
/// when the entry is absent. A NULL default, section or entry reads as an empty string.</returns>
std::string INIClass::Get_String(char const * section, char const * entry, char const * defvalue) const
{
	if (section == NULL || entry == NULL) {
		return(std::string());
	}

	INIEntry * entryptr = Find_Entry(section, entry);
	char const * text = entryptr != NULL ? entryptr->Value.c_str() : defvalue;
	if (text == NULL) {
		return(std::string());
	}

	std::string result(text);
	strtrim(result.data());
	result.resize(strlen(result.data()));
	return(result);
}


/***********************************************************************************************
 * INIClass::Put_Bool -- Store a boolean value into the INI database.                          *
 *                                                                                             *
 *    Use this routine to place a boolean value into the INI database. The boolean value will  *
 *    be stored as "yes" or "no".                                                              *
 *                                                                                             *
 * INPUT:   section  -- The section to place the entry and boolean value into.                 *
 *                                                                                             *
 *          entry    -- The entry identifier to tag to the boolean value.                      *
 *                                                                                             *
 *          value    -- The boolean value to place into the database.                          *
 *                                                                                             *
 * OUTPUT:  bool; Was the boolean value placed into the database?                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool INIClass::Put_Bool(char const * section, char const * entry, bool value)
{
	if (value) {
		return(Put_String(section, entry, "yes"));
	} else {
		return(Put_String(section, entry, "no"));
	}
}


/***********************************************************************************************
 * INIClass::Get_Bool -- Fetch a boolean value for the section and entry specified.            *
 *                                                                                             *
 *    This routine will search under the section specified, looking for a matching entry. If   *
 *    one is found, the value is interpreted as a boolean value and then returned. In the case *
 *    of no matching entry, the default value will be returned instead. The boolean value      *
 *    is interpreted using the standard boolean conventions. e.g., "Yes", "Y", "1", "True",    *
 *    "T" are all consider to be a TRUE boolean value.                                         *
 *                                                                                             *
 * INPUT:   section  -- The section to search under.                                           *
 *                                                                                             *
 *          entry    -- The entry to search for.                                               *
 *                                                                                             *
 *          defvalue -- The default value to use if no matching entry could be located.        *
 *                                                                                             *
 * OUTPUT:  Returns with the boolean value of the specified section and entry. If no match     *
 *          then the default boolean value is returned.                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/02/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool INIClass::Get_Bool(char const * section, char const * entry, bool defvalue) const
{
	/*
	**	Verify that the parameters are nominally correct.
	*/
	if (section == NULL || entry == NULL) return(defvalue);

	INIEntry * entryptr = Find_Entry(section, entry);
	if (entryptr != NULL) {
		switch (toupper((unsigned char)entryptr->Value[0])) {
			case 'Y':
			case 'T':
			case '1':
				return(true);

			case 'N':
			case 'F':
			case '0':
				return(false);
		}
	}
	return(defvalue);
}


/***********************************************************************************************
 * INIClass::Put_Point -- Store a point value to the database.                                 *
 *                                                                                             *
 *    This routine will store the point value to the INI database under the section and entry  *
 *    specified.                                                                               *
 *                                                                                             *
 * INPUT:   section  -- The name of the section to store the entry under.                      *
 *                                                                                             *
 *          entry    -- The entry to store the point data to.                                  *
 *                                                                                             *
 *          value    -- The point value to store.                                              *
 *                                                                                             *
 * OUTPUT:  bool; Was the point value stored to the database?                                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/19/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool INIClass::Put_Point(char const * section, char const * entry, TPoint2D<int> const & value)
{
	char buffer[64];
	sprintf(buffer, "%d,%d", value.X, value.Y);
	return(Put_String(section, entry, buffer));
}


/// <summary>
/// Fetches a two dimensional point from the INI database.
/// </summary>
/// <returns>Returns with the point specified. If the entry is absent, or does not hold two
/// numbers, then the default value is returned.</returns>
TPoint2D<int> const INIClass::Get_Point(char const * section, char const * entry, TPoint2D<int> const & defvalue) const
{
	int values[2];

	if (Read_Numbers(section, entry, values, 2) == INIReadResult::Parsed) {
		return(TPoint2D<int>(values[0], values[1]));
	}
	return(defvalue);
}


/***********************************************************************************************
 * INIClass::Put_Point -- Stores a 3D point to the database.                                   *
 *                                                                                             *
 *    This routine will store the 3D point value to the database under the section and entry   *
 *    specified.                                                                               *
 *                                                                                             *
 * INPUT:   section  -- The name of the section that the entry will be stored under.           *
 *                                                                                             *
 *          entry    -- The name of the entry that the point will be stored to.                *
 *                                                                                             *
 *          value    -- The 3D point value to store.                                           *
 *                                                                                             *
 * OUTPUT:  bool; Was the point stored to the database?                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/19/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool INIClass::Put_Point(char const * section, char const * entry, TPoint3D<int> const & value)
{
	char buffer[64];
	sprintf(buffer, "%d,%d,%d", value.X, value.Y, value.Z);
	return(Put_String(section, entry, buffer));
}


/// <summary>
/// Fetches a three dimensional point from the INI database.
/// </summary>
/// <returns>Returns with the point specified. If the entry is absent, or does not hold three
/// numbers, then the default value is returned.</returns>
TPoint3D<int> const INIClass::Get_Point(char const * section, char const * entry, TPoint3D<int> const & defvalue) const
{
	int values[3];

	if (Read_Numbers(section, entry, values, 3) == INIReadResult::Parsed) {
		return(TPoint3D<int>(values[0], values[1], values[2]));
	}
	return(defvalue);
}


/***********************************************************************************************
 * INIClass::Put_Point -- Stores a 3D point to the database.                                   *
 *                                                                                             *
 *    This routine will store the 3D point value to the database under the section and entry   *
 *    specified.                                                                               *
 *                                                                                             *
 * INPUT:   section  -- The name of the section that the entry will be stored under.           *
 *                                                                                             *
 *          entry    -- The name of the entry that the point will be stored to.                *
 *                                                                                             *
 *          value    -- The 3D point value to store.                                           *
 *                                                                                             *
 * OUTPUT:  bool; Was the point stored to the database?                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/19/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool INIClass::Put_Point(char const * section, char const * entry, TPoint3D<float> const & value)
{
	char buffer[64];
	sprintf(buffer, "%f,%f,%f", (float)value.X, (float)value.Y, (float)value.Z);
	return(Put_String(section, entry, buffer));
}


/// <summary>
/// Fetches a three dimensional floating point vector from the INI database.
/// </summary>
/// <returns>Returns with the vector specified. If the entry is absent, or does not hold
/// three numbers, then the default value is returned.</returns>
TPoint3D<float> const INIClass::Get_Point(char const * section, char const * entry, TPoint3D<float> const & defvalue) const
{
	float values[3];

	if (Read_Numbers(section, entry, values, 3) == INIReadResult::Parsed) {
		return(TPoint3D<float>(values[0], values[1], values[2]));
	}
	return(defvalue);
}


/// <summary>
/// Finds an entry of this section by name. Names are compared byte for byte, so the search
/// is case sensitive.
/// </summary>
/// <param name="entry">The entry name to look for.</param>
/// <returns>The entry, or NULL if the section holds no entry of that name.</returns>
INIClass::INIEntry * INIClass::INISection::Find_Entry(char const * entry) const
{
	if (entry == NULL) {
		return(NULL);
	}
	auto found = EntryIndex.find(entry);
	return(found != EntryIndex.end() ? found->second : NULL);
}


/***********************************************************************************************
 * INIClass::Put_PKey -- Stores the key to the INI database.                                   *
 *                                                                                             *
 *    The key stored to the database will have both the exponent and modulus portions saved.   *
 *    Since the fast key only requires the modulus, it is only necessary to save the slow      *
 *    key to the database. However, storing the slow key stores the information necessary to   *
 *    generate the fast and slow keys. Because public key encryption requires one key to be    *
 *    completely secure, only store the fast key in situations where the INI database will     *
 *    be made public.                                                                          *
 *                                                                                             *
 * INPUT:   key   -- The key to store the INI database.                                        *
 *                                                                                             *
 * OUTPUT:  bool; Was the key stored to the database?                                          *
 *                                                                                             *
 * WARNINGS:   Store the fast key for public INI database availability. Store the slow key if  *
 *             the INI database is secure.                                                     *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/08/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool INIClass::Put_PKey(PKey const & key)
{
	char buffer[MAX_LINE_LENGTH];

	int len = key.Encode_Modulus(buffer);
	Put_UUBlock("PublicKey", buffer, len);

	len = key.Encode_Exponent(buffer);
	Put_UUBlock("PrivateKey", buffer, len);
	return(true);
}


/***********************************************************************************************
 * INIClass::Get_PKey -- Fetch a key from the ini database.                                    *
 *                                                                                             *
 *    This routine will fetch the key from the INI database. The key fetched is controlled by  *
 *    the parameter. There are two choices of key -- the fast or slow key.                     *
 *                                                                                             *
 * INPUT:   fast  -- Should the fast key be retrieved? The fast key has the advantage of       *
 *                   requiring only the modulus value.                                         *
 *                                                                                             *
 * OUTPUT:  Returns with the key retrieved.                                                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/08/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
PKey INIClass::Get_PKey(bool fast) const
{
	PKey key;
	char buffer[MAX_LINE_LENGTH];

	/*
	**	When retrieving the fast key, the exponent is a known constant. Don't parse the
	**	exponent from the database.
	*/
	if (fast) {
		BigInt exp = PKey::Fast_Exponent();
		exp.DEREncode((unsigned char *)buffer);
		key.Decode_Exponent(buffer);
	} else {
		Get_UUBlock("PrivateKey", buffer, sizeof(buffer));
		key.Decode_Exponent(buffer);
	}

	Get_UUBlock("PublicKey", buffer, sizeof(buffer));
	key.Decode_Modulus(buffer);

	return(key);
}


/// <summary>
/// Finds the column layout of a raw text line.
/// This routine is used by the loader when comments are being kept, so that the original
/// spacing of a line can be reproduced when the database is written back out. Tab stops are
/// taken into account, so the columns reported are the ones a viewer would see rather than
/// raw character offsets.
/// </summary>
/// <param name="buffer">Pointer to the null terminated text line to examine.</param>
/// <param name="assign_col">Reference to store the column of the "=" divider into.</param>
/// <param name="value_col">Reference to store the column the value text starts at into.</param>
/// <param name="comment_col">Reference to store the column the comment starts at into.</param>
/// <returns>Returns with a pointer to the comment text within the line. Otherwise, NULL is
/// returned.</returns>
char * INIClass::Scan_Line_For_Columns(char * buffer, int & assign_col, int & value_col, int & comment_col)
{
	char * line_comment = NULL;
	assign_col = -1;
	value_col = -1;
	comment_col = -1;

	int col = 0;

	while (*buffer != '\0') {
		if (assign_col >= 0 && value_col < 0) {
			//if (!isspace(*buffer)) {
			if ((unsigned char)*buffer > _CONTROL) {
				value_col = col;
			}
		}

		if (*buffer == ';') {
			comment_col = col;
			line_comment = buffer + 1;
			break;
		}

		switch (*buffer) {
			case '=' :
				if (assign_col < 0) {
					assign_col = col;
				}
				col++;
				break;

			case '\t' :
				col = (col & -8) + 8;
				break;

			default:
				col++;
				break;
		}

		buffer++;
	};

	assign_col = std::max(0, assign_col);
	value_col = std::max(0, value_col);
	comment_col = std::max(0, comment_col);

	return(line_comment);
}


/***********************************************************************************************
 * INIClass::Strip_Comments -- Strips comments of the specified text line.                     *
 *                                                                                             *
 *    This routine will scan the string (text line) supplied and if any comment portions are   *
 *    found, they will be trimmed off. Leading and trailing blanks are also removed.           *
 *                                                                                             *
 * INPUT:   buffer   -- Pointer to the null terminate string to be processed.                  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void INIClass::Strip_Comments(char * buffer)
{
	if (buffer != NULL) {
		char * comment = strchr(buffer, ';');
		if (comment) {
			*comment = '\0';
			strtrim(buffer);
		}
	}
}


/// <summary>
/// Is this text line a section heading?
/// This routine is used by the loader to recognize where one section of the database ends
/// and the next one begins. Any leading whitespace is tolerated.
/// </summary>
/// <param name="buffer">Pointer to the null terminated text line to examine.</param>
/// <returns>bool; Does the line hold a bracketed section name?</returns>
bool INIClass::Is_A_Section(char * buffer)
{
	if (buffer == NULL) {
		return(false);
	}

	//while (isspace(*buffer)) {
	while ((*buffer != 0) && ((unsigned char)*buffer <= _CONTROL)) {
		buffer++;
	}

	if (buffer[0] == '[' && strchr(buffer, ']') != NULL) {
		return(true);
	}

	return(false);
}


INIClass::INISection & INIClass::Find_Or_Add_Section(std::string_view name, INICommentBlock prefix)
{
	auto found = SectionIndex.find(name);
	if (found != SectionIndex.end()) {
		return(*found->second);
	}

	std::unique_ptr<INISection> section = std::make_unique<INISection>();
	section->Section = name;
	section->PrefixComment = std::move(prefix);

	INISection & added = *section;
	SectionList.push_back(std::move(section));
	SectionIndex.emplace(added.Section, &added);
	return(added);
}


/// <summary>
/// Stores a value on the entry named, creating the entry if the section lacks it.
/// An entry that already exists keeps its object, and with it its comments and layout, but
/// moves to the end of the section so that a later assignment always stands after an
/// earlier one.
/// </summary>
/// <returns>The entry the value was stored on.</returns>
INIClass::INIEntry & INIClass::Store_Entry(INISection & section, std::string_view entry, std::string_view value)
{
	auto found = section.EntryIndex.find(entry);
	if (found != section.EntryIndex.end()) {
		INIEntry & existing = *found->second;
		existing.Value = value;

		auto held = std::find_if(section.EntryList.begin(), section.EntryList.end(), [&existing](std::unique_ptr<INIEntry> const & candidate) {return(candidate.get() == &existing);});
		std::rotate(held, held + 1, section.EntryList.end());
		return(existing);
	}

	std::unique_ptr<INIEntry> created = std::make_unique<INIEntry>();
	created->Entry = entry;
	created->Value = value;

	INIEntry & added = *created;
	section.EntryList.push_back(std::move(created));
	section.EntryIndex.emplace(added.Entry, &added);
	return(added);
}


void INIClass::Remove_Entry(INISection & section, INIEntry & entry)
{
	section.EntryIndex.erase(entry.Entry);
	std::erase_if(section.EntryList, [&entry](std::unique_ptr<INIEntry> const & held) {return(held.get() == &entry);});
}


char const * INIClass::Source_Of(INIEntry const & entry) const
{
	return(entry.Generation < SourceNames.size() ? SourceNames[entry.Generation].c_str() : "");
}


/// <summary>
/// Reads count numbers separated by commas out of the text. Spaces around the commas are
/// allowed, and whatever follows the last number is ignored.
/// </summary>
/// <returns>How many numbers were read before the text stopped conforming.</returns>
template<class T, class Convert>
static int Parse_Numbers(char const * text, T * values, int count, Convert convert)
{
	char const * cursor = text;

	for (int index = 0; index < count; index++) {
		char * end = NULL;
		T const number = convert(cursor, &end);
		if (end == cursor) {
			return(index);
		}
		values[index] = number;
		cursor = end;

		if (index + 1 < count) {
			while (*cursor != '\0' && (unsigned char)*cursor <= 32) cursor++;
			if (*cursor != ',') {
				return(index + 1);
			}
			cursor++;
		}
	}
	return(count);
}


/// <summary>
/// Reads a comma separated list of whole numbers from the entry named.
/// </summary>
/// <param name="values">Receives the numbers. Only a Parsed result fills every slot.</param>
/// <param name="count">How many numbers the value has to hold.</param>
/// <returns>Absent if there is no such entry, Malformed if the value does not hold that many
/// numbers (which is logged), and Parsed otherwise.</returns>
INIClass::INIReadResult INIClass::Read_Numbers(char const * section, char const * entry, int * values, int count) const
{
	INIEntry * entryptr = Find_Entry(section, entry);
	if (entryptr == NULL) {
		return(INIReadResult::Absent);
	}

	char const * text = entryptr->Value.c_str();
	if (Parse_Numbers(text, values, count, [](char const * cursor, char ** end) {return((int)strtol(cursor, end, 10));}) == count) {
		return(INIReadResult::Parsed);
	}

	char const * source = Source_Of(*entryptr);
	DebugString("INI: %s%s[%s] %s=%s is not %d numbers; using the default.\n", source, *source ? " " : "", section, entry, text, count);
	return(INIReadResult::Malformed);
}


/// <summary>
/// Reads a comma separated list of floating point numbers from the entry named.
/// </summary>
/// <param name="values">Receives the numbers. Only a Parsed result fills every slot.</param>
/// <param name="count">How many numbers the value has to hold.</param>
/// <returns>Absent if there is no such entry, Malformed if the value does not hold that many
/// numbers (which is logged), and Parsed otherwise.</returns>
INIClass::INIReadResult INIClass::Read_Numbers(char const * section, char const * entry, float * values, int count) const
{
	INIEntry * entryptr = Find_Entry(section, entry);
	if (entryptr == NULL) {
		return(INIReadResult::Absent);
	}

	char const * text = entryptr->Value.c_str();
	if (Parse_Numbers(text, values, count, [](char const * cursor, char ** end) {return(strtof(cursor, end));}) == count) {
		return(INIReadResult::Parsed);
	}

	char const * source = Source_Of(*entryptr);
	DebugString("INI: %s%s[%s] %s=%s is not %d numbers; using the default.\n", source, *source ? " " : "", section, entry, text, count);
	return(INIReadResult::Malformed);
}
