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

#include "always.h"

#include "vox.h"

#include "audio/audioengine.h"
#include "globals.h"
#include "stimer.h"
#include "timer.h"
#include "voxqueue.h"

#include <algorithm>
#include <cstring>

CDTimerClass<SystemTimerClass> SpeakTimer;

/*
**	This elaborates all the EVA speech voices.
*/
char const * Speech[VOX_COUNT] =  {
	"00-I026",		/// VOX_ACCOMPLISHED
	"00-I028",		/// VOX_FAIL
	"00-I064",		/// VOX_NO_FACTORY
	"00-I018",		/// VOX_CONSTRUCTION
	"00-I076",		/// VOX_UNIT_READY
	"00-I032",		/// VOX_NEW_CONSTRUCT
	"00-I016",		/// VOX_DEPLOY
	"00-I008",		/// VOX_STRUCTURE_DESTROYED
	"00-I022",		/// VOX_NO_CASH
	"00-I012",		/// VOX_CONTROL_EXIT
	"00-I038",		/// VOX_REINFORCEMENTS
	"00-I220",		/// VOX_CANCELED
	"00-I216",		/// VOX_BUILDING
	"00-I024",		/// VOX_LOW_POWER
	"00-I082",		/// VOX_BASE_UNDER_ATTACK
	"00-I034",		/// VOX_PRIMARY_SELECTED
	"00-I074",		/// VOX_UNIT_LOST
	"00-I042",		/// VOX_SELECT_TARGET
	"00-I044",		/// VOX_NEED_MO_CAPACITY
	"00-I218",		/// VOX_SUSPENDED
	"00-I040",		/// VOX_REPAIRING
	"00-I062",		/// VOX_TRAINING
	"00-I068",		/// VOX_UPGRADE_ARMOR
	"00-I070",		/// VOX_UPGRADE_FIREPOWER
	"00-I080",		/// VOX_UPGRADE_SPEED
	"00-I078",		/// VOX_UNIT_REPAIRED
	"00-I228",		/// VOX_STRUCTURE_SOLD
	"00-I090",		/// VOX_HARVESTER_UNDER_ATTACK
	"00-I172",		/// VOX_CLOAKED_DETECTED
	"00-I174",		/// VOX_SUBTERRANEAN_DETECTED
	"00-I122",		/// VOX_TIME_20
	"00-I124",		/// VOX_TIME_10
	"00-I126",		/// VOX_TIME_5
	"00-I128",		/// VOX_TIME_4
	"00-I130",		/// VOX_TIME_3
	"00-I132",		/// VOX_TIME_2
	"00-I134",		/// VOX_TIME_1
	"00-I226",		/// VOX_UNIT_SOLD
	"00-I056",		/// VOX_BUILDING_CAPTURED
	"00-I200",		/// VOX_CONTROL_ESTABLISHED
	"00-I176",		/// VOX_ION_STORM_APPROACHING
	"00-I178",		/// VOX_METEOR_STORM
	"00-I198",		/// VOX_NEW_TERRAIN
	"00-I150",		/// VOX_MISSILE_LAUNCH_DETECTED
	"00-I152",		/// VOX_CHEMICAL_MISSILE_READY
	"00-I154",		/// VOX_CLUSTER_MISSILE_READY
	"00-I156",		/// VOX_ION_CANNON_READY
	"00-I158",		/// VOX_EM_PULSE_CANNON_READY
	"00-I162",		/// VOX_FIRESTORM_DEFENSE_READY
	"00-I170",		/// VOX_FIRESTORM_DEFENSE_OFFLINE
	"00-I100",		/// VOX_PRIMARY_OBJECTIVE_ACHIEVED
	"00-I102",		/// VOX_SECONDARY_OBJECTIVE_ACHIEVED
	"00-I104",		/// VOX_TERTIARY_OBJECTIVE_ACHIEVED
	"00-I106",		/// VOX_QUATERNARY_OBJECTIVE_ACHIEVED
	"00-I194",		/// VOX_CRITICAL_UNIT_LOST
	"00-I196",		/// VOX_CRITICAL_STRUCTURE_LOST
	"00-I208",		/// VOX_MUTANT_SUPPLIES_FOUND
	"00-I210",		/// VOX_COMMANDOS_EN_ROUTE
	"00-I014",		/// VOX_BUILDING_INFILTRATED
	"00-I058",		/// VOX_TIMER_STARTED
	"00-I060",		/// VOX_TIMER_STOPPED
	"00-I118",		/// VOX_BRIDGE_REPAIRED
	"00-I180",		/// VOX_BASE_DEFENSES_OFFLINE
	"00-I230",		/// VOX_BUILDING_OFFLINE
	"00-I232",		/// VOX_BUILDING_ONLINE
	"00-I252",		/// VOX_PLAYER_HAS_RESIGNED
	"00-I268",		/// VOX_PLAYER_WAS_DEFEATED
	"00-I284",		/// VOX_YOU_ARE_VICTORIOUS
	"00-I286",		/// VOX_YOU_HAVE_LOST
	"00-I288",		/// VOX_YOU_HAVE_RESIGNED
	"00-I290",		/// VOX_MUTANT_COMMANDOS_AVAILABLE
	"00-I304",		/// VOX_ALLIANCE_FORMED
	"00-I306",		/// VOX_ALLIANCE_BROKEN
	"00-I308",		/// VOX_ALLY_ATTACK
	"00-I310",		/// VOX_TUTORIAL_POWER
	"00-I312",		/// VOX_TUTORIAL_BARRACKS
	"00-I314",		/// VOX_TUTORIAL_HAND_OF_NOD
	"00-I316",		/// VOX_TUTORIAL_REFINERY
	"00-I318",		/// VOX_TUTORIAL_SILOS
	"00-I344",		/// VOX_GDI_TAUNT_01
	"00-I346",		/// VOX_GDI_TAUNT_02
	"00-I348",		/// VOX_GDI_TAUNT_03
	"00-I352",		/// VOX_GDI_TAUNT_04
	"00-I356",		/// VOX_GDI_TAUNT_05
	"00-I360",		/// VOX_GDI_TAUNT_06
	"00-I370",		/// VOX_GDI_TAUNT_07
	"00-I372",		/// VOX_GDI_TAUNT_08
	"00-I374",		/// VOX_GDI_TAUNT_09
	"00-I376",		/// VOX_GDI_TAUNT_10
	"01-I342",		/// VOX_NOD_TAUNT_01
	"01-I350",		/// VOX_NOD_TAUNT_02
	"01-I352",		/// VOX_NOD_TAUNT_03
	"01-I356",		/// VOX_NOD_TAUNT_04
	"01-I360",		/// VOX_NOD_TAUNT_05
	"01-I362",		/// VOX_NOD_TAUNT_06
	"01-I364",		/// VOX_NOD_TAUNT_07
	"01-I366",		/// VOX_NOD_TAUNT_08
	"01-I368",		/// VOX_NOD_TAUNT_09
	"01-I378",		/// VOX_NOD_TAUNT_10
	"33-N000",		/// VOX_33_N000
	"33-N002",		/// VOX_33_N002
	"33-N004",		/// VOX_33_N004
	"33-N008",		/// VOX_33_N008
	"33-N010",		/// VOX_33_N010
	"33-N012",		/// VOX_33_N012
	"33-N014",		/// VOX_33_N014
	"33-N020",		/// VOX_33_N020
	"33-N022",		/// VOX_33_N022
	"33-N024",		/// VOX_33_N024
	"35-N000",		/// VOX_35_N000
	"35-N002",		/// VOX_35_N002
	"35-N004",		/// VOX_35_N004
	"35-N008",		/// VOX_35_N008
	"35-N010",		/// VOX_35_N010
	"35-N012",		/// VOX_35_N012
	"35-N014",		/// VOX_35_N014
	"40-N000",		/// VOX_40_N000_1
	"00-N000",		/// VOX_00_N000
	"00-N002",		/// VOX_00_N002
	"00-N004",		/// VOX_00_N004
	"00-N006",		/// VOX_00_N006
	"00-N018",		/// VOX_00_N018
	"00-N020",		/// VOX_00_N020
	"00-N022",		/// VOX_00_N022
	"00-N024",		/// VOX_00_N024
	"00-N032",		/// VOX_00_N032
	"00-N034",		/// VOX_00_N034
	"00-N040",		/// VOX_00_N040
	"00-N042",		/// VOX_00_N042
	"00-N044",		/// VOX_00_N044
	"00-N052",		/// VOX_00_N052
	"00-N054",		/// VOX_00_N054
	"00-N055",		/// VOX_00_N055
	"00-N056",		/// VOX_00_N056
	"00-N058",		/// VOX_00_N058
	"00-N059",		/// VOX_00_N059
	"01-N320",		/// VOX_01_N320
	"01-N322",		/// VOX_01_N322
	"01-N324",		/// VOX_01_N324
	"01-N326",		/// VOX_01_N326
	"00-N068",		/// VOX_00_N068
	"00-N070",		/// VOX_00_N070
	"00-N072",		/// VOX_00_N072
	"00-N074",		/// VOX_00_N074
	"00-N075",		/// VOX_00_N075
	"01-N900",		/// VOX_01_N900
	"01-N901",		/// VOX_01_N901
	"00-N084",		/// VOX_00_N084
	"00-N086",		/// VOX_00_N086
	"00-N088",		/// VOX_00_N088
	"00-N090",		/// VOX_00_N090
	"00-N092",		/// VOX_00_N092
	"00-N094",		/// VOX_00_N094
	"00-N096",		/// VOX_00_N096
	"00-N098",		/// VOX_00_N098
	"01-N328",		/// VOX_01_N328
	"01-N330",		/// VOX_01_N330
	"00-N112",		/// VOX_00_N112
	"00-N114",		/// VOX_00_N114
	"00-N128",		/// VOX_00_N128
	"00-N130",		/// VOX_00_N130
	"00-N132",		/// VOX_00_N132
	"00-N134",		/// VOX_00_N134
	"00-N136",		/// VOX_00_N136
	"00-N138",		/// VOX_00_N138
	"00-N140",		/// VOX_00_N140
	"00-N142",		/// VOX_00_N142
	"00-N156",		/// VOX_00_N156
	"00-N158",		/// VOX_00_N158
	"00-N160",		/// VOX_00_N160
	"00-N162",		/// VOX_00_N162
	"00-N166",		/// VOX_00_N166
	"00-N168",		/// VOX_00_N168
	"00-N180",		/// VOX_00_N180
	"00-N182",		/// VOX_00_N182
	"00-N188",		/// VOX_00_N188
	"00-N190",		/// VOX_00_N190
	"00-N192",		/// VOX_00_N192
	"00-N206",		/// VOX_00_N206
	"00-N208",		/// VOX_00_N208
	"00-N210",		/// VOX_00_N210
	"00-N224",		/// VOX_00_N224
	"00-N226",		/// VOX_00_N226
	"00-N228",		/// VOX_00_N228
	"00-N236",		/// VOX_00_N236
	"00-N238",		/// VOX_00_N238
	"00-N240",		/// VOX_00_N240
	"00-N239",		/// VOX_00_N239
	"00-N241",		/// VOX_00_N241
	"00-N243",		/// VOX_00_N243
	"00-N245",		/// VOX_00_N245
	"00-N247",		/// VOX_00_N247
	"00-N248",		/// VOX_00_N248
	"00-N249",		/// VOX_00_N249
	"00-N250",		/// VOX_00_N250
	"00-N251",		/// VOX_00_N251
	"00-N252",		/// VOX_00_N252
	"00-N254",		/// VOX_00_N254
	"00-N255",		/// VOX_00_N255
	"38-N000",		/// VOX_38_N000
	"38-N002",		/// VOX_38_N002
	"38-N006",		/// VOX_38_N006
	"40-N000",		/// VOX_40_N000
	"41-N000",		/// VOX_41_N000
	"41-N002",		/// VOX_41_N002
	"41-N004",		/// VOX_41_N004
	"41-N006",		/// VOX_41_N006
	"43-N000",		/// VOX_43_N000
	"01-N000",		/// VOX_01_N000
	"01-N002",		/// VOX_01_N002
	"01-N004",		/// VOX_01_N004
	"01-N006",		/// VOX_01_N006
	"01-N008",		/// VOX_01_N008
	"01-N010",		/// VOX_01_N010
	"01-N005",		/// VOX_01_N005
	"01-N007",		/// VOX_01_N007
	"01-N009",		/// VOX_01_N009
	"01-N011",		/// VOX_01_N011
	"01-N020",		/// VOX_01_N020
	"01-N022",		/// VOX_01_N022
	"01-N024",		/// VOX_01_N024
	"01-N026",		/// VOX_01_N026
	"01-N027",		/// VOX_01_N027
	"01-N032",		/// VOX_01_N032
	"01-N034",		/// VOX_01_N034
	"01-N036",		/// VOX_01_N036
	"01-N038",		/// VOX_01_N038
	"01-N040",		/// VOX_01_N040
	"01-N054",		/// VOX_01_N054
	"01-N055",		/// VOX_01_N055
	"01-N064",		/// VOX_01_N064
	"01-N066",		/// VOX_01_N066
	"01-N068",		/// VOX_01_N068
	"01-N070",		/// VOX_01_N070
	"01-N072",		/// VOX_01_N072
	"01-N074",		/// VOX_01_N074
	"01-N076",		/// VOX_01_N076
	"01-N086",		/// VOX_01_N086
	"01-N088",		/// VOX_01_N088
	"01-N090",		/// VOX_01_N090
	"01-N100",		/// VOX_01_N100
	"01-N102",		/// VOX_01_N102
	"01-N114",		/// VOX_01_N114
	"01-N116",		/// VOX_01_N116
	"01-N128",		/// VOX_01_N128
	"01-N130",		/// VOX_01_N130
	"01-N132",		/// VOX_01_N132
	"01-N134",		/// VOX_01_N134
	"01-N136",		/// VOX_01_N136
	"01-N144",		/// VOX_01_N144
	"01-N156",		/// VOX_01_N156
	"01-N158",		/// VOX_01_N158
	"01-N160",		/// VOX_01_N160
	"01-N162",		/// VOX_01_N162
	"01-N164",		/// VOX_01_N164
	"01-N174",		/// VOX_01_N174
	"01-N176",		/// VOX_01_N176
	"01-N178",		/// VOX_01_N178
	"01-N180",		/// VOX_01_N180
	"01-N192",		/// VOX_01_N192
	"01-N194",		/// VOX_01_N194
	"01-N196",		/// VOX_01_N196
	"01-N208",		/// VOX_01_N208
	"01-N210",		/// VOX_01_N210
	"01-N212",		/// VOX_01_N212
	"01-N228",		/// VOX_01_N228
	"01-N230",		/// VOX_01_N230
	"01-N232",		/// VOX_01_N232
	"01-N233",		/// VOX_01_N233
	"01-N227",		/// VOX_01_N227
	"01-N229",		/// VOX_01_N229
	"01-N231",		/// VOX_01_N231
	"01-N234",		/// VOX_01_N234
	"01-N256",		/// VOX_01_N256
	"01-N258",		/// VOX_01_N258
	"01-N260",		/// VOX_01_N260
	"01-N262",		/// VOX_01_N262
	"01-N264",		/// VOX_01_N264
	"01-N266",		/// VOX_01_N266
	"01-N268",		/// VOX_01_N268
	"01-N270",		/// VOX_01_N270
	"01-N284",		/// VOX_01_N284
	"01-N286",		/// VOX_01_N286
	"10-N032",		/// VOX_10_N032
	"10-N034",		/// VOX_10_N034
	"10-N036",		/// VOX_10_N036
	"10-N038",		/// VOX_10_N038
	"10-N040",		/// VOX_10_N040
	"10-N042",		/// VOX_10_N042
	"10-N044",		/// VOX_10_N044
	"10-N046",		/// VOX_10_N046
	"44-N000",		/// VOX_44_N000
	"36-N000",		/// VOX_36_N000
	"36-N002",		/// VOX_36_N002
	"36-N004",		/// VOX_36_N004
	"36-N008",		/// VOX_36_N008
	"37-N000",		/// VOX_37_N000
	"37-N002",		/// VOX_37_N002
	"38-N004",		/// VOX_38_N004
	"38-N008",		/// VOX_38_N008
	"38-N010",		/// VOX_38_N010
	"38-N012",		/// VOX_38_N012
	"38-N014",		/// VOX_38_N014
	"38-N016",		/// VOX_38_N016
	"38-N018",		/// VOX_38_N018
	"38-N020",		/// VOX_38_N020
	"38-N022",		/// VOX_38_N022
	"38-N024",		/// VOX_38_N024
	"38-N026",		/// VOX_38_N026
	"38-N028",		/// VOX_38_N028
	"39-N000",		/// VOX_39_N000
	"47-N000",		/// VOX_47_N000
	"00-I020",		/// VOX_INCOMING_TRANSMISSION
	"00-I500",		/// VOX_OBJECTIVE_COMPLETE
	"00-I502",		/// VOX_FINAL_OBJECTIVE_COMPLETE
	"00-I504",		/// VOX_MOBILE_WAR_FACTORY_DEPLOYED
	"00-N400",		/// VOX_00_N400
	"00-N402",		/// VOX_00_N402
	"00-N404",		/// VOX_00_N404
	"00-N406",		/// VOX_00_N406
	"00-N408",		/// VOX_00_N408
	"00-N410",		/// VOX_00_N410
	"00-N412",		/// VOX_00_N412
	"00-N414",		/// VOX_00_N414
	"00-N416",		/// VOX_00_N416
	"00-N418",		/// VOX_00_N418
	"00-N420",		/// VOX_00_N420
	"00-N422",		/// VOX_00_N422
	"00-N424",		/// VOX_00_N424
	"00-N426",		/// VOX_00_N426
	"00-N428",		/// VOX_00_N428
	"00-N430",		/// VOX_00_N430
	"00-N432",		/// VOX_00_N432
	"00-N434",		/// VOX_00_N434
	"00-N436",		/// VOX_00_N436
	"00-N438",		/// VOX_00_N438
	"00-N440",		/// VOX_00_N440
	"00-N442",		/// VOX_00_N442
	"00-N444",		/// VOX_00_N444
	"00-N446",		/// VOX_00_N446
	"00-N448",		/// VOX_00_N448
	"00-N450",		/// VOX_00_N450
	"00-N452",		/// VOX_00_N452
	"00-N454",		/// VOX_00_N454
	"00-N456",		/// VOX_00_N456
	"00-N458",		/// VOX_00_N458
	"00-N460",		/// VOX_00_N460
	"00-N462",		/// VOX_00_N462
	"00-N464",		/// VOX_00_N464
	"00-N466",		/// VOX_00_N466
	"00-N468",		/// VOX_00_N468
	"00-N470",		/// VOX_00_N470
	"00-N472",		/// VOX_00_N472
	"00-N474",		/// VOX_00_N474
	"00-N478",		/// VOX_00_N478
	"00-N479",		/// VOX_00_N479
	"00-N480",		/// VOX_00_N480
	"00-N482",		/// VOX_00_N482
	"00-N484",		/// VOX_00_N484
	"00-N486",		/// VOX_00_N486
	"00-N488",		/// VOX_00_N488
	"00-N490",		/// VOX_00_N490
	"00-N492",		/// VOX_00_N492
	"00-N494",		/// VOX_00_N494
	"00-N496",		/// VOX_00_N496
	"00-N498",		/// VOX_00_N498
	"00-N500",		/// VOX_00_N500
	"00-N502",		/// VOX_00_N502
	"00-N504",		/// VOX_00_N504
	"00-N506",		/// VOX_00_N506
	"00-N508",		/// VOX_00_N508
	"00-N510",		/// VOX_00_N510
	"01-N400",		/// VOX_01_N400
	"01-N402",		/// VOX_01_N402
	"01-N404",		/// VOX_01_N404
	"01-N406",		/// VOX_01_N406
	"01-N408",		/// VOX_01_N408
	"01-N410",		/// VOX_01_N410
	"01-N412",		/// VOX_01_N412
	"01-N414",		/// VOX_01_N414
	"01-N416",		/// VOX_01_N416
	"01-N418",		/// VOX_01_N418
	"01-N420",		/// VOX_01_N420
	"01-N422",		/// VOX_01_N422
	"01-N424",		/// VOX_01_N424
	"01-N426",		/// VOX_01_N426
	"01-N428",		/// VOX_01_N428
	"01-N430",		/// VOX_01_N430
	"01-N432",		/// VOX_01_N432
	"01-N434",		/// VOX_01_N434
	"01-N436",		/// VOX_01_N436
	"01-N438",		/// VOX_01_N438
	"01-N440",		/// VOX_01_N440
	"01-N442",		/// VOX_01_N442
	"01-N444",		/// VOX_01_N444
	"01-N446",		/// VOX_01_N446
	"01-N448",		/// VOX_01_N448
	"01-N450",		/// VOX_01_N450
	"01-N452",		/// VOX_01_N452
	"99-N454",		/// VOX_99_N454
	"99-N456",		/// VOX_99_N456
	"99-N458",		/// VOX_99_N458
	"99-N460",		/// VOX_99_N460
	"01-N462",		/// VOX_01_N462
	"99-N464",		/// VOX_99_N464
	"99-N466",		/// VOX_99_N466
	"01-N468",		/// VOX_01_N468
	"01-N470",		/// VOX_01_N470
	"01-N472",		/// VOX_01_N472
	"01-N474",		/// VOX_01_N474
	"01-N476",		/// VOX_01_N476
	"01-N478",		/// VOX_01_N478
	"19-N100",		/// VOX_19_N100
	"19-N102",		/// VOX_19_N102
	"38-N100",		/// VOX_38_N100
	"38-N102",		/// VOX_38_N102
	"38-N104",		/// VOX_38_N104
	"38-N106",		/// VOX_38_N106
	"38-N108",		/// VOX_38_N108
	"38-N110",		/// VOX_38_N110
	"38-N112",		/// VOX_38_N112
	"38-N114",		/// VOX_38_N114
	"38-N116",		/// VOX_38_N116
	"38-N118",		/// VOX_38_N118
	"38-N120",		/// VOX_38_N120
	"38-N122",		/// VOX_38_N122
	"38-N124",		/// VOX_38_N124
	"38-N126",		/// VOX_38_N126
	"38-N128",		/// VOX_38_N128
	"38-N130",		/// VOX_38_N130
	"38-N132",		/// VOX_38_N132
	"38-N134",		/// VOX_38_N134
	"38-N136",		/// VOX_38_N136
	"38-N138",		/// VOX_38_N138
	"38-N140",		/// VOX_38_N140
	"38-N142",		/// VOX_38_N142
	"38-N144",		/// VOX_38_N144
	"38-N146",		/// VOX_38_N146
	"38-N148",		/// VOX_38_N148
	"38-N150",		/// VOX_38_N150
	"38-N152",		/// VOX_38_N152
	"38-N154",		/// VOX_38_N154
	"38-N156",		/// VOX_38_N156
	"52-N000",		/// VOX_52_N000
	"52-N002",		/// VOX_52_N002
	"52-N004",		/// VOX_52_N004
	"54-N000",		/// VOX_54_N000
	"54-N002",		/// VOX_54_N002
	"54-N004",		/// VOX_54_N004
	"54-N006",		/// VOX_54_N006
	"54-N008",		/// VOX_54_N008
	"54-N010",		/// VOX_54_N010
	"54-N012",		/// VOX_54_N012
	"54-N014",		/// VOX_54_N014
	"54-N016",		/// VOX_54_N016
	"54-N018",		/// VOX_54_N018
	"54-N020",		/// VOX_54_N020
	"55-N000",		/// VOX_55_N000
	"55-N002",		/// VOX_55_N002
	"55-N004",		/// VOX_55_N004
	"55-N006",		/// VOX_55_N006
	"55-N008",		/// VOX_55_N008
	"55-N010",		/// VOX_55_N010
	"55-N012",		/// VOX_55_N012
	"55-N014",		/// VOX_55_N014
	"55-N016",		/// VOX_55_N016
	"55-N018",		/// VOX_55_N018
	"55-N020",		/// VOX_55_N020
	"55-N022",		/// VOX_55_N022
	"55-N024",		/// VOX_55_N024
	"55-N026",		/// VOX_55_N026
	"55-N028",		/// VOX_55_N028
	"55-N030",		/// VOX_55_N030
	"55-N032",		/// VOX_55_N032
	"55-N034",		/// VOX_55_N034
	"55-N036",		/// VOX_55_N036
	"55-N038",		/// VOX_55_N038
	"55-N040",		/// VOX_55_N040
	"55-N042",		/// VOX_55_N042
	"55-N044",		/// VOX_55_N044
	"56-N000",		/// VOX_56_N000
	"56-N002",		/// VOX_56_N002
	"56-N004",		/// VOX_56_N004
	"56-N006",		/// VOX_56_N006
	"56-N008",		/// VOX_56_N008
	"56-N010",		/// VOX_56_N010
	"56-N012",		/// VOX_56_N012
	"56-N014",		/// VOX_56_N014
	"56-N016",		/// VOX_56_N016
	"56-N018",		/// VOX_56_N018
	"56-N020",		/// VOX_56_N020
	"57-N100",		/// VOX_57_N100
	"57-N102",		/// VOX_57_N102
	"57-N104",		/// VOX_57_N104
	"57-N106",		/// VOX_57_N106
	"57-N108",		/// VOX_57_N108
	"57-N110",		/// VOX_57_N110
	"57-N112",		/// VOX_57_N112
	"57-N114",		/// VOX_57_N114
	"57-N116",		/// VOX_57_N116
	"57-N118",		/// VOX_57_N118
	"57-N120",		/// VOX_57_N120
	"57-N122",		/// VOX_57_N122
	"57-N124",		/// VOX_57_N124
	"58-N100",		/// VOX_58_N100
	"58-N102",		/// VOX_58_N102
	"58-N104",		/// VOX_58_N104
	"58-N106",		/// VOX_58_N106
	"58-N108",		/// VOX_58_N108
	"58-N110",		/// VOX_58_N110
	"58-N112",		/// VOX_58_N112
	"58-N114",		/// VOX_58_N114
	"58-N116",		/// VOX_58_N116
	"58-N118",		/// VOX_58_N118
	"58-N120",		/// VOX_58_N120
	"58-N122",		/// VOX_58_N122
	"58-N124",		/// VOX_58_N124
	"58-N126",		/// VOX_58_N126
	"58-N128",		/// VOX_58_N128
	"58-N130",		/// VOX_58_N130
	"58-N132",		/// VOX_58_N132
	"58-N134",		/// VOX_58_N134
	"58-N136",		/// VOX_58_N136
	"58-N138",		/// VOX_58_N138
	"58-N140",		/// VOX_58_N140
	"58-N142",		/// VOX_58_N142
	"59-N100",		/// VOX_59_N100
	"59-N102",		/// VOX_59_N102
	"59-N104",		/// VOX_59_N104
	"61-N000",		/// VOX_61_N000
	"61-N002",		/// VOX_61_N002
	"61-N004",		/// VOX_61_N004
	"61-N006",		/// VOX_61_N006
	"61-N008",		/// VOX_61_N008
	"61-N010",		/// VOX_61_N010
	"61-N012",		/// VOX_61_N012
	"22-N100",		/// VOX_22_N100
	"22-N102",		/// VOX_22_N102
	"22-N104",		/// VOX_22_N104
	"22-N106",		/// VOX_22_N106
	"22-N108",		/// VOX_22_N108
	"22-N110",		/// VOX_22_N110
	"22-N112",		/// VOX_22_N112
	"22-N114",		/// VOX_22_N114
	"22-N116",		/// VOX_22_N116
	"22-N118",		/// VOX_22_N118
	"22-N120",		/// VOX_22_N120
	"71-N000",		/// VOX_71_N000
	"71-N100",		/// VOX_71_N100
	"71-N102",		/// VOX_71_N102
	"71-N104",		/// VOX_71_N104
	"71-N106",		/// VOX_71_N106
	"71-N108",		/// VOX_71_N108
	"71-N110",		/// VOX_71_N110
	"71-N112",		/// VOX_71_N112
	"71-N114",		/// VOX_71_N114
	"71-N116",		/// VOX_71_N116
	"71-N118",		/// VOX_71_N118
	"71-N120",		/// VOX_71_N120
	"71-N122",		/// VOX_71_N122
	"71-N124",		/// VOX_71_N124
	"71-N126",		/// VOX_71_N126
	"71-N128",		/// VOX_71_N128
	"71-N130",		/// VOX_71_N130
	"71-N132",		/// VOX_71_N132
	"71-N134",		/// VOX_71_N134
	"71-N136",		/// VOX_71_N136
	"00-I506",		/// VOX_DROP_PODS_READY
	"62-N000",		/// VOX_62_N000
	"62-N002",		/// VOX_62_N002
	"62-N004",		/// VOX_62_N004
	"62-N006",		/// VOX_62_N006
	"62-N008",		/// VOX_62_N008
	"62-N010",		/// VOX_62_N010
	"62-N012",		/// VOX_62_N012
};

namespace {

int const PRIORITY_NORMAL = 50;
int const PRIORITY_CRITICAL = 255;
int const SETTLE_TICKS = TIMER_SECOND;
int const GAP_TICKS = TIMER_SECOND / 2;

// The lines waiting to be spoken, the one speaking, and the silence between.
VoxQueueClass Queue;
AudioHandle Current;
VoxType CurrentVoice = VOX_NONE;
CDTimerClass<SystemTimerClass> SpeakGapTimer;


bool Is_Critical_Line(VoxType voice)
{
	return(voice == VOX_ACCOMPLISHED || voice == VOX_FAIL);
}


bool Is_Eva_Line(VoxType voice)
{
	return(strncmp(Speech[voice], "00-", 3) == 0 || strncmp(Speech[voice], "01-", 3) == 0);
}

} // namespace

static int SpeechVolume = 255;
static bool SpeechEnabled = true;

#ifdef _DEBUG
/***********************************************************************************************
 * Speech_Name -- Fetches the name for the voice specified.                                    *
 *                                                                                             *
 *    Use this routine to fetch the ASCII name of the speech id specified. Typical use of this *
 *    would be to build a displayable list of the speech types. The trigger system uses this   *
 *    so that a speech type can be selected.                                                   *
 *                                                                                             *
 * INPUT:   speech   -- The speech type id to convert to ASCII string.                         *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the speech ASCII representation of the speech id type.   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/01/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
char const * Speech_Name(VoxType speech)
{
	if (speech == VOX_NONE) return("none");
	return(Speech[speech]);
}
#endif


/***********************************************************************************************
 * Speak -- Computer speaks to the player.                                                     *
 *                                                                                             *
 *    This routine is used to have the game computer (EVA) speak to the player.                *
 *                                                                                             *
 * INPUT:   voice -- The voice number to speak (see defines.h).                                *
 *                                                                                             *
 * OUTPUT:  Returns with the handle of the playing speech (-1 if no voice started).            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/12/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void Speak(VoxType voice, bool now)
{
	if (Debug_Quiet || SpeechVolume <= 0 || !AudioEngine.Is_Available() || voice == VOX_NONE || voice >= VOX_COUNT) {
		return;
	}
	if (!SpeechEnabled && Is_Eva_Line(voice)) {
		return;
	}

	bool idle = !Current.Is_Valid() && Queue.Count() == 0;
	VoxControlType control = now ? VOXC_QUEUED_INTERRUPT : (Is_Critical_Line(voice) ? VOXC_CRITICAL : VOXC_QUEUE);
	int priority = Is_Critical_Line(voice) ? PRIORITY_CRITICAL : PRIORITY_NORMAL;
	if (Queue.Submit(voice, priority, control, Current.Is_Valid() ? CurrentVoice : VOX_NONE)) {
		AudioEngine.Stop_Stream(Current);
		Current.Clear();
		CurrentVoice = VOX_NONE;
	}

	// A burst settles for a second before its first line, unless one is wanted at once.
	if (idle) {
		SpeakTimer = SETTLE_TICKS;
		SpeakGapTimer = 0;
	}
	if (now) {
		SpeakTimer = 0;
	}
	Speak_AI();
}


/***********************************************************************************************
 * Speak_AI -- Handles starting the EVA voices.                                                *
 *                                                                                             *
 *    This starts the EVA voice talking as well. If there is any speech request in the queue,  *
 *    it will be started when the current voice is finished. Call this routine as often as     *
 *    possible (once per game tick is sufficient).                                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/27/1994 JLB : Created.                                                                 *
 *   10/11/1996 JLB : Handles multiple speech buffers.                                         *
 *=============================================================================================*/
void Speak_AI(void)
{
	if (Debug_Quiet || !AudioEngine.Is_Available()) return;

	if (Current.Is_Valid()) {
		return;
	}
	if (CurrentVoice != VOX_NONE) {
		// The line has ended; the next one waits its half second.
		CurrentVoice = VOX_NONE;
		SpeakGapTimer = GAP_TICKS;
	}
	if (Queue.Count() == 0 || SpeakTimer > 0 || SpeakGapTimer > 0) {
		return;
	}

	// A line whose file is missing is passed over for the next.
	VoxType next;
	while (Queue.Next(next)) {
		char name[_MAX_FNAME + _MAX_EXT];
		_makepath(name, NULL, NULL, Speech[next], ".AUD");
		Current = AudioEngine.Open_Stream(name, AUDIO_GROUP_SPEECH, (float)SpeechVolume / 255.0f, false);
		if (Current.Is_Valid()) {
			CurrentVoice = next;
			return;
		}
	}
}


/***********************************************************************************************
 * Stop_Speaking -- Forces the EVA voice to stop talking.                                      *
 *                                                                                             *
 *    Use this routine to immediately stop the EVA voice from speaking. It also clears out     *
 *    the pending voice queue.                                                                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/27/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void Stop_Speaking(void)
{
	Queue.Clear();
	if (Current.Is_Valid()) {
		AudioEngine.Stop_Stream(Current);
	}
	Current.Clear();
	CurrentVoice = VOX_NONE;
	SpeakGapTimer = 0;
}


/***********************************************************************************************
 * Is_Speaking -- Checks to see if the eva voice is still playing.                             *
 *                                                                                             *
 *    Call this routine when the EVA voice being played needs to be checked. A typical use     *
 *    of this would be when some action needs to be delayed until the voice has finished --    *
 *    say the end of the game.                                                                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Is the EVA voice still playing?                                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/12/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool Is_Speaking(void)
{
	Speak_AI();
	return(!Debug_Quiet && AudioEngine.Is_Available() && (Current.Is_Valid() || Queue.Count() > 0));
}


/// <summary>
/// Sets the volume that the EVA voice plays at.
/// The options screen calls this as the player drags the voice slider. Any speech already
/// playing has its volume adjusted to match, so the change is heard immediately.
/// </summary>
/// <param name="volume">The desired volume, where 255 is the loudest.</param>
void Set_Speech_Volume(int volume)
{
	SpeechVolume = std::min(volume, 255);
	if (Current.Is_Valid()) {
		Current.Set_Volume((float)std::max(SpeechVolume, 0) / 255.0f);
	}
}


/// <summary>
/// Sets whether the EVA voice lines may be spoken.
/// The trigger system uses this to keep EVA quiet while a scripted sequence plays. With
/// it turned off, only the speech outside the EVA voice set is allowed through.
/// </summary>
/// <param name="state">Should EVA be allowed to speak her own lines?</param>
void Set_Speech_State(bool state)
{
	SpeechEnabled = state;
}


/// <summary>
/// Determines if the EVA voice lines are enabled.
/// </summary>
/// <returns>bool; Is EVA allowed to speak her own lines?</returns>
bool Get_Speech_State(void)
{
	return(SpeechEnabled);
}
