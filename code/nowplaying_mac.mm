/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#import <Foundation/Foundation.h>
#import <MediaPlayer/MediaPlayer.h>

#include "platform.h"

#include <cstring>
#include <dlfcn.h>


bool Now_Playing_Key(int virtualkey, bool repeat);


namespace
{

// shortcut: MediaRemote is private, drop the call if MediaPlayer receives the keys alone.
using SetCanBeNowPlayingApplication = void (*)(unsigned char);

SetCanBeNowPlayingApplication Claim;


void Claim_Now_Playing(bool enabled)
{
	if (Claim == NULL) {
		void * framework = dlopen("/System/Library/PrivateFrameworks/MediaRemote.framework/MediaRemote", RTLD_LAZY);
		if (framework != NULL) {
			Claim = (SetCanBeNowPlayingApplication)dlsym(framework, "MRMediaRemoteSetCanBeNowPlayingApplication");
		}
		if (Claim == NULL) {
			Claim = (SetCanBeNowPlayingApplication)1;
		}
	}
	if (Claim == (SetCanBeNowPlayingApplication)1) {
		return;
	}
	Claim(enabled ? 1 : 0);
}


MPRemoteCommandHandlerStatus Skip(int virtualkey)
{
	dispatch_async(dispatch_get_main_queue(), ^{
		Now_Playing_Key(virtualkey, false);
	});
	return(MPRemoteCommandHandlerStatusSuccess);
}

}


void Now_Playing_System_Keys(bool enabled, char const * title)
{
	static bool installed;
	static bool active;
	static char shown[64];

	if (!installed) {
		MPRemoteCommandCenter * center = [MPRemoteCommandCenter sharedCommandCenter];
		[center.nextTrackCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent *) {
			return(Skip(VK_MEDIA_NEXT_TRACK));
		}];
		[center.previousTrackCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent *) {
			return(Skip(VK_MEDIA_PREV_TRACK));
		}];
		installed = true;
	}

	char const * name = (title != NULL) ? title : "";
	if (enabled == active && std::strncmp(shown, name, sizeof(shown)) == 0) {
		return;
	}
	active = enabled;
	std::strncpy(shown, name, sizeof(shown) - 1);
	shown[sizeof(shown) - 1] = '\0';

	Claim_Now_Playing(enabled);

	MPRemoteCommandCenter * center = [MPRemoteCommandCenter sharedCommandCenter];
	center.nextTrackCommand.enabled = enabled;
	center.previousTrackCommand.enabled = enabled;

	MPNowPlayingInfoCenter * info = [MPNowPlayingInfoCenter defaultCenter];
	if (!enabled) {
		info.playbackState = MPNowPlayingPlaybackStateStopped;
		info.nowPlayingInfo = nil;
		return;
	}

	NSString * label = shown[0] != '\0' ? [NSString stringWithUTF8String:shown] : @"Tiberian Sun";
	info.playbackState = MPNowPlayingPlaybackStatePlaying;
	info.nowPlayingInfo = @{
		MPMediaItemPropertyTitle: label,
		MPNowPlayingInfoPropertyMediaType: @(MPNowPlayingInfoMediaTypeAudio),
		MPNowPlayingInfoPropertyPlaybackRate: @1.0,
	};
}
