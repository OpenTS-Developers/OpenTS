---
title: Team execution
summary: "Gives every team one logic turn a frame, in which it retests its strength, starts or regroups, gathers its members, and works down its Script."
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

Every team in a scenario takes one logic turn a frame, and that turn is the same for all of them whatever their Script asks for. The turn retests the team's strength against its roster, then starts the team or sends it back to regroup. It recomputes the point its members gather on, looks for new members, and disbands the team when there is nothing left of it. Then it either carries out the line of the Script the team is on or spends the turn pulling the members back together.

[AI triggers and team production](/systems/ai-team-production/#teamtypes-and-ai-triggers-in-brief) introduces teams, [TeamTypes](/mapping/team-types/) and the three sections a TeamType points at, and owns everything up to the moment a team exists. What each line of a Script asks for belongs to [the individual team missions](/mapping/missions/). This page is the framework those missions run inside.

A TeamType that carries the settings this page follows is written like this:

```ini title="AI.INI, AIFS.INI, or map file"
[TeamTypes]
0=MyPatrolTeam

[MyPatrolTeam]
House=Nod                    ; the house whose teams are built from this type
TaskForce=MyPatrolForce      ; the roster the strength recalculation measures
Script=MyPatrolScript        ; the mission list the fork steps through
Waypoint=A                   ; the cell the team enters at and recruits around
Priority=5                   ; recruitment ranking, and what a call-up measures
Reinforce=yes                ; admits the team to the regroup step, which rewinds the Script
GuardSlower=no               ; yes counts a slow member's position twice in the center
Aggressive=no                ; yes exempts a member holding a target from the move coordinator
Annoyance=yes                ; yes raises the reforming flag when a member takes damage
Suicide=no                   ; yes switches the damage response off
TransportsReturnOnUnload=no  ; yes stamps each carrier with a return point when it unloads
OnlyTargetHouseEnemy=no      ; yes restricts the target the Script asks the engine to find
IonImmune=no                 ; yes spares every member ion storm lightning and warhead damage
```

## The logic turn

The steps run in the order below. More than one can fire on the same turn, and three of them can end the team, so where a step sits is often the whole explanation for a behavior further down this page.

1. **Suspension.** A team [suspended by a base defense call-up](/systems/base-attacked/#teams-are-emptied-first) leaves the turn at once and runs none of the rest. On the turn its timer reaches zero it clears the suspension and carries on through the remaining steps.
2. **The strength recalculation.** Run only when the roster has changed since the last one; joining or losing a member marks it. It settles the full-strength, under-strength and reforming flags [below](#the-state-flags). A team left holding nobody that already holds the **started mark** is deleted here, in the same way step 7 deletes one. The rest of the turn does not run. That mark is set the first time a team reaches full strength or is flagged into action, and nothing ever clears it.
3. **The regroup.** A team that is **under way** and under strength is stopped, its script is rewound, and it is sent to gather at a friendly building. A team is under way once it has been flagged into action, and stops being under way when it is sent back to regroup. [Base defense response](/systems/base-attacked/#what-reads-the-map) covers which building it picks.
4. **The start.** A team that is not under way is flagged into action when it is at full strength or **forced active**. A team is at full strength when it holds every member its TaskForce asks for. It is forced active only when a [Reinforcement (team)](/mapping/actions/taction-reinforcements/) or [Reinforcement (team) at waypoint](/mapping/actions/taction-reinforcements-special/) action delivered it, not when a team-creation action built it. This step can fire on the same turn as the regroup above, but then only for a forced active team: a regroup leaves a team short of full strength.
5. **The center.** The team's center and its nearest member are recomputed under **any of:**
   - the team is reforming;
   - it is under way;
   - it has no center recorded;
   - it has no nearest member recorded.
6. **Recruitment.** The team looks for members to fill the places its TaskForce still wants. [The recruitment pass](/systems/ai-team-production/#recruitment) owns the gate, the ranking and the tests a candidate has to clear.
7. **Dissolution.** A team left holding nobody is deleted here, once it holds the started mark or, in a skirmish or multiplayer game, has outlived the [`DissolveUnfilledTeamDelay`](/keys/dissolveunfilledteamdelay/) delay. If it is marked as having left the map, this step springs [Leaves map (team)](/mapping/events/tevent-leaves-map/) first.
8. **The fork.** The script step runs under **all of:**
   - the team is under way;
   - it is not reforming;
   - it is not under strength.

   Otherwise a team that is under way runs the regroup coordinator, and one that is not runs the move coordinator.

### Where the turn sits in the frame

Team turns run at a fixed point in the logic frame, before object turns and before house turns. They are taken over a copy of the team list made immediately beforehand, because a team's turn can delete the team. Orders a team issues are therefore acted on by its members later in the same frame. Anything the team reads about its members is what the previous frame left behind.

## The state flags

Six flags between them decide which branch each step takes. The table gives what raises and what clears each one. The two rows that nothing ever clears are where most of the behavior on this page comes from.

| Flag | Raised by | Cleared by |
| --- | --- | --- |
| Under way | The start step | The regroup step |
| Full strength | The recalculation, when the member count equals what the TaskForce asks for | The recalculation, on any other count |
| Under strength | The recalculation, at the threshold below | The recalculation above that threshold, the start step, and being marked forced active |
| The started mark | The recalculation on reaching full strength, and the start step | Nothing |
| Forced active | Creation as a reinforcement group, which is the only thing that sets it | Nothing. The line that would have cleared it at the start step is disabled |
| Reforming | The recalculation, whenever the under-strength flag came out different from the value it went in with, and [damage to a member](#answering-damage) | The regroup coordinator, once every member is gathered |

The under-strength threshold depends on [`Reinforce`](/keys/reinforce/). A `Reinforce=yes` team whose TaskForce asks for three or more members is under strength while it holds a third of that count or fewer, rounded down. One asking for two or fewer is under strength while it is short of full. A `Reinforce=no` team is under strength only until it first holds the started mark, and is permanently not under strength afterward. A team holding nobody at all is under strength and not at full strength whatever its type says.

Two consequences are worth stating on their own.

**The reforming flag cannot clear while the team is stopped.** Its only clearing site is the regroup coordinator, and the fork reaches that coordinator only for a team that is under way. Whatever raises the flag while the team is stopped, whether the recalculation or damage, leaves it raised until the team is started again.

**The start step reads the reforming flag.** A team flagged into action while reforming has every one of its members counted as being in formation from that moment. A team flagged into action while not reforming leaves each member to reach the center on its own. Being forced active has the same effect here as reforming does.

## Starting and restarting

### The reform delay

The ordinary way a team starts guarantees that it is reforming when it does. A team is created under strength, and it stops being under strength at or before the moment it reaches full strength. Whichever recalculation makes that change raises the reforming flag. Nothing can clear the flag in between, because the only thing that clears it is the regroup coordinator. The fork cannot reach that coordinator while the team is stopped. The flag is therefore still raised when the start step fires, and the start step does not clear it. The fork then sends the team to the regroup coordinator rather than to its script. Only once every member is within [`Stray`](/keys/stray/) of the center does the coordinator report the team gathered. The reforming flag clears then, and the following turn reaches the first line of the Script.

A forced active team skips the delay. A reinforcement group is marked forced active as it is created, before a single member is added, and that mark clears the under-strength flag straight away. The group is created with its whole TaskForce at once, so the recalculation that follows finds it at full strength and leaves the flag where it already was. Nothing raises the reforming flag, and the group reaches its first script line on its very first turn.

### Every regroup rewinds the script

A regroup rewinds the team's Script to before its first line, and nothing remembers where the team had reached. The regroup step stops the script. When the team is next flagged into action, the start step stops it again and raises the flag that tells the team to move on to its next line. That advance then steps the cursor from before the first line onto the first line. A team sent back to regroup restarts its mission list from the top, however far through it had worked.

Only a `Reinforce=yes` team can be sent back at all. The regroup step is reached only by a team that is under strength. A `Reinforce=no` team stops being under strength for good the moment it is started. `Reinforce` therefore governs the restart as firmly as it governs recruitment. It is the setting that admits a team to the regroup step, and every visit to that step costs the team its progress.

## The team's center

A team's **center** is the point its members gather on and are measured against. It is a reference to a cell or to one of the members rather than a fixed coordinate, so it moves when what it refers to moves.

The center is recomputed from scratch each time step 5 runs. A member is counted under **all of:**

- it is alive and out of [limbo](/glossary/#limbo);
- it is **in formation**, meaning it has reached the center once and is treated as taking part from then on, though an aircraft counts without ever having reached it;
- it has entered the playable area.

The counted positions are averaged, with a [`GuardSlower`](/keys/guardslower/) team counting a slow member's position twice.

The average is then usually discarded. Where the member nearest the team's current target could move into the averaged cell as things stand, the center becomes that member instead. The averaged cell survives as the center only where it could not, whether because the ground is impassable or because something is standing there at that moment. The center of a team on open ground is one member's position rather than the middle of the group.

That nearest member is recorded alongside the center, and nothing live measures against it. The [lagging test](#settings-and-state-without-effect) is the only thing that would, and it never runs. It has one remaining effect, in step 5: a team with no nearest member recorded recomputes its center every turn.

A team whose current line is [Follow friendlies](/mapping/missions/tmission-hound-dog/) takes its center from the nearest vehicle or infantry of an allied house other than its own. It has no center at all where there is none.

[`Waypoint`](/keys/waypoint/) gives a team a center as it is created. Step 5 runs on the team's first turn whatever else is true, so that cell is replaced before anything has measured against it. The cell that `Waypoint` names keeps four other roles: ranking recruitment candidates, the edge a retreating member heads for, the landing zone an aircraft carrying a member picks, and the cell a reinforcement group enters at. All four read that cell directly rather than through the center.

The regroup step recomputes the center itself and then reads it back without testing that one was found. A team can still hold members while the count above accepts none of them: all of them in limbo, or none of them yet inside the playable area. Such a team has no center to read, and the step faults there.

## Keeping the members together

### Bringing a member into formation

The regroup and move coordinators start each member with the same test, and so do the mission handlers that give orders to the whole team. A member that is on the map and not yet **in formation** is measured against the center. Farther away than [`Stray`](/keys/stray/) and not already heading somewhere, it is put on the Move mission and sent to the center. Within that distance, it counts as in formation from then on and is never tested again. Until it arrives, everything else the coordinators do passes over it, and it holds up a move.

A team that has not been given a target does not reach this test at all. The move coordinator does nothing whatever without one. Members recruited into a team that is still filling therefore stay exactly where they were recruited, which is why the gathering happens after the team starts rather than before. A team stopped by the regroup step is the exception: it is given its regroup building as a target, so it does walk there, bringing stragglers into formation as it goes.

### Regrouping

The regroup coordinator holds a team in place and closes it up. Each member in formation that is farther than `Stray` from the center is put on the Move mission and sent there, unless it holds an Area Guard mission with a target. A member that already has a destination is left alone, and does not hold the gathering up either. Every other member is put on Guard, unless it is already on Area Guard. The coordinator reports the team gathered only on a turn where it issued no move order at all. That report is what clears the reforming flag.

### Moving

The move coordinator walks the team toward its target. Each member in formation that is not unloading is measured against that target and ordered to it under **any of:**

- it is farther from the target than [`Stray`](/keys/stray/), which is tripled for an aircraft;
- **All of:** it is below ground level, and the next line of the Script is not itself a move;
- **All of:** it is an aircraft, it is above the ground, it is not already over the target, and the next line of the Script is not itself a move.

A member that is none of those is instead released from a move it has finished, under **all of:**

- it is on the Move mission;
- **Any of:** it has no destination left, or it is within [`CloseEnough`](/keys/closeenough/) of its destination and has stopped moving;
- it holds no target.

Release clears the destination and puts the member back into its idle behavior. Either way, a member still holding a destination keeps the move outstanding.

The team counts as having arrived once no member is outstanding and at least one was measured. A team whose members are all still unloading, or all still out of formation, never arrives. Only then is the advance flag raised, and only for a team that is under way. The same coordinator walking a stopped team to its regroup building never advances anything.

[`Aggressive`](/keys/aggressive/) exempts a member that already holds a target. It is passed over entirely: not ordered to the target, and not counted as outstanding either. The rest of the team can then arrive and move on without it. [`TransportsReturnOnUnload`](/keys/transportsreturnonunload/) stamps each member that can carry passengers with a return point at the top of this same coordinator.

## The team's two targets

A team has two targets. The **mission target** is what the current line of the Script asks for. The **current target** is what the members are actually pointed at, and it can be overridden; damage turning the team onto its attacker is what does that. Assigning a mission target normally sets both. Where the current target has already been overridden, only the mission target moves, so the team finishes the fight it is in and picks the new mission target up afterward. On the turns between advances the script step restores the current target from the mission target whenever something has cleared it. The loss of whatever a target referred to clears it as well.

Changing the mission target also puts every member that was aiming at or heading for the old one back on Guard, with its own target and destination cleared.

## The script cursor

Each team runs its own copy of the Script its TeamType names, and that copy has a cursor. The cursor is the line the team is presently on, or a position before the first line while the script has yet to start or has been stopped. A separate advance flag records that the team should move on, and it is the [team mission](/mapping/missions/) handlers that raise it.

The script step runs only when the fork selects it. Where the advance flag is set, the step clears it, steps the cursor on by one, and clears every member's recorded return point. A cursor stepped past the last line deletes the team outright, which is the ordinary end of a team that finishes its work. Otherwise the mission target is cleared and the handler for the new line is run and told this is its first pass. On every later turn the same handler runs again and is told it is not.

:::caution[An unhandled line stalls the team rather than being skipped]
The dispatch that selects a handler has no fallback: a line whose mission has no entry in it does nothing at all. Nothing raises the advance flag on that turn either, so the team sits on that line for the rest of the match. Every one of the 53 team missions has an entry in it. A handler that runs but neither raises the advance flag itself nor leaves the team on a path that will raise it comes to the same end. [Change script...](/mapping/missions/tmission-script/) is one such handler.
:::

## Answering damage

Damage to a member is handed to its team, and what the team does with it turns on whether it is under way. A team that is not under way drops its center and raises the reforming flag, wherever the damage came from. A team that is under way normally turns on a non-allied attacker instead, using the team's house to test the alliance; damage from an allied house leaves the team's target and its members' orders alone. The team raises the reforming flag only where its type sets [`Annoyance=yes`](/keys/annoyance/). That key owns the ordering that decides whether the retarget happens at all. [`Suicide=yes`](/keys/suicide/) switches the whole response off.

## Leaving the map

A team holds a mark recording that it has left the map. [Leaves map (team)](/mapping/events/tevent-leaves-map/) tests that mark together with the team being empty. Both the recalculation and the dissolution step spring the event for the team they are about to delete.

Assigning the team a mission target that is a cell outside the playable area raises the mark, and assigning it one that is a cell inside the playable area clears the mark. Three other things raise it on terms that match its name:

- a vehicle on Guard deleted for standing outside the playfield after having once entered the playable area;
- an aircraft on Retreat deleted for flying outside the playable area;
- each passenger such an aircraft was carrying.

:::danger[Every team holding infantry is marked as having left the map]
Infantry raise the mark on different terms from everything else. Whenever an active infantry that is not inside a tunnel takes its turn, it sets the mark on the team it belongs to, from the moment it has entered the playable area. The check makes no test of where the infantry is standing and none of what it is doing. Object turns run after team turns, so the one path that clears the mark is overwritten inside the same frame. Any team with an infantry member is therefore marked permanently, and the trigger event springs for it the moment it empties. That includes a team wiped out in the middle of the map.
:::

## What belonging to a team changes about a member

A member is not simply an object under new management: several settings reach it through the team and stop reaching it the moment it leaves. The list gathers them, pointing at the page that owns each.

- Its orders are reissued every turn by whichever coordinator the fork selected, overriding whatever it was doing before.
- Its group number is overwritten with the team's, and [its autocreate-recruitable state](/systems/ai-team-production/#recruitment) with the TeamType's, as it joins.
- A team of strictly higher [`Priority`](/keys/priority/#scope-teamtype) can take it, and it is stripped from the team outright when the team's priority falls below the [base defense threshold](/systems/base-attacked/#teams-are-emptied-first).
- [`IonImmune=yes`](/keys/ionimmune/) spares it ion storm lightning and warhead damage for as long as it is a member.
- [`Suicide=yes`](/keys/suicide/) stops it retaliating and stops it scanning for targets as it moves.
- [`AvoidThreats=yes`](/keys/avoidthreats/) overrides its own [threat avoidance](/systems/base-attacked/#what-reads-the-map).
- Beginning a path search toward a temporarily blocked destination, it measures the distance against [`Stray`](/keys/stray/) where an object on no team measures it against [`CloseEnough`](/keys/closeenough/).
- [`Loadable`](/keys/loadable/) decides whether the player may order infantry aboard it.
- It answers a base defense call-up only while its team is a [base defense team](/keys/isbasedefense/#scope-teamtype).

What the member picks for itself is not narrowed by any of this. [`OnlyTargetHouseEnemy=yes`](/keys/onlytargethouseenemy/) restricts the target the team's script asks the engine to find, and nothing else.

## Settings and state without effect

[`GuardSlower`](/keys/guardslower/) has a lagging-formation mechanic alongside its weighting, and that mechanic never runs. The move coordinator asks whether the team has members lagging behind before it does anything else. The routine it asks returns immediately unless a flag is already set saying that it has. The only place that flag is ever assigned is the end of that same routine, past the return. It starts clear, and nothing else in the engine writes it. The ordering that would hold the fast members in place while the slow ones caught up is unreachable.

Four further pieces of team state are maintained and reach no decision. A regrouping flag is written by the damage response and cleared by the regroup coordinator, and nothing reads it, not even the multiplayer checksum. A `GuardSlower` team's record of whether it is above its under-strength threshold is written and never read at all. A running total of the members' threat values, and a second copy of the roster-changed flag kept beside the first, are read by the checksum and by nothing else.
