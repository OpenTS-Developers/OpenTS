---
title: Team execution
summary: "Each frame, every team rechecks its strength, starts or regroups, gathers its members, and works through its Script."
category: ai-teams
keys:
  - Aggressive
  - Annoyance
  - CloseEnough
  - GuardSlower
  - House
  - IonImmune
  - OnlyTargetHouseEnemy
  - Priority
  - Reinforce
  - Script
  - Stray
  - Suicide
  - TaskForce
  - TransportsReturnOnUnload
  - Waypoint
related:
  - type: system
    id: ai-team-production
  - type: system
    id: base-attacked
  - type: format
    id: teamtypes
  - type: format
    id: taskforces
  - type: format
    id: scripts
  - type: event
    id: TEVENT_LEAVES_MAP
---

[AI triggers and team production](/systems/ai-team-production/#teamtypes-and-ai-triggers-in-brief) introduces teams, [TeamTypes](/mapping/team-types/) and the three sections a TeamType points at, and covers everything up to the moment a team exists. [The individual team missions](/mapping/missions/) cover what each Script line does. This page covers the turn every team takes each frame, which is where those missions run.

The TeamType settings this page refers to are written together like this:

```ini title="AI.INI, AIFS.INI, or map file"
[TeamTypes]
0=MyPatrolTeam

[MyPatrolTeam]
House=Nod                    ; the house whose teams are built from this type
TaskForce=MyPatrolForce      ; the roster the team counts its strength against
Script=MyPatrolScript        ; the list of missions the team works through
Waypoint=A                   ; the cell the team enters at and recruits around
Priority=5                   ; how firmly the team holds its members
Reinforce=yes                ; yes keeps recruiting after the start and lets the team fall back to regroup
GuardSlower=no               ; yes counts a slow member's position twice in the center
Aggressive=no                ; yes lets a member holding a target stay and fight during a move
Annoyance=yes                ; yes makes a started team reform when a member takes damage
Suicide=no                   ; yes switches the damage response off
TransportsReturnOnUnload=no  ; yes releases each transport from the team after it unloads
OnlyTargetHouseEnemy=no      ; yes limits the targets the Script picks to the house's enemy
IonImmune=no                 ; yes protects members from ion storm bolts
```

The values are examples. The example omits the `[MyPatrolForce]` TaskForce and `[MyPatrolScript]` Script sections it names.

## The logic turn

Each team takes one turn per logic frame, and the turn runs the steps below in order. Several steps can act on the same turn. Steps 2, 7 and 8 can delete the team, which ends its turn.

1. **Suspension.** A team [suspended by a base defense call-up](/systems/base-attacked/#teams-are-emptied-first) skips the rest of its turn. On the turn its suspension timer reaches zero, the suspension ends and the turn continues with step 2.
2. **The strength recalculation.** This step runs only when the team's roster has changed since the last recalculation, for example when a member joins or leaves. It sets the full-strength, under-strength and reforming flags [below](#the-state-flags). A team with no members left that holds the **started mark** is deleted here, as in step 7. A team gets the started mark the first time it reaches full strength or is flagged into action, and never loses it.
3. **The regroup.** A team that is **under way** and under strength is stopped. Its Script is rewound, and it is sent to gather near one of its house's unarmed structures, or at its own center when none qualifies. A team is under way from the moment it is flagged into action until it is sent back to regroup. [Base defense response](/systems/base-attacked/#what-reads-the-map) covers which structure it picks.
4. **The start.** A team that is not under way is flagged into action when it is at full strength or **forced active**. Full strength means the team holds every member its TaskForce asks for. Only a team delivered by a [Reinforcement (team)](/mapping/actions/taction-reinforcements/) or [Reinforcement (team) at waypoint](/mapping/actions/taction-reinforcements-special/) action is forced active; a team built by a team-creation action is not. A team regrouped in step 3 is short of full strength, so this step restarts it on the same turn only if it is forced active.
5. **The center.** The team recomputes its [center](#the-teams-center) and the member nearest its target under **any of:**
   - the team is reforming;
   - it is under way;
   - it has no center recorded;
   - the member nearest its target at the last recomputation has since been destroyed.
6. **Recruitment.** The team looks for members to fill the places its TaskForce still wants. [The recruitment pass](/systems/ai-team-production/#recruitment) owns the conditions, the ranking and the tests a candidate must pass.
7. **Dissolution.** A team with no members is deleted under **any of:**
   - it holds the started mark;
   - in a skirmish or multiplayer game, it has existed longer than [`DissolveUnfilledTeamDelay`](/keys/dissolveunfilledteamdelay/).

   If the team is marked as having [left the map](#leaving-the-map), [Leaves map (team)](/mapping/events/tevent-leaves-map/) springs before the team is deleted.
8. **The fork.** The team runs its [current Script line](#the-script-cursor) under **all of:**
   - the team is under way;
   - it is not reforming;
   - it is not under strength.

   Otherwise, a team that is under way runs the [regroup coordinator](#regrouping), and a team that is not runs the [move coordinator](#moving).

### Where the turn sits in the frame

Team turns run before object turns and before house turns in each logic frame. Members therefore act on a team's orders later in the same frame, and the team sees its members as the previous frame left them. A team created during another team's turn takes its first turn in the next frame.

## The state flags

Six flags decide which branch each step takes. The table shows what raises and clears each one. Two of them, the started mark and forced active, never clear once raised.

| Flag | Raised by | Cleared by |
| --- | --- | --- |
| Under way | The start step | The regroup step |
| Full strength | The recalculation, when the team holds every member its TaskForce asks for | The recalculation, at any other count |
| Under strength | Creation, and the recalculation at the threshold below | The recalculation above that threshold, the start step, and being marked forced active |
| The started mark | The recalculation on reaching full strength, and the start step | Nothing |
| Forced active | Creation as a reinforcement group, which is the only thing that sets it | Nothing |
| Reforming | The recalculation, whenever it changes the under-strength flag, and [damage to a member](#answering-damage) | The regroup coordinator, on a turn it sends no member back to the center |

The under-strength threshold depends on [`Reinforce`](/keys/reinforce/) and on the total number of members the TaskForce asks for:

- **`Reinforce=yes`, three or more members:** the team is under strength while it holds a third of that total or fewer, rounded down. A TaskForce of seven members gives a threshold of two.
- **`Reinforce=yes`, one or two members:** the team is under strength while it is short of full strength.
- **`Reinforce=no`:** the team is under strength until it first holds the started mark, and never afterward.

A team with no members is under strength and not at full strength, whatever its type says.

A stopped team keeps its reforming flag. Only the regroup coordinator clears the flag, and step 8 runs that coordinator only for a team that is under way. If the recalculation or damage raises the flag while the team is stopped, the flag stays raised until the team starts again.

The reforming flag at the start step decides how the members join the formation. If the team is reforming or forced active when it is flagged into action, every member counts as [in formation](#bringing-a-member-into-formation) from that moment. Otherwise each member that is not yet in formation must first reach the team's center.

## Starting and restarting

### The reform delay

An ordinary team does not run its first Script line on the turn it starts. It first regroups, and runs the first line only after the regroup coordinator reports it gathered.

The delay follows from the flags. A team is created under strength. The under-strength flag clears no later than the recalculation that finds the team at full strength, and that change raises the reforming flag. A stopped team cannot clear that flag, so it is still raised when the start step fires, and step 8 sends the team to the regroup coordinator. The flag clears on a turn when the coordinator sends no member back to the center. The team runs the first line of its Script on the following turn.

A forced active team skips the delay. Being marked forced active clears the under-strength flag before any member is added. The group is normally created with its whole TaskForce, so its first recalculation finds it at full strength and leaves the under-strength flag clear. Nothing raises the reforming flag, and the group runs the first line of its Script on its first turn.

### Every regroup rewinds the script

A team sent back to regroup restarts its Script from the first line, however far it had worked through it. The regroup step rewinds the Script to before its first line, and the next start step moves it onto the first line. Nothing records the line the team had reached.

An ordinary team starts again only once recruitment has brought it back to full strength. A forced active team starts again on the same turn it regroups, and so goes straight back to its first line.

Only a `Reinforce=yes` team can be sent back at all. The regroup step needs a team that is under way and under strength, and a `Reinforce=no` team is never under strength once it has started. `Reinforce` therefore decides whether a started team keeps recruiting, and also whether it can lose its place in its Script.

## The team's center

A team's **center** is the point its members gather on and are measured against. It is either a cell or a particular object, usually one of the members, and it moves with that object.

Step 5 recomputes the center from scratch. A member counts toward it under **all of:**

- it is alive and out of [limbo](/glossary/#limbo);
- it is [in formation](#bringing-a-member-into-formation), or it is an aircraft;
- it has entered the playable area.

The engine averages the positions of the counted members. A [`GuardSlower=yes`](/keys/guardslower/) team counts each slow member's position twice.

The average is usually discarded. If the counted member nearest the team's current target could move into the averaged cell right now, that member becomes the center. The averaged cell stays the center only when that member could not enter it, because the ground is impassable or something is standing there. On open ground, a team's center is therefore one member's position, not the middle of the group.

A team whose current line is [Follow friendlies](/mapping/missions/tmission-hound-dog/) takes as its center a vehicle or infantry that belongs to an allied house other than its own. It picks the one nearest its most recently joined member. If there is none, the team has no center.

[`Waypoint`](/keys/waypoint/) gives a team a center when it is created. Step 5 runs on the team's first turn whatever its flags say, so the computed center replaces that cell before the team uses it. `Waypoint` keeps four other roles, and each reads the waypoint's cell directly:

- recruitment ranks candidates by their distance from it;
- a vehicle or infantry member on the Retreat mission heads for a map edge cell calculated from it;
- an aircraft carrying a member picks its landing zone near it;
- a reinforcement group enters there, unless a Reinforcement (team) at waypoint action names another waypoint.

The regroup step crashes the game when the team has members but none of them counts toward the center. That happens, for example, when every member in formation is in limbo, such as inside a transport, or when none has entered the playable area yet.

## Keeping the members together

### Bringing a member into formation

A member is **in formation** once it has come within [`Stray`](/keys/stray/) of the team's center. The coordinators give their orders only to members in formation and to aircraft. A member stays in formation for as long as it belongs to the team. Some members are in formation without reaching the center:

- the first member to join an empty team;
- every member of a reinforcement group;
- every member of a team that starts while reforming or forced active;
- one of the remaining members, when a departure leaves no member in formation.

The regroup and move coordinators test every member that is not yet in formation, and so do several team missions. A member that is alive, out of limbo and farther than `Stray` from the center is put on the Move mission and sent to the center, unless it is already heading somewhere. A member within `Stray` joins the formation. Until it joins, it holds up a [move](#moving), and the coordinators pass over it unless it is an aircraft.

A stopped team with no target does not run this test. The move coordinator does nothing without a target, and a team has none before it starts. Members recruited while the team is still filling therefore stay where they were recruited, and the team gathers only after it starts. A team stopped by the regroup step is the exception: its regroup point is its target, so it walks there and brings members into formation on the way.

### Regrouping

The regroup coordinator holds a team in place and closes it up. It treats each member in formation, and each aircraft, as follows:

- A member on the Area Guard mission that holds a target is left where it is, however far away it is.
- A member farther than `Stray` from the center that has no destination is put on the Move mission and sent to the center.
- A member farther than `Stray` from the center that already has a destination is left alone. It does not hold up the regroup.
- Every other member is put on the Guard mission and its destination is cleared, unless it is already on Area Guard.

The coordinator reports the team gathered on a turn when it sends no member to the center. That report clears the reforming flag.

### Moving

The move coordinator walks the team toward its current target, or toward its mission target when the current target is empty. Each member in formation or aircraft that is not unloading is ordered toward the target under **any of:**

- it is farther from the target than `Stray`, or three times `Stray` for an aircraft;
- **All of:**
  - it is below ground level;
  - the next line of the Script is not a [Move to waypoint](/mapping/missions/tmission-move/) line.
- **All of:**
  - it is an aircraft above the ground;
  - the cell it is over is not the target, which is always true when the target is an object;
  - the next line of the Script is not a Move to waypoint line.

An ordered member is put on the Move mission if it is not on it already, and is given the target as its destination if it has none.

A member that meets none of those conditions is released from a finished move under **all of:**

- it is on the Move mission;
- **Any of:**
  - it has no destination left;
  - it is within [`CloseEnough`](/keys/closeenough/) of its destination and has stopped moving.
- it holds no target.

Release clears the member's destination and returns it to its idle behavior.

The team has arrived on a turn that meets **all of:**

- at least one member in formation or aircraft, not unloading, was checked;
- no member was ordered toward the target;
- no checked member still has a destination;
- no member is unloading;
- no member is still on its way into formation.

Arrival raises the [advance flag](#the-script-cursor) only for a team that is under way. A stopped team walking to its regroup point therefore never advances its Script. A team whose members are all unloading, or all still out of formation, never arrives.

[`Aggressive=yes`](/keys/aggressive/) exempts a member that already holds a target. If that member would be ordered toward the team's target, it is left alone instead and does not count against arrival. The rest of the team can then arrive and move on without it.

[`TransportsReturnOnUnload=yes`](/keys/transportsreturnonunload/) makes this coordinator record a return point for each member that can carry passengers and has none yet. The point is the cell the member stands in on the first move toward a target after the Script last advanced, and it does not follow the member afterward. Each advance clears the record. The [key page](/keys/transportsreturnonunload/) explains what the transport receives.

## The team's two targets

A team keeps two targets. The **mission target** is what the current Script line asks for. The **current target** is what the members are actually sent against. Damage can override the current target by turning the team on its attacker; see [Answering damage](#answering-damage).

Setting a new mission target also sets the current target, unless the current target has been overridden. In that case the team finishes the fight it is in and takes up the new mission target afterward.

Between advances, whenever the current target is empty, the script step refills it from the mission target. A target also clears when the object it refers to is destroyed.

Changing the mission target also resets the members that were working on the old one. Each member whose target or destination was the old mission target is put on the Guard mission, and whichever of the two pointed at the old target is cleared.

## The script cursor

Each team runs its own copy of the Script its TeamType names. The **cursor** marks the line the team is on. Before the Script starts, and after it is stopped, the cursor sits before the first line. A [team mission](/mapping/missions/) signals that the team should move on by raising the team's **advance flag**.

When step 8 runs the Script and the advance flag is raised:

1. The flag clears, the cursor moves to the next line, and every member's return point is cleared.
2. If the cursor has moved past the last line, the team is deleted. This is the ordinary end of a team that finishes its Script.
3. Otherwise both targets are cleared, and the mission on the new line runs its first pass on the same turn.

On every later turn, the mission on that line runs again, until it raises the advance flag.

:::caution[A line the engine cannot run stalls the team]
A Script line whose mission number is outside `0` to `52` does nothing, because those 53 numbers are the only team missions. The team never advances past the line. Only a [regroup](#every-regroup-rewinds-the-script), which rewinds the Script, gets it moving again, and only a `Reinforce=yes` team can regroup. A mission that never raises the advance flag, and never starts a move that raises it, has the same result.

[Change script...](/mapping/missions/tmission-script/) idles the team in the same way. It replaces the Script and leaves the team before the new Script's first line, where nothing runs.
:::

## Answering damage

When an attacker damages a member, the member's team responds. The response depends on whether the team is under way.

A team that is not under way drops its center and raises the reforming flag, whoever the attacker is.

A team that is under way turns on an attacker from a house it is not allied with, subject to the conditions on the [`Annoyance`](/keys/annoyance/) page. The attacker becomes the team's current target, and the members in formation, and any aircraft, lose their targets and destinations.

Damage from an allied house leaves the team's target and its members' orders alone.

With [`Annoyance=yes`](/keys/annoyance/), damage that passes those conditions makes the team drop its center and reform. With the center gone, the team keeps an armed current target and does not switch to the attacker.

[`Suicide=yes`](/keys/suicide/) switches the whole response off.

## Leaving the map

A team carries a mark recording that it has left the map. [Leaves map (team)](/mapping/events/tevent-leaves-map/) requires both that mark and an empty team. The two steps that delete an empty team, steps 2 and 7, spring the event before deleting it.

Assigning the team a mission target that is a cell outside the playable area raises the mark and clears every member's destination. Assigning a cell inside the playable area clears the mark. Three other events raise the mark on terms that match its name:

- a vehicle on the Guard mission is deleted for standing outside the playfield after it had entered the playable area;
- an aircraft on the Retreat mission is deleted for flying outside the playable area;
- a passenger is deleted with such an aircraft.

:::caution[Teams with infantry always count as having left the map]
Each turn, an infantry member that has entered the playable area sets its team's leave-map mark, wherever it stands and whatever it is doing, unless it is inside a tunnel or in limbo, such as aboard a transport. Object turns run after team turns, so the infantry sets the mark again in the same frame that a new mission target clears it. A team with an infantry member therefore keeps the mark, and Leaves map (team) springs as soon as the team is emptied, including a team wiped out in the middle of the map.
:::

## What belonging to a team changes about a member

Belonging to a team changes a member in the ways listed below. The group number and autocreate-recruitable state set on joining stay after the member leaves. The other effects end when it leaves.

- Whichever coordinator or team mission runs on a turn can replace the member's orders.
- On joining, its group number is set to the team's group, and [its autocreate-recruitable state](/systems/ai-team-production/#recruitment) to the TeamType's setting.
- A team of strictly higher [`Priority`](/keys/priority/#scope-teamtype) can take it. It is removed from its team when a base defense call-up suspends teams below the [base defense threshold](/systems/base-attacked/#teams-are-emptied-first).
- [`IonImmune=yes`](/keys/ionimmune/) keeps ion storm bolts from aiming at it, unless its type is a lightning rod, and keeps the ion storm warhead from damaging it.
- [`Suicide=yes`](/keys/suicide/) stops it retaliating, and stops a computer-owned member from looking for targets while it is on the Move mission.
- [`AvoidThreats=yes`](/keys/avoidthreats/) sets the member's [threat avoidance](/systems/base-attacked/#what-reads-the-map) coefficient to `1`, whatever its type says.
- When its destination is temporarily blocked, it switches to a nearby free cell only while it is farther than [`Stray`](/keys/stray/) from the destination. An object on no team uses [`CloseEnough`](/keys/closeenough/) for the same test.
- When it is a transport, [`Loadable`](/keys/loadable/) decides whether the player may order passengers into it.
- It answers a base defense call-up only while its team is a [base defense team](/keys/isbasedefense/#scope-teamtype).

[`OnlyTargetHouseEnemy=yes`](/keys/onlytargethouseenemy/) narrows only the targets the team's Script picks. Members still choose their own targets without that restriction.

## Settings and state without effect

[`GuardSlower=yes`](/keys/guardslower/) weights the [center](#the-teams-center) toward slow members, but it does not make fast members wait for slow ones. The engine has a check that would hold fast members back until the slow ones caught up, but that check never runs.
