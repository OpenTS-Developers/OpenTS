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

#include "objtype.h"

#include "anim.hh"

class WarheadTypeClass;
class OverlayTypeClass;

/****************************************************************************
**	All the animation objects are controlled by this class. It holds the static
**	data associated with each animation type.
*/
class AnimTypeClass : public ObjectTypeClass
{
		typedef ObjectTypeClass BASECLASS;

	public:

		/*
		**	This is the type number for this animation kind. It can be used as
		**	a unique identifier for animation types.
		*/
		AnimType HeapID;

		/*
		**	This is the frame that the animation is biggest. The biggest frame of animation
		**	will hide any changes to underlying ground (e.g., craters) that the animation
		**	causes, so these effects are delayed until this frame is reached. The end result
		**	is to prevent the player from seeing craters "pop" into existence.
		*/
		int Biggest;

		/*
		 * Some animations (when attached to another object) damage the object they are in
		 * contact with. Fire is a good example of this. This is the damage applied every game
		 * frame, and because it is fractional the damage can be very slight. It is also the
		 * damage a bouncing animation deals to whatever it comes down on.
		 */
		double Damage;

		/*
		**	Simple animation delay value between advancing of frames. This can
		**	be overridden by the control list.
		*/
		int Delay;

		/*
		**	The starting frame number for each animation sequence. Usually this is
		**	zero, but can sometimes be different if this animation is a sub sequence
		**	of a larger animation file.
		*/
		int Start;

		/*
		**	Looping animations might start at a different frame than the initial one.
		**	This is true for smoke effects that have a startup sequence followed by a
		**	continuous looping sequence.
		*/
		int LoopStart;

		/*
		**	For looping animations, this is the frame that will end all the middle loops
		**	of the animation. The last loop of the animation will proceed until the Stages
		**	has been fully completed.
		*/
		int LoopEnd;

		/*
		**	The number of stages that this animation sequence will progress through
		**	before it loops or ends.
		*/
		int Stages;

		/*
		**	This is the normal loop count for this animation. Usually this is one, but
		**	for some animations, it may be larger.
		*/
		int Loops;

		/*
		**	This is the sound effect to play when this animation starts. Usually, this
		**	applies to explosion animations.
		*/
		VocType Sound;

		/*
		**	If the animation is to launch into another animation, then
		**	the secondary animation will be defined here.
		*/
		AnimTypeClass const * ChainTo;

		/*
		 * This is the lowest detail setting (0 - 2) at which this animation is drawn at all.
		 * Purely decorative animations carry a high value so they vanish on a slow machine.
		 */
		int DetailLevel;

		/*
		 * This is the lowest detail setting (0 - 2) at which this animation is drawn
		 * translucently. Below it the animation is blitted solid, which is cheaper.
		 */
		int TranslucencyDetailLevel;

		/*
		 * These bound the random pause, expressed in game frames, inserted between one loop
		 * of the animation and the next. If both are zero, the animation loops with no pause.
		 */
		int RandomLoopDelayMin;
		int RandomLoopDelayMax;

		/*
		 * These bound the random delay between frame advances, converted from the repetitions
		 * per minute given for the animation. When both are zero, the flat Delay is used.
		 */
		int RandomRateMin;
		int RandomRateMax;

		/*
		 * This is the translucency this animation is drawn with (0, 25, 50 or 75 percent).
		 */
		int Translucency;

		/*
		 * This points to the animation that is scattered about the point where a bouncing
		 * animation comes to rest, so that an impact can throw off a shower of debris.
		 */
		AnimTypeClass const * Spawns;

		/*
		 * This bounds how many Spawns animations appear where a bouncer lands. The count is
		 * the sum of two random picks up to this value, so it clusters about the middle.
		 */
		int SpawnCount;

		/// Unused
		VocType StartSound;

		/*
		 * This is the sound effect to play each time a bouncing animation hits the ground.
		 */
		VocType BounceSound;

		/*
		 * This is the sound effect to play when a bouncing animation comes to rest on land.
		 * One that lands in water throws up a splash instead and this sound is not heard.
		 */
		VocType ExpireSound;

		/*
		 * This points to the animation created at the point of contact each time a bouncing
		 * animation strikes the ground -- a puff of dust or a spark, typically.
		 */
		AnimTypeClass const * BounceAnim;

		/*
		 * This points to the animation created where a bouncing animation finally settles. It
		 * also gates the impact damage, so a bouncer without one lands harmlessly.
		 */
		AnimTypeClass const * ExpireAnim;

		/*
		 * This points to the animation left behind at intervals as this one travels, which is
		 * how a bouncing or falling animation trails smoke behind it.
		 */
		AnimTypeClass const * TrailerAnim;

		/*
		 * This is the interval, expressed in game frames, between one TrailerAnim being left
		 * behind and the next. A value of one leaves a trailer on every frame.
		 */
		int TrailerSeperation;

		/*
		 * This is the fraction of its velocity that a bouncing animation keeps each time
		 * it strikes the ground. A value of 1.0 would bounce forever; the default of 0.8
		 * damps the animation down until it settles and removes itself.
		 */
		double Elasticity;

		/*
		 * This is the least vertical velocity a bouncing animation can be launched with,
		 * expressed in leptons per game frame. A bouncer's launch speed is picked at random
		 * between this and MaxZVel, while a meteor always launches at exactly this speed.
		 */
		double MinZVel;

		/*
		 * This is the greatest vertical velocity a bouncing animation can be launched with,
		 * expressed in leptons per game frame.
		 */
		double MaxZVel;

		/*
		 * This is the greatest horizontal speed a bouncing animation can be launched with,
		 * expressed in leptons per game frame. Both the X and the Y velocity are picked at
		 * random between the negative and the positive of this value, so that a burst of
		 * debris scatters evenly in every direction.
		 */
		double MaxXYVel;

		/*
		 * This is the warhead applied when a bouncing animation lands and hurts whatever
		 * it lands on. It decides how the Damage value is weighed against the armor of the
		 * victim and what side effects the impact brings with it.
		 */
		WarheadTypeClass const * Warhead;

		/*
		 * This is how close an object must be to the point of impact for a bouncing
		 * animation to hurt it, expressed in leptons. The damage dealt is taken from the
		 * Damage and Warhead values.
		 */
		int DamageRadius;

		/*
		 * This points to the tiberium overlay that an animation flagged IsTiberium sows
		 * where it lands. The patch actually laid down is picked at random from this type
		 * and the three that follow it in the overlay heap, so the growth is not uniform.
		 */
		OverlayTypeClass const * TiberiumSpawnType;

		/*
		 * This is the radius, expressed in cells, over which an animation flagged
		 * IsTiberium sows new tiberium. Every cell within the circle that is able to
		 * germinate gets a patch of TiberiumSpawnType.
		 */
		int TiberiumSpreadRadius;

		/*
		 * This value is added to the animation's Y sorting position, biasing where it falls
		 * in the ground layer's draw order. A negative value pushes the animation behind
		 * the objects it would otherwise be drawn on top of.
		 */
		int YSortAdjust;

		/*
		 * This is the number of pixels the animation's artwork is shifted downward by as it
		 * is drawn. It allows a shape whose art is not centered on its logical position to
		 * be nudged into place without the shape file being touched.
		 */
		int YDrawOffset;

		/*
		 * This is the number of frames devoted to each of the eight facings of a burning
		 * victim's run cycle. The stage to display is worked out from the direction he is
		 * running, so that the flames appear to face the way he travels.
		 */
		int RunningFrames;

		/*
		 * If this animation is a victim running about on fire, then this flag will be true.
		 * Such an animation steers itself toward the nearest water, plays the run cycle
		 * held in RunningFrames while it travels, and switches over to a death sequence
		 * once it arrives or runs out of anywhere to go.
		 */
		bool IsFlamingGuy;

		/*
		 * If this animation is a veinhole's attack tendril, then this flag will be true. It
		 * lays claim to the cell it occupies, damages whatever is standing there as it
		 * plays, and is drawn with the local player's color scheme.
		 */
		bool IsVeins;

		/*
		 * If this animation is a meteor, then this flag will be true. A meteor appears well
		 * away from its target and is flung at it, accelerating under gravity the whole
		 * way, and it deforms the terrain into a crater where it comes down.
		 */
		bool IsMeteor;

		/*
		 * If this animation should set off any tiberium it lands on top of, then this flag
		 * will be true. The tiberium is consumed, may throw off a piece of debris, and then
		 * explodes for the TiberiumExplosionDamage rule, setting off its neighbors in turn.
		 */
		bool IsTiberiumChainReaction;

		/*
		 * If this animation sows tiberium where it lands, then this flag will be true. The
		 * growth is laid over TiberiumSpreadRadius cells and drawn from TiberiumSpawnType.
		 * Coming down on a bridge sows nothing.
		 */
		bool IsTiberium;

		/*
		 * If this animation should be thrown into the air and left to bounce, then this flag
		 * will be true. Rather than playing in place it is launched with a random velocity
		 * bounded by MaxXYVel and the ZVel pair, then follows that arc until it settles.
		 */
		bool IsBouncer;

		/*
		 * If this animation should be repeated upward until it runs off the top of the
		 * view, then this flag will be true. Typical of this would be a column of smoke
		 * that must reach the top of the screen however tall the screen happens to be.
		 */
		bool IsTiled;

		/*
		 * If this animation should be drawn through the color converter of the cell or the
		 * owner it belongs to instead of the shared animation drawer, then this flag will
		 * be true. It is what lets a building's animations pick up its house colors.
		 */
		bool IsShouldUseCellDrawer;

		/*
		 * If this animation should be drawn at full brightness rather than the lighting of
		 * the ground it stands on, then this flag will be true. Explosions and other light
		 * sources want this, since they light the ground rather than being lit by it.
		 */
		bool IsUseNormalLight;

		/*
		 * If this animation's artwork should be fetched only when it is first needed rather
		 * than up front with the rest of the theater, then this flag will be true. It keeps
		 * rarely played animations out of memory, at the cost of a load when one is asked for.
		 */
		bool IsDemandLoad;

		/*
		 * If a demand loaded animation should release its artwork again as soon as an
		 * instance of it finishes playing, then this flag will be true. Without it the
		 * artwork stays resident for the rest of the scenario once it has been loaded.
		 */
		bool IsFreeAfterPlaying;

		/*
		 * If this animation is the moving overlay that dresses a tiberium field, then this
		 * flag will be true. It removes itself as soon as the overlay beneath it stops
		 * naming this animation as its CellAnim, so the effect goes with the tiberium.
		 */
		bool IsAnimatedTiberium;

		/*
		 * If this animation should be drawn through the first color scheme's converter
		 * instead of the shared animation drawer, then this flag will be true.
		 */
		bool IsAltPalette;

		/*
		**	If this animation should run at a constant apparent rate regardless
		**	of game speed setting, then this flag will be set to true.
		*/
		bool IsNormalized;

		/*
		**	If this animation should be rendered and sorted with the other ground
		**	units, then this flag is true. Typical of this would be fire and other
		**	low altitude animation effects.
		*/
		bool IsGroundLayer;

		/*
		 * If this animation lies flat upon the ground rather than standing upright, then
		 * this flag will be true. It is drawn with a ground depth gradient so that it
		 * sinks into a slope instead of being sorted as though it had height.
		 */
		bool IsFlat;

		/*
		**	If this animation should be rendered in a translucent fashion, this flag
		**	will be true. Translucent colors are some of the reds and some of the
		**	greys. Typically, smoke and some fire effects have this flag set.
		*/
		bool IsTranslucent;

		/*
		**	Some animations leave a scorch mark behind. Napalm and other flame
		**	type explosions are typical of this type.
		*/
		bool IsScorcher;

		/*
		**	If this is the special flame thrower animation, then custom affects
		**	occur as it is playing. Specifically, scorch marks and little fire
		**	pieces appear as the flame jets forth.
		*/
		bool IsFlameThrower;

		/*
		**	Some explosions are of such violence that they leave craters behind.
		**	This flag will be true for those types.
		*/
		bool IsCraterForming;

		/*
		**	If this animation should attach itself to any unit that is in the same
		**	location as itself, then this flag will be true. Most vehicle impact
		**	explosions are of this type.
		*/
		bool IsSticky;

		/*
		 * If this animation should reverse direction each time it reaches either end of
		 * its frames rather than snapping back to the start, then this flag will be true.
		 * It lets a short sequence play back and forth for as many loops as it is given.
		 */
		bool IsPingPong;

		/*
		 * If this animation should play from its last frame back to its first, then this
		 * flag will be true. It begins at the LoopEnd frame and steps backward, finishing
		 * when it arrives at Start.
		 */
		bool IsReverse;

		/*
		 * If this animation should be hidden while the cell it occupies is fogged, then
		 * this flag will be true. An animation belonging to a building is exempt from it,
		 * so a structure keeps animating even where the player cannot currently see it.
		 */
		bool IsShouldFogRemove;

		//---------------------------------------------------------------------------
		AnimTypeClass(char const * ininame = NULL);
		virtual ~AnimTypeClass(void) override;

		virtual ClassID Class_ID(void) const override;

		virtual void Serialize(SaveStreamClass & stream) override;
		virtual void Post_Load(void) override;

		static void Init(TheaterType theater);
		static char const * Name_From(AnimType anim);
		static AnimType From_Name(char const * name);
		static AnimTypeClass * Find_Or_Make(char const * name);

		/*
		**	Query functions.
		*/
		virtual RTTIType Fetch_RTTI(void) const override {return(RTTI_ANIMTYPE);}
		virtual void Compute_CRC(CRCEngine &) const override;
		virtual int Fetch_Heap_ID(void) const override {return(HeapID);}
		virtual void Detach(AbstractClass const * target, bool all = true) override;

		virtual void Init_Theater(TheaterType theater) override;
		virtual bool Read_INI(CCINIClass const & ini) override;

		virtual bool Create_And_Place(Cell const &, HouseClass * house = NULL) const override {return(false);};
		virtual ObjectClass * Create_One_Of(HouseClass *) const override {return(NULL);};
		virtual void const * Get_Image_Data(void) const override;

		virtual void Load_Image(TheaterType theater);
		void Free_Image(void);
};
