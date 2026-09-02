/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "data.h"
#include "language/language.h"
#include "ownrdraw.h"
#include "wdtnet.h"


using namespace WorldDominationTour;


/// <summary>
/// Destroys the campaign property block.
/// The campaign state is only referenced by this block, so it outlives the destruction.
/// </summary>
CampaignProperties::~CampaignProperties(void)
{
	/// nothing
}


/// <summary>
/// Creates a campaign property block over a tour state.
/// This routine is used to give the World Domination Tour screens a safe view of the state
/// received from the ladder server. The state is merely referenced, never owned.
/// </summary>
/// <param name="state">The campaign state to describe, or NULL if none arrived.</param>
CampaignProperties::CampaignProperties(WDTState * state) :
	TourState(state)
{
	/// nothing
}


/// <summary>
/// Creates a copy of a campaign property block.
/// The campaign state is shared rather than duplicated, so the copy describes the very same
/// campaign as the original does.
/// </summary>
CampaignProperties::CampaignProperties(CampaignProperties const & that) :
	TourState(that.TourState)
{
	/// nothing
}


/// <summary>
/// Assigns one campaign property block to another.
/// The campaign state is shared rather than duplicated, so both blocks go on describing the
/// same campaign.
/// </summary>
/// <returns>Returns with a reference to this property block.</returns>
CampaignProperties & CampaignProperties::operator=(CampaignProperties const & that)
{
	TourState = that.TourState;
	return(*this);
}


/// <summary>
/// Do these two property blocks describe the same campaign?
/// </summary>
/// <returns>bool; Do both blocks refer to the same campaign state?</returns>
bool CampaignProperties::operator==(CampaignProperties const & that) const
{
	return(TourState == that.TourState);
}


/// <summary>
/// Fetches the identifier of the tour cycle.
/// </summary>
/// <returns>Returns with the cycle identifier, or zero when there is no campaign state to
/// ask.</returns>
int CampaignProperties::Get_Cycle_ID(void) const
{
	if (TourState != NULL) {
		return(TourState->CycleID);
	}
	return(0);
}


/// <summary>
/// Fetches the screen layout the tour is presented with.
/// The tour only ever shipped the one layout, so every campaign is answered alike.
/// </summary>
/// <returns>Returns with the layout number to present.</returns>
int CampaignProperties::Get_Layout(void) const
{
	return(1);
}


/// <summary>
/// Fetches the identifier of the tour map in play.
/// </summary>
/// <returns>Returns with the map identifier, or the first map when there is no campaign
/// state to ask.</returns>
int CampaignProperties::Get_Map_ID(void) const
{
	if (TourState != NULL) {
		return(TourState->MapID);
	}
	return(1);
}


/// <summary>
/// Fetches the tick the tour cycle has reached.
/// </summary>
/// <returns>Returns with the index of the most recent tick, or zero when there is no
/// campaign state to ask.</returns>
int CampaignProperties::Get_Current_Tick(void) const
{
	if (TourState != NULL) {
		return(TourState->NumTicks - 1);
	}
	return(0);
}


/// <summary>
/// Fetches the short description of the tour cycle.
/// </summary>
/// <returns>Returns with the description the server supplied, or the missing information
/// message when there is no campaign state to ask.</returns>
const char * CampaignProperties::Get_Short_Desc(void) const
{
	if (TourState != NULL) {
		return(TourState->ShortDesc);
	}
	return(Fetch_String(TXT_WDT_INVALID_MISSING));
}


/// <summary>
/// Fetches the long description of the tour cycle.
/// </summary>
/// <returns>Returns with the description the server supplied, or the corrupt information
/// message when there is no campaign state to ask.</returns>
const char * CampaignProperties::Get_Long_Desc(void) const
{
	if (TourState != NULL) {
		return(TourState->LongDesc);
	}
	return(Fetch_String(TXT_WDT_INFO_CORRUPT));
}


/// <summary>
/// Fetches the web address advertised for the tour cycle.
/// </summary>
/// <returns>Returns with the address the server supplied, or an empty string when there is
/// no campaign state to ask.</returns>
const char * CampaignProperties::Get_Web_URL(void) const
{
	if (TourState != NULL) {
		return(TourState->WebURL);
	}
	return("");
}
