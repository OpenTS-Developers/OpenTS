/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "voxlib.h"

#include "voxdrsys.h"
#include "wwfile.h"

#include <algorithm>

short VoxelPixelDeltaTable[VOXEL_BITMAP_WIDTH][2];
unsigned char VoxelNormalTranslateTable[VOXEL_PALETTE_SIZE];

/*
 * Array of pointers to the low-level voxel drawing functions.
 */
typedef void (__cdecl *VoxelFuncPtr)(VoxelFuncArgumentStruct *);
void __cdecl Draw_Voxel_Regular_Normals(VoxelFuncArgumentStruct * state);
void __cdecl Draw_Voxel_Reverse_Normals(VoxelFuncArgumentStruct * state);
void __cdecl Draw_Voxel_Regular_Normals_ZBuffer(VoxelFuncArgumentStruct * state);
void __cdecl Draw_Voxel_Reverse_Normals_ZBuffer(VoxelFuncArgumentStruct * state);
void __cdecl Draw_Voxel_Regular_Normals_Lighting(VoxelFuncArgumentStruct * state);
void __cdecl Draw_Voxel_Reverse_Normals_Lighting(VoxelFuncArgumentStruct * state);
void __cdecl Draw_Voxel_Regular_Normals_ZBuffer_Lighting(VoxelFuncArgumentStruct * state);
void __cdecl Draw_Voxel_Reverse_Normals_ZBuffer_Lighting(VoxelFuncArgumentStruct * state);
void __cdecl Draw_Voxel_Regular(VoxelFuncArgumentStruct * state);
void __cdecl Draw_Voxel_Reverse(VoxelFuncArgumentStruct * state);
void __cdecl Draw_Voxel_Regular_ZBuffer(VoxelFuncArgumentStruct * state);
void __cdecl Draw_Voxel_Reverse_ZBuffer(VoxelFuncArgumentStruct * state);

extern "C" {
void __cdecl Draw_Voxel_Regular_Normals_ASM(VoxelFuncArgumentStruct * state);
void __cdecl Draw_Voxel_Reverse_Normals_ASM(VoxelFuncArgumentStruct * state);
void __cdecl Draw_Voxel_Regular_Lighting_Normals_ASM(VoxelFuncArgumentStruct * state);
void __cdecl Draw_Voxel_Reverse_Lighting_Normals_ASM(VoxelFuncArgumentStruct * state);
void __cdecl Draw_Voxel_Regular_ASM(VoxelFuncArgumentStruct * state);
void __cdecl Draw_Voxel_Reverse_ASM(VoxelFuncArgumentStruct * state);
void __cdecl Draw_Voxel_UNUSED1_ASM(VoxelFuncArgumentStruct * state);
void __cdecl Draw_Voxel_UNUSED2_ASM(VoxelFuncArgumentStruct * state);
}

VoxelFuncPtr VoxelDrawFunctions[32] = {

	/// Assembly routines
	&Draw_Voxel_Regular_Normals_ASM,
	&Draw_Voxel_Reverse_Normals_ASM,
	&Draw_Voxel_Regular_Normals_ZBuffer,
	&Draw_Voxel_Reverse_Normals_ZBuffer,
	&Draw_Voxel_Regular_Lighting_Normals_ASM,
	&Draw_Voxel_Reverse_Lighting_Normals_ASM,
	&Draw_Voxel_Regular_Normals_ZBuffer_Lighting,
	&Draw_Voxel_Reverse_Normals_ZBuffer_Lighting,
	&Draw_Voxel_Regular_ASM,
	&Draw_Voxel_Reverse_ASM,
	&Draw_Voxel_Regular_ZBuffer,
	&Draw_Voxel_Reverse_ZBuffer,
	&Draw_Voxel_Regular_ASM,
	&Draw_Voxel_Reverse_ASM,
	&Draw_Voxel_Regular_ZBuffer,
	&Draw_Voxel_Reverse_ZBuffer,

	/// The same set again, with the C++ drawers in place of the assembly ones.
	&Draw_Voxel_Regular_Normals,
	&Draw_Voxel_Reverse_Normals,
	&Draw_Voxel_Regular_Normals_ZBuffer,
	&Draw_Voxel_Reverse_Normals_ZBuffer,
	&Draw_Voxel_Regular_Normals_Lighting,
	&Draw_Voxel_Reverse_Normals_Lighting,
	&Draw_Voxel_Regular_Normals_ZBuffer_Lighting,
	&Draw_Voxel_Reverse_Normals_ZBuffer_Lighting,
	&Draw_Voxel_Regular,
	&Draw_Voxel_Reverse,
	&Draw_Voxel_Regular_ZBuffer,
	&Draw_Voxel_Reverse_ZBuffer,
	&Draw_Voxel_Regular,
	&Draw_Voxel_Reverse,
	&Draw_Voxel_Regular_ZBuffer,
	&Draw_Voxel_Reverse_ZBuffer,
};

VoxelRenderOrientation VoxelRenderOrientations[VOXEL_BOUNDS_COUNT] = {
	{ 0, 0, 3, 1, 4, 1, 1, -1, -1 },
	{ 0, 1, 2, 0, 5, 1, 0, -1, 1 },
	{ 0, 2, 1, 3, 6, 0, 0, 1, 1 },
	{ 0, 3, 0, 2, 7, 0, 1, 1, -1 },
	{ 1, 4, 7, 5, 0, 1, 1, -1, -1 },
	{ 1, 5, 6, 4, 1, 1, 0, -1, 1 },
	{ 1, 6, 5, 7, 2, 0, 0, 1, 1 },
	{ 1, 7, 4, 6, 3, 0, 1, 1, -1 }
};

extern float VoxelNormals1[][3];
extern float VoxelNormals2[][3];
extern float VoxelNormals3[][3];
extern float VoxelNormals4[][3];

const Vector3 *VoxelNormalTables[] =
{
	NULL,
	(const Vector3 *)VoxelNormals1,
	(const Vector3 *)VoxelNormals2,
	(const Vector3 *)VoxelNormals3,
	(const Vector3 *)VoxelNormals4,
};


/// warning C4305: 'argument' : truncation from 'const double' to 'float'
#pragma warning(disable : 4305)

float VoxelNormals1[][3] = {
	{     0.54946297,       -0.000183,   -0.835518},
	{  0.00014400001,      0.54940403, -0.83555698},
	{    -0.54940403, -0.000068000001, -0.83555698},
	{       0.000106,     -0.54946297,   -0.835518},
	{     0.94900799,   0.00031599999, -0.31525001},
	{      -0.000186,      0.94899702, -0.31528401},
	{    -0.94899702,   0.00031800001, -0.31528401},
	{      -0.000447,     -0.94900799, -0.31525001},
	{     0.95084399,       -0.000279,  0.30967101},
	{       0.000202,      0.95084798,  0.30965701},
	{    -0.95084798, -0.000070000002,  0.30965701},
	{       0.000147,     -0.95084399,  0.30967101},
	{     0.55237001,       -0.000011,  0.83359897},
	{ 0.000019999999,      0.55238003,    0.833592},
	{    -0.55238003,  0.000057000001,  0.83359301},
	{-0.000066000001,     -0.55237001,  0.83359897}
};


float VoxelNormals2[][3] = {
	{  0.67121398,   0.19849201,    -0.714194},
	{  0.26964301,   0.58439398,     -0.76536},
	{   -0.040546,     0.096988,  -0.99445897},
	{ -0.57242799, -0.091913998,  -0.81478697},
	{ -0.17140099,  -0.57270998,  -0.80163902},
	{  0.36255699,  -0.30299899,  -0.88133103},
	{  0.81034702,  -0.34897199,    -0.470698},
	{    0.103962,   0.93867201,    -0.328767},
	{   -0.324047,   0.58766901,  -0.74137598},
	{ -0.80086499,   0.34046099,  -0.49264699},
	{ -0.66549802,  -0.59014702,  -0.45698899},
	{    0.314767,    -0.803002,    -0.506073},
	{  0.97262901,     0.151076,     -0.17655},
	{    0.680291,   0.68423599,  -0.26272699},
	{ -0.52007902,   0.82777703,    -0.210483},
	{ -0.96164399,    -0.179001,    -0.207847},
	{   -0.262714,    -0.937451,  -0.22840101},
	{    0.219707,  -0.97130102,  0.091124997},
	{  0.92380798,    -0.229975,   0.30608699},
	{-0.082488999,   0.97065997,     0.225866},
	{ -0.59179801,   0.69678998,   0.40528899},
	{ -0.92529601,   0.36660099,  0.097111002},
	{   -0.705051,  -0.68777502,     0.172828},
	{      0.7324,  -0.68036699, -0.026304999},
	{  0.85516202,   0.37458199,     0.358311},
	{  0.47300601,   0.83648002,     0.276705},
	{   -0.097617,   0.65411198,     0.750072},
	{ -0.90412402,    -0.153725,   0.39865801},
	{   -0.211916,  -0.85808998,   0.46773201},
	{  0.50022697,  -0.67440802,     0.543091},
	{    0.584539,    -0.110249,   0.80384099},
	{  0.43737301,   0.45464399,   0.77588898},
	{-0.042440999,  0.083318003,     0.995619},
	{ -0.59625101,   0.22013199,   0.77202803},
	{   -0.506455,  -0.39697701,   0.76544899},
	{ 0.070569001,  -0.47847399,   0.87526202}
};


float VoxelNormals3[][3] = {
	{   0.45651099, -0.073968001,     -0.88663799},
	{   0.50769401,   0.38511699,        -0.77067},
	{  0.095431998,   0.22666401,     -0.96928602},
	{  -0.35876599,   0.54318798,     -0.75910097},
	{    -0.361276,   0.13299499,     -0.92292601},
	{  -0.48311701,  -0.32406601,       -0.813375},
	{    -0.018073,    -0.197559,       -0.980124},
	{       0.3211,    -0.501477,     -0.80337799},
	{   0.79949099,  0.069615997,     -0.59662998},
	{     0.390971,   0.77130598,     -0.50222403},
	{  0.080782004,   0.61448997,       -0.784778},
	{     -0.73275,   0.41143101,     -0.54203498},
	{  -0.73525399, 0.0091019999,     -0.67773098},
	{  -0.80249399,  -0.39490801,     -0.44727099},
	{     -0.13413,  -0.58915502,     -0.79680902},
	{   0.71955299,  -0.37622699,     -0.58369303},
	{   0.96687502,     0.173593,       -0.187132},
	{     0.760831,   0.51910597,     -0.38944301},
	{    -0.114642,   0.87551898,     -0.46938601},
	{  -0.53236699,   0.76885903,       -0.354177},
	{  -0.96226698,     0.024977,     -0.27095801},
	{  -0.46738699,    -0.721986,     -0.51018202},
	{  0.058449998,  -0.85235399,     -0.51968902},
	{   0.49823299,  -0.74374002,     -0.44566301},
	{   0.93915099,  -0.27024499,       -0.212044},
	{   0.58393198,   0.80944198,       -0.061857},
	{     0.183797,   0.97322798,       -0.138007},
	{  -0.88435501,   0.45221901,       -0.115822},
	{    -0.943178,  -0.33206701,        0.012138},
	{  -0.69844002,  -0.70656699,       -0.113772},
	{    -0.228411,  -0.95470601,       -0.190694},
	{   0.73156399,    -0.675861,    -0.089588001},
	{   0.96925098,     0.046804,      0.24158201},
	{   0.85564703,   0.50347698,        0.119916},
	{  -0.25115299,   0.96794701, -0.000080999998},
	{  -0.64779502,   0.75674897,     0.087711997},
	{  -0.96916401,   0.14519399,          0.1991},
	{  -0.41479301,  -0.88896698,        0.194126},
	{   0.25077501,    -0.961178,       -0.115109},
	{   0.47862899,  -0.84259301,        0.246883},
	{   0.89004397,  -0.39614201,        0.225595},
	{   0.52405101,   0.76235998,      0.37970701},
	{      0.11962,   0.94548202,         0.30291},
	{  -0.76085001,   0.49007499,      0.42536199},
	{  -0.86978501,     -0.20215,        0.450122},
	{  -0.70946699,  -0.60242403,      0.36570701},
	{  0.019308999,  -0.95887101,      0.28318599},
	{     0.626113,    -0.564677,      0.53770101},
	{     0.769943,    -0.126663,      0.62541503},
	{   0.76419097,   0.35070199,      0.54131401},
	{    -0.001878,   0.74136698,      0.67109799},
	{  -0.37088001,   0.81836802,      0.43900099},
	{  -0.71390897,   0.12865201,      0.68831801},
	{    -0.295165,  -0.73866397,      0.60601401},
	{     0.186195,  -0.73836899,        0.648184},
	{     0.387523,  -0.35878301,      0.84917599},
	{     0.481022,     0.124846,      0.86777401},
	{     0.391808,   0.54505599,        0.741216},
	{-0.0035359999,   0.36559799,      0.93076599},
	{  -0.42049801,     0.484961,      0.76680797},
	{  -0.35490301,  0.019470001,      0.93470001},
	{  -0.54783702,  -0.35920799,      0.75554299},
	{    -0.106662,    -0.445115,      0.88909799},
	{  0.086796001, -0.059307002,      0.99445897}
};


float VoxelNormals4[][3] = {
	{   0.52657801,   -0.35962099,   -0.77031702},
	{     0.150482,    0.43598399,    0.88728398},
	{     0.414195,    0.73825502,   -0.53237402},
	{  0.075152002,    0.91624898,     -0.393498},
	{    -0.316149,    0.93073601,   -0.18379299},
	{  -0.77381903,    0.62333399,      -0.11251},
	{  -0.90084201,    0.42853701,  -0.069568001},
	{  -0.99894202,     -0.010971,   0.044665001},
	{    -0.979761,   -0.15767001,     -0.123324},
	{  -0.91127402,     -0.362371,      -0.19562},
	{  -0.62406898,   -0.72094101,     -0.301301},
	{    -0.310173,   -0.80934501,     -0.498752},
	{     0.146613,   -0.81581903,   -0.55941403},
	{  -0.71651602,   -0.69435602,  -0.066887997},
	{   0.50397199,     -0.114202,   -0.85613698},
	{   0.45549101,    0.87262702,     -0.176211},
	{     -0.00501,     -0.114373,   -0.99342501},
	{    -0.104675,     -0.327701,   -0.93896502},
	{   0.56041199,    0.75258899,   -0.34575599},
	{ -0.060575999,    0.82162797,     -0.566796},
	{  -0.30234101,    0.79700702,     -0.522847},
	{    -0.671543,    0.67074001,     -0.314863},
	{  -0.77840102,   -0.12835699,    0.61450499},
	{  -0.92404997,      0.278382,     -0.261985},
	{  -0.69977301,   -0.55049098,   -0.45527801},
	{  -0.56824797,   -0.51718903,   -0.64000797},
	{  0.054097999,   -0.93286401,     -0.356143},
	{   0.75838202,    0.57289302,   -0.31088799},
	{ 0.0036200001,    0.30502599,   -0.95233703},
	{ -0.060849998,   -0.98688602,   -0.14951099},
	{      0.63523,   0.045478001,   -0.77098298},
	{   0.52170497,      0.241309,   -0.81828701},
	{   0.26940399,    0.63542497,   -0.72364098},
	{     0.045676,    0.67275399,     -0.738455},
	{    -0.180511,    0.67465699,   -0.71571898},
	{    -0.397131,    0.63664001,   -0.66104198},
	{  -0.55200398,    0.47251499,     -0.687038},
	{  -0.77217001,       0.08309,      -0.62996},
	{    -0.669819,     -0.119533,      -0.73284},
	{  -0.54045498,   -0.31844401,   -0.77878201},
	{  -0.38613501,     -0.522789,   -0.75999397},
	{    -0.261466,   -0.68856698,     -0.676395},
	{    -0.019412,   -0.69610298,   -0.71767998},
	{   0.30356899,   -0.48184401,   -0.82199299},
	{   0.68193901,   -0.19512901,   -0.70490003},
	{  -0.24488901,     -0.116562,   -0.96251899},
	{   0.80075902,  -0.022979001,   -0.59854603},
	{  -0.37027499,   0.095583998,   -0.92399102},
	{  -0.33067101,   -0.32657799,   -0.88543999},
	{     -0.16322,   -0.52757901,   -0.83367902},
	{      0.12639,     -0.313146,     -0.941257},
	{   0.34954801,   -0.27222601,   -0.89649802},
	{   0.23991799,  -0.085825004,   -0.96699202},
	{     0.390845,   0.081537001,   -0.91683799},
	{   0.25526699,    0.26869699,   -0.92878503},
	{     0.146245,    0.48043799,   -0.86474901},
	{  -0.32601601,    0.47845599,   -0.81534898},
	{  -0.46968201,     -0.112519,   -0.87563598},
	{   0.81844002,   -0.25852001,   -0.51315099},
	{    -0.474318,      0.292238,   -0.83043301},
	{     0.778943,    0.39584199,   -0.48637101},
	{   0.62409401,    0.39377299,   -0.67487001},
	{   0.74088597,      0.203834,   -0.63995302},
	{   0.48021701,      0.565768,   -0.67029703},
	{   0.38093001,    0.42453501,   -0.82137799},
	{ -0.093422003,    0.50112402,   -0.86031801},
	{    -0.236485,    0.29619801,   -0.92538702},
	{    -0.131531,   0.093959004,   -0.98684901},
	{  -0.82356203,    0.29577699,   -0.48400599},
	{   0.61106598,     -0.624304,     -0.486664},
	{  0.069495998,   -0.52033001,   -0.85113299},
	{     0.226522,   -0.66487902,     -0.711775},
	{   0.47130799,   -0.56890398,   -0.67395699},
	{   0.38842499,   -0.74262398,      -0.54556},
	{   0.78367501,   -0.48072901,   -0.39338499},
	{     0.962394,      0.135676,     -0.235349},
	{     0.876607,      0.172034,     -0.449406},
	{   0.63340503,    0.58979303,   -0.50094098},
	{     0.182276,    0.80065799,   -0.57072097},
	{     0.177003,    0.76413399,    0.62029701},
	{    -0.544016,      0.675515,   -0.49772099},
	{  -0.67929697,    0.28646699,   -0.67564201},
	{  -0.59039098,   0.091369003,     -0.801929},
	{  -0.82436001,   -0.13312399,   -0.55018902},
	{  -0.71579403,   -0.33454201,   -0.61296099},
	{   0.17428599,   -0.89248401,      0.416049},
	{ -0.082528003,   -0.83712298,   -0.54075301},
	{   0.28333101,   -0.88087398,   -0.37918901},
	{     0.675134,   -0.42662701,   -0.60181701},
	{   0.84372002,     -0.512335,     -0.160156},
	{   0.97730398,  -0.098555997,      -0.18752},
	{     0.846295,      0.522672,     -0.102947},
	{   0.67714101,    0.72132498,     -0.145501},
	{   0.32096499,    0.87089199,   -0.37219399},
	{    -0.178978,      0.911533,   -0.37023601},
	{  -0.44716901,    0.82670099,     -0.341474},
	{  -0.70320302,      0.496328,   -0.50908101},
	{  -0.97718102,   0.063562997,     -0.202674},
	{  -0.87817001,     -0.412938,      0.241455},
	{  -0.83583099,   -0.35855001,     -0.415728},
	{    -0.499174,   -0.69343299,   -0.51959199},
	{    -0.188789,   -0.92375302,   -0.33322501},
	{   0.19225401,   -0.96936101,     -0.152896},
	{   0.51594001,     -0.783907,   -0.34539199},
	{   0.90592498,   -0.30095199,   -0.29787099},
	{   0.99111199,     -0.127746,   0.037106998},
	{   0.99513501,   0.098424003, -0.0043830001},
	{   0.76012301,    0.64627701,   0.067367002},
	{     0.205221,       0.95958,     -0.192591},
	{ -0.042750001,    0.97951299,   -0.19679099},
	{  -0.43801701,    0.89892697,  0.0084920004},
	{  -0.82199401,    0.48078501,   -0.30523899},
	{  -0.89991701,   0.081710003,   -0.42833701},
	{  -0.92661202,     -0.144618,     -0.347096},
	{  -0.79365999,   -0.55779201,   -0.24283899},
	{  -0.43134999,   -0.84777898,   -0.30855799},
	{-0.0054919999,   -0.96499997,    0.26219299},
	{   0.58790499,   -0.80402601,  -0.088940002},
	{   0.69949299,   -0.66768599,     -0.254765},
	{   0.88930303,      0.359795,     -0.282291},
	{     0.780972,      0.197037,    0.59267199},
	{   0.52012098,    0.50669599,    0.68755698},
	{   0.40389499,    0.69396102,    0.59605998},
	{    -0.154983,    0.89923602,    0.40909001},
	{  -0.65733802,    0.53716803,      0.528543},
	{  -0.74619502,    0.33409101,      0.575827},
	{  -0.62495202,     -0.049144,    0.77911502},
	{   0.31814101,     -0.254715,      0.913185},
	{    -0.555897,      0.405294,      0.725752},
	{  -0.79443401,   0.099405997,    0.59916002},
	{  -0.64036101,   -0.68946302,    0.33849499},
	{  -0.12671299,   -0.73409498,    0.66711998},
	{     0.105457,   -0.78081697,    0.61579502},
	{   0.40799299,   -0.48091599,    0.77605498},
	{   0.69513601,      -0.54512,      0.468647},
	{   0.97319102, -0.0064889998,      0.229908},
	{   0.94689399,      0.317509,  -0.050799001},
	{   0.56358302,    0.82561201,      0.027183},
	{     0.325773,    0.94542301,  0.0069490001},
	{    -0.171821,    0.98509699, -0.0078149997},
	{  -0.67044097,    0.73993897,   0.054768998},
	{    -0.822981,    0.55496198,      0.121322},
	{  -0.96619302,      0.117857,      0.229307},
	{  -0.95376903,   -0.29470399,      0.058945},
	{  -0.86438698,   -0.50272799,     -0.010015},
	{  -0.53060901,   -0.84200603,  -0.097365998},
	{    -0.162618,   -0.98407501,   0.071772002},
	{  0.081446998,   -0.99601102,   0.036439002},
	{   0.74598402,   -0.66596299, 0.00076199998},
	{   0.94205701,   -0.32926899,  -0.064106002},
	{   0.93970197,   -0.28108999,      0.194803},
	{   0.77121401,    0.55067003,      0.319363},
	{     0.641348,       0.73069,    0.23402099},
	{  0.080682002,    0.99669099,  0.0098789996},
	{ -0.046725001,    0.97664303,    0.20972501},
	{  -0.53107601,    0.82100099,      0.209562},
	{  -0.69581503,       0.65599,    0.29243499},
	{  -0.97612202,      0.216709,     -0.014913},
	{  -0.96166098,   -0.14412899,    0.23331399},
	{    -0.772084,   -0.61364698,      0.165299},
	{  -0.44960001,   -0.83605999,      0.314426},
	{  -0.39269999,   -0.91461599,   0.096247002},
	{     0.390589,   -0.91947001,   0.044890001},
	{   0.58252901,   -0.79919797,      0.148127},
	{     0.866431,   -0.48981199,      0.096864},
	{   0.90458697,      0.111498,       0.41145},
	{   0.95353699,    0.23232999,      0.191806},
	{     0.497311,    0.77080297,      0.398177},
	{     0.194066,    0.95631999,      0.218611},
	{     0.422876,      0.882276,      0.206797},
	{    -0.373797,    0.84956598,    0.37217399},
	{  -0.53449702,    0.71402299,        0.4522},
	{    -0.881827,       0.23716,    0.40759799},
	{    -0.904948,     -0.014069,    0.42528901},
	{    -0.751827,   -0.51281703,    0.41445801},
	{  -0.50101501,   -0.69791698,    0.51175803},
	{     -0.23519,   -0.92592299,      0.295555},
	{     0.228983,   -0.95393997,      0.193819},
	{     0.734025,   -0.63489801,      0.241062},
	{   0.91375297,     -0.063253,   -0.40131599},
	{   0.90573502,     -0.161487,      0.391875},
	{   0.85892999,      0.342446,    0.38074899},
	{   0.62448603,    0.60758102,    0.49077699},
	{   0.28926399,    0.85747898,    0.42550799},
	{     0.069968,    0.90216899,    0.42567101},
	{  -0.28617999,    0.94069999,      0.182165},
	{  -0.57401299,    0.80511898,   -0.14930899},
	{     0.111258,   0.099717997,   -0.98877603},
	{  -0.30539301,   -0.94422799,      -0.12316},
	{  -0.60116601,   -0.78957599,      0.123163},
	{    -0.290645,   -0.81213999,    0.50591898},
	{ -0.064920001,   -0.87716299,    0.47578499},
	{     0.408301,     -0.862216,    0.29978901},
	{   0.56609702,   -0.72556603,    0.39126399},
	{   0.83936399,     -0.427387,    0.33586901},
	{   0.81889999,  -0.041305002,    0.57244802},
	{   0.71978402,    0.41499701,    0.55649698},
	{   0.88174403,       0.45027,      0.140659},
	{   0.40182301,      -0.89822,   -0.17815199},
	{ -0.054019999,    0.79134399,       0.60898},
	{  -0.29377401,    0.76399398,    0.57446498},
	{    -0.450798,    0.61034697,    0.65135098},
	{  -0.63822103,      0.186694,    0.74687302},
	{  -0.87287003,   -0.25712699,    0.41470799},
	{  -0.58725703,   -0.52170998,      0.618828},
	{  -0.35365799,   -0.64197397,      0.680291},
	{  0.041648999,   -0.61127299,    0.79032302},
	{     0.348342,   -0.77918297,    0.52108699},
	{     0.499167,   -0.62244099,      0.602826},
	{   0.79001898,   -0.30383101,    0.53250003},
	{   0.66011798,   0.060733002,    0.74870199},
	{   0.60492098,    0.29416099,    0.73996001},
	{   0.38569701,    0.37934601,    0.84103203},
	{     0.239693,      0.207876,    0.94833201},
	{     0.012623,    0.25853199,    0.96591997},
	{    -0.100557,      0.457147,    0.88368797},
	{     0.046967,    0.62858802,    0.77631903},
	{  -0.43039101,   -0.44540501,      0.785097},
	{  -0.43429101,     -0.196228,    0.87913901},
	{  -0.25663701,     -0.336867,    0.90590203},
	{    -0.131372,   -0.15891001,    0.97851402},
	{     0.102379,     -0.208767,      0.972592},
	{     0.195687,     -0.450129,    0.87125802},
	{   0.62731898,   -0.42314801,    0.65377098},
	{   0.68743902,     -0.171583,    0.70568198},
	{      0.27592,     -0.021255,    0.96094602},
	{   0.45936701,    0.15746599,    0.87417799},
	{     0.285395,      0.583184,    0.76055598},
	{  -0.81217402,    0.46030301,    0.35846099},
	{    -0.189068,    0.64122301,      0.743698},
	{    -0.338875,    0.47648001,      0.811252},
	{  -0.92099398,      0.347186,      0.176727},
	{  0.040638998,      0.024465,    0.99887401},
	{  -0.73913199,   -0.35374701,    0.57318997},
	{  -0.60351199,   -0.28661501,    0.74405998},
	{    -0.188676,     -0.547059,    0.81555402},
	{    -0.026045,      -0.39782,    0.91709399},
	{   0.26789701,     -0.649041,    0.71202302},
	{     0.518246,   -0.28489101,    0.80638599},
	{     0.493451,  -0.066532999,    0.86722499},
	{    -0.328188,      0.140251,    0.93414301},
	{    -0.328188,      0.140251,    0.93414301},
	{    -0.328188,      0.140251,    0.93414301},
	{    -0.328188,      0.140251,    0.93414301},
	{    -0.328188,      0.140251,    0.93414301}
};


int VoxelNormalTableEntryCount[] =
{
	0,
	ARRAY_SIZE(VoxelNormals1),
	ARRAY_SIZE(VoxelNormals2),
	ARRAY_SIZE(VoxelNormals3),
	ARRAY_SIZE(VoxelNormals4),
};


/// <summary>
/// Creates an empty voxel library.
/// Use Read_File to fill it in before asking it to draw anything.
/// </summary>
VoxelLibrary::VoxelLibrary(void) :
	LoadFailed(false),
	LayerCount(0),
	LayerInfoCount(0),
	DataSize(0),
	LayerHeaders(NULL),
	LayerInfos(NULL),
	Data(NULL)
{

}


/// <summary>
/// Creates a voxel library from a file.
/// A read that fails leaves the library empty and flagged as failed, so the caller must
/// check before drawing with it.
/// </summary>
/// <param name="file">The file to read the voxel object from.</param>
/// <param name="load_file_palette">Should the palette in the file become the voxel palette?</param>
VoxelLibrary::VoxelLibrary(FileClass & file, int load_file_palette) :
	LoadFailed(false),
	LayerCount(0),
	LayerInfoCount(0),
	DataSize(0),
	LayerHeaders(NULL),
	LayerInfos(NULL),
	Data(NULL)
{
	if (!Read_File(file, load_file_palette)) {
		LoadFailed = true;
	}
}


/// <summary>
/// Destroys the voxel library, releasing whatever it holds.
/// </summary>
VoxelLibrary::~VoxelLibrary(void)
{
	Clear();
}


/// <summary>
/// Frees the loaded voxel object.
/// This routine releases the layer tables and the voxel data, leaving the library empty
/// and safe to load into again.
/// </summary>
void VoxelLibrary::Clear(void)
{
	if (LayerHeaders != NULL) {
		delete [] LayerHeaders;
	}
	LayerHeaders = NULL;

	if (LayerInfos != NULL) {
		delete [] LayerInfos;
	}
	LayerInfos = NULL;

	if (Data != NULL) {
		delete [] Data;
	}
	Data = NULL;
}


/// <summary>
/// Loads a voxel object from a file.
/// This routine reads the layer headers, the voxel data itself, and the per-layer info
/// records, converting the file's bounds into the eight bounding box corners the render
/// code expects. Any library already loaded is discarded first, and a partial read leaves
/// the library empty rather than half filled.
/// </summary>
/// <param name="file">The file to read the voxel object from.</param>
/// <param name="load_file_palette">Should the palette in the file become the voxel palette?</param>
/// <returns>bool; Was the voxel object loaded?</returns>
int VoxelLibrary::Read_File(FileClass & file, int load_file_palette)
{
	unsigned int i = 0;

	Clear();

	if (!file.Open(FileClass::READ)) {
		return(false);
	}

	VoxelHeaderStruct hdr;
	file.Read(&hdr, sizeof(hdr));

	LayerCount = hdr.LayerCount;
	LayerInfoCount = hdr.LayerInfoCount;
	DataSize = hdr.DataSize;

	LayerHeaders = new LayerStruct[LayerCount];
	LayerInfos = new LayerInfoStruct[LayerInfoCount];
	Data = new unsigned char[DataSize];

	if (!LayerHeaders || !LayerInfos || !Data) {
		Clear();
		file.Close();
		return(false);
	}

	if (load_file_palette) {

		VoxelPaletteLibrary vpl(VoxelRGBColors, VoxelPaletteTranslateTable);

		unsigned char remap_start;
		unsigned char remap_end;

		if (file.Read(&remap_start, sizeof(remap_start)) != sizeof(remap_start)) {
			file.Close();
			Clear();
			return(false);
		}

		if (file.Read(&remap_end, sizeof(remap_end)) != sizeof(remap_end)) {
			file.Close();
			Clear();
			return(false);
		}

		vpl.Header.RemapStart = remap_start;
		vpl.Header.RemapEnd = remap_end;

		/*
		 * Read the raw palette data.
		 */
		for (i = 0; i < VOXEL_PALETTE_SIZE; i++) {
			file.Read(&VoxelRGBColors[i].Red, sizeof(VoxelRGBColors[i].Red));
			file.Read(&VoxelRGBColors[i].Green, sizeof(VoxelRGBColors[i].Green));
			file.Read(&VoxelRGBColors[i].Blue, sizeof(VoxelRGBColors[i].Blue));
		}

		if (hdr.PaletteCount > 1) {

			/*
			 * Skip past any additional palettes as we only support one per voxel object.
			 */
			int palette_size = (VOXEL_PALETTE_SIZE*sizeof(RGBStruct));
			int skip_size = (palette_size * hdr.PaletteCount) - palette_size;
			file.Seek(skip_size);
		}

		vpl.Calculate_Lookup_Table();

	} else {

		/*
		 * Skip past the palette data.
		 */

		int remap_start_size = sizeof(unsigned char);
		int remap_end_size = sizeof(unsigned char);
		int palette_size = (VOXEL_PALETTE_SIZE*sizeof(RGBStruct));
		int skip_size = (remap_start_size + remap_end_size + palette_size) * hdr.PaletteCount;
		file.Seek(skip_size);
	}

	for (i = 0; i < LayerCount; i++) {

		VoxelLayerHeaderStruct lyrhdr;
		if (file.Read(&lyrhdr, sizeof(lyrhdr)) != sizeof(lyrhdr)) {
			Clear();
			file.Close();
			return(false);
		}

		LayerHeaders[i].InfoIndex = lyrhdr.InfoIndex;
		LayerHeaders[i].Unused1 = lyrhdr.Unused1;
		LayerHeaders[i].Unused2 = lyrhdr.Unused2;
	}

	if (file.Read(Data, DataSize) != DataSize) {
		Clear();
		file.Close();
		return(false);
	}

	for (i = 0; i < LayerInfoCount; i++) {

		VoxelLayerInfoStruct layerinfo;
		if (file.Read(&layerinfo, sizeof(layerinfo)) != sizeof(layerinfo)) {
			Clear();
			file.Close();
			return(false);
		}

		LayerInfos[i].XSize = layerinfo.XSize;
		LayerInfos[i].YSize = layerinfo.YSize;
		LayerInfos[i].ZSize = layerinfo.ZSize;
		LayerInfos[i].NormalType = layerinfo.NormalType;
		LayerInfos[i].Scale = layerinfo.Scale;
		LayerInfos[i].Transform = Matrix3D(layerinfo.Transform);

		Vector3 min = Vector3(layerinfo.MinBounds[0], layerinfo.MinBounds[1], layerinfo.MinBounds[2]);
		Vector3 max = Vector3(layerinfo.MaxBounds[0], layerinfo.MaxBounds[1], layerinfo.MaxBounds[2]);

		LayerInfos[i].BoxCorner[0] = Vector3(max[0], max[1], min[2]);
		LayerInfos[i].BoxCorner[1] = Vector3(max[0], min[1], min[2]);
		LayerInfos[i].BoxCorner[2] = Vector3(min[0], min[1], min[2]);
		LayerInfos[i].BoxCorner[3] = Vector3(min[0], max[1], min[2]);
		LayerInfos[i].BoxCorner[4] = Vector3(max[0], max[1], max[2]);
		LayerInfos[i].BoxCorner[5] = Vector3(max[0], min[1], max[2]);
		LayerInfos[i].BoxCorner[6] = Vector3(min[0], min[1], max[2]);
		LayerInfos[i].BoxCorner[7] = Vector3(min[0], max[1], max[2]);

		LayerInfos[i].StartOffset = (unsigned char *)(Data + layerinfo.StartOffset);
		LayerInfos[i].EndOffset = (unsigned char *)(Data + layerinfo.EndOffset);
		LayerInfos[i].DataOffset = (unsigned char *)(Data + layerinfo.DataOffset);
	}

	file.Close();

	return(true);
}


/// <summary>
/// Fetches the header of one voxel layer.
/// </summary>
/// <param name="layer">The layer of interest.</param>
/// <returns>Returns with a reference to the layer header.</returns>
VoxelLibrary::LayerStruct const & VoxelLibrary::Get_Layer(int layer)
{
	return(LayerHeaders[layer]);
}


/// <summary>
/// Fetches one of a layer's info records.
/// The info record carries the dimensions, scale, transform and bounding box the render
/// code needs in order to draw the layer.
/// </summary>
/// <param name="layer">The layer of interest.</param>
/// <param name="info">Which of that layer's info records is wanted.</param>
/// <returns>Returns with a reference to the info record.</returns>
VoxelLibrary::LayerInfoStruct const & VoxelLibrary::Get_Layer_Info(int layer, int info)
{
	return(LayerInfos[LayerHeaders[layer].InfoIndex + info]);
}


/// <summary>
/// Fetches the center of a layer's bounding box.
/// This routine is used when something needs to be positioned or aimed at the middle of a
/// voxel layer rather than at its anchor point.
/// </summary>
/// <param name="layer">The layer of interest.</param>
/// <param name="info">Which of the layer's info records to measure.</param>
/// <returns>Returns with the midpoint of the bounding box, in voxel space.</returns>
Vector3 VoxelLibrary::Get_Bounding_Box_Center(int layer, int info)
{
	LayerInfoStruct const & layerinfo = Get_Layer_Info(layer, info);
	Vector3 v(layerinfo.BoxCorner[VOXEL_BOUNDS_TFR].X + layerinfo.BoxCorner[VOXEL_BOUNDS_BBL].X,
			layerinfo.BoxCorner[VOXEL_BOUNDS_TFR].Y + layerinfo.BoxCorner[VOXEL_BOUNDS_BBL].Y,
			layerinfo.BoxCorner[VOXEL_BOUNDS_TFR].Z + layerinfo.BoxCorner[VOXEL_BOUNDS_BBL].Z);
	return(Vector3(v.X * 0.5f, v.Y * 0.5f, v.Z * 0.5f));
}


/// <summary>
/// Determines how much memory this voxel library occupies.
/// </summary>
/// <returns>Returns with the byte count of the loaded layer tables and voxel data.</returns>
int VoxelLibrary::Memory_Used(void)
{
	return(sizeof(LayerStruct) * LayerCount) + (sizeof(LayerInfoStruct) * LayerInfoCount) + (DataSize + sizeof(VoxelLayerHeaderStruct));
}


/// <summary>
/// Draws one layer of the voxel object.
/// This routine is used by the voxel draw system. It turns the requested layer's bounding
/// box corners into the fixed point projection the low level drawers work in, then picks
/// the drawer that suits the viewing orientation and the current lighting and depth buffer
/// settings.
/// </summary>
/// <param name="voxel">The render request, naming the layer and its transformed corners.</param>
/// <param name="center">The point the projection is centered about.</param>
void VoxelLibrary::Render_Object(VoxelRenderStruct & voxel, Vector3 & center)
{
	LayerInfoStruct const & layerinfo = Get_Layer_Info(voxel.Layer, voxel.Info);
	int orientation = voxel.AnchorCornerIndex;

	VoxelFuncArgumentStruct arg;

	unsigned char x_size = layerinfo.XSize;
	unsigned char y_size = layerinfo.YSize;
	unsigned char z_size = layerinfo.ZSize;

	arg.XSize = x_size;
	arg.YSize = y_size;
	arg.ZSize = z_size;

	arg.StartOffset = layerinfo.StartOffset;
	arg.EndOffset = layerinfo.EndOffset;
	arg.DataOffset = layerinfo.DataOffset;

	Vector3 & corner_0 = voxel.BoxCorner[VoxelRenderOrientations[orientation].Corner0];
	Vector3 & corner_x = voxel.BoxCorner[VoxelRenderOrientations[orientation].CornerX];
	Vector3 & corner_y = voxel.BoxCorner[VoxelRenderOrientations[orientation].CornerY];
	Vector3 & corner_z = voxel.BoxCorner[VoxelRenderOrientations[orientation].CornerZ];

	arg.StrideX = VoxelRenderOrientations[orientation].XIndexStride;
	arg.StrideY = x_size * VoxelRenderOrientations[orientation].YIndexStride;
	arg.StartIndex = (x_size - 1) * VoxelRenderOrientations[orientation].ZIndexFactor + x_size * (y_size - 1) * VoxelRenderOrientations[orientation].YIndexFactor;

	/// The drawer sums these deltas down the length of the model, so an error of
	/// one unit here becomes one unit per voxel by the far end.
	arg.TransformMatrix[0].I = static_cast<unsigned short>(static_cast<int>(((double)corner_0.X + 128 - (double)center.X) * 256.0));
	arg.TransformMatrix[0].J = static_cast<unsigned short>(static_cast<int>(((double)corner_0.Y + 128 - (double)center.Y) * 256.0));
	arg.TransformMatrix[0].K = static_cast<unsigned short>(static_cast<int>(((double)corner_0.Z + 128 - (double)center.Z) * 256.0));

	arg.TransformMatrix[1].I = static_cast<unsigned short>(static_cast<int>((corner_x.X - corner_0.X) / (double)x_size * 256.0));
	arg.TransformMatrix[2].I = static_cast<unsigned short>(static_cast<int>((corner_y.X - corner_0.X) / (double)y_size * 256.0));
	arg.TransformMatrix[3].I = static_cast<unsigned short>(static_cast<int>((corner_z.X - corner_0.X) / (double)z_size * 256.0));

	arg.TransformMatrix[1].J = static_cast<unsigned short>(static_cast<int>((corner_x.Y - corner_0.Y) / (double)x_size * 256.0));
	arg.TransformMatrix[2].J = static_cast<unsigned short>(static_cast<int>((corner_y.Y - corner_0.Y) / (double)y_size * 256.0));
	arg.TransformMatrix[3].J = static_cast<unsigned short>(static_cast<int>((corner_z.Y - corner_0.Y) / (double)z_size * 256.0));

	if (VoxelDrawSystem::EnableZBuffer) {
		arg.TransformMatrix[1].K = static_cast<unsigned short>(static_cast<int>((corner_x.Z - corner_0.Z) / (double)x_size * 256.0));
		arg.TransformMatrix[2].K = static_cast<unsigned short>(static_cast<int>((corner_y.Z - corner_0.Z) / (double)y_size * 256.0));
		arg.TransformMatrix[3].K = static_cast<unsigned short>(static_cast<int>((corner_z.Z - corner_0.Z) / (double)z_size * 256.0));
	}

	int funcnum = VoxelRenderOrientations[orientation].Reversed;

	if (VoxelDrawSystem::EnableZBuffer) {
		funcnum |= 2;
	}

	if (VoxelDrawSystem::EnableLighting) {
		funcnum |= 4;
	}

	if (layerinfo.NormalType == 0) {
		funcnum |= 8;
	}

	VoxelDrawFunctions[funcnum](&arg);
}


/// <summary>
/// Stamps a voxel object's shadow into the draw buffer.
/// This is the low level drawer behind Render_Shadow. Every column that holds any voxel
/// at all marks its projected position, so what lands in the buffer is the flat silhouette
/// of the object rather than any of its detail.
/// </summary>
/// <param name="state">The projection, stride and voxel data setup for this object.</param>
static void __cdecl _voxel_draw_shadow(VoxelFuncArgumentStruct * state)
{
	/// Set starting 2D projection position
	unsigned short pixel_x = state->TransformMatrix[0].I;
	unsigned short pixel_y = state->TransformMatrix[0].J;

	/// Iterate over voxel Y slices (rows)
	for (unsigned int y = 0; y < state->YSize; y++) {
		unsigned int base_index = state->StartIndex;
		unsigned short row_start_x = pixel_x;
		unsigned short row_start_y = pixel_y;

		/// Iterate over voxel X columns (within the current Y row)
		for (unsigned int x = 0; x < state->XSize; x++) {
			unsigned int data_offset = ((unsigned int *)state->EndOffset)[state->StartIndex];
			unsigned short column_start_x = pixel_x;
			unsigned short column_start_y = pixel_y;

			if (data_offset != UINT_MAX) {
				unsigned char color_index = 1;
				unsigned int buffer_index = (pixel_x >> 8) | (pixel_y & 0xFF00);
				VoxelDrawBuffer[buffer_index] = color_index;
				VoxelDrawBuffer[buffer_index + 1] = color_index;
			}

			/// Advance to next voxel in X direction
			pixel_x = column_start_x + state->TransformMatrix[1].I;
			pixel_y = column_start_y + state->TransformMatrix[1].J;
			state->StartIndex = state->StrideX + state->StartIndex;
		}

		/// Advance to next voxel row (Y direction)
		pixel_x = row_start_x + state->TransformMatrix[2].I;
		pixel_y = row_start_y + state->TransformMatrix[2].J;
		state->StartIndex = state->StrideY + base_index;
	}
}


/// <summary>
/// Draws the ground shadow of a voxel object.
/// This routine is used by the voxel draw system to flatten a layer onto the shadow plane.
/// Only the outline matters, so the object is projected as a single flat sheet rather than
/// walked in depth, and every occupied cell is stamped with the shadow color.
/// </summary>
/// <param name="voxel">The shadow render request, naming the layer and its corners.</param>
/// <param name="center">The point the projection is centered about.</param>
void VoxelLibrary::Render_Shadow(VoxelShadowRenderStruct & voxel, Vector3 & center)
{
	LayerInfoStruct const & layerinfo = Get_Layer_Info(voxel.Layer, voxel.Info);

	VoxelFuncArgumentStruct arg;

	unsigned char x_size = layerinfo.XSize;
	unsigned char y_size = layerinfo.YSize;
	unsigned char z_size = 1; /// This function does not iterate along Z

	arg.XSize = x_size;
	arg.YSize = y_size;
	arg.ZSize = z_size;

	arg.StartOffset = NULL;
	arg.EndOffset = layerinfo.EndOffset; /// Uses EndOffset as presence mask
	arg.DataOffset = NULL;

	Vector3 & corner_x = voxel.ShadowCorner[1]; /// X step
	Vector3 & corner_0 = voxel.ShadowCorner[2]; /// Origin
	Vector3 & corner_y = voxel.ShadowCorner[3]; /// Y step

	arg.StrideX = 1;
	arg.StrideY = x_size;
	arg.StartIndex = 0;

	arg.TransformMatrix[0].I = static_cast<unsigned short>(static_cast<int>((corner_0.X + 128 - center.X) * 256.0));
	arg.TransformMatrix[0].J = static_cast<unsigned short>(static_cast<int>((corner_0.Y + 128 - center.Y) * 256.0));

	arg.TransformMatrix[1].I = static_cast<unsigned short>(static_cast<int>((corner_x.X - corner_0.X) / x_size * 256.0));
	arg.TransformMatrix[2].I = static_cast<unsigned short>(static_cast<int>((corner_y.X - corner_0.X) / y_size * 256.0));

	arg.TransformMatrix[1].J = static_cast<unsigned short>(static_cast<int>((corner_x.Y - corner_0.Y) / x_size * 256.0));
	arg.TransformMatrix[2].J = static_cast<unsigned short>(static_cast<int>((corner_y.Y - corner_0.Y) / y_size * 256.0));

	_voxel_draw_shadow(&arg);
}


/// <summary>
/// Computes the bounding box corners of every layer.
/// This routine derives each layer's eight box corners from its voxel dimensions, giving
/// the render code the box it transforms and projects when the corners were not supplied
/// by the file itself.
/// </summary>
void VoxelLibrary::Compute_Bounding_Box(void)
{
	/// Compute the coordinates of the eight corners of the bounding box.
	for (unsigned i = 0; i < LayerInfoCount; i++) {
		LayerInfoStruct &layerinfo = LayerInfos[i];

		// +x +y -z
		layerinfo.BoxCorner[VOXEL_BOUNDS_BFR] = Vector3(+layerinfo.XSize / 2.0f, +layerinfo.YSize / 2.0f, -layerinfo.ZSize / 2.0f);
		// +x -y -z
		layerinfo.BoxCorner[VOXEL_BOUNDS_BBR] = Vector3(+layerinfo.XSize / 2.0f, -layerinfo.YSize / 2.0f, -layerinfo.ZSize / 2.0f);
		// -x -y -z
		layerinfo.BoxCorner[VOXEL_BOUNDS_BBL] = Vector3(-layerinfo.XSize / 2.0f, -layerinfo.YSize / 2.0f, -layerinfo.ZSize / 2.0f);
		// -x +y -z
		layerinfo.BoxCorner[VOXEL_BOUNDS_BFL] = Vector3(-layerinfo.XSize / 2.0f, +layerinfo.YSize / 2.0f, -layerinfo.ZSize / 2.0f);
		// +x +y +z
		layerinfo.BoxCorner[VOXEL_BOUNDS_TFR] = Vector3(+layerinfo.XSize / 2.0f, +layerinfo.YSize / 2.0f, +layerinfo.ZSize / 2.0f);
		// +x -y +z
		layerinfo.BoxCorner[VOXEL_BOUNDS_TBR] = Vector3(+layerinfo.XSize / 2.0f, -layerinfo.YSize / 2.0f, +layerinfo.ZSize / 2.0f);
		// -x -y +z
		layerinfo.BoxCorner[VOXEL_BOUNDS_TBL] = Vector3(-layerinfo.XSize / 2.0f, -layerinfo.YSize / 2.0f, +layerinfo.ZSize / 2.0f);
		// -x +y +z
		layerinfo.BoxCorner[VOXEL_BOUNDS_TFL] = Vector3(-layerinfo.XSize / 2.0f, +layerinfo.YSize / 2.0f, +layerinfo.ZSize / 2.0f);
	}
}


/// The three variants that follow build the same table and differ only in whether each
/// store goes through the local reference or names VoxelPixelDeltaTable outright. Which
/// variant a drawer calls is deliberate; do not merge them.

/// <summary>
/// Fills in the voxel projection delta table.
/// This routine is used by the low level voxel drawers before they walk an object. It
/// tabulates the screen offset of every Z step in the column, so that skipping over a run
/// of empty voxels costs a single lookup.
/// </summary>
/// <param name="state">The projection setup for the object about to be drawn.</param>
inline void Fill_Delta_Table1(VoxelFuncArgumentStruct * state)
{
	short (&ztable)[VOXEL_BITMAP_WIDTH][2] = VoxelPixelDeltaTable;

	ztable[0][0] = 0;
	ztable[0][1] = 0;

	Vector3i16 & step_z = state->TransformMatrix[3];

	for (unsigned int z = 1; z < state->ZSize; z++) {
		ztable[z][0] = ztable[z - 1][0] + step_z.I;
		ztable[z][1] = ztable[z - 1][1] + step_z.J;
	}
}


/// <summary>
/// Fills in the voxel projection delta table.
/// This routine is used by the low level voxel drawers before they walk an object. It
/// tabulates the screen offset of every Z step in the column, so that skipping over a run
/// of empty voxels costs a single lookup.
/// </summary>
/// <param name="state">The projection setup for the object about to be drawn.</param>
inline void Fill_Delta_Table3(VoxelFuncArgumentStruct * state)
{
	short (&ztable)[VOXEL_BITMAP_WIDTH][2] = VoxelPixelDeltaTable;

	ztable[0][0] = 0;
	ztable[0][1] = 0;

	Vector3i16 & step_z = state->TransformMatrix[3];

	for (unsigned int z = 1; z < state->ZSize; z++) {
		ztable[z][0] = ztable[z - 1][0] + step_z.I;
		VoxelPixelDeltaTable[z][1] = ztable[z - 1][1] + step_z.J;
	}
}


/// <summary>
/// Fills in the voxel projection delta table.
/// This routine is used by the low level voxel drawers before they walk an object. It
/// tabulates the screen offset of every Z step in the column, so that skipping over a run
/// of empty voxels costs a single lookup.
/// </summary>
/// <param name="state">The projection setup for the object about to be drawn.</param>
inline void Fill_Delta_Table2(VoxelFuncArgumentStruct * state)
{
	short (&ztable)[VOXEL_BITMAP_WIDTH][2] = VoxelPixelDeltaTable;

	ztable[0][0] = 0;
	ztable[0][1] = 0;

	Vector3i16 & step_z = state->TransformMatrix[3];

	for (unsigned int z = 1; z < state->ZSize; z++) {
		VoxelPixelDeltaTable[z][0] = ztable[z - 1][0] + step_z.I;
		VoxelPixelDeltaTable[z][1] = ztable[z - 1][1] + step_z.J;
	}
}


/// <summary>
/// Draws a voxel object without shading it.
/// This is the low level drawer for the forward orientations, used when the draw system
/// has both lighting and the depth buffer switched off. The normal byte carried by each
/// voxel is stepped over rather than consulted, and the raw color indices go straight into
/// the voxel draw buffer. It is reached through the VoxelDrawFunctions dispatch table.
/// </summary>
/// <param name="state">The projection, stride and voxel data setup for this object.</param>
void __cdecl Draw_Voxel_Regular_Normals(VoxelFuncArgumentStruct * state)
{
	/*
	 * Precompute screen projection deltas for every Z step
	 */
	Fill_Delta_Table2(state);

	/// Set starting 2D projection position
	unsigned short pixel_x = state->TransformMatrix[0].I;
	unsigned short pixel_y = state->TransformMatrix[0].J;

	/// Iterate over voxel Y slices (rows)
	for (unsigned int y = 0; y < state->YSize; y++) {
		unsigned int base_index = state->StartIndex;
		unsigned short row_start_x = pixel_x;
		unsigned short row_start_y = pixel_y;

		/// Iterate over voxel X columns (within the current Y row)
		for (unsigned int x = 0; x < state->XSize; x++) {
			unsigned int data_offset = ((unsigned int *)state->StartOffset)[state->StartIndex];
			unsigned short column_start_x = pixel_x;
			unsigned short column_start_y = pixel_y;

			if (data_offset != UINT_MAX) {
				unsigned char * ptr = state->DataOffset + data_offset;
				unsigned int remaining = state->ZSize;

				/// Parse voxel run-length encoded data along Z axis
				while (remaining) {

					/*
					 * Byte 0 - pixel delta
					 */
					unsigned char delta = *ptr;
					ptr++;

					pixel_x += VoxelPixelDeltaTable[delta][0];
					pixel_y += VoxelPixelDeltaTable[delta][1];

					remaining -= delta;

					// Byte 1 - run length(forward)
					unsigned int run_length = *ptr;
					ptr++;

					if (run_length) {
						remaining -= run_length;
						while (run_length) {

							/*
							 * Byte 2 - color index
							 */
							unsigned char color_index = *ptr;
							ptr++;

							/*
							 * Byte 3 - normal index
							 */
							ptr++;

							/// Compute buffer index and write color
							unsigned int buffer_index = (pixel_x >> 8) | (pixel_y & 0xFF00);
							VoxelDrawBuffer[buffer_index] = color_index;

							pixel_x += state->TransformMatrix[3].I;
							pixel_y += state->TransformMatrix[3].J;
							run_length--;
						}
					}

					// Byte 4 - run length(backward)
					ptr++;
				}
			}

			/// Advance to next voxel in X direction
			pixel_x = column_start_x + state->TransformMatrix[1].I;
			pixel_y = column_start_y + state->TransformMatrix[1].J;
			state->StartIndex = state->StrideX + state->StartIndex;
		}

		/// Advance to next voxel row (Y direction)
		pixel_x = row_start_x + state->TransformMatrix[2].I;
		pixel_y = row_start_y + state->TransformMatrix[2].J;
		state->StartIndex = state->StrideY + base_index;
	}
}


/// <summary>
/// Draws a voxel object without shading it.
/// This is the low level drawer for the reversed orientations, used when the draw system
/// has both lighting and the depth buffer switched off. The normal byte carried by each
/// voxel is stepped over rather than consulted, and the raw color indices go straight into
/// the voxel draw buffer. It is reached through the VoxelDrawFunctions dispatch table.
/// </summary>
/// <param name="state">The projection, stride and voxel data setup for this object.</param>
void __cdecl Draw_Voxel_Reverse_Normals(VoxelFuncArgumentStruct * state)
{
	/*
	 * Precompute screen projection deltas for every Z step
	 */
	Fill_Delta_Table1(state);

	/// Set starting 2D projection position
	unsigned short pixel_x = state->TransformMatrix[0].I;
	unsigned short pixel_y = state->TransformMatrix[0].J;

	/// Iterate over voxel Y slices (rows)
	for (unsigned int y = 0; y < state->YSize; y++) {
		unsigned int base_index = state->StartIndex;
		unsigned short row_start_x = pixel_x;
		unsigned short row_start_y = pixel_y;

		/// Iterate over voxel X columns (within the current Y row)
		for (unsigned int x = 0; x < state->XSize; x++) {
			unsigned int data_offset = ((unsigned int *)state->EndOffset)[state->StartIndex];
			unsigned short column_start_x = pixel_x;
			unsigned short column_start_y = pixel_y;

			if (data_offset != UINT_MAX) {
				unsigned char * ptr = state->DataOffset + data_offset;
				unsigned int remaining = state->ZSize;

				/// Parse voxel run-length encoded data along Z axis
				while (remaining) {

					// Byte 4 - run length(backward)
					unsigned int run_length = *ptr;
					ptr--;

					if (run_length) {
						remaining -= run_length;
						while (run_length) {

							/*
							 * Byte 3 - normal index
							 */
							ptr--;

							/*
							 * Byte 2 - color index
							 */
							unsigned char color_index = *ptr;
							ptr--;

							/// Compute buffer index and write color
							unsigned int buffer_index = (pixel_x >> 8) | (pixel_y & 0xFF00);
							VoxelDrawBuffer[buffer_index] = color_index;

							pixel_x += state->TransformMatrix[3].I;
							pixel_y += state->TransformMatrix[3].J;
							run_length--;
						}
					}

					// Byte 1 - run length(forward)
					ptr--;

					/*
					 * Byte 0 - pixel delta
					 */
					unsigned char delta = *ptr;
					ptr--;

					pixel_x += VoxelPixelDeltaTable[delta][0];
					pixel_y += VoxelPixelDeltaTable[delta][1];

					remaining -= delta;
				}
			}

			/// Advance to next voxel in X direction
			pixel_x = column_start_x + state->TransformMatrix[1].I;
			pixel_y = column_start_y + state->TransformMatrix[1].J;
			state->StartIndex = state->StrideX + state->StartIndex;
		}

		/// Advance to next voxel row (Y direction)
		pixel_x = row_start_x + state->TransformMatrix[2].I;
		pixel_y = row_start_y + state->TransformMatrix[2].J;
		state->StartIndex = state->StrideY + base_index;
	}
}


/// <summary>
/// Draws a depth buffered voxel object without shading it.
/// This is the low level drawer for the forward orientations, used when the draw system
/// has the depth buffer switched on but lighting switched off. The normal byte carried by
/// each voxel is stepped over rather than consulted. It is reached through the
/// VoxelDrawFunctions dispatch table.
/// </summary>
/// <param name="state">The projection, stride and voxel data setup for this object.</param>
void __cdecl Draw_Voxel_Regular_Normals_ZBuffer(VoxelFuncArgumentStruct * state)
{
	/*
	 * Precompute screen projection deltas for every Z step
	 */
	Fill_Delta_Table3(state);

	/// Set starting 2D projection position
	unsigned short pixel_x = state->TransformMatrix[0].I;
	unsigned short pixel_y = state->TransformMatrix[0].J;
	unsigned short pixel_z = state->TransformMatrix[0].K;

	/// Iterate over voxel Y slices (rows)
	for (unsigned int y = 0; y < state->YSize; y++) {
		unsigned int base_index = state->StartIndex;
		unsigned short row_start_x = pixel_x;
		unsigned short row_start_y = pixel_y;
		unsigned short row_start_z = pixel_z;

		/// Iterate over voxel X columns (within the current Y row)
		for (unsigned int x = 0; x < state->XSize; x++) {
			unsigned short column_start_x = pixel_x;
			unsigned short column_start_y = pixel_y;
			unsigned short column_start_z = pixel_z;
			unsigned int data_offset = ((unsigned int *)state->StartOffset)[state->StartIndex];

			if (data_offset != UINT_MAX) {
				unsigned char * ptr = state->DataOffset + data_offset;
				unsigned int remaining = state->ZSize;

				/// Parse voxel run-length encoded data along Z axis
				while (remaining) {

					/*
					 * Byte 0 - pixel delta. This one byte variable serves as
					 * both the pixel delta and, further down, the color index.
					 */
					unsigned char value = *ptr;
					ptr++;

					pixel_x += VoxelPixelDeltaTable[value][0];
					pixel_y += VoxelPixelDeltaTable[value][1];
					pixel_z += state->TransformMatrix[3].K * value;

					remaining -= value;

					// Byte 1 - run length(forward)
					unsigned int run_length = *ptr;
					ptr++;

					if (run_length) {
						remaining -= run_length;
						while (run_length) {

							/*
							 * Byte 2 - color index
							 */
							value = *ptr;
							ptr++;

							/*
							 * Byte 3 - normal index
							 */
							ptr++;

							/// Compute buffer index and write color
							unsigned int buffer_index = (pixel_x >> 8) | (pixel_y & 0xFF00);
							if ((pixel_z >> 8) > VoxelDrawZBuffer[buffer_index]) {
								VoxelDrawZBuffer[buffer_index] = (pixel_z >> 8);
								VoxelDrawBuffer[buffer_index] = value;
								VoxelDrawBuffer[buffer_index + 1] = value;
								VoxelDrawZBuffer[buffer_index + 1] = (pixel_z >> 8);
							}

							pixel_x += state->TransformMatrix[3].I;
							pixel_y += state->TransformMatrix[3].J;
							pixel_z += state->TransformMatrix[3].K;
							run_length--;
						}
					}

					// Byte 4 - run length(backward)
					ptr++;
				}
			}

			/// Advance to next voxel in X direction
			pixel_x = column_start_x + state->TransformMatrix[1].I;
			pixel_y = column_start_y + state->TransformMatrix[1].J;
			pixel_z = column_start_z + state->TransformMatrix[1].K;
			state->StartIndex = state->StrideX + state->StartIndex;
		}

		/// Advance to next voxel row (Y direction)
		pixel_x = row_start_x + state->TransformMatrix[2].I;
		pixel_y = row_start_y + state->TransformMatrix[2].J;
		pixel_z = row_start_z + state->TransformMatrix[2].K;
		state->StartIndex = state->StrideY + base_index;
	}
}


/// <summary>
/// Draws a depth buffered voxel object without shading it.
/// This is the low level drawer for the reversed orientations, used when the draw system
/// has the depth buffer switched on but lighting switched off. The normal byte carried by
/// each voxel is stepped over rather than consulted. It is reached through the
/// VoxelDrawFunctions dispatch table.
/// </summary>
/// <param name="state">The projection, stride and voxel data setup for this object.</param>
void __cdecl Draw_Voxel_Reverse_Normals_ZBuffer(VoxelFuncArgumentStruct * state)
{
	/*
	 * Precompute screen projection deltas for every Z step
	 */
	Fill_Delta_Table2(state);

	/// Set starting 2D projection position
	unsigned short pixel_x = state->TransformMatrix[0].I;
	unsigned short pixel_y = state->TransformMatrix[0].J;
	unsigned short pixel_z = state->TransformMatrix[0].K;

	/// Iterate over voxel Y slices (rows)
	for (unsigned int y = 0; y < state->YSize; y++) {
		unsigned int base_index = state->StartIndex;
		unsigned short row_start_x = pixel_x;
		unsigned short row_start_y = pixel_y;
		unsigned short row_start_z = pixel_z;

		/// Iterate over voxel X columns (within the current Y row)
		for (unsigned int x = 0; x < state->XSize; x++) {
			unsigned short column_start_x = pixel_x;
			unsigned short column_start_y = pixel_y;
			unsigned short column_start_z = pixel_z;
			unsigned int data_offset = ((unsigned int *)state->EndOffset)[state->StartIndex];

			if (data_offset != UINT_MAX) {
				unsigned char * ptr = state->DataOffset + data_offset;
				unsigned int remaining = state->ZSize;

				/// Parse voxel run-length encoded data along Z axis
				while (remaining) {

					// Byte 4 - run length(backward)
					unsigned int run_length = *ptr;
					ptr--;

					/*
					 * One byte variable serves as both the color index and,
					 * further down, the pixel delta.
					 */
					unsigned char value;

					if (run_length) {
						remaining -= run_length;
						while (run_length) {

							/*
							 * Byte 3 - normal index
							 */
							ptr--;

							/*
							 * Byte 2 - color index
							 */
							value = *ptr;
							ptr--;

							/// Compute buffer index and write color
							unsigned int buffer_index = (pixel_x >> 8) | (pixel_y & 0xFF00);
							if ((pixel_z >> 8) > VoxelDrawZBuffer[buffer_index]) {
								VoxelDrawZBuffer[buffer_index] = (pixel_z >> 8);
								VoxelDrawBuffer[buffer_index] = value;
								VoxelDrawBuffer[buffer_index + 1] = value;
								VoxelDrawZBuffer[buffer_index + 1] = (pixel_z >> 8);
							}

							pixel_x += state->TransformMatrix[3].I;
							pixel_y += state->TransformMatrix[3].J;
							pixel_z += state->TransformMatrix[3].K;
							run_length--;
						}
					}

					// Byte 1 - run length(forward)
					ptr--;

					/*
					 * Byte 0 - pixel delta
					 */
					value = *ptr;
					ptr--;

					pixel_x += VoxelPixelDeltaTable[value][0];
					pixel_y += VoxelPixelDeltaTable[value][1];
					pixel_z += state->TransformMatrix[3].K * value;

					remaining -= value;
				}
			}

			/// Advance to next voxel in X direction
			pixel_x = column_start_x + state->TransformMatrix[1].I;
			pixel_y = column_start_y + state->TransformMatrix[1].J;
			pixel_z = column_start_z + state->TransformMatrix[1].K;
			state->StartIndex = state->StrideX + state->StartIndex;
		}

		/// Advance to next voxel row (Y direction)
		pixel_x = row_start_x + state->TransformMatrix[2].I;
		pixel_y = row_start_y + state->TransformMatrix[2].J;
		pixel_z = row_start_z + state->TransformMatrix[2].K;
		state->StartIndex = state->StrideY + base_index;
	}
}


/// <summary>
/// Draws a shaded voxel object.
/// This is the low level drawer for the forward orientations, used when the draw system
/// has lighting switched on but the depth buffer switched off. Each voxel's color is
/// remapped through the normal lighting lookup before it reaches the draw buffer. It is
/// reached through the VoxelDrawFunctions dispatch table.
/// </summary>
/// <param name="state">The projection, stride and voxel data setup for this object.</param>
void __cdecl Draw_Voxel_Regular_Normals_Lighting(VoxelFuncArgumentStruct * state)
{
	/*
	 * Precompute screen projection deltas for every Z step
	 */
	Fill_Delta_Table1(state);

	/// Set starting 2D projection position
	unsigned short pixel_x = state->TransformMatrix[0].I;
	unsigned short pixel_y = state->TransformMatrix[0].J;

	/// Iterate over voxel Y slices (rows)
	for (unsigned int y = 0; y < state->YSize; y++) {
		unsigned int base_index = state->StartIndex;
		unsigned short row_start_x = pixel_x;
		unsigned short row_start_y = pixel_y;

		/// Iterate over voxel X columns (within the current Y row)
		for (unsigned int x = 0; x < state->XSize; x++) {
			unsigned int data_offset = ((unsigned int *)state->StartOffset)[state->StartIndex];
			unsigned short column_start_x = pixel_x;
			unsigned short column_start_y = pixel_y;

			if (data_offset != UINT_MAX) {
				unsigned char * ptr = state->DataOffset + data_offset;
				unsigned int remaining = state->ZSize;

				/// Parse voxel run-length encoded data along Z axis
				while (remaining) {

					/*
					 * Byte 0 - pixel delta
					 */
					unsigned char delta = *ptr;
					ptr++;

					pixel_x += VoxelPixelDeltaTable[delta][0];
					pixel_y += VoxelPixelDeltaTable[delta][1];

					remaining -= delta;

					// Byte 1 - run length(forward)
					unsigned int run_length = *ptr;
					ptr++;

					if (run_length) {
						remaining -= run_length;
						while (run_length) {

							/*
							 * Byte 2 - color index
							 */
							unsigned char color_index = *ptr;
							ptr++;

							/*
							 * Byte 3 - normal index
							 */
							unsigned char normal_index = *ptr;
							unsigned char table_index = VoxelNormalTranslateTable[normal_index];
							ptr++;

							/// Compute buffer index and write color
							unsigned int buffer_index = (pixel_x >> 8) | (pixel_y & 0xFF00);
							color_index = VoxelPaletteTranslateTable[table_index][color_index];

							VoxelDrawBuffer[buffer_index] = color_index;
							VoxelDrawBuffer[buffer_index + 1] = color_index;

							pixel_x += state->TransformMatrix[3].I;
							pixel_y += state->TransformMatrix[3].J;
							run_length--;
						}
					}

					// Byte 4 - run length(backward)
					ptr++;
				}
			}

			/// Advance to next voxel in X direction
			pixel_x = column_start_x + state->TransformMatrix[1].I;
			pixel_y = column_start_y + state->TransformMatrix[1].J;
			state->StartIndex = state->StrideX + state->StartIndex;
		}

		/// Advance to next voxel row (Y direction)
		pixel_x = row_start_x + state->TransformMatrix[2].I;
		pixel_y = row_start_y + state->TransformMatrix[2].J;
		state->StartIndex = state->StrideY + base_index;
	}
}


/// <summary>
/// Draws a shaded voxel object.
/// This is the low level drawer for the reversed orientations, used when the draw system
/// has lighting switched on but the depth buffer switched off. Each voxel's color is
/// remapped through the normal lighting lookup before it reaches the draw buffer. It is
/// reached through the VoxelDrawFunctions dispatch table.
/// </summary>
/// <param name="state">The projection, stride and voxel data setup for this object.</param>
void __cdecl Draw_Voxel_Reverse_Normals_Lighting(VoxelFuncArgumentStruct * state)
{
	/*
	 * Precompute screen projection deltas for every Z step
	 */
	Fill_Delta_Table2(state);

	/// Set starting 2D projection position
	unsigned short pixel_x = state->TransformMatrix[0].I;
	unsigned short pixel_y = state->TransformMatrix[0].J;

	/// Iterate over voxel Y slices (rows)
	for (unsigned int y = 0; y < state->YSize; y++) {
		unsigned int base_index = state->StartIndex;
		unsigned short row_start_x = pixel_x;
		unsigned short row_start_y = pixel_y;

		/// Iterate over voxel X columns (within the current Y row)
		for (unsigned int x = 0; x < state->XSize; x++) {
			unsigned int data_offset = ((unsigned int *)state->EndOffset)[state->StartIndex];
			unsigned short column_start_x = pixel_x;
			unsigned short column_start_y = pixel_y;

			if (data_offset != UINT_MAX) {
				unsigned char * ptr = state->DataOffset + data_offset;
				unsigned int remaining = state->ZSize;

				/// Parse voxel run-length encoded data along Z axis
				while (remaining) {

					// Byte 4 - run length(backward)
					unsigned int run_length = *ptr;
					ptr--;

					if (run_length) {
						remaining -= run_length;
						while (run_length) {

							/*
							 * Byte 3 - normal index
							 */
							unsigned char normal_index = *ptr;
							unsigned char table_index = VoxelNormalTranslateTable[normal_index];
							ptr--;

							/*
							 * Byte 2 - color index
							 */
							unsigned char color_index = *ptr;
							ptr--;

							/// Compute buffer index and write color
							unsigned int buffer_index = (pixel_x >> 8) | (pixel_y & 0xFF00);
							color_index = VoxelPaletteTranslateTable[table_index][color_index];

							VoxelDrawBuffer[buffer_index] = color_index;
							VoxelDrawBuffer[buffer_index + 1] = color_index;

							pixel_x += state->TransformMatrix[3].I;
							pixel_y += state->TransformMatrix[3].J;
							run_length--;
						}
					}

					// Byte 1 - run length(forward)
					ptr--;

					/*
					 * Byte 0 - pixel delta
					 */
					unsigned char delta = *ptr;
					ptr--;

					pixel_x += VoxelPixelDeltaTable[delta][0];
					pixel_y += VoxelPixelDeltaTable[delta][1];

					remaining -= delta;
				}
			}

			/// Advance to next voxel in X direction
			pixel_x = column_start_x + state->TransformMatrix[1].I;
			pixel_y = column_start_y + state->TransformMatrix[1].J;
			state->StartIndex = state->StrideX + state->StartIndex;
		}

		/// Advance to next voxel row (Y direction)
		pixel_x = row_start_x + state->TransformMatrix[2].I;
		pixel_y = row_start_y + state->TransformMatrix[2].J;
		state->StartIndex = state->StrideY + base_index;
	}
}


/// <summary>
/// Draws a shaded and depth buffered voxel object.
/// This is the low level drawer for the forward orientations, used when the draw system
/// has both lighting and the depth buffer switched on. Each voxel is tested against the
/// voxel depth buffer, and the ones that survive are remapped through the normal lighting
/// lookup. It is reached through the VoxelDrawFunctions dispatch table.
/// </summary>
/// <param name="state">The projection, stride and voxel data setup for this object.</param>
void __cdecl Draw_Voxel_Regular_Normals_ZBuffer_Lighting(VoxelFuncArgumentStruct * state)
{
	/*
	 * Precompute screen projection deltas for every Z step
	 */
	Fill_Delta_Table2(state);

	/// Set starting 2D projection position
	unsigned short pixel_x = state->TransformMatrix[0].I;
	unsigned short pixel_y = state->TransformMatrix[0].J;
	unsigned short pixel_z = state->TransformMatrix[0].K;

	/// Iterate over voxel Y slices (rows)
	for (unsigned int y = 0; y < state->YSize; y++) {
		unsigned int base_index = state->StartIndex;
		unsigned short row_start_x = pixel_x;
		unsigned short row_start_y = pixel_y;
		unsigned short row_start_z = pixel_z;

		/// Iterate over voxel X columns (within the current Y row)
		for (unsigned int x = 0; x < state->XSize; x++) {
			unsigned short column_start_x = pixel_x;
			unsigned short column_start_y = pixel_y;
			unsigned short column_start_z = pixel_z;
			unsigned int data_offset = ((unsigned int *)state->StartOffset)[state->StartIndex];

			if (data_offset != UINT_MAX) {
				unsigned char * ptr = state->DataOffset + data_offset;
				unsigned int remaining = state->ZSize;

				/// Parse voxel run-length encoded data along Z axis
				while (remaining) {

					/*
					 * Byte 0 - pixel delta
					 */
					unsigned char delta = *ptr;
					ptr++;

					pixel_x += VoxelPixelDeltaTable[delta][0];
					pixel_y += VoxelPixelDeltaTable[delta][1];
					pixel_z += state->TransformMatrix[3].K * delta;

					remaining -= delta;

					// Byte 1 - run length(forward)
					unsigned int run_length = *ptr;
					ptr++;

					if (run_length) {
						remaining -= run_length;
						while (run_length) {

							/// Compute buffer index and write color
							unsigned int buffer_index = (pixel_x >> 8) | (pixel_y & 0xFF00);
							if ((pixel_z >> 8) > VoxelDrawZBuffer[buffer_index]) {

								/*
								 * Byte 2 - color index
								 */
								unsigned char color_index = *ptr;
								ptr++;

								/*
								 * Byte 3 - normal index
								 */
								unsigned char normal_index = *ptr;
								unsigned char table_index = VoxelNormalTranslateTable[normal_index];
								ptr++;

								color_index = VoxelPaletteTranslateTable[table_index][color_index];

								VoxelDrawZBuffer[buffer_index] = (pixel_z >> 8);
								VoxelDrawZBuffer[buffer_index + 1] = (pixel_z >> 8);
								VoxelDrawBuffer[buffer_index] = color_index;
								VoxelDrawBuffer[buffer_index + 1] = color_index;
							} else {
								/*
								 * Byte 2 - color index
								 */
								ptr++;

								/*
								 * Byte 3 - normal index
								 */
								ptr++;
							}

							pixel_x += state->TransformMatrix[3].I;
							pixel_y += state->TransformMatrix[3].J;
							pixel_z += state->TransformMatrix[3].K;
							run_length--;
						}
					}

					// Byte 4 - run length(backward)
					ptr++;
				}
			}

			/// Advance to next voxel in X direction
			pixel_x = column_start_x + state->TransformMatrix[1].I;
			pixel_y = column_start_y + state->TransformMatrix[1].J;
			pixel_z = column_start_z + state->TransformMatrix[1].K;
			state->StartIndex = state->StrideX + state->StartIndex;
		}

		/// Advance to next voxel row (Y direction)
		pixel_x = row_start_x + state->TransformMatrix[2].I;
		pixel_y = row_start_y + state->TransformMatrix[2].J;
		pixel_z = row_start_z + state->TransformMatrix[2].K;
		state->StartIndex = state->StrideY + base_index;
	}
}


/// <summary>
/// Draws a shaded and depth buffered voxel object.
/// This is the low level drawer for the reversed orientations, used when the draw system
/// has both lighting and the depth buffer switched on. Each voxel is tested against the
/// voxel depth buffer, and the ones that survive are remapped through the normal lighting
/// lookup. It is reached through the VoxelDrawFunctions dispatch table.
/// </summary>
/// <param name="state">The projection, stride and voxel data setup for this object.</param>
void __cdecl Draw_Voxel_Reverse_Normals_ZBuffer_Lighting(VoxelFuncArgumentStruct * state)
{
	/*
	 * Precompute screen projection deltas for every Z step
	 */
	Fill_Delta_Table1(state);

	/// Set starting 2D projection position
	unsigned short pixel_x = state->TransformMatrix[0].I;
	unsigned short pixel_y = state->TransformMatrix[0].J;
	unsigned short pixel_z = state->TransformMatrix[0].K;

	/// Iterate over voxel Y slices (rows)
	for (unsigned int y = 0; y < state->YSize; y++) {
		unsigned int base_index = state->StartIndex;
		unsigned short row_start_x = pixel_x;
		unsigned short row_start_y = pixel_y;
		unsigned short row_start_z = pixel_z;

		/// Iterate over voxel X columns (within the current Y row)
		for (unsigned int x = 0; x < state->XSize; x++) {
			unsigned short column_start_x = pixel_x;
			unsigned short column_start_y = pixel_y;
			unsigned short column_start_z = pixel_z;
			unsigned int data_offset = ((unsigned int *)state->EndOffset)[state->StartIndex];

			if (data_offset != UINT_MAX) {
				unsigned char * ptr = state->DataOffset + data_offset;
				unsigned int remaining = state->ZSize;

				/// Parse voxel run-length encoded data along Z axis
				while (remaining) {

					// Byte 4 - run length(backward)
					unsigned int run_length = *ptr;
					ptr--;

					if (run_length) {
						remaining -= run_length;
						while (run_length) {

							/// Compute buffer index and write color
							unsigned int buffer_index = (pixel_x >> 8) | (pixel_y & 0xFF00);
							if ((pixel_z >> 8) > VoxelDrawZBuffer[buffer_index]) {

								/*
								 * Byte 3 - normal index
								 */
								unsigned char normal_index = *ptr;
								unsigned char table_index = VoxelNormalTranslateTable[normal_index];
								ptr--;

								/*
								 * Byte 2 - color index
								 */
								unsigned char color_index = *ptr;
								ptr--;

								color_index = VoxelPaletteTranslateTable[table_index][color_index];

								VoxelDrawZBuffer[buffer_index] = (pixel_z >> 8);
								VoxelDrawZBuffer[buffer_index + 1] = (pixel_z >> 8);
								VoxelDrawBuffer[buffer_index] = color_index;
								VoxelDrawBuffer[buffer_index + 1] = color_index;
							} else {

								/*
								 * Byte 3 - normal index
								 */
								ptr--;

								/*
								 * Byte 2 - color index
								 */
								ptr--;
							}

							pixel_x += state->TransformMatrix[3].I;
							pixel_y += state->TransformMatrix[3].J;
							pixel_z += state->TransformMatrix[3].K;
							run_length--;
						}
					}

					// Byte 1 - run length(forward)
					ptr--;

					/*
					 * Byte 0 - pixel delta
					 */
					unsigned char delta = *ptr;
					ptr--;

					pixel_x += VoxelPixelDeltaTable[delta][0];
					pixel_y += VoxelPixelDeltaTable[delta][1];
					pixel_z += state->TransformMatrix[3].K * delta;

					remaining -= delta;
				}
			}

			/// Advance to next voxel in X direction
			pixel_x = column_start_x + state->TransformMatrix[1].I;
			pixel_y = column_start_y + state->TransformMatrix[1].J;
			pixel_z = column_start_z + state->TransformMatrix[1].K;
			state->StartIndex = state->StrideX + state->StartIndex;
		}

		/// Advance to next voxel row (Y direction)
		pixel_x = row_start_x + state->TransformMatrix[2].I;
		pixel_y = row_start_y + state->TransformMatrix[2].J;
		pixel_z = row_start_z + state->TransformMatrix[2].K;
		state->StartIndex = state->StrideY + base_index;
	}
}


/// <summary>
/// Draws a voxel object that carries no normals.
/// This is the low level drawer for the forward orientations, used when the draw system
/// has both lighting and the depth buffer switched off. The color indices go straight into
/// the voxel draw buffer. It is reached through the VoxelDrawFunctions dispatch table.
/// </summary>
/// <param name="state">The projection, stride and voxel data setup for this object.</param>
void __cdecl Draw_Voxel_Regular(VoxelFuncArgumentStruct * state)
{
	/*
	 * Precompute screen projection deltas for every Z step
	 */
	Fill_Delta_Table1(state);

	/// Set starting 2D projection position
	unsigned short pixel_x = state->TransformMatrix[0].I;
	unsigned short pixel_y = state->TransformMatrix[0].J;

	/// Iterate over voxel Y slices (rows)
	for (unsigned int y = 0; y < state->YSize; y++) {
		unsigned int base_index = state->StartIndex;
		unsigned short row_start_x = pixel_x;
		unsigned short row_start_y = pixel_y;

		/// Iterate over voxel X columns (within the current Y row)
		for (unsigned int x = 0; x < state->XSize; x++) {
			unsigned int data_offset = ((unsigned int *)state->StartOffset)[state->StartIndex];
			unsigned short column_start_x = pixel_x;
			unsigned short column_start_y = pixel_y;

			if (data_offset != UINT_MAX) {
				unsigned char * ptr = state->DataOffset + data_offset;
				unsigned int remaining = state->ZSize;

				/// Parse voxel run-length encoded data along Z axis
				while (remaining) {

					/*
					 * Byte 0 - pixel delta
					 */
					unsigned char delta = *ptr;
					ptr++;

					pixel_x += VoxelPixelDeltaTable[delta][0];
					pixel_y += VoxelPixelDeltaTable[delta][1];

					remaining -= delta;

					// Byte 1 - run length(forward)
					unsigned int run_length = *ptr;
					ptr++;

					if (run_length) {
						remaining -= run_length;
						while (run_length) {

							/*
							 * Byte 2 - color index
							 */
							unsigned char color_index = *ptr;
							ptr++;

							/// Compute buffer index and write color
							unsigned int buffer_index = (pixel_x >> 8) | (pixel_y & 0xFF00);
							VoxelDrawBuffer[buffer_index] = color_index;
							VoxelDrawBuffer[buffer_index + 1] = color_index;

							pixel_x += state->TransformMatrix[3].I;
							pixel_y += state->TransformMatrix[3].J;
							run_length--;
						}
					}

					// Byte 4 - run length(backward)
					ptr++;
				}
			}

			/// Advance to next voxel in X direction
			pixel_x = column_start_x + state->TransformMatrix[1].I;
			pixel_y = column_start_y + state->TransformMatrix[1].J;
			state->StartIndex = state->StrideX + state->StartIndex;
		}

		/// Advance to next voxel row (Y direction)
		pixel_x = row_start_x + state->TransformMatrix[2].I;
		pixel_y = row_start_y + state->TransformMatrix[2].J;
		state->StartIndex = state->StrideY + base_index;
	}
}


/// <summary>
/// Draws a voxel object that carries no normals.
/// This is the low level drawer for the reversed orientations, used when the draw system
/// has both lighting and the depth buffer switched off. The color indices go straight into
/// the voxel draw buffer. It is reached through the VoxelDrawFunctions dispatch table.
/// </summary>
/// <param name="state">The projection, stride and voxel data setup for this object.</param>
void __cdecl Draw_Voxel_Reverse(VoxelFuncArgumentStruct * state)
{
	/*
	 * Precompute screen projection deltas for every Z step
	 */
	Fill_Delta_Table2(state);

	/// Set starting 2D projection position
	unsigned short pixel_x = state->TransformMatrix[0].I;
	unsigned short pixel_y = state->TransformMatrix[0].J;

	/// Iterate over voxel Y slices (rows)
	for (unsigned int y = 0; y < state->YSize; y++) {
		unsigned int base_index = state->StartIndex;
		unsigned short row_start_x = pixel_x;
		unsigned short row_start_y = pixel_y;

		/// Iterate over voxel X columns (within the current Y row)
		for (unsigned int x = 0; x < state->XSize; x++) {
			unsigned int data_offset = ((unsigned int *)state->EndOffset)[state->StartIndex];
			unsigned short column_start_x = pixel_x;
			unsigned short column_start_y = pixel_y;

			if (data_offset != UINT_MAX) {
				unsigned char * ptr = state->DataOffset + data_offset;
				unsigned int remaining = state->ZSize;

				/// Parse voxel run-length encoded data along Z axis
				while (remaining) {

					// Byte 4 - run length(backward)
					unsigned int run_length = *ptr;
					ptr--;

					if (run_length) {
						remaining -= run_length;
						while (run_length) {

							/*
							 * Byte 2 - color index
							 */
							unsigned char color_index = *ptr;
							ptr--;

							/// Compute buffer index and write color
							unsigned int buffer_index = (pixel_x >> 8) | (pixel_y & 0xFF00);
							VoxelDrawBuffer[buffer_index] = color_index;
							VoxelDrawBuffer[buffer_index + 1] = color_index;

							pixel_x += state->TransformMatrix[3].I;
							pixel_y += state->TransformMatrix[3].J;
							run_length--;
						}
					}

					// Byte 1 - run length(forward)
					ptr--;

					/*
					 * Byte 0 - pixel delta
					 */
					unsigned char delta = *ptr;
					ptr--;

					pixel_x += VoxelPixelDeltaTable[delta][0];
					pixel_y += VoxelPixelDeltaTable[delta][1];

					remaining -= delta;
				}
			}

			/// Advance to next voxel in X direction
			pixel_x = column_start_x + state->TransformMatrix[1].I;
			pixel_y = column_start_y + state->TransformMatrix[1].J;
			state->StartIndex = state->StrideX + state->StartIndex;
		}

		/// Advance to next voxel row (Y direction)
		pixel_x = row_start_x + state->TransformMatrix[2].I;
		pixel_y = row_start_y + state->TransformMatrix[2].J;
		state->StartIndex = state->StrideY + base_index;
	}
}


/// <summary>
/// Draws a depth buffered voxel object that carries no normals.
/// This is the low level drawer for the forward orientations, used when the draw system
/// has the depth buffer switched on but lighting switched off. Each voxel is tested and
/// stamped against the voxel depth buffer before its color reaches the draw buffer. It is
/// reached through the VoxelDrawFunctions dispatch table.
/// </summary>
/// <param name="state">The projection, stride and voxel data setup for this object.</param>
void __cdecl Draw_Voxel_Regular_ZBuffer(VoxelFuncArgumentStruct * state)
{
	/*
	 * Precompute screen projection deltas for every Z step
	 */
	Fill_Delta_Table2(state);

	/// Set starting 2D projection position
	unsigned short pixel_x = state->TransformMatrix[0].I;
	unsigned short pixel_y = state->TransformMatrix[0].J;
	unsigned short pixel_z = state->TransformMatrix[0].K;

	/// Iterate over voxel Y slices (rows)
	for (unsigned int y = 0; y < state->YSize; y++) {
		unsigned int base_index = state->StartIndex;
		unsigned short row_start_x = pixel_x;
		unsigned short row_start_y = pixel_y;
		unsigned short row_start_z = pixel_z;

		/// Iterate over voxel X columns (within the current Y row)
		for (unsigned int x = 0; x < state->XSize; x++) {
			unsigned short column_start_x = pixel_x;
			unsigned short column_start_y = pixel_y;
			unsigned short column_start_z = pixel_z;
			unsigned int data_offset = ((unsigned int *)state->StartOffset)[state->StartIndex];

			if (data_offset != UINT_MAX) {
				unsigned char * ptr = state->DataOffset + data_offset;
				unsigned int remaining = state->ZSize;

				/// Parse voxel run-length encoded data along Z axis
				while (remaining) {

					/*
					 * Byte 0 - pixel delta. This one byte variable serves as
					 * both the pixel delta and, further down, the color index.
					 */
					unsigned char value = *ptr;
					ptr++;

					pixel_x += VoxelPixelDeltaTable[value][0];
					pixel_y += VoxelPixelDeltaTable[value][1];
					pixel_z += state->TransformMatrix[3].K * value;

					remaining -= value;

					// Byte 1 - run length(forward)
					unsigned int run_length = *ptr;
					ptr++;

					if (run_length) {
						remaining -= run_length;
						while (run_length) {

							/*
							 * Byte 2 - color index
							 */
							value = *ptr;
							ptr++;

							/// Compute buffer index and write color
							unsigned int buffer_index = (pixel_x >> 8) | (pixel_y & 0xFF00);
							if ((pixel_z >> 8) > VoxelDrawZBuffer[buffer_index]) {
								VoxelDrawZBuffer[buffer_index] = (pixel_z >> 8);
								VoxelDrawBuffer[buffer_index] = value;
								VoxelDrawBuffer[buffer_index + 1] = value;
								VoxelDrawZBuffer[buffer_index + 1] = (pixel_z >> 8);
							}

							pixel_x += state->TransformMatrix[3].I;
							pixel_y += state->TransformMatrix[3].J;
							pixel_z += state->TransformMatrix[3].K;
							run_length--;
						}
					}

					// Byte 4 - run length(backward)
					ptr++;
				}
			}

			/// Advance to next voxel in X direction
			pixel_x = column_start_x + state->TransformMatrix[1].I;
			pixel_y = column_start_y + state->TransformMatrix[1].J;
			pixel_z = column_start_z + state->TransformMatrix[1].K;
			state->StartIndex = state->StrideX + state->StartIndex;
		}

		/// Advance to next voxel row (Y direction)
		pixel_x = row_start_x + state->TransformMatrix[2].I;
		pixel_y = row_start_y + state->TransformMatrix[2].J;
		pixel_z = row_start_z + state->TransformMatrix[2].K;
		state->StartIndex = state->StrideY + base_index;
	}
}


/// <summary>
/// Draws a depth buffered voxel object that carries no normals.
/// This is the low level drawer for the reversed orientations, used when the draw system
/// has the depth buffer switched on but lighting switched off. Each voxel is tested and
/// stamped against the voxel depth buffer before its color reaches the draw buffer. It is
/// reached through the VoxelDrawFunctions dispatch table.
/// </summary>
/// <param name="state">The projection, stride and voxel data setup for this object.</param>
void __cdecl Draw_Voxel_Reverse_ZBuffer(VoxelFuncArgumentStruct * state)
{
	/*
	 * Precompute screen projection deltas for every Z step
	 */
	Fill_Delta_Table2(state);

	/// Set starting 2D projection position
	unsigned short pixel_x = state->TransformMatrix[0].I;
	unsigned short pixel_y = state->TransformMatrix[0].J;
	unsigned short pixel_z = state->TransformMatrix[0].K;

	/// Iterate over voxel Y slices (rows)
	for (unsigned int y = 0; y < state->YSize; y++) {
		unsigned int base_index = state->StartIndex;
		unsigned short row_start_x = pixel_x;
		unsigned short row_start_y = pixel_y;
		unsigned short row_start_z = pixel_z;

		/// Iterate over voxel X columns (within the current Y row)
		for (unsigned int x = 0; x < state->XSize; x++) {
			unsigned short column_start_x = pixel_x;
			unsigned short column_start_y = pixel_y;
			unsigned short column_start_z = pixel_z;
			unsigned int data_offset = ((unsigned int *)state->EndOffset)[state->StartIndex];

			if (data_offset != UINT_MAX) {
				unsigned char * ptr = state->DataOffset + data_offset;
				unsigned int remaining = state->ZSize;

				/// Parse voxel run-length encoded data along Z axis
				while (remaining) {

					// Byte 4 - run length(backward)
					unsigned int run_length = *ptr;
					ptr--;

					/*
					 * One byte variable serves as both the color index and,
					 * further down, the pixel delta.
					 */
					unsigned char value;

					if (run_length) {
						remaining -= run_length;
						while (run_length) {

							/*
							 * Byte 2 - color index
							 */
							value = *ptr;
							ptr--;

							/// Compute buffer index and write color
							unsigned int buffer_index = (pixel_x >> 8) | (pixel_y & 0xFF00);
							if ((pixel_z >> 8) > VoxelDrawZBuffer[buffer_index]) {
								VoxelDrawZBuffer[buffer_index] = (pixel_z >> 8);
								VoxelDrawBuffer[buffer_index] = value;
								VoxelDrawBuffer[buffer_index + 1] = value;
								VoxelDrawZBuffer[buffer_index + 1] = (pixel_z >> 8);
							}

							pixel_x += state->TransformMatrix[3].I;
							pixel_y += state->TransformMatrix[3].J;
							pixel_z += state->TransformMatrix[3].K;
							run_length--;
						}
					}

					// Byte 1 - run length(forward)
					ptr--;

					/*
					 * Byte 0 - pixel delta
					 */
					value = *ptr;
					ptr--;

					pixel_x += VoxelPixelDeltaTable[value][0];
					pixel_y += VoxelPixelDeltaTable[value][1];
					pixel_z += state->TransformMatrix[3].K * value;

					remaining -= value;
				}
			}

			/// Advance to next voxel in X direction
			pixel_x = column_start_x + state->TransformMatrix[1].I;
			pixel_y = column_start_y + state->TransformMatrix[1].J;
			pixel_z = column_start_z + state->TransformMatrix[1].K;
			state->StartIndex = state->StrideX + state->StartIndex;
		}

		/// Advance to next voxel row (Y direction)
		pixel_x = row_start_x + state->TransformMatrix[2].I;
		pixel_y = row_start_y + state->TransformMatrix[2].J;
		pixel_z = row_start_z + state->TransformMatrix[2].K;
		state->StartIndex = state->StrideY + base_index;
	}
}


/// <summary>
/// Builds the voxel normal lighting lookup for plain diffuse light.
/// This routine is used by the voxel draw system to reduce every entry of the normal table
/// to a palette brightness step, which the lighting-aware voxel drawers then use to shade
/// each voxel from its normal index.
/// </summary>
/// <param name="light">The direction the light arrives from.</param>
/// <param name="normal_type">The voxel normal table the lookup is being built for.</param>
void Precalculate_Normal_Lookup(Vector3 const & light, int normal_type)
{
	Vector3 const * normal_table = VoxelNormalTables[normal_type];
	int count = VoxelNormalTableEntryCount[normal_type];

	for (int i = 0; i < count; i++) {

		Vector3 const & normal = normal_table[i];

		/*
		 * Lambertian diffuse term: cosine of angle between normal and light
		 */
		float diffuse = normal * light;

		/*
		 * Convert to palette index range [0, VOXEL_PALETTE_LOOKUP_NEUTRAL]
		 */
		if (diffuse < 0.0) {
			VoxelNormalTranslateTable[i] = 0;
		} else {
			VoxelNormalTranslateTable[i] = (unsigned char)((double)diffuse * VOXEL_PALETTE_LOOKUP_NEUTRAL);
		}
	}

	VoxelNormalTranslateTable[VOXEL_PALETTE_SIZE - 1] = VOXEL_PALETTE_LOOKUP_NEUTRAL;
	VoxelNormalTranslateTable[VOXEL_PALETTE_SIZE - 2] = VOXEL_PALETTE_LOOKUP_NEUTRAL;
	VoxelNormalTranslateTable[VOXEL_PALETTE_SIZE - 3] = VOXEL_PALETTE_LOOKUP_NEUTRAL;
}


/// <summary>
/// Builds the voxel normal lighting lookup with a specular highlight.
/// This routine is used by the voxel draw system when the scene lighting calls for a
/// specular term as well as a diffuse one. Every entry of the normal table is reduced to
/// a palette brightness step that the lighting-aware voxel drawers then use to shade each
/// voxel from its normal index.
/// </summary>
/// <param name="light">The direction the light arrives from.</param>
/// <param name="viewer">The direction the object is being viewed from.</param>
/// <param name="specular_strength">Tightness of the specular highlight.</param>
/// <param name="normal_type">The voxel normal table the lookup is being built for.</param>
void Precalculate_Normal_Lookup(Vector3 const & light, Vector3 const & viewer, float specular_strength, int normal_type)
{
	Vector3 const * normal_table = VoxelNormalTables[normal_type];

	/*
	 * Halfway vector between light direction and view direction (Blinn-Phong model)
	 */
	Vector3 halfway = Normalize(viewer + light);

	for (int i = 0; i < VoxelNormalTableEntryCount[normal_type]; i++) {

		Vector3 const & normal = normal_table[i];

		/*
		 * Lambertian diffuse term: cosine of angle between normal and light
		 */
		float diffuse = normal * light;

		/*
		 * Specular term: cosine of angle between normal and halfway vector
		 */
		float halfway_dot = normal * halfway;

		/*
		 * Specular boost (empirical) - the formula adjusts how specular highlights behave
		 */
		float specular = halfway_dot / (specular_strength - halfway_dot * specular_strength + halfway_dot);

		/*
		 * Clamp both values to [0, inf)
		 */
		specular = specular >= 0.0f ? specular : 0.0f;
		diffuse = diffuse >= 0.0f ? diffuse : 0.0f;

		/*
		 * Final brightness contribution
		 */
		float brightness = diffuse + specular;

		/*
		 * Neutral is the unshaded row and a highlight belongs above it, so the
		 * ceiling is the last row rather than neutral. The clamp is
		 * load-bearing: specular has no natural bound, and the drawers index
		 * this row unchecked.
		 */
		int row = (int)((double)brightness * VOXEL_PALETTE_LOOKUP_NEUTRAL);
		VoxelNormalTranslateTable[i] = (unsigned char)std::clamp(row, 0, MAX_PALETTE_LOOKUP_ENTRIES - 1);
	}

	VoxelNormalTranslateTable[VOXEL_PALETTE_SIZE - 1] = VOXEL_PALETTE_LOOKUP_NEUTRAL;
	VoxelNormalTranslateTable[VOXEL_PALETTE_SIZE - 2] = VOXEL_PALETTE_LOOKUP_NEUTRAL;
	VoxelNormalTranslateTable[253] = VOXEL_PALETTE_LOOKUP_NEUTRAL;
}


/// <summary>
/// Finds the voxel normal that best matches the direction given.
/// This routine is used to quantize an arbitrary surface direction down to one of the
/// entries in a voxel normal table, so that it can be stored as a single normal index.
/// </summary>
/// <param name="surface_normal">The direction to match against the normal table.</param>
/// <param name="normal_type">The voxel normal table to search.</param>
/// <returns>Returns with the index of the closest normal. A direction too short to give a
/// stable answer yields 255, the neutral entry.</returns>
int Find_Closest_Normal(Vector3 const & surface_normal, int normal_type)
{
	float best_dot = 0.0f;
	int best_index = 0;

	Vector3 const * normal_table = VoxelNormalTables[normal_type];

	/*
	 * Degenerate check: very short vectors can't produce stable normals
	 */
	if (surface_normal.Length() < 0.2f) {
		return(VOXEL_PALETTE_SIZE - 1);
	}

	for (int i = 0; i < VoxelNormalTableEntryCount[normal_type]; i++) {

		Vector3 const & normal = normal_table[i];

		/*
		 * Cosine of angle between voxel vector and current normal
		 */
		float dot = normal * surface_normal;

		/*
		 * Keep the normal with the highest dot (most aligned)
		 */
		if (dot > best_dot) {
			best_dot = dot;
			best_index = i;
		}
	}

	return(best_index);
}


/// <summary>
/// Resets the voxel normal lighting lookup to neutral.
/// This routine is used to put every normal back to plain ambient brightness, so that
/// voxels draw unshaded until a lighting pass fills the lookup in again.
/// </summary>
void Init_Normal_Lookup(void)
{
	memset(VoxelNormalTranslateTable, VOXEL_PALETTE_LOOKUP_NEUTRAL, VOXEL_PALETTE_SIZE);
}
