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

class AbstractClass;
template<class T> class DynamicVectorClass;

void Detach_This_From_All(AbstractClass const * target, bool all = true);
void Process_Deferred_Deletion(void);

/*
 * These trackers exist for speed optimization reasons.
 * For example:
 * BuildingClass and HouseClass have a pointer to FactoryClass.
 * Their constructors add them to FactoryPtrTracker.
 * If you pass a FactoryClass pointer to Detach_This_From_All,
 * it will Detach that pointer from BuildingClass and HouseClass instances
 * tracked by FactoryPtrTracker.
 */
extern DynamicVectorClass<AbstractClass *> ObjectPtrTracker;
extern DynamicVectorClass<AbstractClass *> AbstractTypePtrTracker;
extern DynamicVectorClass<AbstractClass *> TagPtrTracker;
extern DynamicVectorClass<AbstractClass *> TriggerPtrTracker;
extern DynamicVectorClass<AbstractClass *> AnimPtrTracker;
extern DynamicVectorClass<AbstractClass *> FactoryPtrTracker;
extern DynamicVectorClass<AbstractClass *> EventActionPtrTracker;
extern DynamicVectorClass<AbstractClass *> WaypointPathPtrTracker;
extern DynamicVectorClass<AbstractClass *> TeamPtrTracker;
extern DynamicVectorClass<AbstractClass *> HousePtrTracker;

/*
 * This vector holds all objects marked by ObjectClass::Delete_Me.
 */
extern DynamicVectorClass<AbstractClass *> ObjectsToDelete;
