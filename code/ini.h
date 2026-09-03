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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/ini.h                                  $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 11/14/01 1:32a                                              $*
 *                                                                                             *
 *                    $Revision:: 16                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "crc.h"
#include "index.h"

#include <comdef.h>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class FileClass;
class Pipe;
class PKey;
class Straw;
template<class T> class TRect;
typedef TRect<int> Rect;
template<class T> class TPoint2D;
template<class T> class TPoint3D;

/*
**	This is an INI database handler class. It handles a database with a disk format identical
**	to the INI files commonly used by Windows.
*/
class INIClass {
	public:
		INIClass(void) : SourceNames(1) {}
		virtual ~INIClass(void);

		INIClass(INIClass const &) = delete;
		INIClass & operator = (INIClass const &) = delete;

		/*
		**	Fetch and store INI data.
		*/
		int Load(FileClass & file, bool keepcomments = false);
		int Load(Straw & file, bool keepcomments = false);
		int Save(FileClass & file) const;
		int Save(Pipe & file) const;

		/*
		**	Erase all data within this INI file manager.
		*/
		bool Clear(char const * section = NULL, char const * entry = NULL);

//		int Line_Count(char const * section) const;
		bool Is_Loaded(void) const {return(!SectionList.empty());}
		bool Is_Present(char const * section, char const * entry = NULL) const {if (entry == 0) return(Find_Section(section) != 0);return(Find_Entry(section, entry) != 0);}

		/*
		**	Fetch the number of sections in the INI file or verify if a specific
		**	section is present.
		*/
		int Section_Count(void) const;
		bool Section_Present(char const * section) const {return(Find_Section(section) != NULL);}

		/*
		**	Fetch the number of entries in a section or get a particular entry in a section.
		*/
		int Entry_Count(char const * section) const;
		char const * Get_Entry(char const * section, int index) const;

		/*
		**	Get the various data types from the section and entry specified.
		*/
		PKey Get_PKey(bool fast) const;
		bool Get_Bool(char const * section, char const * entry, bool defvalue=false) const;
		double Get_Float(char const * section, char const * entry, double defvalue=0.0) const;
		int Get_Hex(char const * section, char const * entry, int defvalue=0) const;
		int Get_Int(char const * section, char const * entry, int defvalue=0) const;
		int Get_String(char const * section, char const * entry, char const * defvalue, char * buffer, int size) const;
		int Get_TextBlock(char const * section, char * buffer, int len) const;
		int Get_UUBlock(char const * section, void * buffer, int len) const;
		Rect const Get_Rect(char const * section, char const * entry, Rect const & defvalue) const;
		TPoint3D<int> const Get_Point(char const * section, char const * entry, TPoint3D<int> const & defvalue) const;
		TPoint2D<int> const Get_Point(char const * section, char const * entry, TPoint2D<int> const & defvalue) const;
		TPoint3D<float> const Get_Point(char const * section, char const * entry, TPoint3D<float> const & defvalue) const;
		CLSID const Get_CLSID(char const * section, char const * entry, CLSID defvalue) const;

		/*
		**	Put a data type to the section and entry specified.
		*/
		bool Put_Bool(char const * section, char const * entry, bool value);
		bool Put_Float(char const * section, char const * entry, double number);
		bool Put_Hex(char const * section, char const * entry, int number);
		bool Put_Int(char const * section, char const * entry, int number, int format=0);
		bool Put_PKey(PKey const & key);
		bool Put_String(char const * section, char const * entry, char const * string);
		bool Put_TextBlock(char const * section, char const * text);
		bool Put_UUBlock(char const * section, void const * block, int len);
		bool Put_Rect(char const * section, char const * entry, Rect const & value);
		bool Put_Point(char const * section, char const * entry, TPoint3D<int> const & value);
		bool Put_Point(char const * section, char const * entry, TPoint3D<float> const & value);
		bool Put_Point(char const * section, char const * entry, TPoint2D<int> const & value);
		bool Put_CLSID(char const * section, char const * entry, CLSID const & value);

		/*
		**	Callers size the buffers they hand to Get_String from this, and the reader keeps
		**	at most this many characters of a line.
		*/
		enum {MAX_LINE_LENGTH=512};

		/*
		**	A block of lines held exactly as they were read out of the file: comments complete
		**	with the semicolon that introduced them, and the blank lines that spaced the file
		**	out. Save writes a block back ahead of whatever it introduced.
		*/
		using INICommentBlock = std::vector<std::string>;

		/*
		**	Names are looked up by their raw bytes, so lookups are case sensitive. The hash is
		**	transparent so that a C string or a view can be looked up without copying it.
		*/
		struct INIStringHash {
			using is_transparent = void;
			std::size_t operator () (std::string_view text) const noexcept {return(std::hash<std::string_view>()(text));}
		};

		/*
		**	The value entries for the INI file are stored as objects of this type.
		**	The entry identifier and value string are combined into this object.
		*/
		struct INIEntry {
			INIEntry(void) = default;
			INIEntry(INIEntry const &) = delete;
			INIEntry & operator = (INIEntry const &) = delete;

			std::string Entry;
			std::string Value;

			/*
			**	The comment lines that sat immediately above this entry in the file.
			*/
			INICommentBlock PrefixComment;

			/*
			**	The comment text that trailed this entry on its own line, with the semicolon
			**	that introduced it stripped off. It is empty when the entry carried no
			**	trailing comment; a bare semicolon gives an empty but present comment.
			*/
			std::optional<std::string> LineComment;

			/*
			**	The columns that the assignment character, the value, and the trailing comment
			**	stood at in the file this entry was read from. Save pads each line out with
			**	spaces to put them back, so that rewriting a database preserves the layout its
			**	author gave it.
			*/
			int AssignColumn = 0;
			int ValueColumn = 0;
			int CommentColumn = 0;

			/*
			**	The load that created this entry or last wrote to it, so that a repeat within
			**	one file can be told from a later file overriding an earlier one. Zero marks a
			**	value the engine stored itself.
			*/
			unsigned Generation = 0;
		};

		/*
		**	Each section (bracketed) is represented by an object of this type. All entries
		**	subordinate to this section are attached.
		*/
		struct INISection {
			INISection(void) = default;
			INISection(INISection const &) = delete;
			INISection & operator = (INISection const &) = delete;

			INIEntry * Find_Entry(char const * entry) const;

			std::string Section;

			/*
			**	The entries in file order. This order is what Save writes and what the
			**	positional readers see, so the index below is only ever used for lookup.
			*/
			std::vector<std::unique_ptr<INIEntry>> EntryList;
			std::unordered_map<std::string, INIEntry *, INIStringHash, std::equal_to<>> EntryIndex;

			/*
			**	The comment lines that sat immediately above this section's header in the file.
			*/
			INICommentBlock PrefixComment;

			unsigned Generation = 0;
		};

		/*
		**	Utility routines to help find the appropriate section and entry objects.
		*/
		static bool Is_A_Section(char * buffer);
		INISection * Find_Section(char const * section) const;
		INIEntry * Find_Entry(char const * section, char const * entry) const;
		static void Strip_Comments(char * buffer);
		static char * Scan_Line_For_Columns(char * buffer, int & assign_pos, int & value_pos, int & comment_pos);

		/*
		**	The sections in file order, and the lookup index over them.
		*/
		std::vector<std::unique_ptr<INISection>> SectionList;
		std::unordered_map<std::string, INISection *, INIStringHash, std::equal_to<>> SectionIndex;

	protected:
		int Load(Straw & file, bool keepcomments, char const * source);

		/*
		**	The outcome of reading a numeric value: the entry is absent, it is present but does
		**	not hold the numbers asked for, or every number was read.
		*/
		enum class INIReadResult {
			Absent,
			Malformed,
			Parsed,
		};

		INIReadResult Read_Numbers(char const * section, char const * entry, int * values, int count) const;
		INIReadResult Read_Numbers(char const * section, char const * entry, float * values, int count) const;
		char const * Source_Of(INIEntry const & entry) const;

	private:
		INISection & Find_Or_Add_Section(std::string_view name, INICommentBlock prefix = INICommentBlock());
		INIEntry & Store_Entry(INISection & section, std::string_view entry, std::string_view value);
		void Remove_Entry(INISection & section, INIEntry & entry);

		/*
		**	The comment lines that trailed the last section of the file, or the whole of a
		**	file that held no sections at all. Save writes them back out after everything
		**	else, so that nothing is lost off the end of the file.
		*/
		INICommentBlock TailComment;

		/*
		**	The file name of each load this database has seen, indexed by generation, so that
		**	a diagnostic can name the file a value came from. Generation zero is the engine.
		*/
		std::vector<std::string> SourceNames;
		unsigned LoadGeneration = 0;
};
