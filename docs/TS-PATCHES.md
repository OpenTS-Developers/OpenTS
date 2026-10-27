# ts-patches in OpenTS

[ts-patches](https://github.com/CnCNet/ts-patches) is CnCNet's set of
executable patches for Tiberian Sun 2.03. The CnCNet client, Dawn of the
Tiberium Age (DTA), Twisted Insurrection (TI), Tiberian Odyssey (TO) and other
mods build their game executables from it. This page lists what each patch
does and what OpenTS does in its place, as of ts-patches commit
[`ea81a116`](https://github.com/CnCNet/ts-patches/tree/ea81a1163bd270923a1d96735a278a5631e297aa).

The patches are the work of many people. dkeeton, Rampastring, FunkyFr3sh,
CCHyper, Bittah Commander, Crimsonum, AlexB, ZivDero, E1Elite and hifi made the
most commits. The OpenTS change record for each port credits the author of the
patch it follows.

Each row is one behavior. A file that does several things has a row for each,
and the rows can sit in different sections. Where a patch applied only in some
products, or only when a setting in `SUN.INI` or the client's `SPAWN.INI`
turned it on, the row says so. Patches that ts-patches never built or never
enabled, or built only for debugging, are listed only when OpenTS has acted on
them or still plans to. Build scripts, helper code and patch macros are not
listed.

The **In OpenTS** column starts with one of these words:

| Word | Meaning |
| --- | --- |
| Ported | OpenTS does what the patch does. Any key the patch read keeps its name. |
| Reworked | OpenTS reaches the same result by other means: a different key, a narrower or wider scope, or a fix to the cause the patch worked around. The rest of the cell names the difference. |
| Replaced | A different OpenTS feature makes the patch unnecessary. |

A row's link goes to the change record, or where there is none, to the manual page or source that shows the behavior.

## Crashes and bugs

| ts-patches file | What it did | In OpenTS |
| --- | --- | --- |
| `buildingtype_initialization.asm` | Cleared building animation and voxel pivot fields before the sync checksum read them. | Ported. [building-animation-initialization] |
| `buildlimit_fix.asm` | Stopped counting an object in production twice toward a positive `BuildLimit`, so the full limit could be queued. | Ported. [build-limit-queue-count] |
| `whiteboy_cameo_bugfix.asm` | Refused a 76th sidebar cameo instead of writing past the end of the strip. | Reworked: a strip holds 225 cameos and refuses any past that. [sidebar-strip-capacity] |
| `high_res_crash.asm` | Limited the sidebar to 19 cameo rows at high resolutions to prevent a crash. | Reworked: each strip has 60 slots and lays out no more rows than that. [sidebar-slot-ceiling] |
| `isomappack5_limit_extend.asm` | Enlarged the map terrain decoding buffer from 640×400 to 1024×768. | Reworked: the buffer is sized from the map's own terrain data. [isomappack5-decoding-capacity] |
| `tileset255_bridgerepairfix.asm` | Stopped tile set 255 and later sets from corrupting bridge data, which left some broken bridges unrepairable. | Reworked: the lookup has an entry for every tile set. [tileset-lookup-capacity] |
| `destroytrigger_crash.asm` | Stopped the Destroy Trigger action from reading past the last trigger. | Reworked: triggers and tags are destroyed at the end of the frame. [destroy-trigger-deferred] |
| `minimum_burst.asm` | Read `Burst=0` as 1 to prevent a division by zero. | Ported, for every value below 1. [minimum-weapon-burst] |
| `fix_burst_exploit.asm` | Kept a partly fired burst when the player cleared the target and gave it again. | Reworked: the burst is kept for one reload interval, then starts over. [partial-burst-target-loss] |
| `crate_patches.asm` | Made the armor crate multiply armor by its value instead of dividing by it. | Ported: a value of `2` now halves damage. A value below 1, chosen for the old direction, now increases damage. [armor-crate-multiplier] |
| `no_crate_respawn_with_crates_disabled.asm` | Placed no replacement for a collected crate when the match had crates off. The patch replaced the rules test with the match option. | Reworked: both the match option and the rules' `[MultiplayerDefaults] Crates` must be on. [crate-pickup-replacement-option] |
| `reveal_crate_reshroud.asm` | In TI and TO, stopped a second reveal crate from shrouding the map again for a house that could already see all of it. | Ported. [reveal-crate-idempotent] |
| `fix_allied_decloaking.asm` | Stopped an ally's sensors from revealing cloaked allied structures. | Ported. [allied-sensor-cloaking] |
| `fix_unitclass_can_enter_cell_ignoring_terraintype_immunity.asm` | Stopped vehicles firing forever at `Immune=yes` terrain in their path. | Ported. [immune-terrain-pathing] |
| `carryall_click_under_glitch.asm` | Stopped any other aircraft's destination from holding a landing cell, so an enemy could not block a carryall. | Reworked: another aircraft's destination still blocks the cell for the same house or a mutual ally. [hostile-aircraft-landing-reservations] |
| `fix_depot_explosion_glitch.asm` | Scattered a vehicle that ended a move on a cell it could not occupy, such as a service depot's pad after a stop order, instead of destroying it. | Reworked: the vehicle moves off only when a structure stands in the cell; one stranded on terrain is still destroyed. [depot-vehicle-no-longer-explodes] |
| `hp03.asm` | Stopped a passenger aircraft from failing to land after a move to its own cell. | Ported. [aircraft-landing-move-completion] |
| `airtransport_undeployable_on_helipads.asm` | Stopped aircraft from unloading cargo onto a helipad or other building, where it was lost. | Ported. A passenger that cannot be placed also stays aboard. [aircraft-unload-building-guard] |
| `aircraft_not_reloading_fix.c` | Refused attack orders to a landed aircraft with no ammunition, so it kept reloading. | Ported. [aircraft-zero-ammo-fire-orders] |
| `jj_barracks_glitch_fix.asm` | Refused orders to a new jumpjet until it had cleared the barracks. | Reworked: a jumpjet that flies to its rally point releases the barracks at once. [jumpjet-factory-exit] |
| `harvester_block_ref_exploit.asm` | Required a mutual alliance before a harvester could enter another house's refinery. | Ported. [mutual-refinery-docking] |
| `c4_repairable_fix.asm` | Required `Repairable=yes` for C4 sabotage. | Ported. [c4-repairability] |
| `fix_houseclass_checksws_crash.asm` | Stopped a crash in superweapon checks while a scenario was cleared or restarted. | Ported. [superweapon-teardown-player-guard] |
| `fix_infantryclass_take_damage_null_warhead_crash.asm` | Stopped infantry damage from crashing when there was no warhead. | Ported. [infantry-null-warhead] |
| `voxelanim_damage_bug.asm` | Stopped voxel debris from lowering its type's `Damage=` each time it hit something. | Ported. [voxel-debris-damage-isolation] |
| `voxelanim_damage_bug.asm` | Skipped voxel debris damage when the debris had no warhead. | Ported. [voxel-debris-null-warhead] |
| `ionstorm_jumpjet_crash.asm` | Stopped a crash when an ion storm destroyed a flying jumpjet. | Ported. [ion-storm-jumpjet-destruction] |
| `fix_building_damage_state_crash.asm` | Skipped an animation update that crashed while a building changed damage state. | Reworked: a structure animation replaced by its own damage is freed at the end of the frame instead of at once, so its update no longer reads freed memory. [damaging-building-animation-crash] |
| `IonBlastClass_crash.asm` | Forced a detail level at which the ion blast's warp is not drawn, avoiding its crash. | Reworked: the crash is fixed and the effect draws at every detail level. [ion-blast-edge-sampling], [ion-blast-edge-coverage] |
| `laser_draw_it_crash.asm` | Forced a detail level at which laser waves are not drawn, avoiding their crash. | Reworked: the crash is fixed and the effect draws at every detail level. [laser-wave-edge-writes] |
| `hp03.asm` | Limited the disruptor wave's drawing index to nine to prevent a crash. | Reworked: the tables cover every index, and the crash at the view edge is fixed. [sonic-wave-edge-sampling], [sonic-wave-edge-coverage] |
| `hp03.asm` | Drew the cameo of a factory that a spy infiltrated in the right palette. | Ported. [infiltrated-factory-cameo-palette] |
| `minimap_crash.asm` | Kept an object in limbo off the radar to prevent a crash. | Ported. What leaves the stale radar entry has not been traced. [limbo-radar-tracking] |
| `tiberium_on_slope_crash.asm` | Stopped drawing Tiberium and veins on a cell with any slope other than the four plain ramps. | Reworked: every Tiberium overlay frame is checked against its artwork before drawing. [tiberium-overlay-frame-validation] |
| `tiberium_stuff.asm` | Treated a Tiberium chain reaction with `Power=0` as `Power=1`. | Reworked: `Power=0` stays zero and the chain reaction plays no animation. [zero-power-tiberium-chain-reactions] |
| `tiberium4_blocks_infantry.asm` | Removed hardcoded infantry blocking on overlays 27–38. | Reworked: every Tiberium overlay blocks infantry according to its land type. [tiberium-infantry-passability] |
| `veterancy_from_allies.asm` | Gave no experience for a kill when the killer's house counted the victim as an ally. | Reworked: the victim's house decides whether the killer is an ally. The result differs only for a one-way alliance. [allied-kill-veterancy] |
| `veterancy_crate_check_trainable.asm` | Stopped veterancy crates promoting `Trainable=no` objects. | Ported. [trainable-veterancy-crates] |
| `fix_mission_restart_difficulty_bug.asm` | Kept the chosen difficulty when a mission was restarted. | Ported. [mission-restart-difficulty] |
| `fix_mouse_not_found_error.asm` | Removed the startup error for a missing mouse. | Ported. [mouse-presence-startup-check] |
| `flickering_shadow_fix.asm` | Stopped a vehicle's shadow blinking while the vehicle flashed. | Ported. [flashing-voxel-shadow] |
| `fix_score_logging_typo.asm` | Corrected `Losser` to `Loser` in the score lines of the debug log. | Ported. [mpscore.cpp](../code/mpscore.cpp) |
| `random_loop_delay.asm` | Drew animation loop delays from the synchronized random number generator. | Ported. [anim-loop-delay-sync] |
| `singleplayer_objects_on_multiplayer_map_crash.asm` | Skipped a map's objects and triggers owned by `GDI` or `Nod` outside a campaign. | Reworked: an object or trigger loads only when its owner is a house in the match. [scenario-object-owner-resolution] |
| `multiplayer_units_placing.asm` | Kept a map's vehicle follower links pointing at the right vehicles when starting units were created before the map's were read. | Reworked: follower links are resolved through the map's own `[Units]` rows. [scenario-object-owner-resolution] |
| `spawner/spawner.asm` | Stopped a crash on a map with an empty `[Units]` section after starting units were created. | Reworked: follower links are resolved through the map's own `[Units]` rows, so an empty section links nothing. [scenario-object-owner-resolution] |
| `spawner/spawner.asm` | Let a skirmish end when the remaining houses are allied, ignoring passive houses such as Neutral. | Ported. [skirmish-ends-past-passive-houses] |
| `scriptaction4.asm` | Decoded the Move To Cell team mission with the large-map cell numbering. | Reworked: the numbering follows the map's `NewINIFormat`. [move-to-cell-encoding] |
| `briefing_restate_map_file.asm` | Made Restate read the map's own briefing before the mission file's. | Reworked: Restate shows the briefing the mission started with. [restate-briefing-fallback] |
| `nco_bug_fix.asm` (never built) | Tried to stop "New construction options" repeating. | Reworked: the defect is fixed; a cameo appears only when some structure can build it. [cameo-needs-a-factory] |

## Gameplay

| ts-patches file | What it did | In OpenTS |
| --- | --- | --- |
| `crate_patches.asm` | Let armor and firepower crates upgrade the same object more than once. | Reworked as the options `ArmorCrateStacks` and `FirepowerCrateStacks`, off by default. [stacking-crate-upgrades] |
| `dta_hacks.asm`, `fd_hacks.asm` | Made money crates pay their exact rules value, without random extra cash. | Reworked as the option `CrateMoneyBonus`: set it to `0` for the exact value. [crate-money-bonus] |
| `multiple_factory_hack.asm` | Made the build-time bonus for each extra factory cumulative, as in Red Alert 2. In TI, FD, SD, TSA and the client it also reduced the bonus from a factory cheaper than a built-in cost of 1000 or 2000. | Reworked: Red Alert 2's formula, with Vinifera's `MultipleFactoryCap`. The bonus does not depend on the factory's cost. [multiple-factory-bonus] |
| `multi_engineer_ignore_neutral.asm` | Let one engineer capture a structure of the Neutral house under Multi Engineer. | Ported. [multi-engineer-neutral-capture] |
| `guard_mode_patch.asm` | Made a guarding unit pick a new target instead of chasing one out of range. | Ported. [guard-target-reacquisition] |
| `guard_mode_patch2.asm` | Made Stop put every unit except aircraft on guard where it stands. | Ported. [stop-holds-position] |
| `guard_mode_patch2.asm` | Made Guard hold a unit where it stands instead of at its destination. | Ported. [guard-command-scope] |
| `harvesters_autoharvest.asm` | Sent a new harvester to work when it left the factory. | Ported. [harvester-goes-to-work] |
| `harvesters_guardcommand.asm` | Made Guard send a harvester to work. | Ported. [guard-command-scope] |
| `smarter_harvesters.asm` | Made a harvester weigh busy and free refineries across the whole `Dock=` list. | Reworked: a harvester weighs the trip and the wait at each refinery. [refinery-by-trip-cost] |
| `smarter_harvesters.asm` | Sent a harvester back to harvesting when the refinery it was entering let it go. | Reworked: weeders do the same. [harvester-refinery-lost-on-the-way] |
| `aircraft_repair.asm` | Let any building with aircraft repair repair aircraft. | Ported. [aircraft-repair-capability] |
| `manual_aim_sams.asm` | With `AimableSams` in the launch file, let the player aim an anti-air defense. | Reworked: the player can always aim an anti-air defense at an aircraft in range. [aimable-anti-air-defenses] |
| `allow_crate_overlay_in_multiplayer.asm` | In DTA and Rubicon, kept crates a map places in multiplayer games. | Ported. The match's Crates option controls random crates only. [map-crates-in-every-game-type] |
| `attack_neutral_units.asm` | Stopped every house targeting a building whose weapon has no range. | Reworked: only a player's units outside a team pass it over, as they already did an unarmed building. [attack-neutral-units] |
| `spy_fix.asm` | Kept spy effects per player instead of per country. | Ported. [spy-marks-its-own-house] |
| `spy_fix.asm` | Let computer players see through disguises. | Reworked: `AIDetectDisguise` for computer players and `DetectDisguise=` per type, both off by default. [disguise-detection] |
| `no_charge_power_needed.asm` | Let `Charges=yes` weapons fire on low power. | Ported. A `Powered=yes` structure that draws power still stops, as it did under the patch. [charging-defenses-through-low-power] |

## AI

| ts-patches file | What it did | In OpenTS |
| --- | --- | --- |
| `fix_100_unit_bug.asm` | Raised the AI's limit of 100 unit and infantry types to 1000. | Reworked: the AI has no limit on the number of types, aircraft included. [ai-production-type-capacity] |
| `fix_ai_retaliating_against_its_own_stuff.asm` | Stopped an AI team retaliating against an ally and stalling. | Ported. [allied-team-retaliation] |
| `fix_ai_unit_scatter_for_factories_without_weaponsfactory.asm` | Made AI vehicles leave a factory with `WeaponsFactory=no`. | Ported. [ai-vehicle-factory-exit-navigation] |
| `spawner/spawner.asm` | Made a computer player skip its allies when picking its first enemy. | Ported. [first-enemy-skips-allies] |
| `multiplayer_ai_base_nodes.asm`, `spawner/spawner.asm` | With `UseMPAIBaseNodes` in the launch file, let computer players build from the base nodes of their start position, as in a campaign. | Reworked: the map turns this on with `[Basic] UseMPAIBaseNodes`. [spawn-house-base-nodes] |
| `smarter_firesale.asm` | Made a computer house with 8 or more units, counting each structure as two, keep one building while selling the rest. It was meant for Short Game but applied in every game. | Reworked: Short Game skirmish and multiplayer only, with the thresholds in `[AI]` `FireSaleKeepThreshold=` and `FireSaleStructureWeight=`. The structure kept is one the sale would take down that keeps the house in the match, so never a deployed vehicle, and none is kept when an unsellable structure already stands. [short-game-fire-sale-keeps-a-structure] |

## More than two sides

| ts-patches file | What it did | In OpenTS |
| --- | --- | --- |
| `buildconst_harvesterunit_baseunit.asm` | Read every entry of `BuildConst`, `BuildRefinery`, `BuildWeapons` and `HarvesterUnit`, not only the first. | Ported. [construction-yard-whole-list], [economy-counts-whole-lists], [factory-ownership-whole-list], [pad-aircraft-whole-list], [crate-rescue-whole-lists], [harvester-truce-consistency] |
| `buildconst_harvesterunit_baseunit.asm` | Chose each house's harvester, refinery and war factory by its country, and replaced `BaseUnit` with the house's first `Harvester=no` entry of `HarvesterUnit`. | Reworked: `BaseUnit` is its own list, and every such list is read for the country a house acts as. [base-unit-per-country], [role-lists-per-country], [house-acts-for-itself] |
| `buildconst_harvesterunit_baseunit.asm` | Made Center Base find a yard built from any base unit. | Reworked: Center Base finds any `BuildConst` building, preferring the primary one. [center-base-buildconst] |
| `sideindex_improvements_v2.asm` | Gave every country its own `ActsLike` index, except `Neutral` and `Special`. | Reworked: a house acts for its own country, and `[Sides]` declares every side. [house-acts-for-itself], [sides-declared-in-side-list], [side-base-building], [side-factory-prerequisite-planning], [lobby-side-entries-by-country], [ai-player-country-from-roster] |
| `sideindex_improvements_v2.asm` | Required a construction yard of one of a structure's `Owner=` countries, even when the list names several. | Reworked: `MultiMCV` lets any yard build for its owner. [sidebar-gated-by-yard-country], [multi-mcv-option] |
| `sideindex_improvements_v2.asm` | Made an AI trigger's side field `N` match a house whose `ActsLike` is `N−1`, with 0 matching all. | Reworked: the field is a position in `[Sides]`. [ai-trigger-side-by-position] |
| Every product's `*_hacks.asm`, `spawner/spawner.asm` | Kept the player's country, not only GDI or Nod, when loading sidebar and speech archives. `SidebarHack` in the launch file did the same for the client's side value. | Reworked: the player's side comes from their country. [player-side-from-country], [side-roster-before-a-game] |
| `savegame.asm` | Kept the player's GDI or Nod presentation across a save and load. | Reworked: the save records the player's country and its side. [player-side-from-country] |

## Modding options

| ts-patches file | What it did | In OpenTS |
| --- | --- | --- |
| `add_animation_to_factories_without_weaponsfactory.asm` | Played `ProductionAnim` on vehicle factories without `WeaponsFactory=yes`. | Reworked: it plays on every factory, barracks included. [production-anim-any-factory] |
| `allow_building_placement_over_overlay.asm` | In DTA, let buildings be placed over `Insignificant=yes` overlays, and made overlays default to `Insignificant=no`. | Reworked: the overlay key is `BuildableOver=`. Walls still block. [buildable-overlays] |
| `prerequisites_refresh.asm` (never built) | Removed a cameo from the sidebar when its prerequisite was lost. | Reworked: behind `RecheckPrerequisites=`, off by default, and production of the removed item is cancelled. [recheck-prerequisites] |
| `freeunit_enhancements.asm` | Let `FreeUnit=` name infantry or an aircraft, and placed a free aircraft on its pad. | Reworked: `FreeUnit=` finds its type among vehicles, infantry and aircraft. [free-unit-any-kind] |
| `freeunit_enhancements.asm` | In every build except the unnamed default build, refunded a sold object from its type's `Cost=` without the owner's cost modifiers, and always gave a building its free unit. | Reworked: the free unit is always given, as in Vinifera. A sale keeps the refund with the owner's multipliers, and `Soylent=` sets a fixed one. [free-unit-always-given], [soylent] |
| `horv_via_undeploysinto.asm` | Took a harvester's unloading artwork from its `UndeploysInto=`. | Reworked: the key is `UnloadingClass=`. [unloading-class] |
| `mechanics.asm` | Added `Mechanic=` infantry that repair vehicles and aircraft, reading it in place of `Thief=`. | Reworked: `Mechanic=` and `OmniHealer=` on any type, as in Vinifera. `Thief=` is still read. [healer-target-kinds] |
| `oil_derricks.asm` | Made a building pay `ProduceCashAmount` credits every 180 frames. The value was stored where `ICBMLauncher=` was, so that key stopped working. | Reworked: Vinifera's `ProduceCash` keys, counted per building. `ICBMLauncher=` still works. [produce-cash-buildings] |
| `vehicle_transports.asm` | In DTA, FD and SD, let transports with `IsVehicleTransport=yes` carry vehicles. | Reworked: with Red Alert 2's `Size=` and `SizeLimit=`. [vehicle-transports] |
| `cloakstop_to_toobigforcarryalls.asm` | Reused `CloakStop=` as `TooBigForCarryalls=`. | Reworked: the key is `Totable=`, and `CloakStop=` keeps its meaning. [totable] |
| `cloakstop_to_empimmunity.asm` | In TI, reused `CloakStop=` as `EMPImmune=`. | Reworked: the key is `ImmuneToEMP=`, and `CloakStop=` keeps its meaning. [emp-immunity] |
| `remove_iscoredefender_emp_immunity.asm` | In DTA and TO, removed the Core Defender's EMP immunity. | Reworked: set `ImmuneToEMP=no` on it. [emp-immunity] |
| `max_pip_counts.asm` | Raised the longest ammunition and passenger pip rows from 5 pips to 20. | Reworked: `MaxPips=` on each type sets any pip row's length. [pip-row-length] |
| `move_team_group_number.asm`, `team_number_position.asm` | Moved team numbers higher, and in TI to the object's top right, so they cleared the pips. | Reworked: eight `UI.INI` `[Pips]` offsets with Vinifera's key names, defaulting to the original position. [group-number-offsets] |
| `scrap_metal_explosion.asm` | With `ScrapMetal` in the launch file, played `ScrapExplosion=` in place of `Explosion=`. A type without `ScrapExplosion=` then had no death animation. | Reworked: a type without `ScrapExplosion=` keeps its `Explosion=`, and a scenario's `[SpecialFlags]` can also turn it on. [scrap-debris] |
| `unit_self_heal_repair_step.asm` | Made the self-heal step `[General] UnitSelfHealRepairStep`. | Reworked: Vinifera's self-heal step, interval and cap keys. The ts-patches key is not read. [self-healing-settings] |
| `rules_process.asm` | Created the weapons listed in `[Weapons]` before other types, and read `[Maximums]` from every rules file and map. | Ported for `[Weapons]`. `[Maximums]` is read from `rules.ini` only. [weapons-registration-list] |
| `tiberium_damage.asm`, `vinifera_unhardcode.asm` | Removed the hardcoded `Power=17` of the Tiberium type named `Vinifera`. TI's `vinifera_unhardcode.asm` never ran, because the change in `tiberium_damage.asm` already stopped the name matching. | Ported. In stock rules that type now deals the `Power=20` its section states. [vinifera-tiberium-power] |
| `trigger_actions_extended.asm` | Added trigger actions 106–108 and 110–117: give credits, Short Game on and off, destroy a house, make attached objects elite, Ally Reveal on and off, create an autosave, remove attached objects, a mission for all units, and a one-way alliance. | Ported with the same numbers. Give Credits no longer takes a house below zero, and Create Autosave also saves when timed autosaves are off. [cncnet-trigger-actions] |
| `text_triggers.c` | Let a map started by the client replace or add tutorial text lines. | Reworked: every scenario can, and the lines last for that mission only. [map-local-tutorial-text] |
| `ts_hacks.asm`, `tsa_hacks.asm`, `ti_hacks.asm`, `to_hacks.asm`, `rubicon_hacks.asm`, `fd_hacks.asm`, `sd_hacks.asm` | Removed the hardcoded guard range and cost of `NAWALL` and `GAWALL`. | Reworked: these types are read from the rules like any other. [type-overrides-from-rules] |
| `ts_hacks.asm`, `tsa_hacks.asm`, `rubicon_hacks.asm`, `fd_hacks.asm`, `sd_hacks.asm` | Removed the hardcoded values for `GAFSDF`, `HMEC`, `155mm` and `ARTYHE`. | Reworked: these types are read from the rules like any other. [type-overrides-from-rules], [multiplayer-weapon-overrides-from-rules] |
| Every product's `*_hacks.asm` | Gave Tiberium on `Image=2` twelve growth stages instead of one. | Ported. [tiberium-overlay-variety-bounds] |
| `dta_hacks.asm`, `to_hacks.asm` | Drew SHP vehicles with `Facings=32` at 32 facings, and every other count, 8 included, at one facing. | Reworked: SHP vehicles may have 8, 16, 32 or 64 facings. [more-shape-facings] |

## Controls and interface

| ts-patches file | What it did | In OpenTS |
| --- | --- | --- |
| `factory_rallypoints.asm` | Set a rally point with a plain click. `AltToRally` restored the old modifier. | Ported, with the same `AltToRally` setting. [rally-point-on-a-plain-click] |
| `alt_to_undeploy.asm` | With `MoveToUndeploy=no` in `SUN.INI`, required Alt to move or undeploy a building. | Reworked: a building with a rally point moves or undeploys on the force-move key. There is no setting. [rally-point-on-a-plain-click] |
| `alt_scout_fix.asm` | Refused a rally point in the shroud for every factory except a barracks. | Reworked: a unit that cannot enter the shroud ignores a rally point that is still shrouded when it leaves the factory. [rally-point-into-shroud] |
| `no_guard_cursor_for_repair_vehicles.asm` | Showed the deploy cursor instead of the guard cursor on a repair vehicle pointing at itself, which also took the guard cursor from repair vehicles that cannot deploy. | Reworked: only a repair vehicle that can deploy is affected. [deploy-cursor-on-a-repair-vehicle] |
| `no_insignificant_death_announcement.asm` | Stopped loss announcements for insignificant units. | Ported. [insignificant-unit-loss-announcement] |
| `add_team_better.asm` | With `AddTeamStyle2=yes` in `SUN.INI`, made Shift and a number put the whole selection into that team. | Reworked as ten Add To Team commands, unbound by default. [add-selection-to-team] |
| `center_team.asm` | With `DoubleTapInterval` set in `SUN.INI`, centered the view on a team when its key was pressed twice within that many frames. It was off by default. | Reworked: on for every player, with a half-second window and no setting. [team-double-tap-centre] |
| `draw_all_action_lines.asm` | Drew a line to every queued destination, and kept the lines up while the queue-move key was held. | Reworked: also styled by `UI.INI`, with Vinifera's key names. [action-lines-and-ui-ini] |
| `waypoint_enhancements.asm` | Closed a waypoint loop without Shift. | Ported. [waypoint-loop-plain-click] |
| `waypoint_enhancements.asm` | Made Delete Waypoint take a path apart from its end. | Ported. [waypoint-delete-from-end] |
| `place_building_hotkey.c` | Added a `PlaceBuilding` command that picks up a finished building for placement. | Reworked: the command is named `ManualPlace`, as in Vinifera, so a `PlaceBuilding=` line in `KEYBOARD.INI` does not bind it. [place-building-command] |
| `repeat_last_building_hotkey.c` | Added a `RepeatBuilding` command that queued the last building started from the sidebar again. | Reworked as four commands, one each for structures, infantry, vehicles and aircraft, that repeat the last item to finish. The `RepeatBuilding` name does not bind. [repeat-last-production] |
| `radar_event_hacks.asm` | Stopped a new unit leaving its factory from moving the Goto Radar Event location. | Ported. [radar-event-ignores-production] |
| `hotkeys.c` | Added a command that removes one object from the selection. | Ported. [selection-filter-commands] |
| `in-game_message_background.asm`, `hotkeys.c` | Drew a `TextBackgroundColor` background behind messages, off by default, with a command to toggle it. | Reworked: the same setting with no toggle command, and it defaults to `0`, no background, as in the unpatched game. [chat-echo-and-background] |
| `hover_show_health.asm` | Showed the health of the object under the pointer unless `HoverShowHealth=no`, and showed rank and the healer's cross without a selection. | Reworked: always shown; `HoverShowHealth` is not read. [hover-and-always-on-indicators] |
| `autosave.c` | Showed "Game Saved." after a multiplayer save, in builds without the client launcher. | Reworked: every save the player asks for reports "Game saved." [save-confirmation-message] |
| `mouse_behavior.asm` | Made the drag distance that starts a selection band the `DragDistance` setting. | Reworked: the distance is the Windows drag setting. [band-select-drag-threshold] |
| `mouse_always_in_focus.asm` | With `MouseAlwaysInFocus`, kept the game running, with sound and input, when its window lost focus. | Reworked: `SimulateWhileUnfocused` keeps a solo game running, silently and without input. [simulate-while-unfocused] |
| `disable_edge_scrolling.asm` | Added `DisableEdgeScrolling` to turn edge scrolling off. | Reworked: `AutoScroll=no`, also in the controls dialog. [edge-scrolling-option] |
| `no_options_menu_animation.asm` | Removed the wipe-open animation of in-game dialogs. | Reworked: the dialogs are UI documents, and a mod's document with `reveal="none"` on its body opens at once, without the slide or its sound. [UI files](../manual/content/systems/ui-files.md) |
| `main_menu_cursor_bug.asm` | Stopped the graphical main menu hiding and showing the cursor every frame, which slowed it down under Wine. | Ported. The pointer is drawn by the system, so the calls did nothing but cost time. [grphmenu.cpp](../code/grphmenu.cpp) |
| `scrollrate_fix.asm` | Limited scrolling speed with `ScrollDelay`. | Replaced: scrolling runs at one speed on every machine. [frame-rate-independent-scrolling] |
| `sidebar_cameo_sort.asm` | Sorted sidebar cameos by category and rules order. | Reworked, with `SidebarSorting=`, `CameoSortOrder=` and `SortCameoAsBaseDefense=`. [sidebar-cameo-ordering] |
| `briefing_screen_mission_start.asm` | Showed the difficulty when a campaign mission started. A launch file's `DifficultyName` named it. | Ported. [mission-difficulty-message] |
| `briefing_screen_mission_start.asm` | Showed the written briefing at every campaign mission start. | Reworked: the written briefing appears when no briefing movie plays. [mission-start-briefing] |
| `basic_theme_fix.asm` | Returned to the normal music after a map's `Theme` played. | Ported. [scenario-theme-rotation] |

## Multiplayer and launcher

| ts-patches file | What it did | In OpenTS |
| --- | --- | --- |
| `spawner/spawner.asm` | Started a game from the client's `SPAWN.INI` with `-SPAWN`, with the players, sides, colors, handicaps and alliances it names, and skipped the startup movies. | Reworked, reading the same file. OpenTS refuses a file it cannot play instead of falling back to the menu. [client-driven-launch] |
| `spawner/spawner.asm` | Showed the loading picture the client named in `CustomLoadScreen`. | Ported, with `CustomLoadScreenPos` for the bars, and kept in saves. [client-load-screen], [launch-load-screen-kept-in-save] |
| `spawner/spawner.asm` | With `DifficultyBasedAINames`, named computer players Hard AI through Ultimate AI by their difficulty. | Reworked: three names, Hard AI, Medium AI and Easy AI. [client-driven-launch] |
| `spawner/spawner.asm` | In DTA, set the scenario's global flags from the launch file's `[GlobalFlags]` at each mission start. | Reworked: every build reads them for a campaign launch. [client-driven-launch] |
| `spawner/spawner.asm` | Wrote `mpstats.txt` after every game. | Reworked: written after every game against other machines. |
| `spawner/statistics.asm` | Wrote each network match's statistics to `stats.dmp` for the client, with spectator, alliance, start position and map hash fields. | Reworked: written only when the launch file sets `WriteStatistics`, with observers and the alliance field left out and the start position each player began at. [match-statistics-file] |
| `spawner/selectable_spawns.asm` | Let players choose start positions. | Reworked. [client-driven-launch] |
| `spawner/nethack.c` | Sent game traffic to the client's peers and through CnCNet tunnels. | Reworked. [client-driven-launch] |
| `spawner/auto_ally_by_spawn_loc.c` | Allied every house in each group of start positions, from a preset in the map that the client chose. | Reworked: a map writes `Allies=` in `[Spawn1]`–`[Spawn8]`; the client does not choose a preset. [spawn-house-alliances] |
| `spawner/build_off_ally.asm` | Let a player build next to a mutual ally's base. | Ported. [build-off-ally] |
| `spawner/spectators.asm` | Added spectators who see the whole map. | Reworked as observers. [observer-seats-and-coach-mode] |
| `coach_mode.c` | Let a defeated player see what their allies see. | Ported. [observer-seats-and-coach-mode] |
| `shared_control.c` | Was meant to let a player hand control of their units to an ally with a `GrantControl` command, when the launch file set `SharedControl`. It could not be switched on in any build. | Reworked: every machine checks a grant before it runs an order, a saved game keeps grants, and a grant ends when the alliance breaks or either player is defeated or leaves. All of a unit's orders are covered, including the infantry abilities the patch left to the owner. [shared-control] |
| `spawner/protocol_zero.asm`, `spawner/protocol_zero_c.c` | Adjusted network timing to the connection. | Replaced by OpenTS's own adaptive timing. [adaptive-network-timing] |
| `gamespeed.asm` | Paced campaigns and skirmishes by the speed setting, as network games are paced. | Ported, with Fastest uncapped. [solo-pacing-matches-network] |
| `mpdebug.asm` (debug builds only), `hotkeys.c` | Let the `-MPDEBUG` network overlay run at any resolution, with a command to toggle it. | Reworked: the overlay draws along the bottom of the screen at any resolution. There is no toggle command. |
| `short_connection_timeout.asm` | Cut the wait for a player stuck on the loading screen from 60 to 40 seconds. | Reworked: the launch file sets `ConnTimeout` and `ReconnectTimeout`. [disconnect-and-reconnect-policy] |
| `recon_kick.asm` | With three or more players, removed an out-of-sync player and played on instead of ending the game. | Replaced by the out-of-sync dialog. [out-of-sync-recovery] |
| `rage_quit.asm` | Made closing the window or pressing Alt+F4 during a match resign. | Ported. [disconnect-and-reconnect-policy] |
| `spawner/auto-surrender.asm` | Destroyed the base of a player who left instead of handing it to the computer, and made Abort surrender first. | Reworked: `AutoSurrender` decides what happens to a departed player's base, and Abort leaves without surrendering. [disconnect-and-reconnect-policy] |
| `dont_replace_player_name_with_computer.asm` | Kept a player's name when the computer took over their house, while `AutoSurrender` was on. | Reworked: the name is kept whenever the computer takes over. [disconnect-and-reconnect-policy] |
| `dont_save_without_all_players.asm`, `recon_kick.asm` | Refused to save once a player had left. | Reworked: once a player leaves, saving is off for the rest of the match. [Saved games](../manual/content/formats/save-games.md) |
| `dont_save_without_all_players.asm`, `Hook_Main_Loop.c` | Wrote a multiplayer save at the end of the frame instead of partway through it. | Ported. [multiplayer-save-frame-boundary] |
| `spawner/random_map.asm` | Generated a random map when the map file asked for one. | Ported. [map-file-random-map] |
| `internet_cncnet.asm` | Replaced the Westwood Online menu entry with a message pointing to CnCNet. | Replaced: Westwood Online is removed. [retire-westwood-online] |
| `hack_house_from_house_type.asm`, `trigger_actions_extended.asm` | Let triggers name the houses at start positions 1–8 as houses 50–57. A position nobody held named the Neutral house. | Reworked: a position nobody holds names no house, so its triggers and teams are dropped, and house 58 no longer reads past the list. [spawn-houses] |
| `multiplayer_units_placing.asm` | Let a map give preplaced objects to the houses at start positions, and loaded structures owned by the local player. | Ported. [spawn-houses], [multiplayer-player-structures] |
| `reinforcements_player_specific.asm` | Let a map give teams and reinforcements to the houses at start positions. | Ported. [spawn-houses] |
| `attack_neutral_units.asm` | Added `AttackNeutralUnits` to let units target armed neutral objects. | Reworked: the option lets units target any neutral object. [attack-neutral-units] |
| `auto_deploy_mcv.asm` | Added `AutoDeployMCV` to deploy starting MCVs. | Ported. [auto-deploy-mcv] |
| `harvester_truce.asm` | Added `HarvesterTruce` to the launch file, protecting harvesters in any game the client started. | Reworked: `HarvesterTruce` applies only in a game against other machines. [Launch file](../manual/content/formats/spawn-ini.md), [harvester-truce-consistency] |
| `hotkeys.c`, `chatallies.asm` | Added `ChatToAllies` and `ChatToAll` commands, bound to Backspace and Enter when those keys were free. A spectator's ally line went to the other spectators, and only the sender filtered recipients. | Reworked: the kind of line travels with the message and the receiver checks it again. The Backspace and Enter defaults are kept. [typed-chat-recipients] |
| `display_messages_typed_by_yourself.asm` | Showed the sender their own chat line. | Ported. [chat-echo-and-background] |
| `hide_names_qm.asm` | Hid player names on the radar list and loading screen in Quick Match. | Reworked: every place that names a player shows "Player N". [quickmatch-hides-player-names] |
| `skip_score.c` | Skipped the score screen when the client's launch file or the player's `SUN.INI` set `SkipScoreScreen`. | Reworked: only the launch file's `SkipScoreScreen` is read. [client-score-screen] |
| `multiplayer_movies.asm`, `multiplayer_units_placing.asm` | Played mission movies in multiplayer. With `PlayMoviesInMultiplayer`, it also played the win and loss movies and kept the connection alive during them. | Reworked: movies play outside a campaign only with `PlayMoviesInMultiplayer`, and a full-screen movie ends once every player has pressed Escape. [multiplayer-movies] |
| `log_more_oos.c`, `record_rng_func.asm`, `record_facing_changes_func.asm`, `record_tarcom_changes_func.asm`, `record_override_mission_func.asm`, `record_anim_constructor_func.asm` | Added recent events, random draws, facing changes, target changes, mission overrides and animation creations to the out-of-sync file, and named every computer player "Computer" in it. | Reworked as the out-of-sync report. Computer players keep their names. [out-of-sync-report] |

## Files, graphics and platform

| ts-patches file | What it did | In OpenTS |
| --- | --- | --- |
| `saved_games_in_subdir.asm` | Kept saved games in a `Saved Games` folder. | Ported. The game creates the folder, and it follows the user data directory when one is named. [saved-games-folder] |
| `Hook_Main_Loop.c`, `savegame.asm` | Saved campaigns automatically in five rotating slots, and had the host save network games, at the interval the launch file's `AutoSaveGame` set. | Reworked: skirmish games save automatically too, and a network game saves on every machine at the same frame. [autosave] |
| `screenshots_in_subdir.asm` | Kept screenshots in a `Screenshots` folder. | Ported. [screenshots-folder] |
| `write_jpg_png.c`, `spawner/auto_ss.asm` | With `UsePNG`, saved screenshots as PNG. | Reworked: screenshots are always PNG. [screenshots-folder] |
| `scale_movie_fix.c`, `scale_movie_fix_hack.asm` | Scaled movies to fit the screen without distorting them. | Reworked. [movie-aspect-fitting] |
| `load_more_movies.asm` | Loaded movie archives up to `MOVIES03.MIX`. | Reworked: every `MOVIES*.MIX` in any searched folder is loaded. [deployment-search-folders] |
| `disable_file_checks.asm` | Started without the map and multiplayer archives. | Ported. [optional-map-and-multiplayer-archives] |
| `no_movie_and_score_mix_dependency.asm`, `disable_file_checks.asm`, `dta_hacks.asm`, `ti_hacks.asm`, `to_hacks.asm` | Started without `SCORES.MIX` and the movie archives. | Ported. [optional-score-and-movie-archives] |
| `no_sidecd_mix.asm` | Mounted the side CD archive in every game type, and started a campaign without it. | Reworked: a campaign starts without it, and it is still mounted only in campaigns. The `SIDEC` and `SIDENC` archives serve every game type. [optional-side-cd-archive] |
| `new_search_dir.asm` | Added `MIX` and `INI` folders to the file search. | Reworked: a deployment names its folders in `OPENTS.INI`. [deployment-search-folders], [game-data-directory] |
| `dta_hacks.asm`, `ti_hacks.asm`, `to_hacks.asm` | Renamed the settings file and the expansion rules, art, AI, battle and sound files, and in DTA the startup palettes. | Reworked: a deployment names these files in `OPENTS.INI`. [deployment-file-names] |
| `dta_hacks.asm`, `ti_hacks.asm`, `to_hacks.asm` | Renamed theaters and their archive roots. | Replaced: a mod declares its theaters in `[Theaters]`. [theaters-declared-in-rules] |
| `dta_hacks.asm`, `ti_hacks.asm`, `to_hacks.asm` | Renamed the sound, side, movie, menu and multiplayer map archives and lists. | Replaced: a deployment ships these files under the stock names. Every `*.PKT` and `MOVIES*.MIX` is read. |
| `no-cd_iran.asm`, `dta_hacks.asm`, and `no-cd_nyer.asm` and `no-cd_tfd.asm` (never built) | Removed the CD check, and in DTA the CD choice when a multiplayer map started. | Replaced: OpenTS has no CD check. [cd-search-path] |
| `no_blowfish_dll.asm` | Hid the error about the missing `BLOWFISH.DLL`. | Replaced: Blowfish is built into OpenTS. [blowfish-in-binary] |
| `force_firestorm_installed.asm`, `new_search_dir.asm` | Forced Firestorm to count as installed. | Replaced: Firestorm counts as installed wherever `FIRESTRM.INI` is found. [firestorm-detected-by-rules-file] |
| `video_mode_hacks.asm`, `remove_16bit_windowed_check.asm`, `graphics_patch.asm`, `force_conversion_type.c` | Kept the game's resolution in the menus and after a match, and worked around DirectDraw color-depth and surface limits. | Replaced: OpenTS draws through bgfx. [bgfx-presenter] |
| `disable_max_windowed_mode.c`, `win8_compat-func.c` | Set DirectDraw compatibility switches. | Replaced: OpenTS draws through bgfx. [bgfx-presenter] |
| `disable_dpi_scaling.c` | With `DisableHighDpiScaling`, declared the game DPI-aware to Windows. | Reworked: the game always declares itself DPI-aware. [bgfx-presenter] |
| `online_optimizations.asm` | Made the mouse cursor timer's interval configurable. | Replaced: OpenTS has no mouse timer. |
| `fix_ear_blast.asm` | With `DisableScoreScreenAudio`, on by default, shut down all sound when DTA's multiplayer score screen opened, to avoid a DirectSound defect. The score graph's sound was dropped either way. | Replaced: OpenTS plays sound through its own audio engine. [audio-engine] |
| `debugging_help.asm` (debug builds only) | Let several copies of the game run at once. | Reworked: the `-MULTIINSTANCE` launch option allows it in every build. [multiple-instances-option] |
| `logger.asm` | Wrote a log file for each product. | Replaced by the OpenTS debug log. [debug-log-console] |

## Not ported

| ts-patches file | What it did | Why not |
| --- | --- | --- |
| `exception_catch.c` | Resumed the game after 15 known crashes. | OpenTS writes a crash report instead, and each crash is fixed at its cause. [exception-handler] |
| `multi_engineer_ignore_neutral.asm` | Applied the Multi Engineer option in campaigns too. | Multi Engineer keeps its multiplayer scope. [multi-engineer-neutral-capture] |
| `trigger_actions_extended.asm` | Added trigger action 109, which printed the difficulty. | 109 creates a building, as in Vinifera, so a map that uses the old action places a building instead. [cncnet-trigger-actions] |
| `paradrop.asm` | Gave an aircraft carrying cargo ammunition of its cargo divided by five, rounded up, and stopped marking unselectable, unlandable or camera aircraft as loaners. | Paradrops will get a proper implementation. |
| `tiberium_damage.asm` | Changed the damage infantry take in Tiberium to `Power` × 5 in DTA and `Power` ÷ 2 elsewhere, with `Power=0` dealing none. | Balance belongs in rules data. OpenTS deals `Power` ÷ 10, at least 1. |
| `remove_ion_storm_effects.asm` | Removed every ion storm effect in DTA. | Too broad for an engine change. Each effect could become a setting on request. |
| `change_projectile_degeneration_speed.asm` | Changed how fast DTA projectiles lose strength. | Balance belongs in rules data. |
| `change_score_screen_music.asm` | Chose DTA score music by side. | Belongs in mod data. |
| Every product's `*_hacks.asm` | Changed the default `WalkFrames` from 12 to 1. | A mod can set `WalkFrames=` on each type. |
| Every product's `*_hacks.asm` | Stored the Z value of a vehicle's `HalfDamageSmokeLocation=` over its `Jellyfish`, `LimpetDrone`, `MobileEMP` and `CoreDefender` flags, by mistake. | A defect. OpenTS reads the smoke location and each flag separately. |
| `vehicle_transports.asm` | Turned `DoubleOwned=` off in DTA, FD and SD, to store `IsVehicleTransport=` in its place. | `IsVehicleTransport=` has its own field, so `DoubleOwned=` keeps working. |
| `spawner/spawner.asm` | Stopped computer players turning on humans who allied, in every game. | Set `[AI] Paranoid=no` in the rules for the same result. |
| `record_rng_func.asm` | With `NoRNG`, made every random draw from frame 10 on return zero or the bottom of its range. | It changes the game instead of diagnosing it. The out-of-sync report records random draws. |
| `new_armor_types.c`, `new_armor_types_s.asm` (never built) | Experimented with 64 armor types. | Custom armor types will follow Vinifera's design. |
| `binkmovie/*` | Played a `.BIK` in place of a `.VQA` when `BinkW32.dll` and the file were present: full-screen in campaigns, on the radar anywhere, scaled by `StretchMovies`, and paused when the game lost focus. | OpenTS will not load the old Bink library. |
| `single-proc-affinity.c` | Ran the game on one processor core. | It worked around DirectDraw timing, which OpenTS no longer uses. |
| `disable_alt_tab.c` | With `DisableAltTab`, blocked Alt+Tab, Alt+Esc, Ctrl+Esc and the Windows keys system-wide. | It changes input for the whole system. |
| `spawner/auto_ss.asm`, `Hook_Main_Loop.c` | Took screenshots automatically in network games. | Nothing in OpenTS uses them, and it saves pictures of the player's screen unasked. |
| `no_window_frame.asm` | Ran windowed mode without a frame, and a desktop-sized window always. | Not planned. Fullscreen is already borderless. |
| `mumblelink.c`, `spawner/auto_ally_by_spawn_loc.c` | Told Mumble the player's team. | Not planned. |
| `override_colors.c` | Let a player replace the eight multiplayer colors on their own machine. | Not planned. The `[Colors]` list covers most uses. |
| `debugging_help.asm` (debug builds only) | Made a LAN game wait without limit for a player who stopped responding. | `ReconnectTimeout` in the launch file sets the wait, up to 9.6 minutes. [disconnect-and-reconnect-policy] |
| `spawner/spawner.asm` | Let the launch file's `MultipleFactory` override the rules. DTA's lobby sets it for its Multiple Factory Bonus option. | A client sets `[General] MultipleFactory=` in the map it writes, which OpenTS reads like the rules. |
| `binkmovie/bink_asm_patches.asm` | Stopped a window repaint during a mission from drawing the game over a full-screen or radar movie, VQA included. | Not needed: in an OpenTS windowed mission, a movie pauses while the window is moved or loses focus, and no game view appears over it. Clicking back in shows a brief black frame. |
| `laser_draw_it_crash.asm` | Drew laser beams with their glow at the lowest detail level, as a side effect of forcing the detail level to avoid a crash. | OpenTS fixed the crash, so the lowest detail level keeps its plain line. [laser-wave-edge-writes] |
| `radar_event_hacks.asm` | Gave spectators every house's base-under-attack alert. | Not planned. |
| `hotkeys.c` (debug builds only) | Added a `MapSnapshot` command that saved the current game as a map file. | Not planned. |
| `spawn_arg_check.asm` | Refused to run without `-SPAWN`. | Launch requirements belong to the client. |
| `dta_hacks.asm`, `ti_hacks.asm`, `to_hacks.asm` | Started the AI's starting units on Unload instead of Area Guard. | Area Guard already puts them to work: starting harvesters switch to harvesting, and combat units defend their start. Unload would leave combat units unable to fight back until a team recruits them, and would fire a starting mobile EMP. |
| `disable_intro_movie.asm` | Never played `INTRO.VQA`, the animated introduction before a campaign's first mission and from the main menu. | The deployment decides by the files it ships: the game plays `INTR<n>.VQA` for the campaign's `CD=` number, falls back to `INTRO.VQA`, and skips a movie it cannot find. |
| `only_the_host_may_change_gamespeed.asm` (never built) | Let only the host change the game speed. | Not planned. It was never built into any product. |
| `allow_selling_undeployable_sensorarray.asm` (never built) | Stopped treating a `SensorArray=yes` building with `UndeploysInto=` as a deployed vehicle, so selling it sold it instead of packing it up. | Not planned. It was never built into any product. |
| `shared_control.c` | Let observers select and band-select objects of every player and see their order lines. | Not planned beyond what OpenTS already allows: an observer can click any player's object to view it, and gives no orders. [observers](../manual/content/systems/observers.md) |
| `gamespeed.asm` | Raised network frame rates: a client-launched game ran at 60, 55, 45, 40, 30, 20 and 15 frames a second from Fastest to Slowest, and other network games ran Faster at 55. | Not planned for now. Network games keep the stock 60, 45, 30, 20, 15, 12 and 10, which the solo table nearly matches. |
| `cache_alot.asm` | Loaded most archives, including `CONQUER.MIX`, the sound, speech and score archives, and the sidebar archives, into memory when they were opened, to avoid disk reads during a battle. | The operating system's file cache already keeps recently read archives in memory. ts-patches dropped it from its Vinifera builds and stopped caching the `SIDECD` archives after it broke the map selection background. |

## Belongs in mod data

These patches change values for particular products. OpenTS keeps the stock
value; the last column says what a mod sets instead, or that no setting exists
yet.

| ts-patches file | What it did | In OpenTS |
| --- | --- | --- |
| Every product's `*_hacks.asm` | Turned `IsScoreShuffle` on by default. | Set `IsScoreShuffle=yes` in the settings file. |
| `dta_hacks.asm`, `to_hacks.asm` | Turned sidebar cameo text and action lines off by default. | Set `SidebarCameoText=no` and `UnitActionLines=no` in the settings file. |
| `dta_hacks.asm`, `ti_hacks.asm`, `to_hacks.asm` | Limited the tech level slider to 7 in the network and modem game dialogs. DTA also set a default of 7, which the rules replaced at startup. | `[MultiplayerDefaults] TechLevel=` in the rules sets the default. For the limit, a mod's own `netgame.rml` and `skirmish.rml` can give the slider a lower `max`. [UI files](../manual/content/systems/ui-files.md) |
| Every product's `*_hacks.asm`, `res/` | Replaced the window title, the `Language.dll` error message, the icon, the cursor and the internal-error dialog text with the product's own. | A mod replaces `cursor.png` for the pointer over the screens. The window title, icon and error texts have no setting yet. [UI files](../manual/content/systems/ui-files.md) |
| `ti_hacks.asm`, `to_hacks.asm` | Changed the credits text file, and in TI the credits music. | No setting yet. |
| `dta_hacks.asm`, `ti_hacks.asm`, `to_hacks.asm` | Removed the dialog opening sound and the glow along dialog and button edges. | A mod replaces `glow.png` or the kit style sheet to change the glow. The sound plays with the opening slide, so a screen drops it only when its document turns the slide off with `reveal="none"`. [UI files](../manual/content/systems/ui-files.md) |
| `dta_hacks.asm`, `ti_hacks.asm`, `to_hacks.asm` | Changed the frame color of dialog controls, and in TI and TO the list box color. | A mod restyles the controls in its own `kit.rcss`. [UI files](../manual/content/systems/ui-files.md) |
| `ingame_ui_text_color.asm` | Colored in-game menu text by side in DTA. | A mod colors the screens' text for a side in its `side-<name>.rcss`. [UI files](../manual/content/systems/ui-files.md) |
| `briefing_screen_mission_start.asm` | Colored the briefing text blue for side 2 and red for side 3. | A mod colors the objectives screen's text for a side in its `side-<name>.rcss`, as the shipped `side-gdi.rcss` and `side-nod.rcss` do. [UI files](../manual/content/systems/ui-files.md) |
| `dta_hacks.asm`, `tsa_hacks.asm` | Used `BRIEFING.PCX` instead of `SCORE.PCX` behind the Restate screen. | A mod's `restate.rcss` sets `decorator: image(briefing.pcx)` on `#frame`. A side sheet can do the same for one side with a `body[data-model="restate"] #frame` rule. [UI files](../manual/content/systems/ui-files.md) |
| `dta_hacks.asm`, `to_hacks.asm` | Made the Core Defender's selection box two cells wide and 100 leptons high, instead of one cell and 700. | No setting yet. |
| `ts_hacks.asm`, `tsa_hacks.asm`, `rubicon_hacks.asm`, `fd_hacks.asm`, `sd_hacks.asm` | Changed the selection box color of an object with a limpet drone attached. | No setting yet. |
| `ti_hacks.asm` | Drew the selection box of buildings and Core Defenders in palette index 124 instead of white. | No setting yet. |
| `dta_hacks.asm`, `ti_hacks.asm`, `rubicon_hacks.asm` | Changed cluster scatter, and in DTA the projectile bounce limit. | No setting yet. |
| `dta_hacks.asm`, `ti_hacks.asm`, `rubicon_hacks.asm` | Changed subterranean movement speeds. | No setting yet. |
| `dta_hacks.asm` | Drew Tiberium with the theater palette and cell lighting instead of its `Color=` scheme. | No setting yet. |
| `rubicon_hacks.asm` | Spawned projectile trailers every 2 frames instead of every 3. | No setting yet. |
| `ti_hacks.asm`, `rubicon_hacks.asm` | Played the ion storm warning every 334 frames instead of every 225. | No setting yet. |
| `ti_hacks.asm` | Used `CLIFX1`–`CLIFX3` as the cliff collapse animations. | No setting yet. |
| `ti_hacks.asm` | Dropped `DP1` and `DP2` from drop pods instead of `E1` and `E2`. | No setting yet. |

## Still open

| ts-patches file | What it did | What is needed |
| --- | --- | --- |
| `score_screen_player_always_on_left.asm` | In DTA, put the player's casualty bars on the left of the campaign score screen whatever their side. | A decision. Vanilla already puts a side 0 player on the left, so the change moves the stock Nod campaign's bars and a third side's. |
| `ai_target_emp_like_multimissile.asm`, `ai_target_droppods_like_multimissile.asm` | Let the AI aim EMP and drop pod superweapons at enemies. | A superweapon rework. |
| `extra_difficulty.asm` | In DTA and Rubicon, added four easier difficulties and an AI-only `AINormal`. | Waiting for a request. |
| `chat_ignore.c`, `spawner/spawner.asm` | Ignored chat by player color, except whole messages on a fixed list of coordination phrases. | A proper feature with its own interface, and a stable player identity to ignore by. |
| `sidebar.c`, `show_stats.c`, `response_time_func.asm`, `new_events_s.asm`, `high_res_crash.asm`, `hotkeys.c` | Showed a panel under the sidebar, set by `InfoPanel` and cycled by a command, with pages for time, frame rate, actions per minute and score rate; the selected object's statistics; and each player's latency and packet loss. | A design for the panel. |
| `hotkey_help.c` | Showed a list of hotkeys. | A design; the list would come from the command list. |
| `dump_globals.asm` | Wrote the 50 global variables to the debug log as a `Global variables:` line when the player won. DTA's branch of the XNA client reads it to unlock missions that depend on a global; the upstream client does not. | A client that needs it. |
| Every product's `*_hacks.asm` | Skipped the `.MMT` and `.MMS` fallback for a tile set with a missing image. | Finding out whether anything uses them. |

[action-lines-and-ui-ini]: ../manual/changes/action-lines-and-ui-ini.md
[adaptive-network-timing]: ../manual/changes/adaptive-network-timing.md
[add-selection-to-team]: ../manual/changes/add-selection-to-team.md
[ai-player-country-from-roster]: ../manual/changes/ai-player-country-from-roster.md
[ai-production-type-capacity]: ../manual/changes/ai-production-type-capacity.md
[ai-trigger-side-by-position]: ../manual/changes/ai-trigger-side-by-position.md
[ai-vehicle-factory-exit-navigation]: ../manual/changes/ai-vehicle-factory-exit-navigation.md
[aimable-anti-air-defenses]: ../manual/changes/aimable-anti-air-defenses.md
[aircraft-landing-move-completion]: ../manual/changes/aircraft-landing-move-completion.md
[aircraft-repair-capability]: ../manual/changes/aircraft-repair-capability.md
[aircraft-unload-building-guard]: ../manual/changes/aircraft-unload-building-guard.md
[aircraft-zero-ammo-fire-orders]: ../manual/changes/aircraft-zero-ammo-fire-orders.md
[allied-kill-veterancy]: ../manual/changes/allied-kill-veterancy.md
[allied-sensor-cloaking]: ../manual/changes/allied-sensor-cloaking.md
[allied-team-retaliation]: ../manual/changes/allied-team-retaliation.md
[anim-loop-delay-sync]: ../manual/changes/anim-loop-delay-sync.md
[armor-crate-multiplier]: ../manual/changes/armor-crate-multiplier.md
[attack-neutral-units]: ../manual/changes/attack-neutral-units.md
[audio-engine]: ../manual/changes/audio-engine.md
[auto-deploy-mcv]: ../manual/changes/auto-deploy-mcv.md
[autosave]: ../manual/changes/autosave.md
[band-select-drag-threshold]: ../manual/changes/band-select-drag-threshold.md
[base-unit-per-country]: ../manual/changes/base-unit-per-country.md
[bgfx-presenter]: ../manual/changes/bgfx-presenter.md
[blowfish-in-binary]: ../manual/changes/blowfish-in-binary.md
[build-limit-queue-count]: ../manual/changes/build-limit-queue-count.md
[build-off-ally]: ../manual/changes/build-off-ally.md
[buildable-overlays]: ../manual/changes/buildable-overlays.md
[building-animation-initialization]: ../manual/changes/building-animation-initialization.md
[c4-repairability]: ../manual/changes/c4-repairability.md
[cameo-needs-a-factory]: ../manual/changes/cameo-needs-a-factory.md
[cd-search-path]: ../manual/changes/cd-search-path.md
[center-base-buildconst]: ../manual/changes/center-base-buildconst.md
[charging-defenses-through-low-power]: ../manual/changes/charging-defenses-through-low-power.md
[chat-echo-and-background]: ../manual/changes/chat-echo-and-background.md
[client-driven-launch]: ../manual/changes/client-driven-launch.md
[client-load-screen]: ../manual/changes/client-load-screen.md
[client-score-screen]: ../manual/changes/client-score-screen.md
[cncnet-trigger-actions]: ../manual/changes/cncnet-trigger-actions.md
[construction-yard-whole-list]: ../manual/changes/construction-yard-whole-list.md
[crate-money-bonus]: ../manual/changes/crate-money-bonus.md
[crate-pickup-replacement-option]: ../manual/changes/crate-pickup-replacement-option.md
[crate-rescue-whole-lists]: ../manual/changes/crate-rescue-whole-lists.md
[damaging-building-animation-crash]: ../manual/changes/damaging-building-animation-crash.md
[debug-log-console]: ../manual/changes/debug-log-console.md
[deploy-cursor-on-a-repair-vehicle]: ../manual/changes/deploy-cursor-on-a-repair-vehicle.md
[deployment-file-names]: ../manual/changes/deployment-file-names.md
[deployment-search-folders]: ../manual/changes/deployment-search-folders.md
[depot-vehicle-no-longer-explodes]: ../manual/changes/depot-vehicle-no-longer-explodes.md
[destroy-trigger-deferred]: ../manual/changes/destroy-trigger-deferred.md
[disconnect-and-reconnect-policy]: ../manual/changes/disconnect-and-reconnect-policy.md
[disguise-detection]: ../manual/changes/disguise-detection.md
[economy-counts-whole-lists]: ../manual/changes/economy-counts-whole-lists.md
[edge-scrolling-option]: ../manual/changes/edge-scrolling-option.md
[emp-immunity]: ../manual/changes/emp-immunity.md
[exception-handler]: ../manual/changes/exception-handler.md
[factory-ownership-whole-list]: ../manual/changes/factory-ownership-whole-list.md
[firestorm-detected-by-rules-file]: ../manual/changes/firestorm-detected-by-rules-file.md
[first-enemy-skips-allies]: ../manual/changes/first-enemy-skips-allies.md
[flashing-voxel-shadow]: ../manual/changes/flashing-voxel-shadow.md
[frame-rate-independent-scrolling]: ../manual/changes/frame-rate-independent-scrolling.md
[free-unit-always-given]: ../manual/changes/free-unit-always-given.md
[free-unit-any-kind]: ../manual/changes/free-unit-any-kind.md
[game-data-directory]: ../manual/changes/game-data-directory.md
[group-number-offsets]: ../manual/changes/group-number-offsets.md
[guard-command-scope]: ../manual/changes/guard-command-scope.md
[guard-target-reacquisition]: ../manual/changes/guard-target-reacquisition.md
[harvester-goes-to-work]: ../manual/changes/harvester-goes-to-work.md
[harvester-refinery-lost-on-the-way]: ../manual/changes/harvester-refinery-lost-on-the-way.md
[harvester-truce-consistency]: ../manual/changes/harvester-truce-consistency.md
[healer-target-kinds]: ../manual/changes/healer-target-kinds.md
[hostile-aircraft-landing-reservations]: ../manual/changes/hostile-aircraft-landing-reservations.md
[house-acts-for-itself]: ../manual/changes/house-acts-for-itself.md
[hover-and-always-on-indicators]: ../manual/changes/hover-and-always-on-indicators.md
[immune-terrain-pathing]: ../manual/changes/immune-terrain-pathing.md
[infantry-null-warhead]: ../manual/changes/infantry-null-warhead.md
[infiltrated-factory-cameo-palette]: ../manual/changes/infiltrated-factory-cameo-palette.md
[insignificant-unit-loss-announcement]: ../manual/changes/insignificant-unit-loss-announcement.md
[ion-blast-edge-coverage]: ../manual/changes/ion-blast-edge-coverage.md
[ion-blast-edge-sampling]: ../manual/changes/ion-blast-edge-sampling.md
[ion-storm-jumpjet-destruction]: ../manual/changes/ion-storm-jumpjet-destruction.md
[isomappack5-decoding-capacity]: ../manual/changes/isomappack5-decoding-capacity.md
[jumpjet-factory-exit]: ../manual/changes/jumpjet-factory-exit.md
[laser-wave-edge-writes]: ../manual/changes/laser-wave-edge-writes.md
[launch-load-screen-kept-in-save]: ../manual/changes/launch-load-screen-kept-in-save.md
[limbo-radar-tracking]: ../manual/changes/limbo-radar-tracking.md
[lobby-side-entries-by-country]: ../manual/changes/lobby-side-entries-by-country.md
[map-crates-in-every-game-type]: ../manual/changes/map-crates-in-every-game-type.md
[map-file-random-map]: ../manual/changes/map-file-random-map.md
[map-local-tutorial-text]: ../manual/changes/map-local-tutorial-text.md
[match-statistics-file]: ../manual/changes/match-statistics-file.md
[minimum-weapon-burst]: ../manual/changes/minimum-weapon-burst.md
[mission-difficulty-message]: ../manual/changes/mission-difficulty-message.md
[mission-restart-difficulty]: ../manual/changes/mission-restart-difficulty.md
[mission-start-briefing]: ../manual/changes/mission-start-briefing.md
[more-shape-facings]: ../manual/changes/more-shape-facings.md
[mouse-presence-startup-check]: ../manual/changes/mouse-presence-startup-check.md
[move-to-cell-encoding]: ../manual/changes/move-to-cell-encoding.md
[movie-aspect-fitting]: ../manual/changes/movie-aspect-fitting.md
[multi-engineer-neutral-capture]: ../manual/changes/multi-engineer-neutral-capture.md
[multi-mcv-option]: ../manual/changes/multi-mcv-option.md
[multiplayer-movies]: ../manual/changes/multiplayer-movies.md
[multiplayer-player-structures]: ../manual/changes/multiplayer-player-structures.md
[multiplayer-save-frame-boundary]: ../manual/changes/multiplayer-save-frame-boundary.md
[multiplayer-weapon-overrides-from-rules]: ../manual/changes/multiplayer-weapon-overrides-from-rules.md
[multiple-factory-bonus]: ../manual/changes/multiple-factory-bonus.md
[multiple-instances-option]: ../manual/changes/multiple-instances-option.md
[mutual-refinery-docking]: ../manual/changes/mutual-refinery-docking.md
[observer-seats-and-coach-mode]: ../manual/changes/observer-seats-and-coach-mode.md
[optional-map-and-multiplayer-archives]: ../manual/changes/optional-map-and-multiplayer-archives.md
[optional-score-and-movie-archives]: ../manual/changes/optional-score-and-movie-archives.md
[optional-side-cd-archive]: ../manual/changes/optional-side-cd-archive.md
[out-of-sync-recovery]: ../manual/changes/out-of-sync-recovery.md
[out-of-sync-report]: ../manual/changes/out-of-sync-report.md
[pad-aircraft-whole-list]: ../manual/changes/pad-aircraft-whole-list.md
[partial-burst-target-loss]: ../manual/changes/partial-burst-target-loss.md
[pip-row-length]: ../manual/changes/pip-row-length.md
[place-building-command]: ../manual/changes/place-building-command.md
[player-side-from-country]: ../manual/changes/player-side-from-country.md
[produce-cash-buildings]: ../manual/changes/produce-cash-buildings.md
[production-anim-any-factory]: ../manual/changes/production-anim-any-factory.md
[quickmatch-hides-player-names]: ../manual/changes/quickmatch-hides-player-names.md
[radar-event-ignores-production]: ../manual/changes/radar-event-ignores-production.md
[rally-point-into-shroud]: ../manual/changes/rally-point-into-shroud.md
[rally-point-on-a-plain-click]: ../manual/changes/rally-point-on-a-plain-click.md
[recheck-prerequisites]: ../manual/changes/recheck-prerequisites.md
[refinery-by-trip-cost]: ../manual/changes/refinery-by-trip-cost.md
[repeat-last-production]: ../manual/changes/repeat-last-production.md
[restate-briefing-fallback]: ../manual/changes/restate-briefing-fallback.md
[retire-westwood-online]: ../manual/changes/retire-westwood-online.md
[reveal-crate-idempotent]: ../manual/changes/reveal-crate-idempotent.md
[role-lists-per-country]: ../manual/changes/role-lists-per-country.md
[save-confirmation-message]: ../manual/changes/save-confirmation-message.md
[saved-games-folder]: ../manual/changes/saved-games-folder.md
[scenario-object-owner-resolution]: ../manual/changes/scenario-object-owner-resolution.md
[scenario-theme-rotation]: ../manual/changes/scenario-theme-rotation.md
[scrap-debris]: ../manual/changes/scrap-debris.md
[screenshots-folder]: ../manual/changes/screenshots-folder.md
[selection-filter-commands]: ../manual/changes/selection-filter-commands.md
[self-healing-settings]: ../manual/changes/self-healing-settings.md
[shared-control]: ../manual/changes/shared-control.md
[short-game-fire-sale-keeps-a-structure]: ../manual/changes/short-game-fire-sale-keeps-a-structure.md
[side-base-building]: ../manual/changes/side-base-building.md
[side-factory-prerequisite-planning]: ../manual/changes/side-factory-prerequisite-planning.md
[side-roster-before-a-game]: ../manual/changes/side-roster-before-a-game.md
[sidebar-cameo-ordering]: ../manual/changes/sidebar-cameo-ordering.md
[sidebar-gated-by-yard-country]: ../manual/changes/sidebar-gated-by-yard-country.md
[sidebar-slot-ceiling]: ../manual/changes/sidebar-slot-ceiling.md
[sidebar-strip-capacity]: ../manual/changes/sidebar-strip-capacity.md
[sides-declared-in-side-list]: ../manual/changes/sides-declared-in-side-list.md
[simulate-while-unfocused]: ../manual/changes/simulate-while-unfocused.md
[skirmish-ends-past-passive-houses]: ../manual/changes/skirmish-ends-past-passive-houses.md
[solo-pacing-matches-network]: ../manual/changes/solo-pacing-matches-network.md
[sonic-wave-edge-coverage]: ../manual/changes/sonic-wave-edge-coverage.md
[sonic-wave-edge-sampling]: ../manual/changes/sonic-wave-edge-sampling.md
[soylent]: ../manual/changes/soylent.md
[spawn-house-alliances]: ../manual/changes/spawn-house-alliances.md
[spawn-house-base-nodes]: ../manual/changes/spawn-house-base-nodes.md
[spawn-houses]: ../manual/changes/spawn-houses.md
[spy-marks-its-own-house]: ../manual/changes/spy-marks-its-own-house.md
[stacking-crate-upgrades]: ../manual/changes/stacking-crate-upgrades.md
[stop-holds-position]: ../manual/changes/stop-holds-position.md
[superweapon-teardown-player-guard]: ../manual/changes/superweapon-teardown-player-guard.md
[team-double-tap-centre]: ../manual/changes/team-double-tap-centre.md
[theaters-declared-in-rules]: ../manual/changes/theaters-declared-in-rules.md
[tiberium-infantry-passability]: ../manual/changes/tiberium-infantry-passability.md
[tiberium-overlay-frame-validation]: ../manual/changes/tiberium-overlay-frame-validation.md
[tiberium-overlay-variety-bounds]: ../manual/changes/tiberium-overlay-variety-bounds.md
[tileset-lookup-capacity]: ../manual/changes/tileset-lookup-capacity.md
[totable]: ../manual/changes/totable.md
[trainable-veterancy-crates]: ../manual/changes/trainable-veterancy-crates.md
[type-overrides-from-rules]: ../manual/changes/type-overrides-from-rules.md
[typed-chat-recipients]: ../manual/changes/typed-chat-recipients.md
[unloading-class]: ../manual/changes/unloading-class.md
[vehicle-transports]: ../manual/changes/vehicle-transports.md
[vinifera-tiberium-power]: ../manual/changes/vinifera-tiberium-power.md
[voxel-debris-damage-isolation]: ../manual/changes/voxel-debris-damage-isolation.md
[voxel-debris-null-warhead]: ../manual/changes/voxel-debris-null-warhead.md
[waypoint-delete-from-end]: ../manual/changes/waypoint-delete-from-end.md
[waypoint-loop-plain-click]: ../manual/changes/waypoint-loop-plain-click.md
[weapons-registration-list]: ../manual/changes/weapons-registration-list.md
[zero-power-tiberium-chain-reactions]: ../manual/changes/zero-power-tiberium-chain-reactions.md
