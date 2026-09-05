/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "audio/audiohandle.h"
#include "data.h"
#include "msanim.h"
#include "msengine.h"

class MSFont;
class MSAnimEntry;
class MSSfxEntry;
class INIClass;
struct GameOptionsType;
template<class T> class DynamicVectorClass;

#define MAX_WDT_TERRITORIES 100

void WDT_Game_Option_Append_Comma(char *str, int len);

class WDTTerritory {
	public:
		WDTTerritory(void);
		~WDTTerritory(void);

		bool operator==(const WDTTerritory &that) const { return(Index == that.Index); }
		bool operator!=(const WDTTerritory &that) const { return(Index != that.Index); }

		GameOptionsType *Get_Game_Options(void);

		char *To_String(void);

	public:
		/*
		 * This is the territory's position in the tour's territory list, and the number
		 * the ladder server and the click map both know it by.
		 */
		int Index;

		/*
		 * This is the seed the battlefield is generated from, so that every player fighting
		 * over this territory is given the same map. If zero, a seed is picked at random.
		 */
		unsigned short Seed;

		/*
		 * This is the number of vehicles each player starts the battle with (1 - 10).
		 */
		unsigned char UnitCount;

		/*
		 * This is the highest build level the players may reach in the battle (1 - 10),
		 * which is what limits how advanced their armies are allowed to become.
		 */
		unsigned char TechLevel;

		/*
		 * This is the number of credits each player starts the battle with.
		 */
		unsigned short StartingCredits;

		/*
		 * These are the dimensions of the battlefield, expressed as a size step (0 - 3)
		 * rather than in cells. Together they name the shape of map the player is told to
		 * expect -- tiny, narrow, tall, huge and so on.
		 */
		unsigned char Width;
		unsigned char Height;

		/*
		 * This is the number of players the battle over this territory is fought between.
		 * Only two and four are used, which the front end offers as 1on1 and 2on2.
		 */
		unsigned char NumPlayers;

		/*
		 * This is the number of veinhole monsters seeded into the battlefield (0 - 5).
		 */
		unsigned char Veinholes;

		/*
		 * This specifies the terrain the battlefield is generated from -- tundra, desert,
		 * taiga, temperate or mutated. The mutated setting needs the Firestorm addon.
		 */
		unsigned char Biome;

		/*
		 * This specifies the time of day the battle is lit by -- afternoon, dusk, morning
		 * or night.
		 */
		unsigned char Time;

		/*
		 * These are the limits the tour places on how much of the battlefield is broken up
		 * by cliffs, along with the amount it starts out with (0 - 100).
		 */
		unsigned char CliffsMin;
		unsigned char CliffsMax;
		unsigned char CliffsDefault;

		/*
		 * These are the limits the tour places on how freely units may cross the
		 * battlefield, along with the openness the ground starts out with (0 - 100).
		 */
		unsigned char AccessibilityMin;
		unsigned char AccessibilityMax;
		unsigned char AccessibilityDefault;

		/*
		 * These are the limits the tour places on how hilly the battlefield is, along with
		 * the amount of high ground it starts out with (0 - 100).
		 */
		unsigned char HillsMin;
		unsigned char HillsMax;
		unsigned char HillsDefault;

		/*
		 * These are the limits the tour places on how much water the battlefield holds,
		 * along with the amount it starts out with (0 - 100).
		 */
		unsigned char WaterMin;
		unsigned char WaterMax;
		unsigned char WaterDefault;

		/*
		 * These are the limits the tour places on how rich the battlefield's tiberium is,
		 * along with the amount it starts out with (0 - 100).
		 */
		unsigned char TiberiumAmountMin;
		unsigned char TiberiumAmountMax;
		unsigned char TiberiumAmountDefault;

		/*
		 * These are the limits the tour places on how widely the battlefield's tiberium is
		 * spread about, along with the layout it starts out with (0 - 100).
		 */
		unsigned char TiberiumFieldsMin;
		unsigned char TiberiumFieldsMax;
		unsigned char TiberiumFieldsDefault;

		/*
		 * These are the limits the tour places on how much tree cover the battlefield has,
		 * along with the amount it starts out with (0 - 100).
		 */
		unsigned char VegetationMin;
		unsigned char VegetationMax;
		unsigned char VegetationDefault;

		/*
		 * These are the limits the tour places on how built up the battlefield is, along
		 * with the number of civilian settlements it starts out with (0 - 100).
		 */
		unsigned char CitiesMin;
		unsigned char CitiesMax;
		unsigned char CitiesDefault;
		union {
			struct {
				/*
				 * Does the battlefield's lighting shift with the time of day?
				 */
				unsigned int TimeTransitions : 1;

				/*
				 * Do tiberium lifeforms roam the battlefield?
				 */
				unsigned int TiberiumCreatures : 1;

				/*
				 * May the players form alliances during the battle?
				 */
				unsigned int AlliesAllowed : 1;

				/*
				 * Are harvesters protected from being fired upon?
				 */
				unsigned int HarvTruce : 1;

				/*
				 * May the players build bases, or fight the battle out with the units
				 * they start with?
				 */
				unsigned int Bases : 1;

				/*
				 * May a deployed construction yard pack itself back up?
				 */
				unsigned int MCVRedeploy : 1;

				/*
				 * Does explored ground fade back into fog once nothing of the player's
				 * can see it any longer?
				 */
				unsigned int FogOfWar : 1;

				/*
				 * May bridges be brought down by weapon fire?
				 */
				unsigned int BridgeDestruction : 1;

				/*
				 * Are crates scattered across the battlefield?
				 */
				unsigned int Goodies : 1;

				/*
				 * Does the battlefield's tiberium include the richer blue strain as
				 * well as the green?
				 */
				unsigned int BlueTiberium : 1;

				/*
				 * Is a player defeated the moment his last building falls?
				 */
				unsigned int ShortGame : 1;

				/*
				 * Are engineers reduced to damaging a healthy building rather than
				 * capturing it outright?
				 */
				unsigned int CrapEngineer : 1;
			};

			/*
			 * This is the whole set of game option flags as one word, which is how the ladder
			 * server sends them down and how each option's bitmask picks out its own.
			 */
			unsigned int Booleans;
		};
		union {
			struct {
				/*
				 * These flags say which of the territory's settings the player is
				 * left free to change before the battle begins. A setting whose flag
				 * is clear is fixed by the tour, and its control in the map generator
				 * and game option dialogs is greyed out.
				 */
				unsigned int UserModTimeTransitions : 1;
				unsigned int UserModTiberiumCreatures : 1;
				unsigned int UserModAlliances : 1;
				unsigned int UserModHarvesterTruce : 1;
				unsigned int UserModBases : 1;
				unsigned int UserModMCVRedeploy : 1;
				unsigned int UserModFogOfWar : 1;
				unsigned int UserModBridgeDestruction : 1;
				unsigned int UserModCrates : 1;
				unsigned int UserModBlueTiberium : 1;
				unsigned int UserModShortGame : 1;
				unsigned int UserModCrapEngineer : 1;
				unsigned int UserModBiome : 1;
				unsigned int UserModTime : 1;
				unsigned int UserModNumPlayers : 1;
				unsigned int UserModUnitCount : 1;
				unsigned int UserModTechLevel : 1;
				unsigned int UserModCredits : 1;
				unsigned int UserModCliffs : 1;
				unsigned int UserModAccessability : 1;
				unsigned int UserModHills : 1;
				unsigned int UserModTiberiumAmount : 1;
				unsigned int UserModTiberiumFields : 1;
				unsigned int UserModWater : 1;
				unsigned int UserModVegetation : 1;
				unsigned int UserModCities : 1;
				unsigned int UserModWidth : 1;
				unsigned int UserModHeight : 1;
				unsigned int UserModSeed : 1;
				unsigned int UserModVeinholeMonsters : 1;
			};

			/*
			 * This is the whole set of user modifiable flags as one word, which is how the
			 * ladder server sends them down and how each option's bitmask picks out its own.
			 */
			unsigned int UserModBooleans;
		};
};

enum {
	/// the minimum size the data WDTState::Create_Territories requires to be read
	MIN_TERRITORY_DATA_SIZE = size_of(WDTTerritory, Seed)
							+ size_of(WDTTerritory, UnitCount)
							+ size_of(WDTTerritory, TechLevel)
							+ size_of(WDTTerritory, StartingCredits)
							+ size_of(WDTTerritory, Biome)
							+ size_of(WDTTerritory, Time)
							+ size_of(WDTTerritory, Width)
							+ size_of(WDTTerritory, Height)
							+ size_of(WDTTerritory, NumPlayers)
							+ size_of(WDTTerritory, Veinholes)
							+ size_of(WDTTerritory, CliffsDefault)
							+ size_of(WDTTerritory, AccessibilityDefault)
							+ size_of(WDTTerritory, HillsDefault)
							+ size_of(WDTTerritory, TiberiumAmountDefault)
							+ size_of(WDTTerritory, TiberiumFieldsDefault)
							+ size_of(WDTTerritory, WaterDefault)
							+ size_of(WDTTerritory, VegetationDefault)
							+ size_of(WDTTerritory, CitiesDefault)
							+ size_of(WDTTerritory, Booleans)
							+ size_of(WDTTerritory, UserModBooleans),
};

class WDTState {
	public:
		WDTState(void);
		WDTState(void *data);
		~WDTState(void);

		bool Has_Owner_History(void);
		bool Is_Previous_Cycle(void);

		bool Create_History(void *data, int size);
		bool Create_Territories(void *data, int size);

		char *To_String(void);

		enum State {
			UNASSIGNED = -1,
			CONTESTED,
			GDI,
			NOD,
		};

	public:
		/*
		 * This is the one line description of the tour cycle that the ladder server sent
		 * down, used wherever the campaign must be named in passing.
		 */
		char *ShortDesc;

		/*
		 * This is the full description of the tour cycle that the ladder server sent down.
		 * It is the caption the selection screen wears while no territory is picked out.
		 */
		char *LongDesc;

		/*
		 * This is the address of the web page the ladder server advertises for this cycle,
		 * where the standings are published.
		 */
		char *WebURL;

		/*
		 * This specifies which of the tour's world maps the cycle is fought over, and so
		 * which region artwork and territory list the selection screen loads.
		 */
		unsigned char MapID;

		/*
		 * This identifies the tour cycle. A campaign that remembers a different cycle has
		 * been left behind by the tour and is started over rather than resumed.
		 */
		unsigned int CycleID;

		/*
		 * This is the number of ticks a full tour cycle runs for.
		 */
		unsigned int CycleLength;

		/*
		 * This is the number of territories being fought over in this cycle.
		 */
		unsigned int NumTerritories;

		/*
		 * This is the number of ticks the cycle has reached, and so the number of rows the
		 * ownership history holds. The last of them is the state of the world right now.
		 */
		unsigned int NumTicks;

		/*
		 * This is the length of a tour tick, expressed in seconds. A report on a cycle that
		 * has already finished carries no timing of its own.
		 */
		unsigned int TickTime;

		/*
		 * This is the list of territory records for the cycle, each carrying the
		 * battlefield and the game options that the fight over that territory is
		 * settled by.
		 */
		DynamicVectorClass<WDTTerritory *> Territories;

		/*
		 * This records who held each territory at each tick of the cycle -- a row per
		 * tick, an entry per territory. The selection screen replays it to show the player
		 * how the map changed hands while he was away.
		 */
		unsigned char **OwnerHistory;

		/*
		 * If this state is a report on the cycle that has already ended, then this flag
		 * will be true. Such a report brings the finished ownership history alone -- it has
		 * no territory records and no tick timing.
		 */
		bool IsPreviousCycle;
};


namespace WorldDominationTour
{
	enum Outcome {
		OUTCOME_WIN,
		OUTCOME_LOSE,
		OUTCOME_DRAW,
		OUTCOME_COUNT
	};

	class State {
		public:
			enum TerritoryState {
				UNASSIGNED,
				DISPUTED,
				GDI,
				NOD,
			};

			State(void);
			~State(void);
			State(WDTState * state, unsigned int tick);
			State(State const & that);
			State & operator=(State const & that);
			bool operator==(State const & that);
			TerritoryState Get_Territory_State(unsigned int terr);
			int Count_Owned_Territories(int player_faction);
		public:
			/*
			 * This points to the tour state the snapshot was taken from, and is what a
			 * territory number is checked against. The state is not owned.
			 */
			WDTState *TourState;

			/*
			 * This points to the row of the tour state's ownership history belonging to the
			 * tick this snapshot was taken at. If NULL, then nobody owns anything.
			 */
			unsigned char *Owners;
	};

	class History {
		public:
			History(WDTState * state);
			~History(void);

			bool Is_Tick_Valid(unsigned int tick);
			State Get_State(unsigned int tick);

		public:
			/*
			 * This points to the tour state whose ownership history is read out. It belongs to
			 * the campaign rather than to the history, and is NULL when no state arrived.
			 */
			WDTState *TourState;
	};

	class Conflict {
		public:
			Conflict(WDTTerritory * terr = NULL);
			Conflict(Conflict const & that);
			~Conflict(void);
			Conflict & operator=(Conflict const & that);
			bool operator==(Conflict const & that) const;

			bool Is_Territory_Set(void) const;
			int Get_Territory_Index(void) const;

			void Init_Game_Options(void);
			void Deinit_Game_Options(void);
			void Process_Game_Options(char * str, int len);

			bool operator!=(Conflict const & that) const { return(!(*this == that)); }

		public:
			/*
			 * This points to the territory that is to be fought over, which is what supplies
			 * the battlefield and the game options once the player commits. If NULL, then the
			 * conflict is a placeholder with no battle attached to it.
			 */
			WDTTerritory * Territory;
	};

	typedef DynamicVectorClass<Conflict> CONFLICT_LIST;

	class CampaignProperties {
		public:
			CampaignProperties(WDTState * state);
			CampaignProperties(CampaignProperties const & that);
			~CampaignProperties(void);

			bool operator==(CampaignProperties const & that) const;
			CampaignProperties & operator = (CampaignProperties const & that);

			int Get_Cycle_ID(void) const;
			int Get_Layout(void) const;
			int Get_Map_ID(void) const;
			int Get_Current_Tick(void) const;
			const char * Get_Short_Desc(void) const;
			const char * Get_Long_Desc(void) const;
			const char * Get_Web_URL(void) const;

			/*
			 * This points to the tour state being described. Every property is answered out of
			 * it, and a state that never arrived is answered with placeholders instead.
			 */
			WDTState *TourState;
	};

	class Campaign {
		public:
			Campaign(char const *name, CampaignProperties const &props);
			~Campaign(void);

			bool Is_Different_Cycle(void);
			bool Is_Different(void);
			void Write_INI(void);
			void Reset_INI(void);
			void Load_INI(INIClass & ini);
			Conflict * FindConflict(int index);
			void Set_Faction(int faction);
			CampaignProperties & Get_Campaign_Properties(void) { return(Properties); }

		public:
			/*
			 * This describes the tour cycle the campaign belongs to -- its identifier, its map
			 * and how far it has run.
			 */
			CampaignProperties Properties;

			/*
			 * If the campaign is picking up a session the player already began, then this flag
			 * will be true. It is cleared when the tour has moved on to a cycle the recorded
			 * progress no longer belongs to.
			 */
			bool IsContinued;

			/*
			 * This is the record of who held each territory at each tick of the cycle,
			 * which is what the selection screen replays.
			 */
			History CycleHistory;

			/*
			 * These are the battles still to be fought -- one for every territory that stands
			 * disputed at the tick the campaign was built for.
			 */
			DynamicVectorClass<Conflict> Conflicts;

			/*
			 * This is the tour cycle that the campaign's recorded progress belongs to. Once it
			 * no longer matches the cycle the tour is running, the campaign starts over.
			 */
			int CycleID;

			/*
			 * This is the tick the player was last shown, so that a new session replays only
			 * what has happened since.
			 */
			int PreviousTick;

			/*
			 * This is the side the player has chosen to fight for. It decides the artwork, the
			 * voice-overs and the descriptions the campaign is presented with.
			 */
			int PlayerFaction;

			/*
			 * This is the name of the campaign, which doubles as the section its progress is
			 * recorded under in the tour history file.
			 */
			char Name[64];
	};

	class Territory {

		public:
			Territory(int id, const char * fname, INIClass const & ini, const char * name, int side, MSEngine & engine, ConvertClass * drawer, MS_ANIM_LIST * anims, Point2D const & pos);
			~Territory(void);
			void Set_Owner_State(int state, MSEngine & engine, bool wait);
			void Set_Highlight(bool active, MSEngine & engine);
			void Set_Target_Anim(MSAnim * anim);
			MSAnim * Remove_Target_Anim(void);
		public:
			/*
			 * This is the number the territory is known by, both in the tour's territory list
			 * and as the color index that stands for it in the click map.
			 */
			int ID;

			/*
			 * This is the name of the territory as it is shown to the player.
			 */
			char Name[64];

			/*
			 * This is the blurb shown beneath the territory's name, written for the side the
			 * player is fighting for.
			 */
			char Description[128];

			/*
			 * This is the point, relative to the map origin, the territory's target marker is
			 * pinned to.
			 */
			Point2D Target;

			/*
			 * This is the overlay that colors the territory according to who holds it. It is
			 * switched to another frame as the territory changes hands, and hidden outright
			 * while nobody holds it at all.
			 */
			MSOverlayAnim *OwnerAnim;

			/*
			 * This is the overlay that lights the territory up while the mouse is over it.
			 */
			MSOverlayAnim *HighlightAnim;

			/*
			 * This is the marker that throbs over the territory while it stands disputed, or
			 * NULL when there is nothing here to be fought over.
			 */
			MSAnim *TargetAnim;
	};

	typedef DynamicVectorClass<Territory *> TERR_LIST;

	class Map {
		public:
			Map(int id);
			virtual ~Map(void);
			void Add_Territory(const char * fname, INIClass const & ini, const char * name, int side, MSEngine & engine, ConvertClass * drawer, MS_ANIM_LIST * anims, Point2D const & pos);

		public:
			/*
			 * This specifies which of the tour's world maps is depicted.
			 */
			int ID;

			/*
			 * These are the territories the map is divided into, built out of the map control
			 * file and owned by the map.
			 */
			TERR_LIST Territories;
	};

	class Centroid {
		public:
			Centroid(void);
			Centroid const & operator+=(Point2D const & pt);
			Point2D Center_Point(void) const;
			bool operator==(Centroid const & that) const;
			bool operator!=(Centroid const & that) const { return(!(*this == that)); }

		public:
			/*
			 * This is the running sum of every point accumulated so far, which the center
			 * point is averaged out of.
			 */
			Point2D Position;

			/*
			 * This is the number of points accumulated so far, and the divisor the average is
			 * taken with.
			 */
			int Count;
	};

	typedef VectorClass<Centroid> CENTROID_LIST;

	/// Game Options

	enum GameOptionEnum {
		WDT_GAME_OPT_PRIORITY,		/// shown in the first summary pass
		WDT_GAME_OPT_NORMAL,		/// shown in the second summary pass
		WDT_GAME_OPT_USER_MOD,		/// user modifiable, excluded from the summary
	};

	enum GameOptionRangeEnum {
		WDT_GAME_OPT_RANGE_VERY_LOW,
		WDT_GAME_OPT_RANGE_LOW,
		WDT_GAME_OPT_RANGE_HIGH,
		WDT_GAME_OPT_RANGE_VERY_HIGH,
		WDT_GAME_OPT_RANGE_MIDDLE,
	};

	class GameOption {
		public:
			GameOption(unsigned int bitmask) :
				Bitmask(bitmask)
			{
			}

			virtual ~GameOption(void);
			virtual int Get_Display_Priority(const WDTTerritory * territory) = 0;
			virtual void Get_String(const WDTTerritory * territory, char * str, int len) = 0;
			bool Is_User_Modifiable(const WDTTerritory * territory);

		public:
			/*
			 * This is the bit that stands for this option in a territory's Booleans and
			 * UserModBooleans words. It is how the option finds its own setting, and how the
			 * summary keeps from describing the same option twice.
			 */
			unsigned int Bitmask;
	};

	class RangedGameOption : public GameOption {
		public:
			RangedGameOption(unsigned int bitmask, unsigned int offset, unsigned int string, unsigned int range0, unsigned int range1, unsigned int range2, unsigned int range3) :
				GameOption(bitmask),
				Offset(offset),
				StringID(string),
				Range0(range0),
				Range1(range1),
				Range2(range2),
				Range3(range3)
			{
			}
			virtual int Get_Display_Priority(const WDTTerritory * territory) override;
			virtual void Get_String(const WDTTerritory * territory, char * str, int len) override;
			virtual int Get_In_Range_Of(const WDTTerritory * territory) = 0;
			int In_Range_Of(unsigned int number);

		public:
			/*
			 * This is the offset of the setting within WDTTerritory, which is how the option
			 * reaches a value it was never told the name of.
			 */
			unsigned int Offset;

			/*
			 * This is the first of the run of strings that word this option's bands, the band
			 * number being added to it to pick the one that fits.
			 */
			unsigned int StringID;

			/*
			 * These are the thresholds that divide the setting's range into bands, from very
			 * low up to very high. A setting falling between the middle pair is unremarkable
			 * and is left out of the summary.
			 */
			unsigned int Range0;
			unsigned int Range1;
			unsigned int Range2;
			unsigned int Range3;
	};

	template<typename T>
	class RangedGameOptionT : public RangedGameOption {
		public:
			RangedGameOptionT(unsigned int bitmask, unsigned int offset, unsigned int string, unsigned int range0, unsigned int range1, unsigned int range2, unsigned int range3) :
				RangedGameOption(bitmask, offset, string, range0, range1, range2, range3)
			{
			}
			virtual int Get_In_Range_Of(const WDTTerritory * territory) override
			{
				T *ptr = (T *)((unsigned char *)territory + Offset);
				return(RangedGameOption::In_Range_Of(*ptr));
			}

	};

	class FlagGameOption : public GameOption {
		public:
			FlagGameOption(unsigned int bitmask, unsigned int off_string, unsigned int off_value, unsigned int on_string, unsigned int on_value) :
				GameOption(bitmask),
				OffValue(off_value),
				OnValue(on_value),
				OffString(off_string),
				OnString(on_string)
			{
			}
			virtual int Get_Display_Priority(const WDTTerritory * territory) override;
			virtual void Get_String(const WDTTerritory * territory, char * str, int len) override;

		public:
			/*
			 * These are the display priorities the option reports according to whether the
			 * flag is set, so that a rule which is switched on may be called out more urgently
			 * than one which is switched off.
			 */
			unsigned int OffValue;
			unsigned int OnValue;

			/*
			 * These are the strings that word the flag's two states. A state worded TXT_NONE
			 * keeps the option out of the summary altogether.
			 */
			unsigned int OffString;
			unsigned int OnString;
	};

	class NumberOfPlayersGameOption : public GameOption {
		public:
			NumberOfPlayersGameOption(unsigned int bitmask) :
				GameOption(bitmask)
			{
			}
			virtual int Get_Display_Priority(const WDTTerritory * territory) override;
			virtual void Get_String(const WDTTerritory * territory, char * str, int len) override;

	};

	class MapSizeGameOption : public GameOption {
		public:
			MapSizeGameOption(unsigned int bitmask) :
				GameOption(bitmask)
			{
			}
			virtual int Get_Display_Priority(const WDTTerritory * territory) override;
			virtual void Get_String(const WDTTerritory * territory, char * str, int len) override;
	};

	template<typename T>
	class ValueGameOption : public GameOption {
		public:
			ValueGameOption(unsigned int bitmask, unsigned int fallback, unsigned int offset, unsigned int string) :
				GameOption(bitmask),
				Fallback(fallback),
				Offset(offset),
				StringID(string)
			{
			}

			virtual int Get_Display_Priority(const WDTTerritory * territory) override
			{
				if (Is_User_Modifiable(territory)) {
					return(WDT_GAME_OPT_USER_MOD);
				}
				return(Fallback);
			}

			virtual void Get_String(const WDTTerritory * territory, char * str, int len) override
			{
				char buffer[256];

				WDT_Game_Option_Append_Comma(str, len);
				T *ptr = (T *)((unsigned char *)territory + Offset);
				sprintf(buffer, Fetch_String(StringID), *ptr);
				strncat(str, buffer, len);
			}

		public:
			/*
			 * This is the display priority the option reports for a setting the player is not
			 * free to change.
			 */
			unsigned int Fallback;

			/*
			 * This is the offset of the setting within WDTTerritory, which is how the option
			 * reaches a value it was never told the name of.
			 */
			unsigned int Offset;

			/*
			 * This is the format string the setting's value is printed through.
			 */
			unsigned int StringID;
	};

	typedef DynamicVectorClass<unsigned int> VOICEINDEX_LIST;

	class Voices
	{
		public:
			class Anim : public MSAnim {
				public:
					Anim(Voices *voices);
					virtual ~Anim(void);

					virtual void Set_Active(bool active);
					virtual void Pause(void);
					virtual void Resume(void);
					virtual bool Advance(Surface * surface, Rect & rect);
					virtual void Redraw(Surface * surface, Rect const * rect=NULL);
					virtual Rect Get_Rect(void) const;
					virtual bool Has_Finished(void) const;
					virtual void Restore(Rect const & rect);

				public:
					/*
					 * This points to the voice player the animation drives. The player
					 * belongs to the screen rather than to the animation.
					 */
					Voices * VoicePlayer;
			};

			class Sample {
				public:
					Sample(const char * name, int volume = 255);
					~Sample(void);

					bool Playing(void) const;
					void Start(void);
					void Stop(void);

				public:
					/*
					 * This is the volume the sample is played at, scaled by the player's
					 * own sound volume setting.
					 */
					int Volume;

					/*
					 * This is the audio engine's handle for the sample while it is
					 * playing, and a null handle whenever it is not.
					 */
					AudioHandle SoundHandle;

					/*
					 * This points to the sound data. If NULL, the sound was found
					 * neither in the mixfiles nor on the disk, and the sample will
					 * never make any noise.
					 */
					void * File;

					/*
					 * If the sound data was read off the disk, then this flag will be
					 * true and the data is freed along with the sample. Data that came
					 * out of a mixfile is left where it is.
					 */
					bool Allocated;
			};

			struct VoiceCategory {
				public:
					VoiceCategory(void);
					void Set_Name(const char *name);
					bool Pick_Standard_Sound(int side, Outcome outcome, char * buffer, int bufsize);
					bool Pick_Emphasis_Sound(int side, Outcome outcome, char * buffer, int bufsize);
					void Read(INIClass const & ini, const char * section, int side);
					void Read_Voiceover(INIClass const & ini, const char * section, int side, Outcome outcome, const char * outcome_string);

				public:
					/*
					 * This is the name the category is known by, which forms part
					 * of the INI entry each of its voice-over lists is read from.
					 */
					char Name[16];

					/*
					 * These are the voice-over numbers the category may draw on, kept
					 * per outcome and split into the ordinary lines and the emphatic
					 * ones that follow them.
					 */
					VOICEINDEX_LIST VoiceIndexes[OUTCOME_COUNT][2];
			};

			enum VoiceCategoryType {
				VOICECAT_OLD_CYCLE,
				VOICECAT_HISTORY,
				VOICECAT_STATUS,
				VOICECAT_TERRITORY,
				VOICECAT_GAME,
				VOICECAT_EVACUATE,
				VOICECAT_COUNT
			};

			enum StandaloneVoiceType {
				VOICE_STARTUP,
				VOICE_TERRITORY_SELECT,
				VOICE_COUNT
			};

			Voices(void) {}
			Voices(int side);
			~Voices(void);

			void Init(INIClass const & ini, const char * section);
			void Init_Standalone(INIClass const & ini, const char * section, StandaloneVoiceType voice, const char * voice_name);
			bool Pick_Standard_Voice(VoiceCategoryType category, Outcome outcome, char * buffer, int);
			bool Pick_Emphasis_Voice(VoiceCategoryType category, Outcome outcome, char * buffer, int);
			bool Pick_Standalone_Voice(StandaloneVoiceType voice, char * buffer, int);

			void Queue(VoiceCategoryType category, Outcome outcome, bool emphasis, bool count_queued, int delay);
			void Queue(StandaloneVoiceType voice, bool count_queued, int delay);
			void Advance(void);
			void Discard(void);
			void Cleanup(void);
			bool Playing(void);

		public:
			/*
			 * This is the side whose voice-overs this player speaks. The two sides keep their
			 * lines in separate numbered sets.
			 */
			int Side;

			/*
			 * These are the voice-over lists for every occasion the tour comments on -- an
			 * old cycle, the history replay, the player's standing, a territory, a game and
			 * an evacuation.
			 */
			VoiceCategory VoiceCategories[VOICECAT_COUNT];

			/*
			 * These are the voice-over lists for the one-off announcements that are tied to no
			 * outcome -- the startup greeting and the territory selection prompt.
			 */
			VOICEINDEX_LIST StandaloneVoices[VOICE_COUNT];

			/*
			 * This is the delay still to run before the queue starts speaking, which is how a
			 * line can be held back until the moment it comments on has arrived.
			 */
			CDTimerClass<SystemTimerClass> Timer;

			/*
			 * If the voice-over at the head of the queue has yet to be started, then this flag
			 * will be true. It keeps a line that has not been heard from being retired as
			 * though it had already played itself out.
			 */
			bool IsIdle;

			/*
			 * This is the number of queued voice-overs that must be heard rather than dropped.
			 * Anything queued beyond that count is thrown away when the next line arrives.
			 */
			int QueuedSampleCount;

			/*
			 * These are the voice-overs waiting to be spoken. Only the one at the head of the
			 * queue is ever sounding; the rest wait their turn.
			 */
			DynamicVectorClass<Sample *> QueuedSamples;
	};

	class Selection : public MSEngine {
		typedef DynamicVectorClass<MSSfxEntry *> SFX_LIST;

		public:
			Selection(Campaign *campaign, bool vq_anim);
			~Selection(void);

			void Set_Long_Description(void);
			Territory * Pick_Territory(void);
			void Do_Target_Selection(void);
			void Presentation(void);

		private:
			void Init_Voices(INIClass const & ini, const char * name);
			void Init_Sounds(INIClass const & ini, const char * name);
			void Init_Dimensions(INIClass const & ini, const char * name);
			void Init_Regions(INIClass const & ini, const char * name, bool vqanim);
			void Init_Art(INIClass const & ini, const char * name);
			void Init_Logo(INIClass const & ini, const char * name);
			void Init_Target(INIClass const & ini, const char * name);

			void Handle_Click(Point2D const & point, bool & selected);
			bool Select_Territory_At(Point2D const & point);
			void Update_Hovered_Territory(Point2D const & point);
			void Do_Target_Anim(State const & state, bool show_targets, bool silent);
			bool Select_Territory(Territory * territory);

			void Remove_Target_Anims(void);
			void Remove_Target_Anims2(void);

			void On_Map_Enter(void);
			void On_Map_Leave(void);
			void On_Cancel_Enter(void);
			void On_Cancel_Leave(void);

			Outcome Get_Outcome(int tick, bool & mega);
			bool Present_Ticks(int tick_from, int tick_to);

			bool Present_History(int tick_from, int tick_to);
			void Check_For_Breakout(void);
			void Set_Text(char const * text = NULL, int start_delay = 0);
			void Play_SFX(char const * name);

			void Add_Sfx_Entry(MSSfxEntry * entry) { if (entry != NULL) SfxEntries.Add(entry); }

			void Start(void);
			void End(void);

		public:
			/*
			 * This points to the campaign the screen presents: its map, its conflicts and the
			 * side the player fights for all come from here. The campaign is not owned.
			 */
			Campaign * TourCampaign;

			/*
			 * This is the conflict waiting on the territory now selected, and stays empty
			 * while the selection has no battle to offer.
			 */
			Conflict SelectedConflict;

			/*
			 * This is the announcer that comments on the campaign, speaking for the side the
			 * player has chosen.
			 */
			Voices VoicePlayer;

			/*
			 * These are the territory counts that decide how the announcer reads the player's
			 * standing. The plain pair choose between grim, even and triumphant commentary,
			 * and the mega pair decide whether it is delivered emphatically.
			 */
			int WinningThreshold;
			int LosingThreshold;
			int MegaWinningThreshold;
			int MegaLosingThreshold;

			/*
			 * If the player has pressed ESC to cut the presentation short, then this flag will
			 * be true. Every step of the replay polls it, so the map stops advancing and the
			 * sound effects that would have accompanied it are suppressed.
			 */
			bool Brokeout;

			/*
			 * This is the mouse position the screen last acted on, so that the hover work is
			 * only done over again once the mouse has actually moved.
			 */
			Point2D LastPoint;

			/*
			 * This is the tick the tour had reached when the screen opened. A territory only
			 * offers up its conflict while the tour has not moved on past it.
			 */
			int StartTick;

			/*
			 * This is the top left corner of the screen within the display, the screen being
			 * centered rather than filling it. Every part of the layout is placed relative to
			 * this point.
			 */
			Point2D Position;

			/*
			 * This is the part of the screen the mouse was last within -- the map or the
			 * cancel button -- so that entering and leaving each is only handled on a change.
			 */
			int LastHoverZone;

			/*
			 * This is the region of the world being fought over, together with the territories
			 * it is divided into.
			 */
			Map WorldMap;

			/*
			 * This is where the map begins within the screen. Mouse positions are made
			 * relative to it before the click map is asked which territory they fall on.
			 */
			Point2D MapOrigin;

			/// Unused
			ConvertClass * RegionDrawer;

			/*
			 * This is the palette the territory overlays are drawn through, taken from the
			 * region artwork so that they match the map beneath them.
			 */
			ConvertClass * TerritoryDrawer;

			/*
			 * This is the palette the player's faction logo is drawn through.
			 */
			ConvertClass * LogoDrawer;

			/*
			 * This is the backdrop the campaign map is painted over, playing either as a movie
			 * or as a still image. The screen waits it out before the player is given control.
			 */
			MSAnim * RegionAnim;

			/*
			 * This is the mask that turns a point on the map into a territory: the color index
			 * it carries at that point is the number of the territory lying there.
			 */
			Surface * ClickMapImage;

			/*
			 * This is the territory the mouse last read out of the click map. If -1, then the
			 * mouse is over no territory at all.
			 */
			int HoveredTerritoryID;

			/*
			 * This points to the territory the screen is presenting -- the one wearing the
			 * highlight and named in the caption -- or NULL while none is.
			 */
			Territory * MousedTerritory;

			/*
			 * This is the palette the target markers are drawn through.
			 */
			ConvertClass * TargetDrawer;

			/*
			 * This is the marker that zooms down onto a territory as it falls into dispute,
			 * announcing a fresh battle to be fought.
			 */
			MSAnimEntry * ZoomingTarget;

			/*
			 * This is the marker that pulses over a disputed territory for as long as the
			 * battle over it goes unclaimed.
			 */
			MSAnimEntry * ThrobbingTarget;

			/*
			 * This is the frame that divides the throbbing marker's artwork in two. The frames
			 * below it play while the territory merely stands disputed, and those from it on
			 * while the player has that territory selected.
			 */
			int ThrobbingTargetDividingFrame;

			/*
			 * This is the area of the screen the cancel button occupies, and so the region a
			 * click or a hover is measured against.
			 */
			Rect CancelButtonRectangle;

			/*
			 * This is the artwork the cancel button wears while the mouse is elsewhere.
			 */
			MSPCXAnim * CancelButtonAnim;

			/*
			 * This is the artwork the cancel button wears while the mouse is over it.
			 */
			MSPCXAnim * CancelButtonHoverAnim;

			/*
			 * If the mouse is over the cancel button, then this flag will be true. It keeps
			 * the button from being repainted on every frame the mouse rests there.
			 */
			char CancelButtonHovered;

			/*
			 * This is the panel the screen's caption is printed within, and the width the text
			 * is wrapped to.
			 */
			Rect TextRect;

			/*
			 * This is the animation that types the caption out a character at a time, or NULL
			 * while the panel is blank.
			 */
			MSPrintAnim * TextAnim;

			/*
			 * This is the caption being shown. It is wrapped in place before it is printed, so
			 * this holds the wrapped copy rather than the text the caller supplied.
			 */
			char Text[256];

			/*
			 * This is the font the caption is printed with, and the one its wrapping is
			 * measured by.
			 */
			MSFont * Font;

			/*
			 * This is the ownership snapshot the map is currently showing. The next one is
			 * compared against it to find the territories that have changed hands.
			 */
			State CurrentState;

			/*
			 * This is the name of the music the screen plays, which is faded out again as the
			 * screen closes.
			 */
			char *ThemeName;

			/*
			 * These are the screen's sound effects, each carrying the name it was given in the
			 * map control file so that the presentation may ask for one by name.
			 */
			DynamicVectorClass<MSSfxEntry *> SfxEntries;
	};

	Territory * Find_Territory_By_ID(TERR_LIST & terr, int id);
	Voices::Anim * WDT_New_Voiced_Animation(Voices & voices);
	void Write_Map_INI(char const * map_name, char const * pcx1_file_name, char const * pcx2_file_name);
};

void WDT_Review_Campaign(WorldDominationTour::Campaign * campaign);
bool WDT_Select_Campaign(WorldDominationTour::Campaign * campaign, bool vq_anim);

enum WDTStateRequestEnum {
	WDT_REQ_STATE_EVERYTHING = 3,
	WDT_REQ_STATE_LAST_CYCLE = 5,
};
