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

	const build = functionBody(
		scenario,
		'static DynamicVectorClass<Cell> Build_Start_Waypoint_List(bool official, bool keep_identity)',
	);
	assertOrdered(build, [
		'if (keep_identity) {',
		'waypts.Add(declared ? Scen->Get_Waypoint_Cell(waycount) : CELL_NONE);',
		'Append_Open_Start_Positions(',
		'return(waypts);',
	], 'the numbered list keeps an undeclared position as a hole and appends any shortfall past it');

	const create = functionBody(
		scenario.slice(scenario.search(/static void Create_Units\(bool official\)\s*\{/)),
		'static void Create_Units(bool official)',
	);
	assertOrdered(create, [
		'Houses[index]->SpawnWaypoint >= 0',
		'Build_Start_Waypoint_List(official, choices)',
		'taken[index] = choices && index < waypts.Count() && waypts[index] == CELL_NONE;',
		'if (choices && hptr->SpawnWaypoint >= 0',
		'} else if (numtaken == 0) {',
	], 'holes are spoken for before the claim, and the claim comes before the game picks');
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
		'type != GAME_NORMAL && type != GAME_SKIRMISH',
		'LoadOptionsClass().Load_File(SpawnConfig.SaveGameName.c_str())',
		'gameloaded = true;',
	], 'a save is named, found, stamped and of a kind that can be resumed before it is read');

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
		['code/saveload.cpp', 'static bool Save_Game(const char *file_name, char const * descr)'],
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
