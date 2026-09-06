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

/* $Header: /CounterStrike/RULES.CPP 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : RULES.CPP                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 05/12/96                                                     *
 *                                                                                             *
 *                  Last Update : September 10, 1996 [JLB]                                     *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Difficulty_Get -- Fetch the difficulty bias values.                                       *
 *   RulesClass::AI -- Processes the AI control constants from the database.                   *
 *   RulesClass::General -- Process the general main game rules.                               *
 *   RulesClass::Heap_Maximums -- Fetch and process the heap override values.                  *
 *   RulesClass::IQ -- Fetches the IQ control values from the INI database.                    *
 *   RulesClass::Land_Types -- Inits the land type values.                                     *
 *   RulesClass::MPlayer -- Fetch and process the multiplayer default settings.                *
 *   RulesClass::Powerups -- Process the powerup values from the database.                     *
 *   RulesClass::Process -- Fetch the bulk of the rule data from the control file.             *
 *   RulesClass::Recharge -- Process the super weapon recharge statistics.                     *
 *   RulesClass::RulesClass -- Default constructor for rules class object.                     *
 *   RulesClass::Themes -- Fetches the theme control values from the INI database.             *
 *   Techno_Get -- Get rule data common for all techno type objects.                           *
 *   _Scale_To_256 -- Scales a 1..100 number into a 1..255 number.                             *
 *   RulesClass::Difficulty -- Fetch the various difficulty group settings.                    *
 *   RulesClass::Objects -- Fetch all the object characteristic values.                        *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "rules.h"

#include "_bench.h"
#include "_palette.h"
#include "_rules.h"
#include "_warhead.h"
#include "_weapon.h"
#include "addon.h"
#include "airctype.h"
#include "anim.h"
#include "animtype.h"
#include "bench.h"
#include "builtype.h"
#include "bullettype.h"
#include "ccfile.h"
#include "conquer.h"
#include "convert.h"
#include "dbgprint.h"
#include "findmake.h"
#include "globals.h"
#include "house.h"
#include "houstype.h"
#include "infatype.h"
#include "levitate.h"
#include "mission.h"
#include "movie.h"
#include "overtype.h"
#include "psystype.h"
#include "ptype.h"
#include "savestream.h"
#include "scheme.h"
#include "script.h"
#include "side.h"
#include "smudtype.h"
#include "stimer.h"
#include "sun.h"
#include "suprtype.h"
#include "taskforc.h"
#include "teamtype.h"
#include "terrtype.h"
#include "tiberium.h"
#include "trim.h"
#include "unittype.h"
#include "vanimtype.h"
#include "vector.h"
#include "warhead.h"
#include "weapon.h"

#include "bench.hh"

#include <algorithm>
#include <cstring>


void Difficulty_Get(CCINIClass const & ini, DifficultyClass & diff, char const * section);

/***********************************************************************************************
 * _Scale_To_256 -- Scales a 1..100 number into a 1..255 number.                               *
 *                                                                                             *
 *    This is a helper routine that will take a decimal percentage number and convert it       *
 *    into a game based fixed point number.                                                    *
 *                                                                                             *
 * INPUT:   val   -- Decimal percent number to convert.                                        *
 *                                                                                             *
 * OUTPUT:  Returns with the decimal percent number converted to a game fixed point number.    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/17/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
static inline int _Scale_To_256(int val)
{
	val = 100.0 / (MPH_LIGHT_SPEED + 1) * val;
	val = std::min<int>(val, MPH_LIGHT_SPEED);
	return(val);
}


/***********************************************************************************************
 * RulesClass::RulesClass -- Default constructor for rules class object.                       *
 *                                                                                             *
 *    This is the default constructor for the rules class object. Although it initializes the  *
 *    rule data with default values, it is expected that they will all be overridden by the    *
 *    rules control file.                                                                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/17/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
RulesClass::RulesClass(void) :
	TurboBoost(1.5),
	AttackInterval(3),
	AttackDelay(5),
	PowerEmergencyFraction(.75),
	AirstripRatio(.12),
	AirstripLimit(5),
	HelipadRatio(.12),
	HelipadLimit(5),
	TeslaRatio(.16),
	TeslaLimit(10),
	AARatio(.14),
	AALimit(10),
	DefenseRatio(.5),
	DefenseLimit(40),
	WarRatio(.1),
	WarLimit(2),
	BarracksRatio(.16),
	BarracksLimit(2),
	RefineryLimit(4),
	RefineryRatio(.16),
	BaseSizeAdd(3),
	PowerSurplus(50),
	InfantryReserve(2000),
	InfantryBaseMult(2),
	SoloCrateMoney(2000),
	UnitCrateType(NULL),
	PatrolTime(.016),
	CloakDelay(0),
	GameSpeedBias(1),
	NervousBias(1),
	ExplosionSpread(.5),
	SupressRadius(CELL_LEPTON_W),
	MaxIQ(5),
	IQSuperWeapons(4),
	IQProduction(5),
	IQGuardArea(4),
	IQRepairSell(3),
	IQCrush(2),
	IQScatter(3),
	IQContentScan(4),
	IQAircraft(4),
	IQHarvester(3),
	IQSellBack(2),
	AIBaseSpacing(1),
	SilverCrate(CRATE_HEAL_BASE),
	WoodCrate(CRATE_MONEY),
	CrateMinimum(1),
	CrateMaximum(255),
	LZScanRadius(16*CELL_LEPTON_W),
	FlareAnim(NULL),
	MPMoney(3000),
	MPMaxMoney(10000),
	MPUnitCount(10),
	MPBuildLevel(10),
	IsMPShadowGrow(true),
	IsMPBasesOn(true),
	IsMPTiberiumGrow(true),
	IsMPCrates(true),
	IsMPAIPlayers(false),
	IsMPCaptureTheFlag(false),
	IsMPBridgeDestruction(true),
	IsMPBuildOffAllyAnyStructure(true),
	DropZoneRadius(4*CELL_LEPTON_W),
	MessageDelay(.6),
	SavourDelay(.03),
	MaxPlayers(MAX_PLAYERS),
	BaseDefenseDelay(.25),
	SuspendPriority(20),
	SuspendDelay(2),
	SurvivorFraction(.5),
	SurvivorDivisor(100),
	ReloadRate(.05),
	AutocreateTime(5),
	BuildupTime(.05),
	HarvesterLoadRate(2),
	HarvesterDumpRate(.016),
	AtomDamage(1000),
	Diff(),
	IsComputerParanoid(true),
	IsCurleyShuffle(false),
	IsBlendedFog(true),
	IsCompEasyBonus(true),
	IsFineDifficulty(false),
	IsExplosiveHarvester(false),
	IsHealthBar(true),
	IsAllyReveal(true),
	IsSeparate(false),
	IsTreeTarget(false),
	IsNamed(false),
	IsAutoCrush(false),
	IsSmartDefense(false),
	IsScatter(false),
	GrowthRate(2),
	ShroudRate(4),
	FogRate(.05),
	IceGrowthRate(1),
	VeinGrowthRate(1),
	IceSolidifyDelay(500),
	AmbientLightChangeRate(.2),
	AmbientLightChangeStep(.1),
	CrateTime(10),
	TimerWarning(2),
	NukeTime(14),
	EMPulseTime(5),
	IonCannonTime(10),
	FirestormTime(10),
	SpeakDelay(2),
	DamageDelay(1),
	Gravity(3),
	LeptonsPerSightIncrease(50),
	Incoming(MPH_IMMOBILE),
	MinDamage(1),
	MaxDamage(1000),
	RepairStep(5),
	RepairPercent(.25),
	IRepairStep(1),
	RepairRate(.016),
	URepairRate(.016),
	IRepairRate(.016),
	ConditionGreen(1),
	ConditionYellow(.5),
	ConditionRed(.5),
	RandomAnimateTime(.083),
	CloseEnoughDistance(5 * CELL_LEPTON / 2),
	StrayDistance(2 * CELL_LEPTON),
	CrushDistance(3 * CELL_LEPTON / 2),
	CrateRadius(5 * CELL_LEPTON / 2),
	HomingScatter(2 * CELL_LEPTON),
	BallisticScatter(CELL_LEPTON),
	RefundPercent(.5),
	BridgeStrength(1000),
	BuildSpeedBias(1),
	C4Delay(.03),
	RepairThreshhold(1000),
	PathDelay(.016),
	BlockagePathDelay(4 * TICKS_PER_SECOND),
	MovieTime(.25),
	TiberiumShortScan(6 * CELL_LEPTON),
	TiberiumLongScan(32 * CELL_LEPTON),
	TreeStrength(25),
	TeamDelays(),
	AIHateDelays(),
	DissolveUnfilledTeamDelay(5000),
	AIIonCannonConYardValue(),
	AIIonCannonWarFactoryValue(),
	AIIonCannonPowerValue(),
	AIIonCannonEngineerValue(),
	AIIonCannonThiefValue(),
	AIIonCannonHarvesterValue(),
	AIIonCannonMCVValue(),
	AIIonCannonAPCValue(),
	AIIonCannonBaseDefenseValue(),
	AIIonCannonPlugValue(),
	AIIonCannonHelipadValue(),
	AIIonCannonTempleValue(),
	AIAlternateProductionCreditCutoff(1000),
	MultiplayerAICreditMultipliers(),
	MinimumAIDefensiveTeams(),
	MaximumAIDefensiveTeams(),
	TotalAITeamCap(),
	AIUseTurbineUpgradeChance(1),
	FillEarliestTeamProbability(),
	LightningFrequency(250),
	LightningRandomness(75),
	LightningDamage(500),
	LightningDuration(20),
	LightningDeferment(31),
	CollapseChance(100),
	WeedCapacity(0),
	ExtraUnitLight(0),
	ExtraInfantryLight(0),
	ExtraAircraftLight(0),
	IsRevealByHeight(true),
	IsShroudedSubteranneanMovesAllowed(false),
	IsShroudGrow(false),
	NodAIBuildsWalls(true),
	AIBuildsWalls(true),
	UseMinDefenseRule(true),
	EMPulseSparkles(NULL),
	WebbedInfantry(NULL),
	JumpjetCloakDetectionRadius(0),
	DropPodInfantryMinimum(3),
	DropPodInfantryMaximum(5),
	TalkBubbleTime(5 * TIMER_SECOND),
	PrerequisiteGDIFactory(),
	PrerequisiteNodFactory(),
	EngineerCaptureLevel(1),
	EngineerDamage(0),
	AmmoCrateDamage(100),
	LargeVisceroid(NULL),
	SmallVisceroid(NULL),
	UnloadingHarvester(NULL),
	AttackingAircraftSightRange(5),
	TunnelSpeed(1),
	TiberiumHeal(1.0/60.0),
	HSBuilding(),
	IsFreeMCV(false),
	IsBerzerkAllowed(false),
	PoseDir(DIR_N),
	DropPodPuff(NULL),
	WaypointAnimationSpeed(12),
	BarrelExplode(NULL),
	BarrelDebris(),
	BarrelParticle(NULL),
	RadarEventColorSpeed(.05f),
	RadarEventMinRadius(5),
	RadarEventSpeed(1),
	RadarEventRotationSpeed(.1f),
	FlashFrameTime(7),
	RadarCombatFlashTime(21),
	MaxWaypointPathLength(15),
	Wake(NULL),
	FlamingInfantry(NULL),
	AITriggerSuccessWeightDelta(1),
	AITriggerFailureWeightDelta(-1),
	AITriggerTrackRecordCoefficient(1),
	VeinholeMonsterStrength(10000),
	MaxVeinholeGrowth(1000),
	VeinholeGrowthRate(100),
	VeinholeShrinkRate(100),
	VeinAttack(NULL),
	VeinDamage(2),
	MaximumQueuedObjects(5),
	AircraftFogReveal(6),
	WoodCrateImg(NULL),
	CrateImg(NULL),
	DropPod(),
	DeadBodies(),
	MetallicDebris(),
	BridgeExplosions(),
	DigSound(VOC_NONE),
	Dig(NULL),
	IonBlast(NULL),
	IonBeam(NULL),
	InfantryExplode(NULL),
	AtmosphereEntry(NULL),
	PrerequisitePower(),
	PrerequisiteFactory(),
	PrerequisiteBarracks(),
	PrerequisiteRadar(),
	PrerequisiteTech(),
	GateUpSound(VOC_NONE),
	GateDownSound(VOC_NONE),
	JumpjetTurnRate(3),
	JumpjetSpeed(30),
	JumpjetClimb(5),
	JumpjetCruiseHeight(400),
	JumpjetAcceleration(.25),
	JumpjetWobblesPerSecond(.25),
	JumpjetWobbleDeviation(40),
	RadarEventSuppressionDistances(),
	RadarEventVisibilityDurations(),
	RadarEventDurations(),
	IonCannonDamage(700),
	RailgunDamageRadius(CELL_LEPTON / 2),
	ZoomInFactor(2),
	ConditionRedSparkingProbability(.02),
	ConditionYellowSparkingProbability(.01),
	TiberiumExplosionDamage(100),
	TiberiumStrength(10),
	MinLowPowerProductionSpeed(.5),
	MultipleFactory(1),
	CraterLevel(4),
	TreeFlammability(.1),
	MissileSpeedVar(.25),
	MissileROTVar(.25),
	DropPodWeapon(NULL),
	DropPodHeight(1500),
	DropPodSpeed(40),
	DropPodAngle(M_PI/2), /// 90 deg
	ScrollMultiplier(1),
	CrewEscape(.5),
	ShakeScreen(400),
	HoverHeight(120),
	HoverBob(30),
	HoverBoost(1.3),
	HoverAcceleration(.03),
	HoverBrake(.03),
	HoverDampen(.8),
	PlacementDelay(.05),
	ExplosiveVoxelDebris(),
	TireVoxelDebris(NULL),
	ScrapVoxelDebris(NULL),
	BridgeVoxelMax(3),
	CloakingStages(9),
	RevealTriggerRadius(5),
	IceCrackingWeight(2),
	IceBreakingWeight(4),
	IceCrackSounds(),
	CliffBackImpassability(0),
	VeteranRatio(10),
	VeteranCombat(1),
	VeteranSpeed(1),
	VeteranSight(1),
	VeteranArmor(1),
	VeteranROF(1),
	VeteranCap(1),
	CloakSound(VOC_NONE),
	SellSound(VOC_NONE),
	GameClosed(VOC_NONE),
	IncomingMessage(VOC_NONE),
	SystemError(VOC_NONE),
	OptionsChanged(VOC_NONE),
	GameForming(VOC_NONE),
	PlayerLeft(VOC_NONE),
	PlayerJoined(VOC_NONE),
	Construction(VOC_NONE),
	CreditTicks(),
	CrumbleSound(VOC_NONE),
	BuildingSlam(VOC_NONE),
	RadarOn(VOC_NONE),
	RadarOff(VOC_NONE),
	ScoldSound(VOC_NONE),
	TeslaCharge(VOC_NONE),
	TeslaZap(VOC_NONE),
	GenericClick(VOC_NONE),
	GenericBeep(VOC_NONE),
	BlowupSound(VOC_NONE),
	HealCrateSound(VOC_NONE),
	ChuteSound(VOC_NONE),
	StopSound(VOC_NONE),
	GuardSound(VOC_NONE),
	ScatterSound(VOC_NONE),
	DeploySound(VOC_NONE),
	LightningSound(VOC_NONE),
	WorstLowPowerBuildRateCoefficient(.3),
	BestLowPowerBuildRateCoefficient(.75),
	WallBuildSpeedCoefficient(.5),
	ChargeToDrainRatio(3),
	DamageToFirestormDamageCoefficient(.1),
	TrackedUphill(1),
	TrackedDownhill(1),
	WheeledUphill(1),
	WheeledDownhill(1),
	SpotlightMovementRadius(2000),
	SpotlightLocationRadius(1000),
	SpotlightSpeed(.05),
	SpotlightAcceleration(.005),
	SpotlightAngle(20),
	SpotlightRadius(175),
	WindDirection(FACING_NONE),
	CameraRange(9*CELL_LEPTON_W),
	FlightLevel(500),
	BuildingDrop(VOC_NONE),
	Scorches(),
	Scorches1(),
	Scorches2(),
	Scorches3(),
	Scorches4(),
	Craters(),
	RepairBay(NULL),
	GDIGateOne(NULL),
	GDIGateTwo(NULL),
	NodGateOne(NULL),
	NodGateTwo(NULL),
	WallTower(NULL),
	GDIPowerPlant(NULL),
	GDIPowerTurbine(NULL),
	NodRegularPower(NULL),
	NodAdvancedPower(NULL),
	GDIFirestormGenerator(NULL),
	GDIHunterSeeker(NULL),
	NodHunterSeeker(NULL),
	BuildConst(),
	BuildPower(),
	BuildRefinery(),
	BuildBarracks(),
	BuildTech(),
	BuildWeapons(),
	BuildDefense(),
	BuildPDefense(),
	BuildAA(),
	BuildHelipad(),
	BuildRadar(),
	ConcreteWalls(),
	NSGates(),
	EWGates(),
	GDIWallDefense(6),
	GDIWallDefenseCoefficient(3),
	NodBaseDefenseCoefficient(1),
	GDIBaseDefenseCoefficient(1),
	ComputerBaseDefenseResponse(3),
	AIDetectDisguise(false),
	MaximumBaseDefenseValue(60),
	BaseUnit(NULL),
	HarvesterUnit(),
	PadAircraft(),
	OnFire(),
	TreeFire(),
	Smoke1(NULL),
	Smoke2(NULL),
	FirestormActiveAnim(NULL),
	FirestormIdleAnim(NULL),
	FirestormAirAnim(NULL),
	FirestormGroundAnim(NULL),
	MoveFlash(NULL),
	BombParachute(NULL),
	Parachute(NULL),
	SplashList(),
	SmallFire(NULL),
	LargeFire(NULL),
	Paratrooper(NULL),
	Disguise(NULL),
	Technician(NULL),
	Engineer(NULL),
	Pilot(NULL),
	Crew(NULL),
	FlameDamage(NULL),
	FlameDamage2(NULL),
	NukeWarhead(NULL),
	NukeProjectile(NULL),
	NukeDown(NULL),
	EMPulseWarhead(NULL),
	EMPulseProjectile(NULL),
	C4Warhead(NULL),
	IonCannonWarhead(NULL),
	FirestormWarhead(NULL),
	VeinholeWarhead(NULL),
	IonStormWarhead(NULL),
	VeinholeTypeClass(NULL),
	DefaultLargeGreySmokeSystem(NULL),
	DefaultSmallGreySmokeSystem(NULL),
	DefaultSparkSystem(NULL),
	DefaultLargeRedSmokeSystem(NULL),
	DefaultSmallRedSmokeSystem(NULL),
	DefaultDebrisSmokeSystem(NULL),
	DefaultFireStreamSystem(NULL),
	DefaultFirestormExplosionSystem(NULL),
	DefaultTestParticleSystem(NULL),
	DefaultRepairParticleSystem(NULL),
	MyEffectivenessCoefficientDefault(0),
	TargetEffectivenessCoefficientDefault(0),
	TargetSpecialThreatCoefficientDefault(0),
	TargetStrengthCoefficientDefault(0),
	TargetDistanceCoefficientDefault(0),
	DumbMyEffectivenessCoefficient(0),
	DumbTargetEffectivenessCoefficient(0),
	DumbTargetSpecialThreatCoefficient(0),
	DumbTargetStrengthCoefficient(0),
	DumbTargetDistanceCoefficient(0),
	EnemyHouseThreatBonus(0),
	HunterSeekerDetonateProximity(0),
	HunterSeekerDescendProximity(0),
	HunterSeekerDescentSpeed(0),
	HunterSeekerAscentSpeed(0),
	HunterSeekerEmergeSpeed(0)
{
	/// nothing
}


/// <summary>
/// Destructor for the rules class object.
/// The rules own nothing that needs releasing -- the type objects they point at belong to
/// their own heaps and outlive the rules.
/// </summary>
RulesClass::~RulesClass(void)
{

}


/// <summary>
/// Builds the game's rule data from scratch.
/// This routine wipes every object type heap clean, reloads the art database, and then
/// processes the rule file supplied. The Firestorm and language specific rule files are
/// layered over the top afterwards, so that each may override what came before it.
/// </summary>
/// <remarks>Every type object in the game is destroyed here, so nothing may be holding a
/// pointer to one when this routine is called.</remarks>
void RulesClass::Initialize(CCINIClass const & ini)
{
	while (ColorSchemes.Count()) {
		delete ColorSchemes[0];
	}
	while (TeamTypes.Count()) {
		delete TeamTypes[0];
	}
	while (TaskForces.Count()) {
		delete TaskForces[0];
	}
	while (ScriptTypes.Count()) {
		delete ScriptTypes[0];
	}
	while (OverlayTypes.Count()) {
		delete OverlayTypes[0];
	}
	while (SmudgeTypes.Count()) {
		delete SmudgeTypes[0];
	}
	while (TerrainTypes.Count()) {
		delete TerrainTypes[0];
	}
	while (InfantryTypes.Count()) {
		delete InfantryTypes[0];
	}
	while (UnitTypes.Count()) {
		delete UnitTypes[0];
	}
	while (AircraftTypes.Count()) {
		delete AircraftTypes[0];
	}
	while (BulletTypes.Count()) {
		delete BulletTypes[0];
	}
	while (BuildingTypes.Count()) {
		delete BuildingTypes[0];
	}
	while (SuperWeaponTypes.Count()) {
		delete SuperWeaponTypes[0];
	}
	while (AnimTypes.Count()) {
		delete AnimTypes[0];
	}
	while (Weapons.Count()) {
		delete Weapons[0];
	}
	while (::Warheads.Count()) {
		delete ::Warheads[0];
	}
	while (VoxelAnimTypes.Count()) {
		delete VoxelAnimTypes[0];
	}
	while (ParticleTypes.Count()) {
		delete ParticleTypes[0];
	}
	while (ParticleSystemTypes.Count()) {
		delete ParticleSystemTypes[0];
	}
	while (::Houses.Count()) {
		delete ::Houses[0];
	}
	while (HouseTypes.Count()) {
		delete HouseTypes[0];
	}

	WebbedInfantry = NULL;
	PrerequisiteGDIFactory.Clear();
	PrerequisiteNodFactory.Clear();

	Load_Art_INI();

	if (Addon_Enabled(ADDON_FIRESTORM) == true) {
		CCFileClass artfsfile("ARTFS.INI");
		if (artfsfile.Is_Available() == true) {
			ArtINI.Load(artfsfile, false);
		}
	}

	Heap_Maximums(ini);
	Addition(ini);

	CCFileClass langfile("LANGRULE.INI");
	if (langfile.Is_Available() == true) {
		CCINIClass langini;
		if (langini.Load(langfile, true) > 1) {
			return;
		}
		DebugString("Processing LangRule.ini\n");
		Addition(langini);
	}

	if (Addon_Enabled(ADDON_FIRESTORM) == true) {
		Addition(FSRuleINI);
	}

	CCFileClass langfsfile("LANGFS.INI");
	if (langfsfile.Is_Available() == true) {
		CCINIClass langfsini;
		langfsini.Load(langfsfile, false);
		Addition(langfsini);
	}
}


/***********************************************************************************************
 * RulesClass::Process -- Fetch the bulk of the rule data from the control file.               *
 *                                                                                             *
 *    This routine will fetch the rule data from the control file.                             *
 *                                                                                             *
 * INPUT:   file  -- Reference to the rule file to process.                                    *
 *                                                                                             *
 * OUTPUT:  bool; Was the rule file processed?                                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/17/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool RulesClass::Addition(CCINIClass const & ini)
{
	BStart(BENCH_RULES);

	Color_Schemes(ini);
	Do_HouseTypes(ini);
	Do_Sides(ini);
	Do_OverlayTypes(ini);
	Do_SuperWeaponTypes(ini);
	Do_WarheadTypes(ini);
	Do_SmudgeTypes(ini);
	Do_TerrainTypes(ini);
	Do_BuildingTypes(ini);
	Do_VehicleTypes(ini);
	Do_AircraftTypes(ini);
	Do_InfantryTypes(ini);
	Do_AnimTypes(ini);
	Do_VoxelAnimTypes(ini);
	Do_ParticleTypes(ini);
	Do_ParticleSystemTypes(ini);

	Jumpjet_Controls(ini);
	MPlayer(ini);
	AI(ini);
	Powerups(ini);
	Land_Types(ini);
	IQ(ini);

	General(ini);
	Objects(ini);
	Difficulty(ini);
	Crate_Rules(ini);
	Combat_Damage(ini);
	Audio_Visual_Rules(ini);
	Special_Weapons(ini);
	bool result = TiberiumClass::Process(ini);

	BEnd(BENCH_RULES);

	return(result);
}


/// <summary>
/// Fetches the special weapon control values.
/// These are the warheads, projectiles and animations the nuclear missile and the EM pulse
/// are built from. Every warhead in the game is given a chance to reread itself here as
/// well, since the special weapons can redefine what the ordinary ones do.
/// </summary>
/// <returns>bool; Was a special weapons section found in the control file?</returns>
bool RulesClass::Special_Weapons(CCINIClass const & ini)
{
	static char const * const SPECIALWEAPONS = "SpecialWeapons";
	if (ini.Is_Present(SPECIALWEAPONS)) {
		HSBuilding = TGet_TypeList<BuildingTypeClass>(ini, SPECIALWEAPONS, "HSBuilding", HSBuilding);
		NukeWarhead = TGet_Class(ini, SPECIALWEAPONS, "NukeWarhead", NukeWarhead);
		NukeProjectile = TGet_Class(ini, SPECIALWEAPONS, "NukeProjectile", NukeProjectile);
		NukeDown = TGet_Class(ini, SPECIALWEAPONS, "NukeDown", NukeDown);
		EMPulseWarhead = TGet_Class(ini, SPECIALWEAPONS, "EMPulseWarhead", EMPulseWarhead);
		EMPulseProjectile = TGet_Class(ini, SPECIALWEAPONS, "EMPulseProjectile", EMPulseProjectile);
		for (int i = 0; i < ::Warheads.Count(); i++) {
			::Warheads[i]->Read_INI(ini);
		}
		return(true);
	}
	return(false);
}


/// <summary>
/// Fetches the audio and visual control values.
/// This is the catch-all section for presentation -- the sounds the interface plays, the
/// animations attached to smoke, fire, parachutes and drop pods, the shroud and fog rates,
/// and the extra lighting the various object kinds are drawn with.
/// </summary>
/// <returns>bool; Was an audio visual section found in the control file?</returns>
bool RulesClass::Audio_Visual_Rules(CCINIClass const & ini)
{
	static char const * const AUDIOVISUAL = "AudioVisual";
	if (ini.Is_Present(AUDIOVISUAL)) {
		UnloadingHarvester = TGet_Class(ini, AUDIOVISUAL, "UnloadingHarvester", UnloadingHarvester);
		PoseDir = (Dir256)ini.Get_Int(AUDIOVISUAL, "PoseDir", PoseDir);
		DropPodPuff = TGet_Class(ini, AUDIOVISUAL, "DropPodPuff", DropPodPuff);
		WaypointAnimationSpeed = ini.Get_Int(AUDIOVISUAL, "WaypointAnimationSpeed", WaypointAnimationSpeed);
		BarrelExplode = TGet_Class(ini, AUDIOVISUAL, "BarrelExplode", BarrelExplode);
		BarrelDebris = TGet_TypeList<VoxelAnimTypeClass>(ini, AUDIOVISUAL, "BarrelDebris", BarrelDebris);
		BarrelParticle = TGet_Class(ini, AUDIOVISUAL, "BarrelParticle", BarrelParticle);
		Wake = TGet_Class(ini, AUDIOVISUAL, "Wake", Wake);
		FlamingInfantry = TGet_Class(ini, AUDIOVISUAL, "FlamingInfantry", FlamingInfantry);
		VeinAttack = TGet_Class(ini, AUDIOVISUAL, "VeinAttack", VeinAttack);
		DropPod = TGet_TypeList<AnimTypeClass>(ini, AUDIOVISUAL, "DropPod", DropPod);
		DigSound = ini.Get_VocType(AUDIOVISUAL, "DigSound", DigSound);
		Dig = TGet_Class(ini, AUDIOVISUAL, "Dig", Dig);
		IonBlast = TGet_Class(ini, AUDIOVISUAL, "IonBlast", IonBlast);
		IonBeam = TGet_Class(ini, AUDIOVISUAL, "IonBeam", IonBeam);
		InfantryExplode = TGet_Class(ini, AUDIOVISUAL, "InfantryExplode", InfantryExplode);
		AtmosphereEntry = TGet_Class(ini, AUDIOVISUAL, "AtmosphereEntry", AtmosphereEntry);
		GateUpSound = ini.Get_VocType(AUDIOVISUAL, "GateUp", GateUpSound);
		GateDownSound = ini.Get_VocType(AUDIOVISUAL, "GateDown", GateDownSound);
		IsShroudGrow = ini.Get_Bool(AUDIOVISUAL, "ShroudGrow", IsShroudGrow);
		ScrollMultiplier = ini.Get_Float(AUDIOVISUAL, "ScrollMultiplier", ScrollMultiplier);
		ShakeScreen = ini.Get_Int(AUDIOVISUAL, "ShakeScreen", ShakeScreen);
		CloakSound = ini.Get_VocType(AUDIOVISUAL, "CloakSound", CloakSound);
		SellSound = ini.Get_VocType(AUDIOVISUAL, "SellSound", SellSound);
		GameClosed = ini.Get_VocType(AUDIOVISUAL, "GameClosed", GameClosed);
		IncomingMessage = ini.Get_VocType(AUDIOVISUAL, "IncomingMessage", IncomingMessage);
		SystemError = ini.Get_VocType(AUDIOVISUAL, "SystemError", SystemError);
		OptionsChanged = ini.Get_VocType(AUDIOVISUAL, "OptionsChanged", OptionsChanged);
		GameForming = ini.Get_VocType(AUDIOVISUAL, "GameForming", GameForming);
		PlayerLeft = ini.Get_VocType(AUDIOVISUAL, "PlayerLeft", PlayerLeft);
		PlayerJoined = ini.Get_VocType(AUDIOVISUAL, "PlayerJoined", PlayerJoined);
		Construction = ini.Get_VocType(AUDIOVISUAL, "Construction", Construction);
		CreditTicks = ini.Get_VocType_List(ini, AUDIOVISUAL, "CreditTicks", CreditTicks);
		CrumbleSound = ini.Get_VocType(AUDIOVISUAL, "CrumbleSound", CrumbleSound);
		BuildingSlam = ini.Get_VocType(AUDIOVISUAL, "BuildingSlam", BuildingSlam);
		RadarOn = ini.Get_VocType(AUDIOVISUAL, "RadarOn", RadarOn);
		RadarOff = ini.Get_VocType(AUDIOVISUAL, "RadarOff", RadarOff);
		ScoldSound = ini.Get_VocType(AUDIOVISUAL, "ScoldSound", ScoldSound);
		TeslaCharge = ini.Get_VocType(AUDIOVISUAL, "TeslaCharge", TeslaCharge);
		TeslaZap = ini.Get_VocType(AUDIOVISUAL, "TeslaZap", TeslaZap);
		BlowupSound = ini.Get_VocType(AUDIOVISUAL, "BlowupSound", BlowupSound);
		ChuteSound = ini.Get_VocType(AUDIOVISUAL, "ChuteSound", ChuteSound);
		GenericClick = ini.Get_VocType(AUDIOVISUAL, "GenericClick", GenericClick);
		GenericBeep = ini.Get_VocType(AUDIOVISUAL, "GenericBeep", GenericBeep);
		BuildingDrop = ini.Get_VocType(AUDIOVISUAL, "BuildingDrop", BuildingDrop);
		StopSound = ini.Get_VocType(AUDIOVISUAL, "StopSound", StopSound);
		GuardSound = ini.Get_VocType(AUDIOVISUAL, "GuardSound", GuardSound);
		ScatterSound = ini.Get_VocType(AUDIOVISUAL, "ScatterSound", ScatterSound);
		DeploySound = ini.Get_VocType(AUDIOVISUAL, "DeploySound", DeploySound);
		LightningSound = ini.Get_VocType(AUDIOVISUAL, "LightningSound", LightningSound);
		TreeFire = TGet_TypeList<AnimTypeClass>(ini, AUDIOVISUAL, "TreeFire", TreeFire);
		DeadBodies = TGet_TypeList<AnimTypeClass>(ini, AUDIOVISUAL, "DeadBodies", DeadBodies);
		MetallicDebris = TGet_TypeList<AnimTypeClass>(ini, AUDIOVISUAL, "MetallicDebris", MetallicDebris);
		BridgeExplosions = TGet_TypeList<AnimTypeClass>(ini, AUDIOVISUAL, "BridgeExplosions", BridgeExplosions);
		OnFire = TGet_TypeList<AnimTypeClass>(ini, AUDIOVISUAL, "OnFire", OnFire);
		Smoke1 = TGet_Class(ini, AUDIOVISUAL, "Smoke", Smoke1);
		Smoke2 = TGet_Class(ini, AUDIOVISUAL, "Smoke", Smoke2);
		FirestormActiveAnim = TGet_Class(ini, AUDIOVISUAL, "FirestormActiveAnim", FirestormActiveAnim);
		FirestormIdleAnim = TGet_Class(ini, AUDIOVISUAL, "FirestormIdleAnim", FirestormIdleAnim);
		FirestormAirAnim = TGet_Class(ini, AUDIOVISUAL, "FirestormAirAnim", FirestormAirAnim);
		FirestormGroundAnim = TGet_Class(ini, AUDIOVISUAL, "FirestormGroundAnim", FirestormGroundAnim);
		MoveFlash = TGet_Class(ini, AUDIOVISUAL, "MoveFlash", MoveFlash);
		Parachute = TGet_Class(ini, AUDIOVISUAL, "Parachute", Parachute);
		BombParachute = TGet_Class(ini, AUDIOVISUAL, "BombParachute", BombParachute);
		SmallFire = TGet_Class(ini, AUDIOVISUAL, "SmallFire", SmallFire);
		LargeFire = TGet_Class(ini, AUDIOVISUAL, "LargeFire", LargeFire);
		IsAllyReveal = ini.Get_Bool(AUDIOVISUAL, "AllyReveal", IsAllyReveal);
		ConditionGreen = 1;
		ConditionRed = ini.Get_Float(AUDIOVISUAL, "ConditionRed", ConditionRed);
		ConditionYellow = ini.Get_Float(AUDIOVISUAL, "ConditionYellow", ConditionYellow);
		DropZoneRadius = ini.Get_Lepton(AUDIOVISUAL, "DropZoneRadius", DropZoneRadius);
		FlareAnim = TGet_Class(ini, AUDIOVISUAL, "DropZoneAnim", FlareAnim);
		IsHealthBar = ini.Get_Bool(AUDIOVISUAL, "EnemyHealth", IsHealthBar);
		Gravity = ini.Get_Int(AUDIOVISUAL, "Gravity", Gravity);
		RandomAnimateTime = ini.Get_Float(AUDIOVISUAL, "IdleActionFrequency", RandomAnimateTime);
		MessageDelay = ini.Get_Float(AUDIOVISUAL, "MessageDelay", MessageDelay);
		MovieTime = ini.Get_Float(AUDIOVISUAL, "MovieTime", MovieTime);
		IsNamed = ini.Get_Bool(AUDIOVISUAL, "NamedCivilians", IsNamed);

		SavourDelay = ini.Get_Float(AUDIOVISUAL, "SavourDelay", SavourDelay);
		ShroudRate = ini.Get_Float(AUDIOVISUAL, "ShroudRate", ShroudRate);
		FogRate = ini.Get_Float(AUDIOVISUAL, "FogRate", FogRate);
		VeinGrowthRate = ini.Get_Float(AUDIOVISUAL, "VeinGrowthRate", VeinGrowthRate);
		IceGrowthRate = ini.Get_Float(AUDIOVISUAL, "IceGrowthRate", IceGrowthRate);
		IceSolidifyDelay = ini.Get_Int(AUDIOVISUAL, "IceSolidifyFrameTime", IceSolidifyDelay);

		IceCrackSounds.Clear();
		IceCrackSounds = ini.Get_VocType_List(ini, AUDIOVISUAL, "IceCrackSounds", IceCrackSounds);

		AmbientLightChangeRate = ini.Get_Float(AUDIOVISUAL, "AmbientChangeRate", AmbientLightChangeRate);
		AmbientLightChangeStep = ini.Get_Float(AUDIOVISUAL, "AmbientChangeStep", AmbientLightChangeStep);
		SpeakDelay = ini.Get_Float(AUDIOVISUAL, "SpeakDelay", SpeakDelay);
		TimerWarning = ini.Get_Float(AUDIOVISUAL, "TimerWarning", TimerWarning);

		ExtraUnitLight = (int)(NORMAL_LIGHT * ini.Get_Float(AUDIOVISUAL, "ExtraUnitLight", ExtraUnitLight / NORMAL_LIGHT));
		ExtraInfantryLight = (int)(NORMAL_LIGHT * ini.Get_Float(AUDIOVISUAL, "ExtraInfantryLight", ExtraInfantryLight / NORMAL_LIGHT));
		ExtraAircraftLight = (int)(NORMAL_LIGHT * ini.Get_Float(AUDIOVISUAL, "ExtraAircraftLight", ExtraAircraftLight / NORMAL_LIGHT) );

		EMPulseSparkles = TGet_Class(ini, AUDIOVISUAL, "EMPulseSparkles", EMPulseSparkles);
		WebbedInfantry = TGet_Class(ini, AUDIOVISUAL, "WebbedInfantry", WebbedInfantry);
		return(true);
	}
	return(false);
}


/// <summary>
/// Fetches the crate control values.
/// These are the values the crate system works from -- how many crates the map is to carry,
/// how far from the crate a unit may be and still claim it, how long before a claimed crate
/// is replaced, and which bonus the wood and silver crates hand out.
/// </summary>
/// <returns>bool; Was a crate rules section found in the control file?</returns>
bool RulesClass::Crate_Rules(CCINIClass const & ini)
{
	static char const * const CRATERULES = "CrateRules";
	if (ini.Is_Present(CRATERULES)) {
		IsFreeMCV = ini.Get_Bool(CRATERULES, "FreeMCV", IsFreeMCV);
		WoodCrateImg = TGet_Class(ini, CRATERULES, "WoodCrateImg", WoodCrateImg);
		CrateImg = TGet_Class(ini, CRATERULES, "CrateImg", CrateImg);
		HealCrateSound = ini.Get_VocType(CRATERULES, "HealCrateSound", HealCrateSound);
		CrateMinimum = ini.Get_Int(CRATERULES, "CrateMinimum", CrateMinimum);
		CrateMaximum = ini.Get_Int(CRATERULES, "CrateMaximum", CrateMaximum);
		CrateRadius = ini.Get_Lepton(CRATERULES, "CrateRadius", CrateRadius);
		CrateTime = ini.Get_Float(CRATERULES, "CrateRegen", CrateTime);
		UnitCrateType = TGet_Class(ini, CRATERULES, "UnitCrateType", UnitCrateType);
		SoloCrateMoney = ini.Get_Int(CRATERULES, "SoloCrateMoney", SoloCrateMoney);
		SilverCrate = ini.Get_CrateType(CRATERULES, "SilverCrate", SilverCrate);
		WoodCrate = ini.Get_CrateType(CRATERULES, "WoodCrate", WoodCrate);
		//WaterCrate = ini.Get_CrateType(CRATERULES, "WaterCrate", WaterCrate);
		return(true);
	}
	return(false);
}


/// <summary>
/// Fetches the combat and damage control values.
/// This section holds the constants the combat system works from -- the damage figures for
/// the fixed weapons, the scatter of ballistic and homing projectiles, the crushing and
/// suppression distances, and the scorches, craters and smoke systems left behind.
/// </summary>
/// <returns>bool; Was a combat damage section found in the control file?</returns>
bool RulesClass::Combat_Damage(CCINIClass const & ini)
{
	static char const * const COMBATDAMAGE = "CombatDamage";
	if (ini.Is_Present(COMBATDAMAGE)) {
		AmmoCrateDamage = ini.Get_Int(COMBATDAMAGE, "AmmoCrateDamage", AmmoCrateDamage);
		IonCannonDamage = ini.Get_Int(COMBATDAMAGE, "IonCannonDamage", IonCannonDamage);
		RailgunDamageRadius = ini.Get_Int(COMBATDAMAGE, "RailgunDamageRadius", RailgunDamageRadius);
		TiberiumExplosionDamage = ini.Get_Int(COMBATDAMAGE, "TiberiumExplosionDamage", TiberiumExplosionDamage);
		TiberiumStrength = ini.Get_Int(COMBATDAMAGE, "TiberiumStrength", TiberiumStrength);
		Scorches = TGet_TypeList<SmudgeTypeClass>(ini, COMBATDAMAGE, "Scorches", Scorches);
		Scorches1 = TGet_TypeList<SmudgeTypeClass>(ini, COMBATDAMAGE, "Scorches1", Scorches1);
		Scorches2 = TGet_TypeList<SmudgeTypeClass>(ini, COMBATDAMAGE, "Scorches2", Scorches2);
		Scorches3 = TGet_TypeList<SmudgeTypeClass>(ini, COMBATDAMAGE, "Scorches3", Scorches3);
		Scorches4 = TGet_TypeList<SmudgeTypeClass>(ini, COMBATDAMAGE, "Scorches4", Scorches4);
		Craters = TGet_TypeList<SmudgeTypeClass>(ini, COMBATDAMAGE, "Craters", Craters);
		SplashList = TGet_TypeList<AnimTypeClass>(ini, COMBATDAMAGE, "SplashList", SplashList);
		FlameDamage = TGet_Class(ini, COMBATDAMAGE, "FlameDamage", FlameDamage);
		FlameDamage2 = TGet_Class(ini, COMBATDAMAGE, "FlameDamage2", FlameDamage2);
		C4Warhead = TGet_Class(ini, COMBATDAMAGE, "C4Warhead", C4Warhead);
		IonCannonWarhead = TGet_Class(ini, COMBATDAMAGE, "IonCannonWarhead", IonCannonWarhead);
		FirestormWarhead = TGet_Class(ini, COMBATDAMAGE, "FirestormWarhead", FirestormWarhead);
		VeinholeWarhead = TGet_Class(ini, COMBATDAMAGE, "VeinholeWarhead", VeinholeWarhead);
		DefaultFirestormExplosionSystem = TGet_Class(ini, COMBATDAMAGE, "DefaultFirestormExplosionSystem", DefaultFirestormExplosionSystem);
		DefaultLargeGreySmokeSystem = TGet_Class(ini, COMBATDAMAGE, "DefaultLargeGreySmokeSystem", DefaultLargeGreySmokeSystem);
		DefaultSmallGreySmokeSystem = TGet_Class(ini, COMBATDAMAGE, "DefaultSmallGreySmokeSystem", DefaultSmallGreySmokeSystem);
		DefaultSparkSystem = TGet_Class(ini, COMBATDAMAGE, "DefaultSparkSystem", DefaultSparkSystem);
		DefaultLargeRedSmokeSystem = TGet_Class(ini, COMBATDAMAGE, "DefaultLargeRedSmokeSystem", DefaultLargeRedSmokeSystem);
		DefaultSmallRedSmokeSystem = TGet_Class(ini, COMBATDAMAGE, "DefaultSmallRedSmokeSystem", DefaultSmallRedSmokeSystem);
		DefaultDebrisSmokeSystem = TGet_Class(ini, COMBATDAMAGE, "DefaultDebrisSmokeSystem", DefaultDebrisSmokeSystem);
		DefaultFireStreamSystem = TGet_Class(ini, COMBATDAMAGE, "DefaultFireStreamSystem", DefaultFireStreamSystem);
		DefaultTestParticleSystem = TGet_Class(ini, COMBATDAMAGE, "DefaultTestParticleSystem", DefaultTestParticleSystem);
		DefaultRepairParticleSystem = TGet_Class(ini, COMBATDAMAGE, "DefaultRepairParticleSystem", DefaultRepairParticleSystem);
		IsBerzerkAllowed = ini.Get_Bool(COMBATDAMAGE, "BerzerkAllowed", IsBerzerkAllowed);
		TurboBoost = ini.Get_Float(COMBATDAMAGE, "TurboBoost", TurboBoost);
		AtomDamage = ini.Get_Int(COMBATDAMAGE, "AtomDamage", AtomDamage);
		BallisticScatter = ini.Get_Lepton(COMBATDAMAGE, "BallisticScatter", BallisticScatter);
		BridgeStrength = ini.Get_Int(COMBATDAMAGE, "BridgeStrength", BridgeStrength);
		C4Delay = ini.Get_Float(COMBATDAMAGE, "C4Delay", C4Delay);
		CrushDistance = ini.Get_Lepton(COMBATDAMAGE, "Crush", CrushDistance);
		ExplosionSpread = ini.Get_Float(COMBATDAMAGE, "ExpSpread", ExplosionSpread);
		SupressRadius = ini.Get_Lepton(COMBATDAMAGE, "FireSupress", SupressRadius);
		HomingScatter = ini.Get_Lepton(COMBATDAMAGE, "HomingScatter", HomingScatter);
		MaxDamage = ini.Get_Int(COMBATDAMAGE, "MaxDamage", MaxDamage);
		MinDamage = ini.Get_Int(COMBATDAMAGE, "MinDamage", MinDamage);
		IsExplosiveHarvester = ini.Get_Bool(COMBATDAMAGE, "TiberiumExplosive", IsExplosiveHarvester);
		IsAutoCrush = ini.Get_Bool(COMBATDAMAGE, "PlayerAutoCrush", IsAutoCrush);
		IsSmartDefense = ini.Get_Bool(COMBATDAMAGE, "PlayerReturnFire", IsSmartDefense);
		IsScatter = ini.Get_Bool(COMBATDAMAGE, "PlayerScatter", IsScatter);
		IsTreeTarget = ini.Get_Bool(COMBATDAMAGE, "TreeTargeting", IsTreeTarget);
		Incoming = ini.Get_MPHType(COMBATDAMAGE, "Incoming", Incoming);
		CollapseChance = ini.Get_Int(COMBATDAMAGE, "CollapseChance", CollapseChance);
		return(true);
	}
	return(false);
}


/// <summary>
/// Creates the color schemes declared in the control file.
/// Each entry names a scheme and gives the hue, saturation and value that the game palette
/// is to be remapped through. Both a flat scheme and a full intensity ramp are built for
/// every entry, since the drawing code asks for whichever suits the object being drawn.
/// </summary>
/// <returns>bool; Were any color schemes declared?</returns>
bool RulesClass::Color_Schemes(CCINIClass const & ini)
{
	static char const * const COLORS = "Colors";
	if (ini.Is_Present(COLORS)) {
		int count = ini.Entry_Count(COLORS);
		for (int i = 0; i < count; i++) {
			HSVClass hsv = ini.Get_HSVClass(COLORS, ini.Get_Entry(COLORS, i), HSVClass());
			ColorScheme::Find_Or_Make(ini.Get_Entry(COLORS, i), hsv, SchemePalette, GamePalette, 1);
			ColorScheme::Find_Or_Make(ini.Get_Entry(COLORS, i), hsv, SchemePalette, GamePalette, NUM_INTENSITY_LEVELS);
		}
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * RulesClass::General -- Process the general main game rules.                                 *
 *                                                                                             *
 *    This fetches the control constants uses for regular game processing. Any game behavior   *
 *    controlling values that don't properly fit in any of the other catagories will be        *
 *    stored here.                                                                             *
 *                                                                                             *
 * INPUT:   ini   -- Reference to the database to fetch the values from.                       *
 *                                                                                             *
 * OUTPUT:  bool; Was the general section found and processed?                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/08/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool RulesClass::General(CCINIClass const & ini)
{
	static char const * const GENERAL = "General";

	if (ini.Is_Present(GENERAL)) {
		LargeVisceroid = TGet_Class(ini, GENERAL, "LargeVisceroid", LargeVisceroid);
		SmallVisceroid = TGet_Class(ini, GENERAL, "SmallVisceroid", SmallVisceroid);
		TiberiumHeal = ini.Get_Float(GENERAL, "TiberiumHeal", TiberiumHeal);
		PrerequisitePower = ini.Get_BuildingType_List(ini, GENERAL, "PrerequisitePower", PrerequisitePower);
		PrerequisiteFactory = ini.Get_BuildingType_List(ini, GENERAL, "PrerequisiteFactory", PrerequisiteFactory);
		PrerequisiteBarracks = ini.Get_BuildingType_List(ini, GENERAL, "PrerequisiteBarracks", PrerequisiteBarracks);
		PrerequisiteRadar = ini.Get_BuildingType_List(ini, GENERAL, "PrerequisiteRadar", PrerequisiteRadar);
		PrerequisiteTech = ini.Get_BuildingType_List(ini, GENERAL, "PrerequisiteTech", PrerequisiteTech);
		PrerequisiteGDIFactory = ini.Get_BuildingType_List(ini, GENERAL, "PrerequisiteGDIFactory", PrerequisiteGDIFactory);
		PrerequisiteNodFactory = ini.Get_BuildingType_List(ini, GENERAL, "PrerequisiteNodFactory", PrerequisiteNodFactory);
		ZoomInFactor = ini.Get_Float(GENERAL, "ZoomInFactor", ZoomInFactor);
		IsRevealByHeight = ini.Get_Bool(GENERAL, "RevealByHeight", IsRevealByHeight);
		IsShroudedSubteranneanMovesAllowed = ini.Get_Bool(GENERAL, "AllowShroudedSubteranneanMoves", IsShroudedSubteranneanMovesAllowed);
		AircraftFogReveal = ini.Get_Int(GENERAL, "AircraftFogReveal", AircraftFogReveal);
		MinLowPowerProductionSpeed = ini.Get_Float(GENERAL, "MinProductionSpeed", MinLowPowerProductionSpeed);
		MultipleFactory = ini.Get_Float(GENERAL, "MultipleFactory", MultipleFactory);
		CraterLevel = ini.Get_Int(GENERAL, "CraterLevel", CraterLevel);
		TreeFlammability = ini.Get_Float(GENERAL, "TreeFlammability", TreeFlammability);
		MissileROTVar = ini.Get_Float(GENERAL, "MissileROTVar", MissileROTVar);
		MissileSpeedVar = ini.Get_Float(GENERAL, "MissileSpeedVar", MissileSpeedVar);
		DropPodWeapon = TGet_Class(ini, GENERAL, "DropPodWeapon", DropPodWeapon);
		DropPodHeight = ini.Get_Int(GENERAL, "DropPodHeight", DropPodHeight);
		DropPodSpeed = ini.Get_Int(GENERAL, "DropPodSpeed", DropPodSpeed);
		DropPodAngle = ini.Get_Float(GENERAL, "DropPodAngle", DropPodAngle);
		DropPodAngle = std::min(DropPodAngle, DEG_TO_RAD(67.5));
		DropPodAngle = std::max(DropPodAngle, DEG_TO_RAD(22.5));
		CrewEscape = ini.Get_Float(GENERAL, "CrewEscape", CrewEscape);
		TunnelSpeed = ini.Get_Float(GENERAL, "TunnelSpeed", TunnelSpeed);
		HoverDampen = ini.Get_Float(GENERAL, "HoverDampen", HoverDampen);
		HoverBob = ini.Get_Float(GENERAL, "HoverBob", HoverBob);
		HoverHeight = ini.Get_Int(GENERAL, "HoverHeight", HoverHeight);
		HoverBoost = ini.Get_Float(GENERAL, "HoverBoost", HoverBoost);
		HoverAcceleration = ini.Get_Float(GENERAL, "HoverAcceleration", HoverAcceleration);
		HoverBrake = ini.Get_Float(GENERAL, "HoverBrake", HoverBrake);
		VeteranRatio = ini.Get_Float(GENERAL, "VeteranRatio", VeteranRatio);
		VeteranCombat = ini.Get_Float(GENERAL, "VeteranCombat", VeteranCombat);
		VeteranSpeed = ini.Get_Float(GENERAL, "VeteranSpeed", VeteranSpeed);
		VeteranSight = ini.Get_Float(GENERAL, "VeteranSight", VeteranSight);
		VeteranArmor = ini.Get_Float(GENERAL, "VeteranArmor", VeteranArmor);
		VeteranROF = ini.Get_Float(GENERAL, "VeteranROF", VeteranROF);
		VeteranCap = ini.Get_Float(GENERAL, "VeteranCap", VeteranCap);
		ExplosiveVoxelDebris = TGet_TypeList<VoxelAnimTypeClass>(ini, GENERAL, "ExplosiveVoxelDebris", ExplosiveVoxelDebris);
		BridgeVoxelMax = ini.Get_Int(GENERAL, "BridgeVoxelMax", BridgeVoxelMax);
		TireVoxelDebris = TGet_Class(ini, GENERAL, "TireVoxelDebris", TireVoxelDebris);
		ScrapVoxelDebris = TGet_Class(ini, GENERAL, "ScrapVoxelDebris", ScrapVoxelDebris);
		CloakingStages = ini.Get_Int(GENERAL, "CloakingStages", CloakingStages);
		IceCrackingWeight = ini.Get_Float(GENERAL, "IceCrackingWeight", IceCrackingWeight);
		IceBreakingWeight = ini.Get_Float(GENERAL, "IceBreakingWeight", IceBreakingWeight);
		CliffBackImpassability = ini.Get_Int(GENERAL, "CliffBackImpassability", CliffBackImpassability);
		PlacementDelay = ini.Get_Float(GENERAL, "PlacementDelay", PlacementDelay);
		TrackedUphill = ini.Get_Float(GENERAL, "TrackedUphill", TrackedUphill);
		TrackedDownhill = ini.Get_Float(GENERAL, "TrackedDownhill", TrackedDownhill);
		WheeledUphill = ini.Get_Float(GENERAL, "WheeledUphill", WheeledUphill);
		WheeledDownhill = ini.Get_Float(GENERAL, "WheeledDownhill", WheeledDownhill);
		WindDirection = (FacingType)ini.Get_Int(GENERAL, "WindDirection", WindDirection);
		CameraRange = ini.Get_Lepton(GENERAL, "CameraRange", CameraRange);
		FlightLevel = ini.Get_Int(GENERAL, "FlightLevel", FlightLevel);
		RepairBay = TGet_Class(ini, GENERAL, "RepairBay", RepairBay);
		GDIGateOne = TGet_Class(ini, GENERAL, "GDIGateOne", GDIGateOne);
		GDIGateTwo = TGet_Class(ini, GENERAL, "GDIGateTwo", GDIGateTwo);
		NodGateOne = TGet_Class(ini, GENERAL, "NodGateOne", NodGateOne);
		NodGateTwo = TGet_Class(ini, GENERAL, "NodGateTwo", NodGateTwo);
		WallTower = TGet_Class(ini, GENERAL, "WallTower", WallTower);
		GDIPowerPlant = TGet_Class(ini, GENERAL, "GDIPowerPlant", GDIPowerPlant);
		GDIPowerTurbine = TGet_Class(ini, GENERAL, "GDIPowerTurbine", GDIPowerTurbine);
		NodRegularPower = TGet_Class(ini, GENERAL, "NodRegularPower", NodRegularPower);
		NodAdvancedPower = TGet_Class(ini, GENERAL, "NodAdvancedPower", NodAdvancedPower);
		GDIFirestormGenerator = TGet_Class(ini, GENERAL, "GDIFirestormGenerator", GDIFirestormGenerator);
		GDIHunterSeeker = TGet_Class(ini, GENERAL, "GDIHunterSeeker", GDIHunterSeeker);
		NodHunterSeeker = TGet_Class(ini, GENERAL, "NodHunterSeeker", NodHunterSeeker);
		BaseUnit = TGet_Class(ini, GENERAL, "BaseUnit", BaseUnit);
		HarvesterUnit = TGet_TypeList<UnitTypeClass>(ini, GENERAL, "HarvesterUnit", HarvesterUnit);
		PadAircraft = TGet_TypeList<AircraftTypeClass>(ini, GENERAL, "PadAircraft", PadAircraft);
		Paratrooper = TGet_Class(ini, GENERAL, "Paratrooper", Paratrooper);
		Disguise = TGet_Class(ini, GENERAL, "Disguise", Disguise);
		Engineer = TGet_Class(ini, GENERAL, "Engineer", Engineer);
		Technician = TGet_Class(ini, GENERAL, "Technician", Technician);
		Pilot = TGet_Class(ini, GENERAL, "Pilot", Pilot);
		Crew = TGet_Class(ini, GENERAL, "Crew", Crew);
		IsCurleyShuffle = ini.Get_Bool(GENERAL, "CurleyShuffle", IsCurleyShuffle);
		IsFineDifficulty = ini.Get_Bool(GENERAL, "FineDiffControl", IsFineDifficulty);
		TeamDelays = ini.Get_IntList(GENERAL, "TeamDelays", TeamDelays);
		AIHateDelays = ini.Get_IntList(GENERAL, "AIHateDelays", AIHateDelays);
		AIAlternateProductionCreditCutoff = ini.Get_Int(GENERAL, "AIAlternateProductionCreditCutoff", AIAlternateProductionCreditCutoff);
		AIUseTurbineUpgradeChance = ini.Get_Float(GENERAL, "AIUseTurbineUpgradeProbability", AIUseTurbineUpgradeChance);
		NodAIBuildsWalls = ini.Get_Bool(GENERAL, "NodAIBuildsWalls", NodAIBuildsWalls);
		AIBuildsWalls = ini.Get_Bool(GENERAL, "AIBuildsWalls", AIBuildsWalls);
		FillEarliestTeamProbability = ini.Get_IntList(GENERAL, "FillEarliestTeamProbability", FillEarliestTeamProbability);
		MinimumAIDefensiveTeams = ini.Get_IntList(GENERAL, "MinimumAIDefensiveTeams", MinimumAIDefensiveTeams);
		MaximumAIDefensiveTeams = ini.Get_IntList(GENERAL, "MaximumAIDefensiveTeams", MaximumAIDefensiveTeams);
		TotalAITeamCap = ini.Get_IntList(GENERAL, "TotalAITeamCap", TotalAITeamCap);
		UseMinDefenseRule = ini.Get_Bool(GENERAL, "UseMinDefenseRule", UseMinDefenseRule);
		DissolveUnfilledTeamDelay = ini.Get_Int(GENERAL, "DissolveUnfilledTeamDelay", DissolveUnfilledTeamDelay);
		MultiplayerAICreditMultipliers = ini.Get_IntList(GENERAL, "MultiplayerAICM", MultiplayerAICreditMultipliers);
		AIIonCannonConYardValue = ini.Get_IntList(GENERAL, "AIIonCannonConYardValue", AIIonCannonConYardValue);
		AIIonCannonWarFactoryValue = ini.Get_IntList(GENERAL, "AIIonCannonWarFactoryValue", AIIonCannonWarFactoryValue);
		AIIonCannonPowerValue = ini.Get_IntList(GENERAL, "AIIonCannonPowerValue", AIIonCannonPowerValue);
		AIIonCannonEngineerValue = ini.Get_IntList(GENERAL, "AIIonCannonEngineerValue", AIIonCannonEngineerValue);
		AIIonCannonThiefValue = ini.Get_IntList(GENERAL, "AIIonCannonThiefValue", AIIonCannonThiefValue);
		AIIonCannonHarvesterValue = ini.Get_IntList(GENERAL, "AIIonCannonHarvesterValue", AIIonCannonHarvesterValue);
		AIIonCannonMCVValue = ini.Get_IntList(GENERAL, "AIIonCannonMCVValue", AIIonCannonMCVValue);
		AIIonCannonAPCValue = ini.Get_IntList(GENERAL, "AIIonCannonAPCValue", AIIonCannonAPCValue);
		AIIonCannonBaseDefenseValue = ini.Get_IntList(GENERAL, "AIIonCannonBaseDefenseValue", AIIonCannonBaseDefenseValue);
		AIIonCannonPlugValue = ini.Get_IntList(GENERAL, "AIIonCannonPlugValue", AIIonCannonPlugValue);
		AIIonCannonHelipadValue = ini.Get_IntList(GENERAL, "AIIonCannonHelipadValue", AIIonCannonHelipadValue);
		AIIonCannonTempleValue = ini.Get_IntList(GENERAL, "AIIonCannonTempleValue", AIIonCannonTempleValue);
		CloakDelay = ini.Get_Float(GENERAL, "CloakDelay", CloakDelay);
		GameSpeedBias = ini.Get_Float(GENERAL, "GameSpeedBias", GameSpeedBias);
		NervousBias = ini.Get_Float(GENERAL, "BaseBias", NervousBias);
		IsSeparate = ini.Get_Bool(GENERAL, "SeparateAircraft", IsSeparate);
		BaseDefenseDelay = ini.Get_Float(GENERAL, "BaseDefenseDelay", BaseDefenseDelay);
		SuspendPriority = ini.Get_Int(GENERAL, "SuspendPriority", SuspendPriority);
		SuspendDelay = ini.Get_Float(GENERAL, "SuspendDelay", SuspendDelay);
		SurvivorFraction = ini.Get_Float(GENERAL, "SurvivorRate", SurvivorFraction);
		SurvivorDivisor = ini.Get_Int(GENERAL, "SurvivorDivisor", SurvivorDivisor);
		ReloadRate = ini.Get_Float(GENERAL, "ReloadRate", ReloadRate);
		BuildupTime = ini.Get_Float(GENERAL, "BuildupTime", BuildupTime);
		HarvesterDumpRate = ini.Get_Float(GENERAL, "HarvesterDumpRate", HarvesterDumpRate);
		HarvesterLoadRate = ini.Get_Int(GENERAL, "HarvesterLoadRate", HarvesterLoadRate);
		BuildSpeedBias = ini.Get_Float(GENERAL, "BuildSpeed", BuildSpeedBias);
		DamageDelay = ini.Get_Float(GENERAL, "DamageDelay", DamageDelay);
		GrowthRate = ini.Get_Float(GENERAL, "GrowthRate", GrowthRate);
		RefundPercent = ini.Get_Float(GENERAL, "RefundPercent", RefundPercent);
		RepairPercent = ini.Get_Float(GENERAL, "RepairPercent", RepairPercent);
		RepairStep = ini.Get_Int(GENERAL, "RepairStep", RepairStep);
		IRepairStep = ini.Get_Int(GENERAL, "IRepairStep", IRepairStep);
		RepairRate = ini.Get_Float(GENERAL, "RepairRate", RepairRate);
		URepairRate = ini.Get_Float(GENERAL, "URepairRate", URepairRate);
		IRepairRate = ini.Get_Float(GENERAL, "IRepairRate", IRepairRate);
		StrayDistance = ini.Get_Lepton(GENERAL, "Stray", StrayDistance);
		CloseEnoughDistance = ini.Get_Lepton(GENERAL, "CloseEnough", CloseEnoughDistance);
		IsBlendedFog = ini.Get_Bool(GENERAL, "BlendedFog", IsBlendedFog);
		AttackingAircraftSightRange = ini.Get_Int(GENERAL, "AttackingAircraftSightRange", AttackingAircraftSightRange);
		LeptonsPerSightIncrease = ini.Get_Int(GENERAL, "LeptonsPerSightIncrease", LeptonsPerSightIncrease);
		TiberiumTransmogrify = ini.Get_Int(GENERAL, "TiberiumTransmogrify", TiberiumTransmogrify);
		LightningFrequency = int(ini.Get_Float(GENERAL, "IonLightningFrequency", LightningFrequency / 10) * 10);
		LightningRandomness = ini.Get_Int(GENERAL, "IonLightningRandomness", LightningRandomness);
		LightningDamage = ini.Get_Int(GENERAL, "IonLightningDamage", LightningDamage);
		LightningDuration = ini.Get_Int(GENERAL, "IonStormDuration", LightningDuration);
		LightningDeferment = ini.Get_Int(GENERAL, "IonStormWarning", LightningDeferment);
		IonStormWarhead = TGet_Class(ini, GENERAL, "IonStormWarhead", IonStormWarhead);
		SpotlightMovementRadius = ini.Get_Int(GENERAL, "SpotlightMovementRadius", SpotlightMovementRadius);
		SpotlightLocationRadius = ini.Get_Int(GENERAL, "SpotlightLocationRadius", SpotlightLocationRadius);
		SpotlightSpeed = ini.Get_Float(GENERAL, "SpotlightSpeed", SpotlightSpeed);
		SpotlightAcceleration = ini.Get_Float(GENERAL, "SpotlightAcceleration", SpotlightAcceleration);
		SpotlightAngle = ini.Get_Float(GENERAL, "SpotlightAngle", SpotlightAngle);
		SpotlightRadius = ini.Get_Int(GENERAL, "SpotlightRadius", SpotlightRadius);
		RevealTriggerRadius = ini.Get_Int(GENERAL, "RevealTriggerRadius", RevealTriggerRadius);
		ChargeToDrainRatio = ini.Get_Float(GENERAL, "ChargeToDrainRatio", ChargeToDrainRatio);
		DamageToFirestormDamageCoefficient = ini.Get_Float(GENERAL, "DamageToFirestormDamageCoefficient", DamageToFirestormDamageCoefficient);
		WallBuildSpeedCoefficient = ini.Get_Float(GENERAL, "WallBuildSpeedCoefficient", WallBuildSpeedCoefficient);
		WorstLowPowerBuildRateCoefficient = ini.Get_Float(GENERAL, "WorstLowPowerBuildRateCoefficient", WorstLowPowerBuildRateCoefficient);
		BestLowPowerBuildRateCoefficient = ini.Get_Float(GENERAL, "BestLowPowerBuildRateCoefficient", BestLowPowerBuildRateCoefficient);
		ConditionYellowSparkingProbability = ini.Get_Float(GENERAL, "ConditionYellowSparkingProbability", ConditionYellowSparkingProbability);
		ConditionRedSparkingProbability = ini.Get_Float(GENERAL, "ConditionRedSparkingProbability", ConditionRedSparkingProbability);
		AITriggerSuccessWeightDelta = ini.Get_Float(GENERAL, "AITriggerSuccessWeightDelta", AITriggerSuccessWeightDelta);
		AITriggerFailureWeightDelta = ini.Get_Float(GENERAL, "AITriggerFailureWeightDelta", AITriggerFailureWeightDelta);
		AITriggerTrackRecordCoefficient = ini.Get_Float(GENERAL, "AITriggerTrackRecordCoefficient", AITriggerTrackRecordCoefficient);
		WeedCapacity = ini.Get_Int(GENERAL, "WeedCapacity", WeedCapacity);
		FlashFrameTime = ini.Get_Int(GENERAL, "FlashFrameTime", FlashFrameTime);
		RadarCombatFlashTime = ini.Get_Int(GENERAL, "RadarCombatFlashTime", RadarCombatFlashTime);
		RadarEventSpeed = (float)ini.Get_Float(GENERAL, "RadarEventSpeed", RadarEventSpeed);
		RadarEventRotationSpeed = (float)ini.Get_Float(GENERAL, "RadarEventRotationSpeed", RadarEventRotationSpeed);
		RadarEventSuppressionDistances = ini.Get_IntList(GENERAL, "RadarEventSuppressionDistances", RadarEventSuppressionDistances);
		RadarEventVisibilityDurations = ini.Get_IntList(GENERAL, "RadarEventVisibilityDurations", RadarEventVisibilityDurations);
		RadarEventDurations = ini.Get_IntList(GENERAL, "RadarEventDurations", RadarEventDurations);
		RadarEventMinRadius = ini.Get_Int(GENERAL, "RadarEventMinRadius", RadarEventMinRadius);
		RadarEventColorSpeed = (float)ini.Get_Float(GENERAL, "RadarEventColorSpeed", RadarEventColorSpeed);
		HunterSeekerDetonateProximity = ini.Get_Int(GENERAL, "HunterSeekerDetonateProximity", HunterSeekerDetonateProximity);
		HunterSeekerDescendProximity = ini.Get_Int(GENERAL, "HunterSeekerDescendProximity", HunterSeekerDescendProximity);
		HunterSeekerDescentSpeed = ini.Get_Int(GENERAL, "HunterSeekerDescentSpeed", HunterSeekerDescentSpeed);
		HunterSeekerAscentSpeed = ini.Get_Int(GENERAL, "HunterSeekerAscentSpeed", HunterSeekerAscentSpeed);
		HunterSeekerEmergeSpeed = ini.Get_Int(GENERAL, "HunterSeekerEmergeSpeed", HunterSeekerEmergeSpeed);
		MyEffectivenessCoefficientDefault = ini.Get_Float(GENERAL, "MyEffectivenessCoefficientDefault", MyEffectivenessCoefficientDefault);
		TargetEffectivenessCoefficientDefault = ini.Get_Float(GENERAL, "TargetEffectivenessCoefficientDefault", TargetEffectivenessCoefficientDefault);
		TargetSpecialThreatCoefficientDefault = ini.Get_Float(GENERAL, "TargetSpecialThreatCoefficientDefault", TargetSpecialThreatCoefficientDefault);
		TargetStrengthCoefficientDefault = ini.Get_Float(GENERAL, "TargetStrengthCoefficientDefault", TargetStrengthCoefficientDefault);
		TargetDistanceCoefficientDefault = ini.Get_Float(GENERAL, "TargetDistanceCoefficientDefault", TargetDistanceCoefficientDefault);
		DumbMyEffectivenessCoefficient = ini.Get_Float(GENERAL, "DumbMyEffectivenessCoefficient", DumbMyEffectivenessCoefficient);
		DumbTargetEffectivenessCoefficient = ini.Get_Float(GENERAL, "DumbTargetEffectivenessCoefficient", DumbTargetEffectivenessCoefficient);
		DumbTargetSpecialThreatCoefficient = ini.Get_Float(GENERAL, "DumbTargetSpecialThreatCoefficient", DumbTargetSpecialThreatCoefficient);
		DumbTargetStrengthCoefficient = ini.Get_Float(GENERAL, "DumbTargetStrengthCoefficient", DumbTargetStrengthCoefficient);
		DumbTargetDistanceCoefficient = ini.Get_Float(GENERAL, "DumbTargetDistanceCoefficient", DumbTargetDistanceCoefficient);
		EnemyHouseThreatBonus = ini.Get_Float(GENERAL, "EnemyHouseThreatBonus", EnemyHouseThreatBonus);
		VeinholeMonsterStrength = ini.Get_Int(GENERAL, "VeinholeMonsterStrength", VeinholeMonsterStrength);
		MaxVeinholeGrowth = ini.Get_Int(GENERAL, "MaxVeinholeGrowth", MaxVeinholeGrowth);
		VeinholeGrowthRate = ini.Get_Int(GENERAL, "VeinholeGrowthRate", VeinholeGrowthRate);
		VeinholeShrinkRate = ini.Get_Int(GENERAL, "VeinholeShrinkRate", VeinholeShrinkRate);
		VeinDamage = ini.Get_Int(GENERAL, "VeinDamage", VeinDamage);
		VeinholeTypeClass = TGet_Class(ini, GENERAL, "VeinholeTypeClass", VeinholeTypeClass);
		MaximumQueuedObjects = ini.Get_Int(GENERAL, "MaximumQueuedObjects", MaximumQueuedObjects);
		MaxWaypointPathLength = ini.Get_Int(GENERAL, "MaxWaypointPathLength", MaxWaypointPathLength);
		TreeStrength = ini.Get_Int(GENERAL, "TreeStrength", TreeStrength);
		DropPodInfantryMinimum = ini.Get_Int(GENERAL, "DropPodInfantryMinimum", DropPodInfantryMinimum);
		DropPodInfantryMaximum = ini.Get_Int(GENERAL, "DropPodInfantryMaximum", DropPodInfantryMaximum);
		LevitateLocomotionClass::Read_INI(ini);
		EngineerCaptureLevel = (float)ini.Get_Float(GENERAL, "EngineerCaptureLevel", EngineerCaptureLevel);
		EngineerDamage = (float)ini.Get_Float(GENERAL, "EngineerDamage", EngineerDamage);
		TalkBubbleTime = int(TIMER_SECOND * ini.Get_Float(GENERAL, "TalkBubbleTime", TalkBubbleTime * (1.0f/TIMER_SECOND)));
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * RulesClass::MPlayer -- Fetch and process the multiplayer default settings.                  *
 *                                                                                             *
 *    This is used to set the default settings for the multiplayer system.                     *
 *                                                                                             *
 * INPUT:   ini   -- Reference to the INI database.                                            *
 *                                                                                             *
 * OUTPUT:  bool; Was the multiplayer default override section found and processed?            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/08/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool RulesClass::MPlayer(CCINIClass const & ini)
{
	static char const * const MPLAYER = "MultiplayerDefaults";
	if (ini.Is_Present(MPLAYER)) {
		MPMoney = ini.Get_Int(MPLAYER, "Money", MPMoney);
		MPMaxMoney = ini.Get_Int(MPLAYER, "MaxMoney", MPMaxMoney);
		MPUnitCount = ini.Get_Int(MPLAYER, "UnitCount", MPUnitCount);
		MPBuildLevel = ini.Get_Int(MPLAYER, "TechLevel", MPBuildLevel);
		IsMPBridgeDestruction = ini.Get_Bool(MPLAYER, "BridgeDestruction", IsMPBridgeDestruction);
		IsMPShadowGrow = ini.Get_Bool(MPLAYER, "ShadowGrow", IsMPShadowGrow);
		IsMPBasesOn = ini.Get_Bool(MPLAYER, "Bases", IsMPBasesOn);
		IsMPTiberiumGrow = ini.Get_Bool(MPLAYER, "TiberiumGrows", IsMPTiberiumGrow);
		IsMPCrates = ini.Get_Bool(MPLAYER, "Crates", IsMPCrates);
		IsMPCaptureTheFlag = ini.Get_Bool(MPLAYER, "CaptureTheFlag", IsMPCaptureTheFlag);
		IsMPBuildOffAllyAnyStructure = ini.Get_Bool(MPLAYER, "BuildOffAllyAnyStructure", IsMPBuildOffAllyAnyStructure);
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * RulesClass::Heap_Maximums -- Fetch and process the heap override values.                    *
 *                                                                                             *
 *    This fetches the maximum heap sizes from the database specified. The heaps will be       *
 *    initialized by this routine as indicated.                                                *
 *                                                                                             *
 * INPUT:   ini   -- Reference to the INI database.                                            *
 *                                                                                             *
 * OUTPUT:  bool; Was the maximum section found and processed?                                 *
 *                                                                                             *
 * WARNINGS:   This process is catastrophic to any data currently existing in the heaps        *
 *             modified. This should only be processed during the game initialization stage.   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/08/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool RulesClass::Heap_Maximums(CCINIClass const & ini)
{
	/*
	**	Heap maximum values.
	*/
	static char const * const MAXIMUMS = "Maximums";
	if (ini.Is_Present(MAXIMUMS)) {
		MaxPlayers = ini.Get_Int(MAXIMUMS, "Players", MaxPlayers);
	}

	return(true);
}


/// <summary>
/// Creates the infantry types declared in the control file.
/// Each entry of the infantry list names a soldier, which is created if the game has not
/// heard of it before. The soldier then reads its own section for its statistics.
/// </summary>
/// <returns>bool; Were any infantry declared?</returns>
bool RulesClass::Do_InfantryTypes(CCINIClass const & ini)
{
	static char const * const INFANTRYTYPES = "InfantryTypes";
	char buffer[32];
	int count = ini.Entry_Count(INFANTRYTYPES);
	for (int i = 0; i < count; i++) {
		if (ini.Get_String(INFANTRYTYPES, ini.Get_Entry(INFANTRYTYPES, i), "", buffer, sizeof(buffer))) {
			InfantryTypeClass::Find_Or_Make(buffer);
		}
	}
	return(count > 0);
}


/// <summary>
/// Creates the house types declared in the control file.
/// Each entry of the house list names a house, which is created if the game has not heard
/// of it before. The house then reads its own section for its personality and colors.
/// </summary>
/// <returns>bool; Were any houses declared?</returns>
bool RulesClass::Do_HouseTypes(CCINIClass const & ini)
{
	static char const * const HOUSES = "Houses";
	char buffer[32];
	int count = ini.Entry_Count(HOUSES);
	for (int i = 0; i < count; i++) {
		if (ini.Get_String(HOUSES, ini.Get_Entry(HOUSES, i), "", buffer, sizeof(buffer))) {
			HouseTypeClass::Find_Or_Make(buffer);
		}
	}
	return(count > 0);
}


/// <summary>
/// Creates the vehicle types declared in the control file.
/// Each entry of the vehicle list names a vehicle, which is created if the game has not
/// heard of it before. The vehicle then reads its own section for its statistics.
/// </summary>
/// <returns>bool; Were any vehicles declared?</returns>
bool RulesClass::Do_VehicleTypes(CCINIClass const & ini)
{
	static char const * const VEHICLETYPES = "VehicleTypes";
	char buffer[32];
	int count = ini.Entry_Count(VEHICLETYPES);
	for (int i = 0; i < count; i++) {
		if (ini.Get_String(VEHICLETYPES, ini.Get_Entry(VEHICLETYPES, i), "", buffer, sizeof(buffer))) {
			UnitTypeClass::Find_Or_Make(buffer);
		}
	}
	return(count > 0);
}


/// <summary>
/// Creates the aircraft types declared in the control file.
/// Each entry of the aircraft list names an aircraft, which is created if the game has not
/// heard of it before. The aircraft then reads its own section for its statistics.
/// </summary>
/// <returns>bool; Were any aircraft declared?</returns>
bool RulesClass::Do_AircraftTypes(CCINIClass const & ini)
{
	static char const * const AIRCRAFTTYPES = "AircraftTypes";
	char buffer[32];
	int count = ini.Entry_Count(AIRCRAFTTYPES);
	for (int i = 0; i < count; i++) {
		if (ini.Get_String(AIRCRAFTTYPES, ini.Get_Entry(AIRCRAFTTYPES, i), "", buffer, sizeof(buffer))) {
			AircraftTypeClass::Find_Or_Make(buffer);
		}
	}
	return(count > 0);
}


/// <summary>
/// Creates the sides declared in the control file and populates them.
/// A side is the umbrella a group of houses fights under -- GDI and Nod being the obvious
/// pair. Each entry names a side and lists the houses that belong to it, and every house
/// named is pointed back at its side so that either can be reached from the other.
/// </summary>
/// <returns>bool; Were any sides declared?</returns>
bool RulesClass::Do_Sides(CCINIClass const & ini)
{
	static char const * const SIDES = "Sides";
	int count = ini.Entry_Count(SIDES);
	DebugString("Processing sides.\n");
	for (int i = 0; i < count; i++) {
		char const * name = ini.Get_Entry(SIDES, i);
		int side = SideClass::From_Name(name);
		SideClass * sidep;
		if (side == SIDE_NONE) {
			sidep = new SideClass(name);
		} else {
			sidep = ::Sides[side];
		}
		DebugString("Side %d: %s \n", i, (char const *)sidep->IniName);
		sidep->Houses = ini.Get_House_List(SIDES, name, sidep->Houses);
		for (int house = 0; house < sidep->Houses.Count(); house++) {
			side = sidep->Houses[house];
			HouseTypes[side]->Side = (SideType)::Sides.ID(sidep);
			DebugString("  %s\n", (char const *)HouseTypes[side]->IniName);
		}
	}
	return(count > 0);
}


/// <summary>
/// Creates the super weapon types declared in the control file.
/// Each entry of the super weapon list names a super weapon, which is created if the game
/// has not heard of it before. The weapon then reads its own section for its behavior.
/// </summary>
/// <returns>bool; Were any super weapons declared?</returns>
bool RulesClass::Do_SuperWeaponTypes(CCINIClass const & ini)
{
	static char const * const SUPERWEAPONTYPES = "SuperWeaponTypes";
	char buffer[32];
	int count = ini.Entry_Count(SUPERWEAPONTYPES);
	for (int i = 0; i < count; i++) {
		if (ini.Get_String(SUPERWEAPONTYPES, ini.Get_Entry(SUPERWEAPONTYPES, i), "", buffer, sizeof(buffer))) {
			SuperWeaponTypeClass::Find_Or_Make(buffer);
		}
	}
	return(count > 0);
}


/// <summary>
/// Creates the building types declared in the control file.
/// Each entry of the building list names a building, which is created if the game has not
/// heard of it before. The building then reads its own section for its statistics.
/// </summary>
/// <returns>bool; Were any buildings declared?</returns>
bool RulesClass::Do_BuildingTypes(CCINIClass const & ini)
{
	static char const * const BUILDINGTYPES = "BuildingTypes";
	char buffer[32];
	int count = ini.Entry_Count(BUILDINGTYPES);
	for (int i = 0; i < count; i++) {
		if (ini.Get_String(BUILDINGTYPES, ini.Get_Entry(BUILDINGTYPES, i), "", buffer, sizeof(buffer))) {
			BuildingTypeClass::Find_Or_Make(buffer);
		}
	}
	return(count > 0);
}


/// <summary>
/// Creates the terrain types declared in the control file.
/// Each entry of the terrain list names a terrain object, which is created if the game has
/// not heard of it before. The object then reads its own section for its behavior.
/// </summary>
/// <returns>bool; Were any terrain objects declared?</returns>
bool RulesClass::Do_TerrainTypes(CCINIClass const & ini)
{
	static char const * const TERRAINTYPES = "TerrainTypes";
	char buffer[32];
	int count = ini.Entry_Count(TERRAINTYPES);
	for (int i = 0; i < count; i++) {
		if (ini.Get_String(TERRAINTYPES, ini.Get_Entry(TERRAINTYPES, i), "", buffer, sizeof(buffer))) {
			TerrainTypeClass::Find_Or_Make(buffer);
		}
	}
	return(count > 0);
}


/// <summary>
/// Creates the team types declared in the control file.
/// Unlike the other type readers, each team is filled in as soon as it is created, since a
/// team declaration carries its whole roster and script assignment with it.
/// </summary>
/// <returns>bool; Were any teams declared?</returns>
bool RulesClass::Do_TeamTypes(CCINIClass const & ini)
{
	static char const * const TEAMS = "Teams";
	int count = ini.Entry_Count(TEAMS);
	for (int i = 0; i < count; i++) {
		TeamTypeClass * team = TGet_Class<TeamTypeClass>(ini, TEAMS, ini.Get_Entry(TEAMS, i), NULL);
		team->Read_INI(ini);
	}
	return(count > 0);
}


/// <summary>
/// Creates the smudge types declared in the control file.
/// Each entry of the smudge list names a smudge, which is created if the game has not
/// heard of it before. The smudge then reads its own section for its behavior.
/// </summary>
/// <returns>bool; Were any smudges declared?</returns>
bool RulesClass::Do_SmudgeTypes(CCINIClass const & ini)
{
	static char const * const SMUDGETYPES = "SmudgeTypes";
	char buffer[32];
	int count = ini.Entry_Count(SMUDGETYPES);
	for (int i = 0; i < count; i++) {
		if (ini.Get_String(SMUDGETYPES, ini.Get_Entry(SMUDGETYPES, i), "", buffer, sizeof(buffer))) {
			SmudgeTypeClass::Find_Or_Make(buffer);
		}
	}
	return(count > 0);
}


/// <summary>
/// Creates the overlay types declared in the control file.
/// Each entry of the overlay list names an overlay, which is created if the game has not
/// heard of it before. The overlay then reads its own section for its behavior.
/// </summary>
/// <returns>bool; Were any overlays declared?</returns>
bool RulesClass::Do_OverlayTypes(CCINIClass const & ini)
{
	static char const * const OVERLAYTYPES = "OverlayTypes";
	char buffer[32];
	int count = ini.Entry_Count(OVERLAYTYPES);
	for (int i = 0; i < count; i++) {
		if (ini.Get_String(OVERLAYTYPES, ini.Get_Entry(OVERLAYTYPES, i), "", buffer, sizeof(buffer))) {
			OverlayTypeClass::Find_Or_Make(buffer);
		}
	}
	return(count > 0);
}


/// <summary>
/// Creates the animation types declared in the control file.
/// Each entry of the animation list names an animation, which is created if the game has
/// not heard of it before. The animation then reads its own section for its behavior.
/// </summary>
/// <returns>bool; Were any animations declared?</returns>
bool RulesClass::Do_AnimTypes(CCINIClass const & ini)
{
	char const * const ANIMATIONS = "Animations";
	char buffer[32];
	int count = ini.Entry_Count(ANIMATIONS);
	for (int i = 0; i < count; i++) {
		if (ini.Get_String(ANIMATIONS, ini.Get_Entry(ANIMATIONS, i), "", buffer, sizeof(buffer))) {
			AnimTypeClass::Find_Or_Make(buffer);
		}
	}
	return(count > 0);
}


/// <summary>
/// Creates the voxel animation types declared in the control file.
/// Each entry of the voxel animation list names a type, which is created if the game has
/// not heard of it before. The type then reads its own section for its behavior.
/// </summary>
/// <returns>bool; Were any voxel animations declared?</returns>
bool RulesClass::Do_VoxelAnimTypes(CCINIClass const & ini)
{
	char const * const VOXELANIMS = "VoxelAnims";
	char buffer[32];
	int count = ini.Entry_Count(VOXELANIMS);
	for (int i = 0; i < count; i++) {
		if (ini.Get_String(VOXELANIMS, ini.Get_Entry(VOXELANIMS, i), "", buffer, sizeof(buffer))) {
			VoxelAnimTypeClass::Find_Or_Make(buffer);
		}
	}
	return(count > 0);
}


/// <summary>
/// Creates the warhead types declared in the control file.
/// Each entry of the warhead list names a warhead, which is created if the game has not
/// heard of it before. The warhead then reads its own section for its damage behavior.
/// </summary>
/// <returns>bool; Were any warheads declared?</returns>
bool RulesClass::Do_WarheadTypes(CCINIClass const & ini)
{
	char buffer[32];
	int count = ini.Entry_Count("Warheads");
	for (int i = 0; i < count; i++) {
		if (ini.Get_String("Warheads", ini.Get_Entry("Warheads", i), "", buffer, sizeof(buffer))) {
			WarheadTypeClass::Find_Or_Make(buffer);
		}
	}
	return(count > 0);
}


/// <summary>
/// Creates the particle types declared in the control file.
/// Each entry of the particle list names a particle, which is created if the game has not
/// heard of it before. The particle then reads its own section for its behavior.
/// </summary>
/// <returns>bool; Were any particles declared?</returns>
bool RulesClass::Do_ParticleTypes(CCINIClass const & ini)
{
	char const * const PARTICLES = "Particles";
	char buffer[32];
	int count = ini.Entry_Count(PARTICLES);
	for (int i = 0; i < count; i++) {
		if (ini.Get_String(PARTICLES, ini.Get_Entry(PARTICLES, i), "", buffer, sizeof(buffer))) {
			ParticleTypeClass::Find_Or_Make(buffer);
		}
	}
	return(count > 0);
}


/// <summary>
/// Creates the particle system types declared in the control file.
/// Each entry of the particle system list names a system, which is created if the game has
/// not heard of it before. The system then reads its own section for its behavior.
/// </summary>
/// <returns>bool; Were any particle systems declared?</returns>
bool RulesClass::Do_ParticleSystemTypes(CCINIClass const & ini)
{
	char const * const PARTICLESYSTEMS = "ParticleSystems";
	char buffer[32];
	int count = ini.Entry_Count(PARTICLESYSTEMS);
	for (int i = 0; i < count; i++) {
		if (ini.Get_String(PARTICLESYSTEMS, ini.Get_Entry(PARTICLESYSTEMS, i), "", buffer, sizeof(buffer))) {
			ParticleSystemTypeClass::Find_Or_Make(buffer);
		}
	}
	return(count > 0);
}


/***********************************************************************************************
 * RulesClass::AI -- Processes the AI control constants from the database.                     *
 *                                                                                             *
 *    This will examine the database specified and set the AI override values accordingly.     *
 *                                                                                             *
 * INPUT:   ini   -- Reference to the INI database that holds the AI overrides.                *
 *                                                                                             *
 * OUTPUT:  bool; Was the AI section found and processed?                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/08/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool RulesClass::AI(CCINIClass const & ini)
{
	static char const * const AI = "AI";
	if (ini.Is_Present(AI)) {
		BuildConst = TGet_TypeList<BuildingTypeClass>(ini, AI, "BuildConst", BuildConst);
		BuildPower = TGet_TypeList<BuildingTypeClass>(ini, AI, "BuildPower", BuildPower);
		BuildRefinery = TGet_TypeList<BuildingTypeClass>(ini, AI, "BuildRefinery", BuildRefinery);
		BuildBarracks = TGet_TypeList<BuildingTypeClass>(ini, AI, "BuildBarracks", BuildBarracks);
		BuildTech = TGet_TypeList<BuildingTypeClass>(ini, AI, "BuildTech", BuildTech);
		BuildWeapons = TGet_TypeList<BuildingTypeClass>(ini, AI, "BuildWeapons", BuildWeapons);
		BuildDefense = TGet_TypeList<BuildingTypeClass>(ini, AI, "BuildDefense", BuildDefense);
		BuildPDefense = TGet_TypeList<BuildingTypeClass>(ini, AI, "BuildPDefense", BuildPDefense);
		BuildAA = TGet_TypeList<BuildingTypeClass>(ini, AI, "BuildAA", BuildAA);
		BuildHelipad = TGet_TypeList<BuildingTypeClass>(ini, AI, "BuildHelipad", BuildHelipad);
		BuildRadar = TGet_TypeList<BuildingTypeClass>(ini, AI, "BuildRadar", BuildRadar);
		ConcreteWalls = TGet_TypeList<BuildingTypeClass>(ini, AI, "ConcreteWalls", ConcreteWalls);
		NSGates = TGet_TypeList<BuildingTypeClass>(ini, AI, "NSGates", NSGates);
		EWGates = TGet_TypeList<BuildingTypeClass>(ini, AI, "EWGates", EWGates);
		AttackInterval = ini.Get_Float(AI, "AttackInterval", AttackInterval);
		AttackDelay = ini.Get_Float(AI, "AttackDelay", AttackDelay);
		PatrolTime = ini.Get_Float(AI, "PatrolScan", PatrolTime);
		RepairThreshhold = ini.Get_Int(AI, "CreditReserve", RepairThreshhold);
		PathDelay = ini.Get_Float(AI, "PathDelay", PathDelay);
		BlockagePathDelay = ini.Get_Int(AI, "BlockagePathDelay", BlockagePathDelay);
		TiberiumShortScan = ini.Get_Lepton(AI, "TiberiumNearScan", TiberiumShortScan);
		TiberiumLongScan = ini.Get_Lepton(AI, "TiberiumFarScan", TiberiumLongScan);
		AutocreateTime = ini.Get_Float(AI, "AutocreateTime", AutocreateTime);
		InfantryReserve = ini.Get_Int(AI, "InfantryReserve", InfantryReserve);
		InfantryBaseMult = ini.Get_Int(AI, "InfantryBaseMult", InfantryBaseMult);
		PowerSurplus = ini.Get_Int(AI, "PowerSurplus", PowerSurplus);
		BaseSizeAdd = ini.Get_Int(AI, "BaseSizeAdd", BaseSizeAdd);
		RefineryRatio = ini.Get_Float(AI, "RefineryRatio", RefineryRatio);
		RefineryLimit = ini.Get_Int(AI, "RefineryLimit", RefineryLimit);
		BarracksRatio = ini.Get_Float(AI, "BarracksRatio", BarracksRatio);
		BarracksLimit = ini.Get_Int(AI, "BarracksLimit", BarracksLimit);
		WarRatio = ini.Get_Float(AI, "WarRatio", WarRatio);
		WarLimit = ini.Get_Int(AI, "WarLimit", WarLimit);
		DefenseRatio = ini.Get_Float(AI, "DefenseRatio", DefenseRatio);
		DefenseLimit = ini.Get_Int(AI, "DefenseLimit", DefenseLimit);
		AARatio = ini.Get_Float(AI, "AARatio", AARatio);
		AALimit = ini.Get_Int(AI, "AALimit", AALimit);
		TeslaRatio = ini.Get_Float(AI, "TeslaRatio", TeslaRatio);
		TeslaLimit = ini.Get_Int(AI, "TeslaLimit", TeslaLimit);
		HelipadRatio = ini.Get_Float(AI, "HelipadRatio", HelipadRatio);
		HelipadLimit = ini.Get_Int(AI, "HelipadLimit", HelipadLimit);
		AirstripRatio = ini.Get_Float(AI, "AirstripRatio", AirstripRatio);
		AirstripLimit = ini.Get_Int(AI, "AirstripLimit", AirstripLimit);
		IsCompEasyBonus = ini.Get_Bool(AI, "CompEasyBonus", IsCompEasyBonus);
		IsComputerParanoid = ini.Get_Bool(AI, "Paranoid", IsComputerParanoid);
		PowerEmergencyFraction = ini.Get_Float(AI, "PowerEmergency", PowerEmergencyFraction);
		AIBaseSpacing = ini.Get_Int(AI, "AIBaseSpacing", AIBaseSpacing);
		GDIWallDefense = ini.Get_Float(AI, "GDIWallDefense", GDIWallDefense);
		GDIWallDefenseCoefficient = ini.Get_Float(AI, "GDIWallDefenseCoefficient", GDIWallDefenseCoefficient);
		NodBaseDefenseCoefficient = ini.Get_Float(AI, "NodBaseDefenseCoefficient", NodBaseDefenseCoefficient);
		GDIBaseDefenseCoefficient = ini.Get_Float(AI, "GDIBaseDefenseCoefficient", GDIBaseDefenseCoefficient);
		MaximumBaseDefenseValue = ini.Get_Int(AI, "MaximumBaseDefenseValue", MaximumBaseDefenseValue);
		ComputerBaseDefenseResponse = ini.Get_Int(AI, "ComputerBaseDefenseResponse", ComputerBaseDefenseResponse);
		AIDetectDisguise = ini.Get_Bool(AI, "AIDetectDisguise", AIDetectDisguise);
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * RulesClass::Powerups -- Process the powerup values from the database.                       *
 *                                                                                             *
 *    This will examine the database and initialize the powerup override values accordingly.   *
 *                                                                                             *
 * INPUT:   ini   -- Reference to the INI database the the powerup values are to be            *
 *                   initialized from.                                                         *
 *                                                                                             *
 * OUTPUT:  bool; Was the powerup section found and processed?                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/08/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool RulesClass::Powerups(CCINIClass const & ini)
{
	static char const * const POWERUPS = "Powerups";
	if (ini.Is_Present(POWERUPS)) {
		for (int crate = CRATE_FIRST; crate < CRATE_COUNT; crate++) {
			char buffer[128];
			if (ini.Get_String(POWERUPS, CrateNames[crate], "0,NONE", buffer, sizeof(buffer))) {

				/*
				**	Share odds.
				*/
				char * token = strtok(buffer, ",");
				if (token) {
					strtrim(token);
					CrateShares[crate] = atoi(token);
				}

				/*
				**	Animation to use.
				*/
				token = strtok(NULL, ",");
				if (token) {
					strtrim(token);
					CrateAnims[crate] = Anim_From_Name(token);
				}

				/*
				**	Optional data number.
				*/
				token = strtok(NULL, ",");
				if (token != NULL) {
					if (strchr(token, '%') != NULL) {
						CrateData[crate] = atof(token) / 100;
					} else {
						strtrim(token);
						CrateData[crate] = atof(token);
					}
				}
			}
		}
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * RulesClass::Land_Types -- Inits the land type values.                                       *
 *                                                                                             *
 *    This will set the land movement attributes from the database specified.                  *
 *                                                                                             *
 * INPUT:   ini   -- Reference to the database that has the land value overrides.              *
 *                                                                                             *
 * OUTPUT:  bool; Was the land type sections found and processed?                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/08/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool RulesClass::Land_Types(CCINIClass const & ini)
{
	/*
	**	Fetch the movement characteristic data for terrain types.
	*/
	for (int land = LAND_FIRST; land < LAND_COUNT; land++) {
		static char const * _lands[LAND_COUNT] = {
			"Clear",
			"Road",
			"Water",
			"Rock",
			"Wall",
			"Tiberium",
			"Beach",
			"Rough",
			"Ice",
			"Railroad",
			"Tunnel",
			"Weeds",
		};

		static char const * _speeds[SPEED_COUNT] = {
			"Foot",
			"Track",
			"Wheel",
			"Hover",
			"Winged",
			"Float",
			"Amphibious",
			"Creep",
		};

		GroundType * gptr = &Ground[land];

		if (ini.Is_Present(_lands[land])) {
			for (int speed = 0; speed < SPEED_COUNT; speed++) {
				gptr->Cost[speed] = std::min(ini.Get_Float(_lands[land], _speeds[speed], gptr->Cost[speed]), 1.0);
			}
			gptr->Build = ini.Get_Bool(_lands[land], "Buildable", gptr->Build);
		}
	}
	return(true);
}


/***********************************************************************************************
 * RulesClass::IQ -- Fetches the IQ control values from the INI database.                      *
 *                                                                                             *
 *    This will scan the database specified and retrieve the IQ control values from it. These  *
 *    IQ control values are what gives the IQ rating meaning. It fundimentally controls how    *
 *    the computer behaves.                                                                    *
 *                                                                                             *
 * INPUT:   ini   -- Reference to the INI database to read the IQ controls from.               *
 *                                                                                             *
 * OUTPUT:  bool; Was the IQ section found and processed?                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool RulesClass::IQ(CCINIClass const & ini)
{
	static char const * const IQCONTROL = "IQ";
	if (ini.Is_Present(IQCONTROL)) {
		MaxIQ = ini.Get_Int(IQCONTROL, "MaxIQLevels", MaxIQ);
		IQSuperWeapons = ini.Get_Int(IQCONTROL, "SuperWeapons", IQSuperWeapons);
		IQProduction = ini.Get_Int(IQCONTROL, "Production", IQProduction);
		IQGuardArea = ini.Get_Int(IQCONTROL, "GuardArea", IQGuardArea);
		IQRepairSell = ini.Get_Int(IQCONTROL, "RepairSell", IQRepairSell);
		IQCrush = ini.Get_Int(IQCONTROL, "AutoCrush", IQCrush);
		IQScatter = ini.Get_Int(IQCONTROL, "Scatter", IQScatter);
		IQContentScan = ini.Get_Int(IQCONTROL, "ContentScan", IQContentScan);
		IQAircraft = ini.Get_Int(IQCONTROL, "Aircraft", IQAircraft);
		IQHarvester = ini.Get_Int(IQCONTROL, "Harvester", IQHarvester);
		IQSellBack = ini.Get_Int(IQCONTROL, "SellBack", IQSellBack);
		return(true);
	}
	return(false);
}


/// <summary>
/// Fetches the flight controls used by jumpjet units.
/// These are the values the jumpjet locomotor consults for its turn rate, speed, cruising
/// height and the lazy wobble it performs while hovering.
/// </summary>
/// <returns>bool; Was a jumpjet control section found in the control file?</returns>
bool RulesClass::Jumpjet_Controls(CCINIClass const & ini)
{
	static char const * const SECTION = "JumpjetControls";
	if (ini.Is_Present(SECTION)) {
		JumpjetTurnRate = ini.Get_Int(SECTION, "TurnRate", JumpjetTurnRate);
		JumpjetSpeed = ini.Get_Int(SECTION, "Speed", JumpjetSpeed);
		JumpjetClimb = ini.Get_Float(SECTION, "Climb", JumpjetClimb);
		JumpjetCruiseHeight = ini.Get_Int(SECTION, "CruiseHeight", JumpjetCruiseHeight);
		JumpjetAcceleration = ini.Get_Float(SECTION, "Acceleration", JumpjetAcceleration);
		JumpjetWobblesPerSecond = ini.Get_Float(SECTION, "WobblesPerSecond", JumpjetWobblesPerSecond);
		JumpjetWobbleDeviation = ini.Get_Int(SECTION, "WobbleDeviation", JumpjetWobbleDeviation);
		JumpjetCloakDetectionRadius = ini.Get_Int(SECTION, "CloakDetectionRadius", JumpjetCloakDetectionRadius);
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * RulesClass::Difficulty -- Fetch the various difficulty group settings.                      *
 *                                                                                             *
 *    This routine is used to fetch the various group settings for the difficulty levels.      *
 *                                                                                             *
 * INPUT:   ini   -- Reference to the INI database that has the difficulty setting values.     *
 *                                                                                             *
 * OUTPUT:  bool; Was the difficulty section found and processed.                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/10/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool RulesClass::Difficulty(CCINIClass const & ini)
{
	Difficulty_Get(ini, Diff[DIFF_EASY], "Easy");
	Difficulty_Get(ini, Diff[DIFF_NORMAL], "Normal");
	Difficulty_Get(ini, Diff[DIFF_HARD], "Difficult");
	return(true);
}


/***********************************************************************************************
 * Difficulty_Get -- Fetch the difficulty bias values.                                         *
 *                                                                                             *
 *    This will fetch the difficulty bias values for the section specified.                    *
 *                                                                                             *
 * INPUT:   ini   -- Reference the INI database to fetch the values from.                      *
 *                                                                                             *
 *          diff  -- Reference to the difficulty class object to fill in with the values.      *
 *                                                                                             *
 *          section  -- The section identifier to lift the values from.                        *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/11/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
static void Difficulty_Get(CCINIClass const & ini, DifficultyClass & diff, char const * section)
{
	if (ini.Is_Present(section)) {
		diff.FirepowerBias = ini.Get_Float(section, "FirePower", 1);
		diff.GroundspeedBias = ini.Get_Float(section, "Groundspeed", 1);
		diff.AirspeedBias = ini.Get_Float(section, "Airspeed", 1);
		diff.ArmorBias = ini.Get_Float(section, "Armor", 1);
		diff.ROFBias = ini.Get_Float(section, "ROF", 1);
		diff.CostBias = ini.Get_Float(section, "Cost", 1);
		diff.RepairDelay = ini.Get_Float(section, "RepairDelay", .02);
		diff.BuildDelay = ini.Get_Float(section, "BuildDelay", .03);
		diff.IsBuildSlowdown = ini.Get_Bool(section, "BuildSlowdown", false);
		diff.BuildSpeedBias = ini.Get_Float(section, "BuildTime", 1);
		diff.IsWallDestroyer = ini.Get_Bool(section, "DestroyWalls", true);
		diff.IsContentScan = ini.Get_Bool(section, "ContentScan", false);
	}
}


/// <summary>
/// Registers any movies declared in the control file.
/// Movies named in the control file that the game does not already recognize are added to
/// the global movie list, so that a mission can call for a movie the executable was never
/// built to know about.
/// </summary>
/// <returns>bool; Was a movie section found in the control file?</returns>
bool RulesClass::Do_Movies(CCINIClass const & ini)
{
	static char const * const MOVIES = "Movies";
	char buffer[32];
	if (ini.Is_Present(MOVIES)) {
		int count = ini.Entry_Count(MOVIES);
		for (int i = 0; i < count; i++) {
			if (ini.Get_String(MOVIES, ini.Get_Entry(MOVIES, i), "<none>", buffer, sizeof(buffer))) {
				if (VQ_From_Name(buffer) == -1) {
					::Movies.Add(strdup(buffer));
				}
			}
		}
		return(true);
	}
	return(false);
}


/// <summary>
/// Writes the rule data out to a save game stream.
/// </summary>
void RulesClass::Save(IStream * stream)
{
	SaveStreamClass savestream(stream, SaveStreamClass::MODE_SAVE);
	Serialize(savestream);
}


/// <summary>
/// Reads the rule data back from a save game stream.
/// </summary>
/// <remarks>Be sure the object heaps have been loaded before calling this routine, since
/// the pointer swizzle needs them.</remarks>
void RulesClass::Load(IStream * stream)
{
	SaveStreamClass savestream(stream, SaveStreamClass::MODE_LOAD);
	savestream.Set_Context("RulesClass");
	Serialize(savestream);
}


/// <summary>
/// Lists the members the rules hold.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void RulesClass::Serialize(SaveStreamClass & stream)
{
	stream.Serialize(AmmoCrateDamage);
	stream.Serialize(LargeVisceroid);
	stream.Serialize(SmallVisceroid);
	stream.Serialize(UnloadingHarvester);
	stream.Serialize(AttackingAircraftSightRange);
	stream.Serialize(TunnelSpeed);
	stream.Serialize(TiberiumHeal);
	stream.Serialize(HSBuilding);
	stream.Serialize(IsFreeMCV);
	stream.Serialize(IsBerzerkAllowed);
	stream.Serialize(PoseDir);
	stream.Serialize(DropPodPuff);
	stream.Serialize(WaypointAnimationSpeed);
	stream.Serialize(BarrelExplode);
	stream.Serialize(BarrelDebris);
	stream.Serialize(BarrelParticle);
	stream.Serialize(RadarEventColorSpeed);
	stream.Serialize(RadarEventMinRadius);
	stream.Serialize(RadarEventSpeed);
	stream.Serialize(RadarEventRotationSpeed);
	stream.Serialize(FlashFrameTime);
	stream.Serialize(RadarCombatFlashTime);
	stream.Serialize(MaxWaypointPathLength);
	stream.Serialize(Wake);
	stream.Serialize(FlamingInfantry);
	stream.Serialize(AITriggerSuccessWeightDelta);
	stream.Serialize(AITriggerFailureWeightDelta);
	stream.Serialize(AITriggerTrackRecordCoefficient);
	stream.Serialize(VeinholeMonsterStrength);
	stream.Serialize(MaxVeinholeGrowth);
	stream.Serialize(VeinholeGrowthRate);
	stream.Serialize(VeinholeShrinkRate);
	stream.Serialize(VeinAttack);
	stream.Serialize(VeinDamage);
	stream.Serialize(MaximumQueuedObjects);
	stream.Serialize(AircraftFogReveal);
	stream.Serialize(WoodCrateImg);
	stream.Serialize(CrateImg);
	stream.Serialize(DropPod);
	stream.Serialize(DeadBodies);
	stream.Serialize(MetallicDebris);
	stream.Serialize(BridgeExplosions);
	stream.Serialize(DigSound);
	stream.Serialize(Dig);
	stream.Serialize(IonBlast);
	stream.Serialize(IonBeam);
	stream.Serialize(InfantryExplode);
	stream.Serialize(AtmosphereEntry);
	stream.Serialize(PrerequisitePower);
	stream.Serialize(PrerequisiteFactory);
	stream.Serialize(PrerequisiteBarracks);
	stream.Serialize(PrerequisiteRadar);
	stream.Serialize(PrerequisiteTech);
	stream.Serialize(GateUpSound);
	stream.Serialize(GateDownSound);
	stream.Serialize(JumpjetTurnRate);
	stream.Serialize(JumpjetSpeed);
	stream.Serialize(JumpjetClimb);
	stream.Serialize(JumpjetCruiseHeight);
	stream.Serialize(JumpjetAcceleration);
	stream.Serialize(JumpjetWobblesPerSecond);
	stream.Serialize(JumpjetWobbleDeviation);
	stream.Serialize(RadarEventSuppressionDistances);
	stream.Serialize(RadarEventVisibilityDurations);
	stream.Serialize(RadarEventDurations);
	stream.Serialize(IonCannonDamage);
	stream.Serialize(RailgunDamageRadius);
	stream.Serialize(ZoomInFactor);
	stream.Serialize(ConditionRedSparkingProbability);
	stream.Serialize(ConditionYellowSparkingProbability);
	stream.Serialize(TiberiumExplosionDamage);
	stream.Serialize(TiberiumStrength);
	stream.Serialize(MinLowPowerProductionSpeed);
	stream.Serialize(MultipleFactory);
	stream.Serialize(CraterLevel);
	stream.Serialize(TreeFlammability);
	stream.Serialize(MissileSpeedVar);
	stream.Serialize(MissileROTVar);
	stream.Serialize(DropPodWeapon);
	stream.Serialize(DropPodHeight);
	stream.Serialize(DropPodSpeed);
	stream.Serialize(DropPodAngle);
	stream.Serialize(ScrollMultiplier);
	stream.Serialize(CrewEscape);
	stream.Serialize(ShakeScreen);
	stream.Serialize(HoverHeight);
	stream.Serialize(HoverBob);
	stream.Serialize(HoverBoost);
	stream.Serialize(HoverAcceleration);
	stream.Serialize(HoverBrake);
	stream.Serialize(HoverDampen);
	stream.Serialize(PlacementDelay);
	stream.Serialize(ExplosiveVoxelDebris);
	stream.Serialize(TireVoxelDebris);
	stream.Serialize(ScrapVoxelDebris);
	stream.Serialize(BridgeVoxelMax);
	stream.Serialize(CloakingStages);
	stream.Serialize(RevealTriggerRadius);
	stream.Serialize(IceCrackingWeight);
	stream.Serialize(IceBreakingWeight);
	stream.Serialize(IceCrackSounds);
	stream.Serialize(CliffBackImpassability);
	stream.Serialize(VeteranRatio);
	stream.Serialize(VeteranCombat);
	stream.Serialize(VeteranSpeed);
	stream.Serialize(VeteranSight);
	stream.Serialize(VeteranArmor);
	stream.Serialize(VeteranROF);
	stream.Serialize(VeteranCap);
	stream.Serialize(CloakSound);
	stream.Serialize(SellSound);
	stream.Serialize(GameClosed);
	stream.Serialize(IncomingMessage);
	stream.Serialize(SystemError);
	stream.Serialize(OptionsChanged);
	stream.Serialize(GameForming);
	stream.Serialize(PlayerLeft);
	stream.Serialize(PlayerJoined);
	stream.Serialize(Construction);
	stream.Serialize(CreditTicks);
	stream.Serialize(CrumbleSound);
	stream.Serialize(BuildingSlam);
	stream.Serialize(RadarOn);
	stream.Serialize(RadarOff);
	stream.Serialize(ScoldSound);
	stream.Serialize(TeslaCharge);
	stream.Serialize(TeslaZap);
	stream.Serialize(GenericClick);
	stream.Serialize(GenericBeep);
	stream.Serialize(BlowupSound);
	stream.Serialize(HealCrateSound);
	stream.Serialize(ChuteSound);
	stream.Serialize(StopSound);
	stream.Serialize(GuardSound);
	stream.Serialize(ScatterSound);
	stream.Serialize(DeploySound);
	stream.Serialize(LightningSound);
	stream.Serialize(WorstLowPowerBuildRateCoefficient);
	stream.Serialize(BestLowPowerBuildRateCoefficient);
	stream.Serialize(WallBuildSpeedCoefficient);
	stream.Serialize(ChargeToDrainRatio);
	stream.Serialize(DamageToFirestormDamageCoefficient);
	stream.Serialize(TrackedUphill);
	stream.Serialize(TrackedDownhill);
	stream.Serialize(WheeledUphill);
	stream.Serialize(WheeledDownhill);
	stream.Serialize(SpotlightMovementRadius);
	stream.Serialize(SpotlightLocationRadius);
	stream.Serialize(SpotlightSpeed);
	stream.Serialize(SpotlightAcceleration);
	stream.Serialize(SpotlightAngle);
	stream.Serialize(SpotlightRadius);
	stream.Serialize(WindDirection);
	stream.Serialize(CameraRange);
	stream.Serialize(FlightLevel);
	stream.Serialize(BuildingDrop);
	stream.Serialize(Scorches);
	stream.Serialize(Scorches1);
	stream.Serialize(Scorches2);
	stream.Serialize(Scorches3);
	stream.Serialize(Scorches4);
	stream.Serialize(Craters);
	stream.Serialize(RepairBay);
	stream.Serialize(GDIGateOne);
	stream.Serialize(GDIGateTwo);
	stream.Serialize(NodGateOne);
	stream.Serialize(NodGateTwo);
	stream.Serialize(WallTower);
	stream.Serialize(GDIPowerPlant);
	stream.Serialize(GDIPowerTurbine);
	stream.Serialize(NodRegularPower);
	stream.Serialize(NodAdvancedPower);
	stream.Serialize(GDIFirestormGenerator);
	stream.Serialize(GDIHunterSeeker);
	stream.Serialize(NodHunterSeeker);
	stream.Serialize(BuildConst);
	stream.Serialize(BuildPower);
	stream.Serialize(BuildRefinery);
	stream.Serialize(BuildBarracks);
	stream.Serialize(BuildTech);
	stream.Serialize(BuildWeapons);
	stream.Serialize(BuildDefense);
	stream.Serialize(BuildPDefense);
	stream.Serialize(BuildAA);
	stream.Serialize(BuildHelipad);
	stream.Serialize(BuildRadar);
	stream.Serialize(ConcreteWalls);
	stream.Serialize(NSGates);
	stream.Serialize(EWGates);
	stream.Serialize(GDIWallDefense);
	stream.Serialize(GDIWallDefenseCoefficient);
	stream.Serialize(NodBaseDefenseCoefficient);
	stream.Serialize(GDIBaseDefenseCoefficient);
	stream.Serialize(ComputerBaseDefenseResponse);
	stream.Serialize(AIDetectDisguise);
	stream.Serialize(MaximumBaseDefenseValue);
	stream.Serialize(BaseUnit);
	stream.Serialize(HarvesterUnit);
	stream.Serialize(PadAircraft);
	stream.Serialize(OnFire);
	stream.Serialize(TreeFire);
	stream.Serialize(Smoke1);
	stream.Serialize(Smoke2);
	stream.Serialize(FirestormActiveAnim);
	stream.Serialize(FirestormIdleAnim);
	stream.Serialize(FirestormAirAnim);
	stream.Serialize(FirestormGroundAnim);
	stream.Serialize(MoveFlash);
	stream.Serialize(BombParachute);
	stream.Serialize(Parachute);
	stream.Serialize(SplashList);
	stream.Serialize(SmallFire);
	stream.Serialize(LargeFire);
	stream.Serialize(Paratrooper);
	stream.Serialize(Disguise);
	stream.Serialize(Technician);
	stream.Serialize(Engineer);
	stream.Serialize(Pilot);
	stream.Serialize(Crew);
	stream.Serialize(FlameDamage);
	stream.Serialize(FlameDamage2);
	stream.Serialize(NukeWarhead);
	stream.Serialize(NukeProjectile);
	stream.Serialize(NukeDown);
	stream.Serialize(EMPulseWarhead);
	stream.Serialize(EMPulseProjectile);
	stream.Serialize(C4Warhead);
	stream.Serialize(IonCannonWarhead);
	stream.Serialize(FirestormWarhead);
	stream.Serialize(VeinholeWarhead);
	stream.Serialize(IonStormWarhead);
	stream.Serialize(VeinholeTypeClass);
	stream.Serialize(DefaultLargeGreySmokeSystem);
	stream.Serialize(DefaultSmallGreySmokeSystem);
	stream.Serialize(DefaultSparkSystem);
	stream.Serialize(DefaultLargeRedSmokeSystem);
	stream.Serialize(DefaultSmallRedSmokeSystem);
	stream.Serialize(DefaultDebrisSmokeSystem);
	stream.Serialize(DefaultFireStreamSystem);
	stream.Serialize(DefaultFirestormExplosionSystem);
	stream.Serialize(DefaultTestParticleSystem);
	stream.Serialize(DefaultRepairParticleSystem);
	stream.Serialize(MyEffectivenessCoefficientDefault);
	stream.Serialize(TargetEffectivenessCoefficientDefault);
	stream.Serialize(TargetSpecialThreatCoefficientDefault);
	stream.Serialize(TargetStrengthCoefficientDefault);
	stream.Serialize(TargetDistanceCoefficientDefault);
	stream.Serialize(DumbMyEffectivenessCoefficient);
	stream.Serialize(DumbTargetEffectivenessCoefficient);
	stream.Serialize(DumbTargetSpecialThreatCoefficient);
	stream.Serialize(DumbTargetStrengthCoefficient);
	stream.Serialize(DumbTargetDistanceCoefficient);
	stream.Serialize(EnemyHouseThreatBonus);
	stream.Serialize(HunterSeekerDetonateProximity);
	stream.Serialize(HunterSeekerDescendProximity);
	stream.Serialize(HunterSeekerDescentSpeed);
	stream.Serialize(HunterSeekerAscentSpeed);
	stream.Serialize(HunterSeekerEmergeSpeed);
	stream.Serialize(TurboBoost);
	stream.Serialize(AttackInterval);
	stream.Serialize(AttackDelay);
	stream.Serialize(PowerEmergencyFraction);
	stream.Serialize(AirstripRatio);
	stream.Serialize(AirstripLimit);
	stream.Serialize(HelipadRatio);
	stream.Serialize(HelipadLimit);
	stream.Serialize(TeslaRatio);
	stream.Serialize(TeslaLimit);
	stream.Serialize(AARatio);
	stream.Serialize(AALimit);
	stream.Serialize(DefenseRatio);
	stream.Serialize(DefenseLimit);
	stream.Serialize(WarRatio);
	stream.Serialize(WarLimit);
	stream.Serialize(BarracksRatio);
	stream.Serialize(BarracksLimit);
	stream.Serialize(RefineryLimit);
	stream.Serialize(RefineryRatio);
	stream.Serialize(BaseSizeAdd);
	stream.Serialize(PowerSurplus);
	stream.Serialize(InfantryReserve);
	stream.Serialize(InfantryBaseMult);
	stream.Serialize(SoloCrateMoney);
	stream.Serialize(TreeStrength);
	stream.Serialize(UnitCrateType);
	stream.Serialize(PatrolTime);
	stream.Serialize(TeamDelays);
	stream.Serialize(AIHateDelays);
	stream.Serialize(DissolveUnfilledTeamDelay);
	stream.Serialize(AIIonCannonConYardValue);
	stream.Serialize(AIIonCannonWarFactoryValue);
	stream.Serialize(AIIonCannonPowerValue);
	stream.Serialize(AIIonCannonEngineerValue);
	stream.Serialize(AIIonCannonThiefValue);
	stream.Serialize(AIIonCannonHarvesterValue);
	stream.Serialize(AIIonCannonMCVValue);
	stream.Serialize(AIIonCannonAPCValue);
	stream.Serialize(AIIonCannonBaseDefenseValue);
	stream.Serialize(AIIonCannonPlugValue);
	stream.Serialize(AIIonCannonHelipadValue);
	stream.Serialize(AIIonCannonTempleValue);
	stream.Serialize(AIAlternateProductionCreditCutoff);
	stream.Serialize(MultiplayerAICreditMultipliers);
	stream.Serialize(MinimumAIDefensiveTeams);
	stream.Serialize(MaximumAIDefensiveTeams);
	stream.Serialize(TotalAITeamCap);
	stream.Serialize(AIUseTurbineUpgradeChance);
	stream.Serialize(FillEarliestTeamProbability);
	stream.Serialize(CloakDelay);
	stream.Serialize(GameSpeedBias);
	stream.Serialize(NervousBias);
	stream.Serialize(ExplosionSpread);
	stream.Serialize(SupressRadius);
	stream.Serialize(MaxIQ);
	stream.Serialize(IQSuperWeapons);
	stream.Serialize(IQProduction);
	stream.Serialize(IQGuardArea);
	stream.Serialize(IQRepairSell);
	stream.Serialize(IQCrush);
	stream.Serialize(IQScatter);
	stream.Serialize(IQContentScan);
	stream.Serialize(IQAircraft);
	stream.Serialize(IQHarvester);
	stream.Serialize(IQSellBack);
	stream.Serialize(AIBaseSpacing);
	stream.Serialize(SilverCrate);
	stream.Serialize(WoodCrate);
	stream.Serialize(CrateMinimum);
	stream.Serialize(CrateMaximum);
	stream.Serialize(LZScanRadius);
	stream.Serialize(FlareAnim);
	stream.Serialize(MPMoney);
	stream.Serialize(MPMaxMoney);
	stream.Serialize(MPUnitCount);
	stream.Serialize(MPBuildLevel);
	stream.Serialize(DropZoneRadius);
	stream.Serialize(MessageDelay);
	stream.Serialize(SavourDelay);
	stream.Serialize(MaxPlayers);
	stream.Serialize(BaseDefenseDelay);
	stream.Serialize(SuspendPriority);
	stream.Serialize(SuspendDelay);
	stream.Serialize(SurvivorFraction);
	stream.Serialize(SurvivorDivisor);
	stream.Serialize(ReloadRate);
	stream.Serialize(AutocreateTime);
	stream.Serialize(BuildupTime);
	stream.Serialize(HarvesterLoadRate);
	stream.Serialize(HarvesterDumpRate);
	stream.Serialize(AtomDamage);
	stream.Serialize(Diff);
	stream.Serialize(QuakeDamagePercent);
	stream.Serialize(QuakeChance);
	stream.Serialize(GrowthRate);
	stream.Serialize(ShroudRate);
	stream.Serialize(FogRate);
	stream.Serialize(IceGrowthRate);
	stream.Serialize(VeinGrowthRate);
	stream.Serialize(IceSolidifyDelay);
	stream.Serialize(AmbientLightChangeRate);
	stream.Serialize(AmbientLightChangeStep);
	stream.Serialize(CrateTime);
	stream.Serialize(TimerWarning);
	stream.Serialize(TiberiumTransmogrify);
	stream.Serialize(NukeTime);
	stream.Serialize(EMPulseTime);
	stream.Serialize(IonCannonTime);
	stream.Serialize(FirestormTime);
	stream.Serialize(SpeakDelay);
	stream.Serialize(DamageDelay);
	stream.Serialize(Gravity);
	stream.Serialize(LeptonsPerSightIncrease);
	stream.Serialize(Incoming);
	stream.Serialize(MinDamage);
	stream.Serialize(MaxDamage);
	stream.Serialize(RepairStep);
	stream.Serialize(RepairPercent);
	stream.Serialize(IRepairStep);
	stream.Serialize(RepairRate);
	stream.Serialize(URepairRate);
	stream.Serialize(IRepairRate);
	stream.Serialize(ConditionGreen);
	stream.Serialize(ConditionYellow);
	stream.Serialize(ConditionRed);
	stream.Serialize(RandomAnimateTime);
	stream.Serialize(CloseEnoughDistance);
	stream.Serialize(StrayDistance);
	stream.Serialize(CrushDistance);
	stream.Serialize(CrateRadius);
	stream.Serialize(HomingScatter);
	stream.Serialize(BallisticScatter);
	stream.Serialize(RefundPercent);
	stream.Serialize(BridgeStrength);
	stream.Serialize(BuildSpeedBias);
	stream.Serialize(C4Delay);
	stream.Serialize(RepairThreshhold);
	stream.Serialize(PathDelay);
	stream.Serialize(BlockagePathDelay);
	stream.Serialize(MovieTime);
	stream.Serialize(TiberiumShortScan);
	stream.Serialize(TiberiumLongScan);
	stream.Serialize(LightningFrequency);
	stream.Serialize(LightningRandomness);
	stream.Serialize(LightningDamage);
	stream.Serialize(LightningDuration);
	stream.Serialize(LightningDeferment);
	stream.Serialize(CollapseChance);
	stream.Serialize(WeedCapacity);
	stream.Serialize(ExtraUnitLight);
	stream.Serialize(ExtraInfantryLight);
	stream.Serialize(ExtraAircraftLight);
	stream.Serialize(IsComputerParanoid);
	stream.Serialize(IsCurleyShuffle);
	stream.Serialize(IsBlendedFog);
	stream.Serialize(IsCompEasyBonus);
	stream.Serialize(IsFineDifficulty);
	stream.Serialize(IsExplosiveHarvester);
	stream.Serialize(IsHealthBar);
	stream.Serialize(IsAllyReveal);
	stream.Serialize(IsSeparate);
	stream.Serialize(IsTreeTarget);
	stream.Serialize(IsNamed);
	stream.Serialize(IsAutoCrush);
	stream.Serialize(IsSmartDefense);
	stream.Serialize(IsScatter);
	stream.Serialize(IsRevealByHeight);
	stream.Serialize(IsShroudedSubteranneanMovesAllowed);
	stream.Serialize(IsShroudGrow);
	stream.Serialize(IsMPShadowGrow);
	stream.Serialize(IsMPBasesOn);
	stream.Serialize(IsMPTiberiumGrow);
	stream.Serialize(IsMPCrates);
	stream.Serialize(IsMPAIPlayers);
	stream.Serialize(IsMPCaptureTheFlag);
	stream.Serialize(IsMPBridgeDestruction);
	stream.Serialize(IsMPBuildOffAllyAnyStructure);
	stream.Serialize(NodAIBuildsWalls);
	stream.Serialize(AIBuildsWalls);
	stream.Serialize(UseMinDefenseRule);
	stream.Serialize(EMPulseSparkles);
	stream.Serialize(WebbedInfantry);
	stream.Serialize(JumpjetCloakDetectionRadius);
	stream.Serialize(DropPodInfantryMinimum);
	stream.Serialize(DropPodInfantryMaximum);
	stream.Serialize(TalkBubbleTime);
	stream.Serialize(PrerequisiteGDIFactory);
	stream.Serialize(PrerequisiteNodFactory);
	stream.Serialize(EngineerCaptureLevel);
	stream.Serialize(EngineerDamage);
}


/// <summary>
/// Removes all references to the specified object from the rules.
/// The rules hold direct pointers to a great many object types and warheads, and keep
/// lists of others. This routine is called as part of the general detach process when an
/// object is about to be destroyed, so that no rule is left pointing at freed memory.
/// </summary>
void RulesClass::Detach(AbstractClass const * target, bool all)
{
	if (target == LargeVisceroid) {
		LargeVisceroid = NULL;
	}
	if (target == SmallVisceroid) {
		SmallVisceroid = NULL;
	}
	if (target == UnloadingHarvester) {
		UnloadingHarvester = NULL;
	}
	if (target == VeinAttack) {
		VeinAttack = NULL;
	}
	if (target == Wake) {
		Wake = NULL;
	}
	if (target == FlamingInfantry) {
		FlamingInfantry = NULL;
	}
	if (target == DropPodWeapon) {
		DropPodWeapon = NULL;
	}
	if (target == FlameDamage) {
		FlameDamage = NULL;
	}
	if (target == FlameDamage2) {
		FlameDamage2 = NULL;
	}
	if (target == NukeWarhead) {
		NukeWarhead = NULL;
	}
	if (target == EMPulseWarhead) {
		EMPulseWarhead = NULL;
	}
	if (target == C4Warhead) {
		C4Warhead = NULL;
	}
	if (target == IonCannonWarhead) {
		IonCannonWarhead = NULL;
	}
	if (target == FirestormWarhead) {
		FirestormWarhead = NULL;
	}
	if (target == EMPulseSparkles) {
		EMPulseSparkles = NULL;
	}
	if (target == VeinholeWarhead) {
		VeinholeWarhead = NULL;
	}
	if (target == DefaultFirestormExplosionSystem) {
		DefaultFirestormExplosionSystem = NULL;
	}
	if (target == DefaultLargeGreySmokeSystem) {
		DefaultLargeGreySmokeSystem = NULL;
	}
	if (target == DefaultSmallGreySmokeSystem) {
		DefaultSmallGreySmokeSystem = NULL;
	}
	if (target == DefaultSparkSystem) {
		DefaultSparkSystem = NULL;
	}
	if (target == DefaultLargeRedSmokeSystem) {
		DefaultLargeRedSmokeSystem = NULL;
	}
	if (target == DefaultSmallRedSmokeSystem) {
		DefaultSmallRedSmokeSystem = NULL;
	}
	if (target == DefaultDebrisSmokeSystem) {
		DefaultDebrisSmokeSystem = NULL;
	}
	if (target == DefaultFireStreamSystem) {
		DefaultFireStreamSystem = NULL;
	}
	if (target == DefaultTestParticleSystem) {
		DefaultTestParticleSystem = NULL;
	}
	if (target == DefaultRepairParticleSystem) {
		DefaultRepairParticleSystem = NULL;
	}
	if (target == VeinholeTypeClass) {
		VeinholeTypeClass = NULL;
	}
	if (target == Smoke1) {
		Smoke1 = NULL;
	}
	if (target == Smoke2) {
		Smoke2 = NULL;
	}
	if (target == MoveFlash) {
		MoveFlash = NULL;
	}
	if (target == BombParachute) {
		BombParachute = NULL;
	}
	if (target == Parachute) {
		Parachute = NULL;
	}
	if (target == SmallFire) {
		SmallFire = NULL;
	}
	if (target == LargeFire) {
		LargeFire = NULL;
	}
	if (target == FlareAnim) {
		FlareAnim = NULL;
	}
	if (target == BaseUnit) {
		BaseUnit = NULL;
	}
	if (target == UnitCrateType) {
		UnitCrateType = NULL;
	}
	if (target == Paratrooper) {
		Paratrooper = NULL;
	}
	if (target == Disguise) {
		Disguise = NULL;
	}
	if (target == Technician) {
		Technician = NULL;
	}
	if (target == Engineer) {
		Engineer = NULL;
	}
	if (target == Pilot) {
		Pilot = NULL;
	}
	if (target == Crew) {
		Crew = NULL;
	}
	if (target == RepairBay) {
		RepairBay = NULL;
	}
	if (target == GDIGateOne) {
		GDIGateOne = NULL;
	}
	if (target == GDIGateTwo) {
		GDIGateTwo = NULL;
	}
	if (target == NodGateOne) {
		NodGateOne = NULL;
	}
	if (target == NodGateTwo) {
		NodGateTwo = NULL;
	}
	if (target == WallTower) {
		WallTower = NULL;
	}
	if (target == GDIPowerPlant) {
		GDIPowerPlant = NULL;
	}
	if (target == GDIPowerTurbine) {
		GDIPowerTurbine = NULL;
	}
	if (target == NodRegularPower) {
		NodRegularPower = NULL;
	}
	if (target == NodAdvancedPower) {
		NodAdvancedPower = NULL;
	}
	if (target == GDIFirestormGenerator) {
		GDIFirestormGenerator = NULL;
	}
	if (target == GDIHunterSeeker) {
		GDIHunterSeeker = NULL;
	}
	if (target == NodHunterSeeker) {
		NodHunterSeeker = NULL;
	}
	if (target == NukeProjectile) {
		NukeProjectile = NULL;
	}
	if (target == NukeDown) {
		NukeDown = NULL;
	}
	if (target == EMPulseProjectile) {
		EMPulseProjectile = NULL;
	}
	if (target == TireVoxelDebris) {
		TireVoxelDebris = NULL;
	}
	if (target == ScrapVoxelDebris) {
		ScrapVoxelDebris = NULL;
	}
	if (target == AtmosphereEntry) {
		AtmosphereEntry = NULL;
	}
	if (target == InfantryExplode) {
		InfantryExplode = NULL;
	}
	if (target == IonBlast) {
		IonBlast = NULL;
	}
	if (target == IonBeam) {
		IonBeam = NULL;
	}
	if (target == Dig) {
		Dig = NULL;
	}
	if (target == FirestormIdleAnim) {
		FirestormIdleAnim = NULL;
	}
	if (target == FirestormAirAnim) {
		FirestormAirAnim = NULL;
	}
	if (target == FirestormActiveAnim) {
		FirestormActiveAnim = NULL;
	}
	if (target == FirestormGroundAnim) {
		FirestormGroundAnim = NULL;
	}
	if (target == BarrelExplode) {
		BarrelExplode = NULL;
	}
	if (target == BarrelParticle) {
		BarrelParticle = NULL;
	}
	if (target == DropPodPuff) {
		DropPodPuff = NULL;
	}
	if (target == IonStormWarhead) {
		IonStormWarhead = NULL;
	}
	if (target == WebbedInfantry) {
		WebbedInfantry = NULL;
	}

	BarrelDebris.Delete((VoxelAnimTypeClass const *)target);

	OnFire.Delete((AnimTypeClass const *)target);

	TreeFire.Delete((AnimTypeClass const *)target);

	SplashList.Delete((AnimTypeClass const *)target);

	Scorches.Delete((SmudgeTypeClass const *)target);
	Scorches1.Delete((SmudgeTypeClass const *)target);
	Scorches2.Delete((SmudgeTypeClass const *)target);
	Scorches3.Delete((SmudgeTypeClass const *)target);
	Scorches4.Delete((SmudgeTypeClass const *)target);

	Craters.Delete((SmudgeTypeClass const *)target);

	HarvesterUnit.Delete((UnitTypeClass const *)target);

	BuildConst.Delete((BuildingTypeClass const *)target);
	BuildPower.Delete((BuildingTypeClass const *)target);
	BuildRefinery.Delete((BuildingTypeClass const *)target);
	BuildBarracks.Delete((BuildingTypeClass const *)target);
	BuildTech.Delete((BuildingTypeClass const *)target);
	BuildWeapons.Delete((BuildingTypeClass const *)target);
	BuildDefense.Delete((BuildingTypeClass const *)target);
	BuildPDefense.Delete((BuildingTypeClass const *)target);
	BuildAA.Delete((BuildingTypeClass const *)target);
	BuildHelipad.Delete((BuildingTypeClass const *)target);
	BuildRadar.Delete((BuildingTypeClass const *)target);

	ConcreteWalls.Delete((BuildingTypeClass const *)target);

	NSGates.Delete((BuildingTypeClass const *)target);
	EWGates.Delete((BuildingTypeClass const *)target);

	PadAircraft.Delete((AircraftTypeClass const *)target);

	DeadBodies.Delete((AnimTypeClass const *)target);

	DropPod.Delete((AnimTypeClass const *)target);

	MetallicDebris.Delete((AnimTypeClass const *)target);

	BridgeExplosions.Delete((AnimTypeClass const *)target);

	HSBuilding.Delete((BuildingTypeClass const *)target);
}


/***********************************************************************************************
 * RulesClass::Objects -- Fetch all the object characteristic values.                          *
 *                                                                                             *
 *    This will parse the specified INI database and fetch all the object characteristic       *
 *    values specified therein.                                                                *
 *                                                                                             *
 * INPUT:   ini   -- Reference to the ini database to scan.                                    *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/10/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool RulesClass::Objects(CCINIClass const & ini)
{
	/*
	**	Fetch the house attribute override values.
	*/
	for (int house = HOUSE_FIRST; house < HouseTypes.Count(); house++) {
		HouseTypes[house]->Read_INI(ini);
	}

	/*
	**	Fetch the game object values from the rules file.
	*/
	for (int index = 0; index < SuperWeaponTypes.Count(); index++) {
		SuperWeaponTypes[index]->Read_INI(ini);
	}

	for (int anim = 0; anim < AnimTypes.Count(); anim++) {
		AnimTypes[anim]->Read_INI(ArtINI);
	}

	int bindex;
	for (bindex = 0; bindex < BuildingTypes.Count(); bindex++) {
		BuildingTypes[bindex]->Read_INI(ini);
	}

	for (int aindex = 0; aindex < AircraftTypes.Count(); aindex++) {
		AircraftTypes[aindex]->Read_INI(ini);
	}

	for (int uindex = 0; uindex < UnitTypes.Count(); uindex++) {
		UnitTypes[uindex]->Read_INI(ini);
	}

	for (int iindex = 0; iindex < InfantryTypes.Count(); iindex++) {
		stricmp(InfantryTypes[iindex]->IniName, "MUTANT"); /// the result of this comparison is discarded
		InfantryTypes[iindex]->Read_INI(ini);
	}

	int windex;
	for (windex = 0; windex < Weapons.Count(); windex++) {
		Weapons[windex]->Read_INI(ini);
	}

	for (int proj = 0; proj < BulletTypes.Count(); proj++) {
		BulletTypes[proj]->Read_INI(ini);
	}

	for (int whead = 0; whead < ::Warheads.Count(); whead++) {
		::Warheads[whead]->Read_INI(ini);
	}

	for (windex = 0; windex < Weapons.Count(); windex++) {
		Weapons[windex]->Init_Max_Speed();
	}

	for (bindex = 0; bindex < BuildingTypes.Count(); bindex++) {
		BuildingTypes[bindex]->Calculate_Base_Defense_Values();
	}

	for (int tindex = 0; tindex < TerrainTypes.Count(); tindex++) {
		TerrainTypes[tindex]->Read_INI(ini);
	}

	for (int sindex = 0; sindex < SmudgeTypes.Count(); sindex++) {
		SmudgeTypes[sindex]->Read_INI(ini);
	}

	for (int oindex = 0; oindex < OverlayTypes.Count(); oindex++) {
		OverlayTypes[oindex]->Read_INI(ini);
	}

	for (int pindex = 0; pindex < ParticleTypes.Count(); pindex++) {
		ParticleTypes[pindex]->Read_INI(ini);
	}

	for (int psindex = 0; psindex < ParticleSystemTypes.Count(); psindex++) {
		ParticleSystemTypes[psindex]->Read_INI(ini);
	}

	for (int vindex = 0; vindex < VoxelAnimTypes.Count(); vindex++) {
		VoxelAnimTypes[vindex]->Read_INI(ini);
	}

	/*
	**	Fetch the mission control values.
	*/
	for (int mission = MISSION_FIRST; mission < MISSION_COUNT; mission++) {
		MissionControlClass * miss = &MissionControl[mission];
		miss->Mission = (MissionType)mission;
		miss->Read_INI(ini);
	}
	return(true);
}


/// <summary>
/// Fetches the identifying checksum of the main rule file.
/// This routine is used when comparing rule versions between machines, so that a
/// multiplayer game can be refused when the players are not running the same rules. The
/// Firestorm rule file is folded into the result whenever that addon is enabled.
/// </summary>
/// <returns>Returns with the unique ID of the rules currently in force.</returns>
int RulesClass::Get_Rule_Unique_ID(void)
{
	int id = RuleINI->Get_Unique_ID();
	if (Addon_Enabled(ADDON_FIRESTORM)) {
		id += FSRuleINI.Get_Unique_ID();
	}
	return(id);
}


/// <summary>
/// Fetches the identifying checksum of the art control file.
/// This routine serves the same purpose as its rule and AI counterparts -- it lets the
/// game tell whether two machines are working from the same art data. The Firestorm art
/// file is folded into the result whenever that addon is enabled.
/// </summary>
/// <returns>Returns with the unique ID of the art rules currently in force.</returns>
int RulesClass::Get_Art_Unique_ID(void)
{
	int id = ArtINI.Get_Unique_ID();
	if (Addon_Enabled(ADDON_FIRESTORM) == true) {
		CCFileClass artfs("ARTFS.INI");
		if (artfs.Is_Available() == true) {
			CCINIClass artfsini;
			artfsini.Load(artfs, false);
			id += artfsini.Get_Unique_ID();
		}
	}
	return(id);
}


/// <summary>
/// Fetches the identifying checksum of the AI control file.
/// This routine is used when comparing rule versions between machines, so that a
/// multiplayer game can be refused when the players are not running the same AI rules.
/// The Firestorm AI file is folded into the result whenever that addon is enabled.
/// </summary>
/// <returns>Returns with the unique ID of the AI rules currently in force.</returns>
int RulesClass::Get_AI_Unique_ID(void)
{
	int id = AIINI.Get_Unique_ID();
	if (Addon_Enabled(ADDON_FIRESTORM)) {
		id += FSAIINI.Get_Unique_ID();
	}
	return(id);
}


/// <summary>
/// Loads the art control file into the art database.
/// This routine throws away whatever art rules were previously loaded and reads ART.INI
/// afresh. The initialization pass calls it before any object type is created, since the
/// type objects take their imagery from this database.
/// </summary>
void RulesClass::Load_Art_INI(void)
{
	ArtINI.Clear();
	CCFileClass art("ART.INI");
	ArtINI.Load(art, false);
}
