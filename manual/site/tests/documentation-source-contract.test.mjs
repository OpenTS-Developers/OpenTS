import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import test from 'node:test';
import { fileURLToPath } from 'node:url';

const site = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const repository = resolve(site, '../..');
const source = (path) => readFileSync(resolve(repository, path), 'utf8');

function functionBody(text, signature) {
	const signatureAt = text.indexOf(signature);
	assert.notEqual(signatureAt, -1, `Missing source function ${signature}`);
	const open = text.indexOf('{', signatureAt + signature.length);
	assert.notEqual(open, -1, `Missing body for ${signature}`);
	let depth = 0;
	for (let index = open; index < text.length; index++) {
		if (text[index] === '{') depth++;
		if (text[index] !== '}') continue;
		depth--;
		if (depth === 0) return text.slice(open + 1, index);
	}
	assert.fail(`Unterminated body for ${signature}`);
}

function assertOrdered(text, needles, label) {
	let cursor = -1;
	for (const needle of needles) {
		const next = text.indexOf(needle, cursor + 1);
		assert.notEqual(next, -1, `${label} is missing ${JSON.stringify(needle)} after offset ${cursor}`);
		cursor = next;
	}
}

test('Drop pod approach selection keeps its ordered candidates and unconditional southwest fallback', () => {
	const droppod = source('code/droppod.cpp');
	const moveTo = functionBody(
		droppod,
		'void STDMETHODCALLTYPE DropPodLocomotionClass::Move_To(Coord to)',
	);

	assert.match(
		moveTo,
		/double\s+dropradius\s*=\s*\(double\)Rule->DropPodHeight\s*\/\s*std::tan\(Rule->DropPodAngle\)/,
	);
	assert.equal(
		(moveTo.match(/Map\.In_Local_Radar\(dropcoord\)/g) ?? []).length,
		3,
		'the southwest fallback must not add a fourth map-area predicate',
	);
	assertOrdered(moveTo, [
		'dropcoord.X += dropradius;',
		'Direction = DPOD_DIR_NE;',
		'dropcoord.X = DestinationCoord.X - dropradius;',
		'Direction = DPOD_DIR_NW;',
		'dropcoord.X = DestinationCoord.X;',
		'dropcoord.Y = DestinationCoord.Y + dropradius;',
		'Direction = DPOD_DIR_SE;',
		'dropcoord.Y = DestinationCoord.Y - dropradius;',
		'Direction = DPOD_DIR_SW;',
	], 'Drop pod approach selection');
});

test('Drop pod directions retain their hard-coded airborne and landing-art mapping', () => {
	const header = source('code/droppod.h');
	const droppod = source('code/droppod.cpp');
	const infantry = source('code/infantry.cpp');
	const directionEnum = header.match(/enum\s+DropPodDirType\s*{([\s\S]*?)}/)?.[1];
	assert.ok(directionEnum, 'DropPodDirType is missing');
	assert.deepEqual(
		[...directionEnum.matchAll(/DPOD_DIR_(NE|NW|SE|SW)/g)].map((match) => match[1]),
		['NE', 'NW', 'SE', 'SW'],
	);

	const drawingCode = functionBody(
		droppod,
		'int STDMETHODCALLTYPE DropPodLocomotionClass::Drawing_Code(void)',
	);
	assert.match(drawingCode, /Direction\s*%\s*2/);
	assertOrdered(infantry, [
		'MFCD::Retrieve("POD.SHP")',
		'Locomotion->Drawing_Code()',
	], 'Drop pod airborne shape selection');

	const process = functionBody(
		droppod,
		'boolean STDMETHODCALLTYPE DropPodLocomotionClass::Process(void)',
	);
	assert.match(process, /Rule->DropPod\[Direction\s*%\s*Rule->DropPod\.Count\(\)\]/);

	const moveTo = functionBody(
		droppod,
		'void STDMETHODCALLTYPE DropPodLocomotionClass::Move_To(Coord to)',
	);
	assertOrdered(moveTo, [
		'dropcoord.Z += Rule->DropPodHeight;',
		'LinkedTo->Unlimbo(dropcoord, DIR_S)',
		'new AnimClass(Rule->AtmosphereEntry, dropcoord);',
	], 'Drop pod elevated entry effect');
});

test('Blocked Drop pod touchdown retains its exact damage, animation, and deletion payload', () => {
	const process = functionBody(
		source('code/droppod.cpp'),
		'boolean STDMETHODCALLTYPE DropPodLocomotionClass::Process(void)',
	);
	assertOrdered(process, [
		'FootClass * linked = LinkedTo;',
		'coord = linked->PositionCoord;',
		'linked->Limbo();',
		'End_Piggyback(&LinkedTo->Locomotion);',
		'if (!linked->Unlimbo(coord, DIR_N)) {',
		'Explosion_Damage(coord, 100, LinkedTo, Rule->C4Warhead);',
		'Combat_Anim(100, Rule->C4Warhead, LAND_CLEAR, coord)',
		'linked->Delete_Me();',
	], 'Blocked Drop pod touchdown');
});

test('DropPodWeapon remains a null default loaded before object type registration', () => {
	const rules = source('code/rules.cpp');
	const constructorAt = rules.indexOf('RulesClass::RulesClass(void) :');
	assert.notEqual(constructorAt, -1);
	const constructorOpen = rules.indexOf('{', constructorAt);
	assert.match(rules.slice(constructorAt, constructorOpen), /DropPodWeapon\(NULL\)/);

	const general = functionBody(rules, 'bool RulesClass::General(CCINIClass const & ini)');
	assert.match(
		general,
		/DropPodWeapon\s*=\s*TGet_Class\(ini,\s*GENERAL,\s*"DropPodWeapon",\s*DropPodWeapon\)/,
	);

	const addition = functionBody(rules, 'bool RulesClass::Addition(CCINIClass const & ini)');
	assertOrdered(addition, ['General(ini);', 'Objects(ini);'], 'Rules addition order');
});

test('Drop pod superweapon placement draws on one shared 3-per-passenger attempt budget', () => {
	const dropPods = functionBody(
		source('code/super.cpp'),
		'void SuperClass::Drop_Pods(Cell const & cell) const',
	);
	assert.match(
		dropPods,
		/int count = Random_Pick\(Rule->DropPodInfantryMinimum, Rule->DropPodInfantryMaximum\);/,
	);
	assert.match(dropPods, /int attempts = 3 \* count;/);
	assert.match(dropPods, /while \(toplace && attempts--\)/);
});

test('Find_Or_Make reserves the none aliases as null before the registry lookup', () => {
	const findOrMake = functionBody(
		source('code/findmake.h'),
		'T * TFind_Or_Make(char const * name, DynamicVectorClass<T *> const & vector)',
	);
	assertOrdered(findOrMake, [
		'strcmpi("<none>", name)',
		'strcmpi("none", name)',
		'return(new T(name));',
	], 'Find_Or_Make reserved values');
});

test('Building main-shape Image is additive to the inherited ObjectType Image reader', () => {
	const objectType = functionBody(
		source('code/objtype.cpp'),
		'bool ObjectTypeClass::Read_INI(CCINIClass const & ini)',
	);
	assert.match(
		objectType,
		/ini\.Get_String\(IniName,\s*"Image",\s*GraphicName\)/,
	);

	const building = source('code/builtype.cpp');
	const buildingRead = functionBody(
		building,
		'bool BuildingTypeClass::Read_INI(CCINIClass const & ini)',
	);
	assert.match(buildingRead, /BASECLASS::Read_INI\(ini\)/);

	const fetchImage = functionBody(
		building,
		'void BuildingTypeClass::Fetch_Building_Normal_Image(TheaterType theater)',
	);
	assert.match(
		fetchImage,
		/ArtINI\.Get_String\(Graphic_Name\(\),\s*"Image",\s*"",\s*buffer,\s*sizeof\(buffer\)\)/,
	);
	assertOrdered(fetchImage, [
		'ArtINI.Get_String(Graphic_Name(), "Image", "", buffer, sizeof(buffer));',
		'if (strlen(buffer)) {',
		'_makepath(fullname, NULL, NULL, buffer, ext);',
		'_makepath(fullname, NULL, NULL, Graphic_Name(), ext);',
		'strncpy(TheaterImageFile, fullname, sizeof(TheaterImageFile) - 1);',
	], 'Building main-shape selection');
	assert.doesNotMatch(fetchImage, /\bGraphicName\s*=/);
});

test('Every field the launch file reader carries is bound or named as unhonored', () => {
	const header = source('code/spawnerconfig.h');
	const spawner = source('code/spawner.cpp');

	assert.match(
		spawner,
		/Read, not honored/,
		'the binding step keeps its ledger of fields it deliberately leaves alone',
	);

	const fields = [];
	for (const line of header.split('\n')) {
		const declaration = /^\t{2,3}(?!static |enum |struct |\/)[A-Za-z_][^;(]*?[\s>*&]([A-Za-z_]\w*)\s*(?:=[^;]*)?;\s*$/.exec(line);
		if (declaration) fields.push(declaration[1]);
	}
	assert.ok(fields.length > 30, `expected the reader to carry many fields, found ${fields.length}`);

	for (const field of fields) {
		assert.match(
			spawner,
			new RegExp(String.raw`\b${field}\b`),
			`${field} is read from a launch file but code/spawner.cpp neither binds it nor names it in the "Read, not honored" ledger`,
		);
	}
});

test('A session node is left to its own constructor rather than zeroed by hand', () => {
	assert.doesNotMatch(
		source('code/netdlg2.cpp'),
		/memset\(who, 0, sizeof\(\*who\)\)/,
		'zeroing a node by hand would wipe the defaults its constructor sets',
	);
});

test('House assignment takes each seat as written before the neutral houses exist', () => {
	const assign = functionBody(source('code/scenario.cpp'), 'void Assign_Houses(void)');

	assertOrdered(assign, [
		'housep->SpawnWaypoint = player->Player.SpawnChoice;',
		'seat->Player.House != -1',
		'seat->Player.Color != -1',
		'seat->Player.Handicap >= 0',
		'housep->SpawnWaypoint = seat->Player.SpawnChoice;',
		'seat->Player.ID = housep->HeapID;',
	], 'a seated house takes its country, color, difficulty and start position');

	assertOrdered(assign, [
		'Seated_Node(seatnum)',
		'Make_Ally',
		'HouseTypeClass::From_Name("Neutral")',
	], 'the alliance table names seats, so it is applied before any house that is not one');
});

test('A chosen start position keeps its number and is claimed before the game picks', () => {
	const scenario = source('code/scenario.cpp');

	const assign = functionBody(
		scenario.slice(scenario.search(/static void Assign_Start_Positions\(bool official\)\s*\{/)),
		'static void Assign_Start_Positions(bool official)',
	);
	assertOrdered(assign, [
		'housep->SpawnWaypoint = -1;',
		'if (official && !choices) {',
		'open[spot] = spot < look_for && Scen->Is_Valid_Waypoint(spot);',
		'if (spot < MAX_PLAYERS && open[spot]) {',
		'held[spot] = true;',
		'housep->SpawnWaypoint >= 0) {',
		'best = candidates[Random_Pick(0, count - 1)];',
		'housep->SpawnWaypoint = best;',
	], 'every named position is held before the game picks for anybody who named none');

	const read = functionBody(
		scenario.slice(scenario.search(/bool Read_Scenario_INI\(CCINIClass const & ini, bool is_mapgen\)\s*\{/)),
		'bool Read_Scenario_INI(CCINIClass const & ini, bool is_mapgen)',
	);
	assertOrdered(read, [
		'Scen->Read_Waypoints(ini);',
		'Assign_Start_Positions(official);',
		'Read_Spawn_Houses(ini);',
		'TeamTypeClass::Read_All(AIINI, SCOPE_GLOBAL);',
	], 'positions are settled and the spawn house sections read before any team, trigger or object');
});

test('A house following a map plan builds under the campaign rules', () => {
	const house = source('code/house.cpp');

	assert.match(
		functionBody(house, 'bool HouseClass::Can_Build_Here(BuildingTypeClass *building, Cell const & cell)'),
		/if \(Scen->Is_Campaign_Base_AI\(\)\) \{\s*return\(true\);/,
		'the compactness test passes for a house following a map plan',
	);
	assert.match(
		functionBody(house, 'int HouseClass::AI_Building(void)'),
		/if \(!Scen->Is_Campaign_Base_AI\(\) && b->Drain \+ Drain > Power - PowerSurplus/,
		'a power plant is inserted only for a house not following a map plan',
	);
	assert.match(
		functionBody(house, 'int HouseClass::Expert_AI(void)'),
		/if \(!Scen->Is_Campaign_Base_AI\(\)\) \{/,
		'money and fire-sale interventions run only for a house not following a map plan',
	);
	assert.match(
		functionBody(house, 'void HouseClass::Invalidate_Base_Node_Position(BuildingClass * building)'),
		/building->Class->IsBaseDefense && !Scen->Is_Campaign_Base_AI\(\)/,
		'a base defense node is retired only for a house not following a map plan',
	);
	assert.match(
		functionBody(source('code/scenario.cpp'), 'bool ScenarioClass::Is_Campaign_Base_AI(void) const'),
		/Session\.Type == GAME_NORMAL \|\| IsMPAIBaseNodes/,
		'the campaign rules apply in a campaign or when the map asks for them',
	);
});

test('Starting units are placed from three to thirty-two cells out and are no longer scattered', () => {
	const scenario = source('code/scenario.cpp');

	assert.match(
		source('code/scenario.h'),
		/int Scan_Place_Object\(ObjectClass \* obj, Cell const & cell, int min_dist = 1, int max_dist = 31\);/,
		'the base unit keeps the one-to-thirty-one search by default',
	);

	const scan = functionBody(
		scenario,
		'int Scan_Place_Object(ObjectClass * obj, Cell const & cell, int min_dist, int max_dist)',
	);
	assertOrdered(scan, [
		'if (Map.In_Radar(cell)) {',
		'for (dist = min_dist; dist <= max_dist; dist++) {',
		'newcell = Clip_Move(cell, rot, dist);',
		'newcell = Clip_Scatter(newcell, 1);',
		'if (Map.In_Radar(newcell) && !skipit) {',
	], 'the start cell is tried first, then each distance from min_dist to max_dist, inside the playfield only');

	const create = functionBody(
		scenario.slice(scenario.search(/static void Create_Units\(bool official\)\s*\{/)),
		'static void Create_Units(bool official)',
	);
	assert.match(create, /int average_cost = total_objs > 0 \? total_cost \/ total_objs : 0;/, 'an empty pool gives a zero budget instead of dividing by zero');
	assert.match(create, /constexpr int MIN_PLACEMENT_DISTANCE = 3;/);
	assert.match(create, /constexpr int MAX_PLACEMENT_DISTANCE = 32;/);
	assertOrdered(create, [
		'tech = infantry[Random_Pick(0, infantry.Count() - 1)];',
		'if (tech == NULL) {',
		'break;',
		'tech->Create_One_Of(hptr)',
	], 'a house with nothing left to draw stops before calling through a type it never picked');
	assertOrdered(create, [
		'if (!Scan_Place_Object(obj, centroid)) {',
		'Scan_Place_Object(obj, centroid, MIN_PLACEMENT_DISTANCE, MAX_PLACEMENT_DISTANCE)',
		'DebugString("Finished unit generation. Random number is %d\\n", Random_Pick(0, 65535));',
	], 'the base unit keeps the default search, the random objects use the ring, and the sync checkpoint stays last');
	assert.equal(
		(create.match(/Scan_Place_Object\(obj, centroid\)/g) ?? []).length,
		1,
		'only the base unit is placed with the default search',
	);
	assert.doesNotMatch(create, /->Scatter\(/, 'no starting object is scattered after placement');
	assert.doesNotMatch(create, /deployed_list/, 'the list that existed only to scatter is gone');
});

test('The campaign handicap pair lives on the session, and the mission reader never asks the spawner', () => {
	const scenario = source('code/scenario.cpp');

	assertOrdered(
		functionBody(scenario, 'bool Read_Scenario_INI(CCINIClass const & ini, bool is_mapgen)'),
		[
			'Scen->Difficulty = Session.CampaignDifficulty;',
			'Scen->CDifficulty = Session.CampaignCDifficulty;',
		],
		'the mission takes the pair the session carries',
	);
	assert.doesNotMatch(
		scenario,
		/#include "spawner\.h"/,
		'the mission reader has no line to the spawner',
	);

	assertOrdered(
		functionBody(source('code/init.cpp'), 'bool Select_Game(bool )'),
		[
			'Session.CampaignDifficulty = (DiffType)Options.Difficulty;',
			'Session.CampaignCDifficulty = (DiffType)(DIFF_COUNT - 1 - Options.Difficulty);',
		],
		'the menu derives the pair the mission reader used to compute, ahead of the start',
	);
});

test('A campaign spawn writes the game its own state and nothing more', () => {
	const spawner = source('code/spawner.cpp');

	assertOrdered(functionBody(spawner, 'static bool Spawner_Setup_Campaign(void)'), [
		'Session.Type = GAME_NORMAL;',
		'Session.CampaignDifficulty = (DiffType)SpawnConfig.CampaignDifficulty;',
		'Session.CampaignCDifficulty = (DiffType)SpawnConfig.CampaignCDifficulty;',
		'Scen->Campaign = (CampaignType)SpawnConfig.CampaignID;',
		'new (&Environment) EnvironmentClass;',
		'Environment.Globals[index] = SpawnConfig.GlobalFlags[index];',
	], 'a campaign launch lands in the game’s own state');

	assertOrdered(functionBody(source('code/init.cpp'), 'bool Select_Game(bool )'), [
		'Spawner_Is_Active() ? Scen->Campaign : CAMPAIGN_NONE',
		'Scen->Set_Global_To(index, Environment.Globals[index]);',
	], 'a spawned mission is named by the file and starts with the flags it carried');
});

test('A resume is judged before it is loaded, and the save answers for the rest', () => {
	assertOrdered(functionBody(source('code/spawner.cpp'), 'static bool Spawner_Resume(bool & gameloaded)'), [
		'SpawnConfig.SaveGameName.empty()',
		'Get_Savefile_Info(SpawnConfig.SaveGameName.c_str(), &info)',
		'info.Get_Internal_Version() != ExpectedGameVersion',
		'type == GAME_IPX',
		'SpawnConfig.Is_Playable(HouseTypes.Count(), MAX_MPLAYER_COLORS, fault)',
		'Spawner_Seat_Humans();',
		'Spawner_Wire_Network()',
		'Session.LoadGame = true;',
		'LoadOptionsClass().Load_File(SpawnConfig.SaveGameName.c_str())',
		'Reconcile_Players()',
		'gameloaded = true;',
	], 'a network resume seats the players and opens the network before the save is read');

	for (const dialog of ['IDD_OPT_CTRL_WOL']) {
		const template = source('code/language/language.rc');
		const body = template.slice(template.indexOf(dialog + ' DIALOG'));
		assert.match(
			body.slice(0, body.indexOf('END')),
			/IDC_SAVE_GAME/,
			`${dialog} offers the synchronized save the options handler has always known`,
		);
	}

	assertOrdered(functionBody(source('code/saveload.cpp'), 'bool Reconcile_Players(void)'), [
		'stricmp(Session.Players[i]->Name, Houses[house]->IniName) == 0',
		'Session.Players[i]->Player.ID = found->HeapID;',
		'Houses[Session.Players[0]->Player.ID] != PlayerPtr',
		'housep->IsHuman = false;',
		'housep->IniName = Fetch_String(TXT_COMPUTER);',
	], 'every seat is matched and this machine identified before any house changes hands');

	assertOrdered(functionBody(source('code/saveload.cpp'), 'bool Load_Game(const char *file_name)'), [
		'Session.Type = (GameType)info.Get_Game_Type();',
		'Post_Load_Game();',
		'Session.CampaignDifficulty = Scen->Difficulty;',
		'Session.CampaignCDifficulty = Scen->CDifficulty;',
	], 'a load takes the kind of game and the campaign pair from the save');
});

test('Saved games are named in one folder rather than searched for', () => {
	const gamedirs = source('code/gamedirs.cpp');

	assertOrdered(functionBody(gamedirs, 'std::string Saved_Game_Name(char const * filename)'), [
		'UserDirectory + SavedGamesFolder',
		'CreateDirectory(folder.c_str(), NULL);',
	], 'a saved game is named inside the user directory, and the folder is made on the way');

	for (const [file, signature] of [
		['code/saveload.cpp', 'bool Save_Game(const char *file_name, char const * descr)'],
		['code/saveload.cpp', 'bool Load_Game(const char *file_name)'],
		['code/saveload.cpp', 'bool Get_Savefile_Info(char const * name, SaveVersionInfo * info)'],
		['code/loaddlg.cpp', 'void LoadOptionsClass::Fill_List(HWND window)'],
		['code/loaddlg.cpp', 'bool LoadOptionsClass::Files_Present(void)'],
		['code/loaddlg.cpp', 'bool LoadOptionsClass::Delete_File(const char * file_name)'],
	]) {
		assert.match(
			functionBody(source(file), signature),
			/Saved_Game_Name\(/,
			`${signature} names the folder saved games are kept in`,
		);
	}

	assert.doesNotMatch(
		functionBody(source('code/loaddlg.cpp'), 'void LoadOptionsClass::Fill_List(HWND window)') +
			functionBody(source('code/loaddlg.cpp'), 'bool LoadOptionsClass::Files_Present(void)'),
		/Search_Files\(/,
		'the listing no longer scans the folders the game reads from',
	);
});

test('Automatic saves are serviced at the frame boundary ahead of the pending write', () => {
	assertOrdered(functionBody(source('code/mainloop.cpp'), 'bool Main_Loop(void)'), [
		'Frame++;',
		'Process_Deferred_Deletion();',
		'SaveManager.Service();',
	], 'the save manager runs after the frame has retired its dead objects');
	assertOrdered(functionBody(source('code/savemgr.cpp'), 'void SaveManagerClass::Service(void)'), [
		'Autosave_Service();',
		'Quick_Save_Service();',
		'Process_Pending_Save_Game();',
		'Process_Pending_Load_Game();',
	], 'an automatic save is written after the frame has retired its dead objects, and an agreed load after that');
});

// A definition that shares its text with a forward declaration is found from the end.
function definitionFrom(text, signature) {
	const at = text.lastIndexOf(signature);
	assert.notEqual(at, -1, `Missing source function ${signature}`);
	return text.slice(at);
}

test('An out-of-sync frame is reported before the players are asked to decide', () => {
	assertOrdered(definitionFrom(source('code/queue.cpp'), 'static int Execute_DoList(int max_houses, HousesType base_house,'), [
		'Report_Out_Of_Sync(mismatches, mismatch_count, CRC, ARRAY_SIZE(CRC));',
		'Multiplayer_Load_Is_Pending()',
		'DesyncDialog.Run()',
		'Destroy_Connection(id, -1);',
		'Sign_Off_Match();',
	], 'the report describes the frame before any decision changes the session');
});

test('A multiplayer load replaces the match around the seats it keeps', () => {
	assertOrdered(functionBody(source('code/savemgr.cpp'), 'bool SaveManagerClass::Perform_Multiplayer_Load(char const * file_name)'), [
		'PacketTransport->Discard_In_Buffers();',
		'Ipx.Delete_Connection(Ipx.Connection_ID(0));',
		'DoList.clear();',
		'Session.LoadGame = true;',
		'LoadOptionsClass().Load_File(file_name)',
		'Reconcile_Players()',
		'Session.Create_Connections()',
		'Spawner_Announce_Master();',
		'Reset_Multiplayer_Save_State();',
	], 'the old traffic is discarded, the save read, the seats matched, and the connections rebuilt in that order');

	const template = source('code/language/language.rc');
	const body = template.slice(template.indexOf('IDD_OPT_CTRL_WOL DIALOG'));
	assert.match(
		body.slice(0, body.indexOf('END')),
		/IDC_LOAD_GAME/,
		'the internet options offer the load the master starts for every machine',
	);

	assertOrdered(definitionFrom(source('code/goptions.cpp'), 'BOOL CALLBACK Game_Options_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)'), [
		'case IDC_LOAD_GAME:',
		'LoadOptionsClass().Load()',
		'Multiplayer_Load_Is_Allowed()',
		'SpecialDialog = SDLG_LOAD;',
	], 'a network game defers the list to the menu loop rather than nesting it in the options dialog');

	assertOrdered(definitionFrom(source('code/conquer.cpp'), 'void Ingame_Menu_Dialog(void)'), [
		'case SDLG_OPTIONS:',
		'case SDLG_LOAD:',
		'Multiplayer_Load_Prompt()',
	], 'the menu loop opens the multiplayer list between frames, where the match keeps running under it');
});

test('A match against other machines is assembled whole and wired to its network last', () => {
	const spawner = source('code/spawner.cpp');

	assertOrdered(functionBody(spawner, 'bool Spawner_Prepare(bool & gameloaded)'), [
		'SpawnConfig.Is_Playable(HouseTypes.Count(), MAX_MPLAYER_COLORS, fault)',
		'Spawner_Setup_Session();',
		'SpawnConfig.Session_Identity_CRC()',
		'Session.Type == GAME_INTERNET && !Spawner_Wire_Network()',
	], 'the match is judged, assembled and named before its network is opened');

	assertOrdered(functionBody(spawner, 'static bool Spawner_Wire_Network(void)'), [
		'Ipx.Configure_Tunnel(',
		'Ipx.Configure_Direct_Peers(',
		'Ipx.Add_Peer(Session.Players[index]->Address);',
		'if (!Ipx.Init()) {',
	], 'the transport is chosen, the peers named, and only then the network opened');

	assertOrdered(functionBody(source('code/scenario.cpp'), 'static NodeNameType * Seated_Node(int seat)'), [
		'Session.Players[i]->Player.ID == seat',
		'Session.Computers[i]->Player.ID == seat',
	], 'a seat is found by the house it was assigned, not by its place in the list');


	assert.match(
		functionBody(spawner, 'static void Spawner_Setup_Session(void)'),
		/LaunchType::Multiplayer\s*\n?\s*\?\s*GAME_INTERNET : GAME_SKIRMISH;/,
		'one assembly serves both kinds of match',
	);

	assertOrdered(functionBody(spawner, 'static void Spawner_Seat_Human(int index)'), [
		'if (SpawnConfig.TunnelPort != 0) {',
		'node->Address.Set_Address(0, htons((unsigned short)seat.Port));',
		'inet_addr(seat.Address.c_str())',
	], 'a tunnelled machine is named by its tunnel number before an address is read');

	assertOrdered(functionBody(spawner, 'static void Spawner_Seat_Humans(void)'), [
		'Spawner_Seat_Human(SpawnConfig.LocalSlot);',
		'if (index != SpawnConfig.LocalSlot) {',
	], 'the local seat leads the player list the rest of the game reads');

	assertOrdered(functionBody(spawner, 'static void Spawner_Setup_Session(void)'), [
		'GAME_INTERNET : GAME_SKIRMISH;',
		'Seed = SpawnConfig.Seed;',
	], 'one seed is taken as written, since no lobby hands one around');

	assertOrdered(
		functionBody(
			source('code/spawnerconfig.cpp'),
			'bool SpawnerConfigClass::Is_Playable(int countries, int colors, std::string & fault) const',
		),
		[
			'kind == LaunchType::Multiplayer ||',
			'(kind == LaunchType::Resume && HumanCount > 1)',
			'if (human && multiplayer) {',
			'slot.Name.empty()',
			'_stricmp(Slots[other].Name.c_str(), slot.Name.c_str()) == 0',
			'Slots[other].Color == slot.Color',
			'slot.Port < 1 || slot.Port > 65535',
		],
		'the seat order the machines share is what the name and color rules are held for',
	);
});
