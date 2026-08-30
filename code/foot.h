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

/* $Header: /CounterStrike/FOOT.H 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : FOOT.H                                                       *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : April 14, 1994                                               *
 *                                                                                             *
 *                  Last Update : April 14, 1994   [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "ftimer.h"
#include "iloco.h"
#include "team.h"
#include "techno.h"

class UnitClass;
class BuildingClass;
class WaypointClass;
struct PathStruct;


/****************************************************************************
**	Movable objects are handled by this class definition. Moveable objects
**	cover everything except buildings.
*/
class FootClass : public TechnoClass
{
		typedef TechnoClass BASECLASS;
	public:
		/*
		 * This is the recorded waypoint path that this object is following, or PATH_NONE if
		 * it is not on one. Rather than heading for a single destination, an object on a path
		 * carries out the order stored at each of the path's waypoints in turn.
		 */
		PathType CurrentPath;

		/*
		 * When the waypoint's own cell cannot be used, this is the offset from that cell to
		 * the nearby cell the object was sent to instead. It is zero whenever the object is
		 * heading straight for the waypoint.
		 */
		Cell WaypointOffsetCell;

		/*
		 * This is the cell that the object was actually ordered to for the waypoint it is
		 * heading for. It is compared against the waypoint's own location so that a waypoint
		 * dragged elsewhere causes the order to be issued again.
		 */
		Cell WaypointTargetCell;

		/*
		 * This specifies how strongly the path finder should steer this object clear of
		 * dangerous regions of the map, copied from its type as the object is unlimboed. A
		 * value of zero ignores threat altogether; larger values buy a wider detour.
		 */
		double ThreatAvoidanceCoefficient;

		/*
		 * This is the running count of animation steps this object has taken. It advances
		 * once every WalkRate game frames while the locomotor reports movement, and the walk
		 * cycle, the tread animation and the harvesting animation are all indexed from it.
		 */
		int TotalFramesWalked;

		/*
		 * This is the cell this object was standing in when it last found a path, or the
		 * cell it is currently stepping toward once a locomotor takes over. A faster object
		 * that finds this one in its way reads this to learn where it is committed to going,
		 * and routes around that instead of shoving through -- so it is the whole basis of
		 * traffic avoidance between moving units. It is the cell being moved to, not the cell
		 * occupied.
		 */
		Cell LastPathingCell;

		/*
		 * This is the cell whose neighbors are currently credited with this object's
		 * presence -- their adjacent object counts, and the threat this object lends to its
		 * map region. It trails the real position until the object finishes moving into a
		 * new cell, at which point the old credits are withdrawn and applied to the new one.
		 */
		Cell LastAdjacencyCell;

		/*
		 * While the object is traveling through a tunnel, this is the coordinate of the
		 * next bend it is heading for. Reaching it moves the object on to the following
		 * tunnel segment, and at the end of the tube it carries the height that the object
		 * will surface at.
		 */
		Coord LastTubeCoord;

		/*
		**	This is the "throttle setting" of the unit. It is a fractional value with 0 = stop
		**	and 255 = full speed.
		*/
		double Speed;

		/*
		**	This is the override speed adjuster. Normally, this is a fixed point
		**	value of 0x0100, but it can be modified by crate powerups.
		*/
		double SpeedBias;

		/*
		 * This is the list of intermediate travel legs (bridge entry/exit and zone
		 * connection cells) generated by route planning toward the destination. As
		 * each leg is reached, the next is popped and assigned as the destination.
		 */
		DynamicVectorClass<AbstractClass *> RouteQueue;

		/*
		**
		**	This is the desired destination of the unit. The unit will attempt to head
		**	toward this target (avoiding intervening obstacles).
		*/
		AbstractClass * NavCom;
		AbstractClass * SuspendedNavCom;

		/*
		**	A sequence of move destinations can be given to a unit. The sequence is
		**	stores as an array of movement targets.
		*/
		DynamicVectorClass<AbstractClass *> NavQueue;

		/*
		**	This points to the team that "owns" this object. This pointer is used to
		**	quickly process the team when this object is the source of the change. An
		**	example would be if this object were to be destroyed, it would inform the
		**	team of this fact by using this pointer.
		*/
		TeamClass * Team;

		/*
		**	This points to the next member in the team that this object is part of. This
		**	is used to quickly process each team member when the team class is the source
		**	of the change. An example would be if the team decided that everyone is going
		**	to move to a new location, it would inform each of the objects by chaining
		**	through this pointer.
		*/
		FootClass * Member;

		/*
		 * This is the cell this object holds as its post while it breaks off a patrol to
		 * fight, or NULL when it is not engaged. Threat scans and reachability tests are
		 * measured from here, so a patrolling unit cannot be lured across the map.
		 */
		CellClass * PatrolCell;

		/*
		**	Since all objects derived from this class move according to a path list.
		**	This is the path list. It specifies, as a simple list of facings, the
		**	path that the object should follow in order to reach its destination.
		**	This path list is limited in size, so it might require several generations
		**	of path lists before the ultimate destination is reached. The game logic
		**	handles regenerating the path list as necessary.
		*/
		FacingType Path[CONQUER_PATH_MAX];

		/*
		**	When there is a complete findpath failure, this timer is initialized so
		**	that a findpath won't be calculated until this timer expires.
		*/
		CDTimerClass<FrameTimerClass> PathDelay;
		enum {PATH_RETRY=10};
		int TryTryAgain;		// Number of retry attempts remaining.

		/*
		**	If the object has recently attacked a base, then this timer will not
		**	have expired yet.  It is used so a building does not keep calling
		**	for help from the same attacker.
		*/
		CDTimerClass<FrameTimerClass> BaseAttackTimer;

		/*
		 * When the object first finds another object standing in its way, this timer is
		 * started. Until it expires the object keeps trying the direct route in the hope
		 * that the obstruction moves on, and only afterwards is the path finder allowed to
		 * spend the effort of routing the long way around it.
		 */
		CDTimerClass<FrameTimerClass> BlockagePathDelay;

		/*
		 * This is the locomotor that actually moves the object, created from the class ID
		 * named by its type. It can be swapped while the game runs -- a falling object is
		 * handed to a ballistic locomotor and a unit crossing a tunnel walks -- so all
		 * movement is asked of this interface rather than of the type's setting.
		 */
		ILocomotionPtr Locomotion;

		/*
		**	This is the coordinate that the unit is heading to
		**	as an immediate destination. This coordinate is never further
		**	than once cell (or track) from the unit's location. When this coordinate
		**	is reached, then the next location in the path list becomes the
		**	next HeadTo coordinate.
		*/
		Coord HeadToCoord;

		/*
		 * If this object is traveling through a tunnel, then this is the tube it entered.
		 * It is TUBE_NONE at all other times. This is what the rest of the game consults to
		 * know that the object is underground, and therefore must not be drawn, targeted,
		 * or blocked by whatever happens to be standing above it.
		 */
		char CurrentTube;

		/*
		 * This is how far along the tube's list of facings the object has walked. Each
		 * segment reached steps it on, and the FACING_NONE entry that ends the list means
		 * the object has arrived at the far mouth and may surface.
		 */
		char CurrentTubeDir;

		/*
		 * This is the waypoint within the current path that the object is heading for. As
		 * the order recorded at each waypoint is carried out, this advances to the one after
		 * it, and the path is abandoned once there are no waypoints left.
		 */
		char NextWaypoint;

		/*
		**	This flag tells a unit that, if after reaching its destination, it
		**	should scatter away. It's meant to help a LST unload its units by
		**	having its previous passengers get out of the way.
		*/
		bool IsToScatter;

		/*
		**	This flag controls whether a range limiting effect should be in place. If
		**	true, then target scanning will be limited to the range of the object
		**	regardless of what was requested from the target scanning logic. This value
		**	is used for ships so that they won't permanently stick on a an attack mission
		**	for a target they can never get within range of. This value will toggle when
		**	a path cannot be generated and the target is not within range. It will also
		**	toggle when path limiting is true, but there is not target found within
		**	the limited range.
		*/
		bool IsScanLimited;

		/*
		**	If this unit has officially joined the team's group, then this flag is
		**	true. A newly assigned unit to a team is not considered part of the
		**	team until it actually reaches the location where the team is. By
		**	using this flag, it allows a team to continue to intelligently attack
		**	a target without falling back to regroup the moment a distant member
		**	joins.
		*/
		bool IsInitiated;

		/*
		**	When the player gives this object a navigation target AND that target
		**	does not result in any movement of the unit, then a beep should be
		**	sounded. This typically occurs when selecting an invalid location for
		**	movement. This flag is cleared if any movement was able to be performed.
		**	It never gets set for computer controlled units.
		*/
		bool IsNewNavCom;

		/*
		**	There are certain cases where a unit should perform a full scan rather than
		**	the more efficient "ring scan". This situation occurs when a unit first
		**	appears on the map or when it finishes a multiple cell movement track.
		*/
		bool IsPlanningToLook;

		/*
		**	Certain units have the ability to metamorphize into a building. When this
		**	operation begins, certain processes must occur. During these operations, this
		**	flag will be true. This ensures that any necessary special case code gets
		**	properly executed for this unit.
		*/
		bool IsDeploying;

		/*
		**	This flag tells the system that the unit is doing a firing animation. This is
		**	critical to the firing logic.
		*/
		bool IsFiring;

		/*
		**	This unit could be either rotating its body or rotating its turret. During the
		**	process of rotation, this flag is set. By examining this flag, unnecessary logic
		**	can be avoided.
		*/
		bool IsRotating;

		/*
		**	If this object is unloading from a hover transport, then this flag will be
		**	set to true. This handles the unusual case of an object disembarking from the
		**	hover lander yet not necessarily tethered but still located in an overlapping
		**	position. This flag will be cleared automatically when the object moves to the
		**	center of a cell.
		*/
		bool IsUnloading;

		/*
		**	If the navigation movement queue is to be looped rather than consumed, then
		**	this flag will be true. By looping, the unit will travel through the locations
		**	in the queue indefinately.
		*/
		bool IsNavQueueLoop;

		/*
		**	If this object is scattering, then this flag will be true. While true, the
		**	NavCom should not be arbitrarily changed. This flag will automatcially be
		**	cleared when the object moves one cell.
		*/
		bool IsScattering;

		/*
		 * If this object has already been put into its idle state during the current game
		 * frame, then this flag will be true. It is cleared again at the start of every
		 * frame, and it keeps the idle handling from running twice over for one object.
		 */
		bool IsIdle;

		/*
		 * This is the distance, expressed in screen pixels, that the object is lifted up
		 * the display while an ion storm blast tosses it about. It is thrown away at the
		 * start of every game frame, so the blast must keep renewing it to hold the object
		 * in the air.
		 */
		char IonBlastYDrawOffset;

		/*
		 * If this vehicle is in the act of crushing something beneath it, then this flag
		 * will be true. While it is set the vehicle is held down to a crawl and rocks
		 * through a shallower angle, so that it appears to labor over the obstacle.
		 */
		bool IsCrushing;

		/*
		 * If this object currently claims the cells it stands over, then this flag will be
		 * true. A hovering or levitating object gives up its claim while it is off the
		 * ground so that others may pass beneath it, and takes it up again upon landing.
		 */
		bool IsOccupyingCell;

		/*
		 * If this object has found another object standing in its way, then this flag will
		 * be true. It is cleared again once the object moves, and with the BlockagePathDelay
		 * timer it decides when the path finder may spend the effort of routing around.
		 */
		bool IsToPathAroundBlockage;

		/*
		 * If this object has been released from a team, then this flag will be true. A weed
		 * harvester consults it to return to harvesting rather than standing guard where the
		 * team left it, and clears it once it does.
		 */
		bool IsDroppedFromTeam;

		/*---------------------------------------------------------------------
		**	Constructors, Destructors, and overloaded operators.
		*/
		FootClass(HouseClass * house);
		virtual ~FootClass(void) override;

		virtual void Serialize(SaveStreamClass & stream) override;

		/*---------------------------------------------------------------------
		**	Member function prototypes.
		*/
		bool Basic_Path(Cell cell, int path_offset = 0, int avoidance = 0);
		void Advance_Path(int count);

		virtual void Compute_CRC(CRCEngine &) const override;
		virtual Coord Destination_Coord(void) const override;
		virtual RadioMessageType Receive_Message(RadioClass * from, RadioMessageType message, int & param) override;
		virtual bool Can_Demolish(void) const override;
		bool Is_Recruitable(HouseClass const * house=NULL) const;
		bool Is_On_Priority_Mission(void) const;
		virtual bool Is_Moving_Onto_Bridge(void) const override;
		virtual bool Occupies_Cells(void) const override {return(IsOccupyingCell);};
		virtual void Delete_Me(void) override;
		virtual bool Should_Delete_Off_Map(void);
		virtual bool Is_Considered_Slow(void);
		virtual bool Deploy_To_Fire(void) const {return(false);};

		/*
		**	Coordinate inquiry functions. These are used for both display and
		**	combat purposes.
		*/
		virtual bool On_Ground(void) const override;
		virtual bool In_Air(void) const override;
		virtual Coord Likely_Coord(void) const;
		virtual bool Is_In_Same_Zone_As(ObjectClass const * object) const override;
		virtual bool Is_In_Same_Zone(Coord const & coord) const override;

		virtual void On_Movement_Blocked(void);
		virtual bool JumpJet_To_Walk(void) {return(false);};

		/*
		**	Driver control support functions. These are used to control cell
		**	occupation flags and driver instructions.
		*/
		virtual bool Start_Driver(Coord & headto);
		virtual bool Stop_Driver(void);
		virtual void Assign_Destination(AbstractClass * target, bool = true) override;
		virtual bool Enter_Idle_Mode(bool initial=false, bool = true) override;
		virtual bool Is_Allowed_To_Leave_Map(void) const override;
		virtual bool Is_Docked_For_Repair(void);
		void Link_DropPod(void);
		Cell Move_Order(Cell const & where, bool consider_fog);

		/*
		**	Display and rendering support functionality. Supports imagery and how
		**	object interacts with the map and thus indirectly controls rendering.
		*/
		virtual VisualType Visual_Character(bool raw = false, HouseClass const * = NULL) const override;
		virtual bool Limbo(void) override;
		virtual bool Unlimbo(Coord const & , Dir256 dir = DIR_N) override;
		virtual LayerType In_Which_Layer(void) const override;
		virtual void Draw_It(Point2D const & point, Rect const & cliprect) const override;
		virtual bool Mark(MarkType mark=MARK_CHANGE) override;
		virtual int Get_Z_Adjust(void) const override;
		virtual ZGradientType Get_Z_Gradient(void) const override;
		virtual void Draw_Action_Line(void) const override;
		virtual void Draw_Voxel(VoxelDataStruct const & voxeldata, int frame, int key, VoxelIndexClass * cache, Rect const & cliprect, Point2D const & point, Matrix3D const & matrix, int brightness, ShapeFlags_Type flags) const override;
		void Draw_Voxel_Shadow(VoxelDataStruct const & voxeldata, int layer_index, int key, VoxelIndexClass * cache, Rect const & cliprect, Point2D const & point, Matrix3D const & matrix, bool force_cache) const;
		virtual void Draw_Object(ShapeSet const * shapefile, int shapenum, Point2D const & xy, Rect const & rect, Dir256 rotation=DIR_N, int scale=0x0100, int zadjust=0, ZGradientType zgrad=ZGRAD_GROUND, bool=false, int brightness=0, ShapeSet const * zshapefile=0, int zshapenum=0, Point2D zoff=Point2D(0,0), ShapeFlags_Type flags=SHAPE_NORMAL) const;

		/*
		**	User I/O.
		*/
		virtual ActionType What_Action(ObjectClass const *, bool disallow_force = false) const override;
		virtual ActionType What_Action(Cell const &, bool check_fog = false, bool disallow_force = false) const override;
		virtual bool Active_Click_With(ActionType action, ObjectClass * object, bool) override;
		virtual bool Active_Click_With(ActionType action, Cell const & cell, bool) override;

		/*
		**	Combat related.
		*/
		virtual void Stun(void) override;
		virtual ResultType Take_Damage(int & damage, int distance, WarheadTypeClass const * warhead, TechnoClass * source=0, bool forced=false, bool=false) override;
		virtual void Death_Announcement(TechnoClass const * source=0) const override;
		virtual bool Captured(HouseClass * newowner) override;
		virtual void Berzerk(void) {};
		virtual void Start_Fear(void) {};
		virtual void Stop_Fear(void) {};
		virtual void Do_Idle(int second) {};

		/*
		**	AI.
		*/
		virtual void AI(void) override;
		virtual void Sell_Back(int control) override;
		virtual void Set_Waypoint_Path(PathType path, char index) override;
		void Execute_Waypoint_Path(WaypointClass * waypoint);
		virtual int Offload_Tiberium_Bail(void);
		virtual AbstractClass * Greatest_Threat(ThreatType threat, Coord const & coord, bool) const override;
		virtual void Detach(AbstractClass const * target, bool all) override;
		virtual void Detach_All(bool all=true) override;
		virtual int Do_MISSION_RETREAT(void) override;
		virtual int Do_MISSION_ENTER(void) override;
		virtual int Do_MISSION_MOVE(void) override;
		virtual int Do_MISSION_CAPTURE(void) override;
		virtual int Do_MISSION_ATTACK(void) override;
		virtual int Do_MISSION_GUARD(void) override;
		virtual int Do_MISSION_HUNT(void) override;
		virtual int Do_MISSION_GUARD_AREA(void) override;
		virtual int Do_MISSION_RESCUE(void) override;
		virtual int Do_MISSION_PATROL(void) override;
		virtual bool Is_Allowed_To_Recloak(void) const override;
		virtual bool Is_In_Team(void) const override {return(Team!=NULL);};
		virtual void Advance_Waypoint_Path(void) override;
		void Remove_From_Team(void) {if (Team != NULL) Team->Remove(this);}
		double Threat_Avoidance_Value(void) const;

		/*
		**	Scenario and debug support.
		*/
#ifdef _DEBUG
		virtual void Debug_Dump(MonoClass *mono) const override;
#endif

		/*
		**	Movement and animation.
		*/
		void Handle_Navigation_List(void);
		void Queue_Navigation_List(AbstractClass * target);
		void Clear_Navigation_List(void);
		virtual void Per_Cell_Process(PCPType why) override;
		virtual void Overrun_Square(Cell const & cell, bool threaten) {};
		virtual int Current_Speed(void);
		virtual void Approach_Target(void);
		virtual void Fixup_Path(PathStruct *) {};
		virtual void Set_Speed(double speed);
		virtual void Stop_Movement_Animation(void);
		virtual MoveType Can_Enter_Cell(CellClass const * cellptr, FacingType dir = FACING_NONE, int cell_height = -1, CellClass const * = 0, bool = true) const override;
		virtual MoveType Can_Reach(CellClass const * current_cell, FacingType facing, int & cell_height, bool & onto_bridge, CellClass const * adjacent_cell) const override;
		virtual void Set_Coord(Coord const & coord) override;
		int Optimize_Moves(PathStruct *path, MoveType threshhold);
		virtual void Override_Mission(MissionType mission, AbstractClass * tarcom, AbstractClass * navcom) override;
		virtual bool Restore_Mission(void) override;
		Cell Adjust_Dest(Cell const & cell) const;

		/*
		**	Landing zone support functionality.
		*/
		virtual bool Is_LZ_Clear(AbstractClass * target) const;

		Cell Safety_Point(Cell const & src, Cell const & dst, int start, int max);
		int Rescue_Mission(AbstractClass * tarcom);
		bool Tiberium_Check(Cell & center);
		bool Goto_Tiberium(int rad, bool allow_weighted = false);
		Cell Search_For_Tiberium(int rad, bool allow_weighted = false);
		Cell Search_For_Tiberium_Weighted(int rad);

		bool Weed_Check(Cell & center, int x, int y);
		bool Goto_Weed(int rad);
		Cell Search_For_Weed(int rad);

	private:
		PathStruct * Find_Path(Cell const & dest, FacingType * final_moves, int maxlen, MoveType threshhold, int path_offset, int avoidance);
};


inline void FootClass::Draw_Object(ShapeSet const * shapefile, int shapenum, Point2D const & xy, Rect const & rect, Dir256 rotation, int scale, int zadjust, ZGradientType zgrad, bool zwrite, int brightness, ShapeSet const * zshapefile, int zshapenum, Point2D zoff, ShapeFlags_Type negflags) const
{
	Techno_Draw_Object(shapefile, shapenum, xy, rect, rotation, scale, zadjust, zgrad, zwrite, brightness, zshapefile, zshapenum, zoff, negflags);
}


inline FootClass * AbstractClass::As_FootClass(void)
{
	return(dynamic_cast<FootClass *>(this));
}


inline FootClass const * AbstractClass::As_FootClass(void) const
{
	return(dynamic_cast<FootClass const *>(this));
}


extern DynamicVectorClass<FootClass *> Feet;
