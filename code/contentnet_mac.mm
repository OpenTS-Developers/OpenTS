/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "contentnet.h"

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

#include <cstdint>
#include <cstdio>
#include <cstring>


static dispatch_time_t Content_Deadline(void)
{
	return(dispatch_time(DISPATCH_TIME_NOW, (int64_t)60 * NSEC_PER_SEC));
}


@interface ContentHttpDelegate : NSObject<NSURLSessionDataDelegate>
{
@public
	NSMutableData * body;
	dispatch_semaphore_t ready;
	dispatch_semaphore_t dataReady;
	NSURLSessionDataTask * task;
	NSInteger status;
	long long total;
	long long rangeStart;
	BOOL headers;
	BOOL done;
	BOOL suspended;
	NSError * failure;
}

- (void)append:(NSData *)data;
- (NSData *)take:(int)capacity;
@end


@implementation ContentHttpDelegate

- (id)init
{
	self = [super init];
	if (self != nil) {
		body = [[NSMutableData alloc] init];
		ready = dispatch_semaphore_create(0);
		dataReady = dispatch_semaphore_create(0);
		status = 0;
		total = -1;
		rangeStart = -1;
		headers = NO;
		done = NO;
		suspended = NO;
	}
	return(self);
}


- (void)dealloc
{
	[body release];
	[failure release];
	[task release];
	dispatch_release(ready);
	dispatch_release(dataReady);
	[super dealloc];
}


- (void)append:(NSData *)data
{
	@synchronized (self) {
		[body appendData:data];
		if (body.length > 1024 * 1024 && task != nil && !suspended) {
			suspended = YES;
			[task suspend];
		}
	}
	dispatch_semaphore_signal(dataReady);
}


- (NSData *)take:(int)capacity
{
	@synchronized (self) {
		if (body.length == 0 || capacity <= 0) {
			return(nil);
		}
		NSUInteger count = body.length;
		if (count > (NSUInteger)capacity) {
			count = (NSUInteger)capacity;
		}
		NSData * taken = [body subdataWithRange:NSMakeRange(0, count)];
		[body replaceBytesInRange:NSMakeRange(0, count) withBytes:NULL length:0];
		if (suspended && body.length < 256 * 1024 && task != nil) {
			suspended = NO;
			[task resume];
		}
		return(taken);
	}
}


- (void)URLSession:(NSURLSession *)session dataTask:(NSURLSessionDataTask *)dataTask didReceiveResponse:(NSURLResponse *)response completionHandler:(void (^)(NSURLSessionResponseDisposition disposition))completionHandler
{
	(void)session;
	(void)dataTask;
	NSHTTPURLResponse * http = (NSHTTPURLResponse *)response;
	status = http.statusCode;
	NSString * range = http.allHeaderFields[@"Content-Range"];
	if ([range isKindOfClass:[NSString class]]) {
		NSRange slash = [range rangeOfString:@"/"];
		NSRange bytes = [range rangeOfString:@"bytes "];
		if (bytes.location != NSNotFound) {
			rangeStart = [[range substringFromIndex:bytes.location + bytes.length] longLongValue];
		}
		if (slash.location != NSNotFound && slash.location + 1 < range.length) {
			total = [[range substringFromIndex:slash.location + 1] longLongValue];
		}
	} else if (response.expectedContentLength >= 0) {
		total = response.expectedContentLength;
	}
	headers = YES;
	dispatch_semaphore_signal(ready);
	completionHandler(NSURLSessionResponseAllow);
}


- (void)URLSession:(NSURLSession *)session dataTask:(NSURLSessionDataTask *)dataTask didReceiveData:(NSData *)data
{
	(void)session;
	(void)dataTask;
	[self append:data];
}


- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)sessionTask didCompleteWithError:(NSError *)error
{
	(void)session;
	(void)sessionTask;
	if (error != nil) {
		failure = [error retain];
	}
	done = YES;
	dispatch_semaphore_signal(ready);
	dispatch_semaphore_signal(dataReady);
}

@end


struct ContentHttpGet::State
{
	ContentHttpDelegate * Delegate = nullptr;
	NSURLSession * Session = nullptr;
	std::uint64_t TotalBytes = 0;
	std::uint64_t Offset = 0;
};


ContentHttpGet::ContentHttpGet(void) :
	Data(std::make_unique<State>())
{
}


ContentHttpGet::~ContentHttpGet(void)
{
	Close();
}


bool ContentHttpGet::Open(char const * url, char const * user_agent, char const * key, std::uint64_t offset, std::string & error)
{
	Close();
	if (url == nullptr || url[0] == '\0') {
		error = "The download address is empty.";
		return(false);
	}

	@autoreleasepool {
		Data->Offset = offset;
		Data->Delegate = [[ContentHttpDelegate alloc] init];
		NSURLSessionConfiguration * configuration = [NSURLSessionConfiguration defaultSessionConfiguration];
		configuration.timeoutIntervalForRequest = 60;
		Data->Session = [[NSURLSession sessionWithConfiguration:configuration delegate:Data->Delegate delegateQueue:nil] retain];

		NSString * address = [NSString stringWithUTF8String:url];
		NSMutableURLRequest * request = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:address]];
		NSString * agent = [NSString stringWithUTF8String:(user_agent != nullptr ? user_agent : "siliconsun")];
		[request setValue:agent forHTTPHeaderField:@"User-Agent"];
		if (key != nullptr && key[0] != '\0') {
			[request setValue:[NSString stringWithUTF8String:key] forHTTPHeaderField:@"X-Siliconsun-Key"];
		}
		if (offset > 0) {
			NSString * range = [NSString stringWithFormat:@"bytes=%llu-", (unsigned long long)offset];
			[request setValue:range forHTTPHeaderField:@"Range"];
		}

		NSURLSessionDataTask * task = [Data->Session dataTaskWithRequest:request];
		Data->Delegate->task = [task retain];
		[task resume];

		if (dispatch_semaphore_wait(Data->Delegate->ready, Content_Deadline()) != 0) {
			error = "The download connection timed out.";
			Close();
			return(false);
		}
		if (Data->Delegate->failure != nil && !Data->Delegate->headers) {
			error = "The download connection failed.";
			Close();
			return(false);
		}

		NSInteger status = Data->Delegate->status;
		if (status == 429) {
			error = "The download server asked to wait. Try again shortly.";
			Close();
			return(false);
		}
		if (status == 416) {
			error = "The download could not be resumed.";
			Close();
			return(false);
		}
		if (status != 200 && status != 206) {
			char buffer[64];
			std::snprintf(buffer, sizeof(buffer), "The download server returned %ld.", (long)status);
			error = buffer;
			Close();
			return(false);
		}
		if (status == 200 && offset > 0) {
			error = "The download server did not resume.";
			Close();
			return(false);
		}
		if (status == 206 && Data->Delegate->rangeStart != (long long)offset) {
			error = "The download server resumed at the wrong position.";
			Close();
			return(false);
		}
		if (Data->Delegate->total < 0) {
			error = "The download server did not say how large the file is.";
			Close();
			return(false);
		}
		Data->TotalBytes = (std::uint64_t)Data->Delegate->total;
		return(true);
	}
}


int ContentHttpGet::Read(void * buffer, int capacity, std::string & error)
{
	if (Data->Delegate == nullptr || buffer == nullptr || capacity <= 0) {
		error = "The download connection failed.";
		return(-1);
	}
	for (;;) {
		NSData * taken = [Data->Delegate take:capacity];
		if (taken != nil) {
			std::memcpy(buffer, taken.bytes, taken.length);
			return((int)taken.length);
		}
		if (Data->Delegate->done) {
			if (Data->Delegate->failure != nil) {
				error = "The download connection failed.";
				return(-1);
			}
			return(0);
		}
		if (dispatch_semaphore_wait(Data->Delegate->dataReady, Content_Deadline()) != 0) {
			error = "The download connection timed out.";
			return(-1);
		}
	}
}


void ContentHttpGet::Close(void)
{
	if (Data == nullptr) {
		return;
	}
	if (Data->Delegate != nullptr && Data->Delegate->task != nullptr) {
		[Data->Delegate->task cancel];
		[Data->Delegate->task release];
		Data->Delegate->task = nil;
	}
	if (Data->Session != nullptr) {
		[Data->Session invalidateAndCancel];
		[Data->Session release];
		Data->Session = nullptr;
	}
	if (Data->Delegate != nullptr) {
		[Data->Delegate release];
		Data->Delegate = nullptr;
	}
}


std::uint64_t ContentHttpGet::Total(void) const
{
	return(Data->TotalBytes);
}


bool Content_Pick_Folder(std::string & path)
{
	@autoreleasepool {
		NSOpenPanel * panel = [NSOpenPanel openPanel];
		panel.canChooseFiles = NO;
		panel.canChooseDirectories = YES;
		panel.allowsMultipleSelection = NO;
		panel.prompt = @"Choose";
		panel.message = @"Choose the folder that holds the Tiberian Sun files.";
		if ([panel runModal] != NSModalResponseOK || panel.URL.path == nil) {
			return(false);
		}
		path = panel.URL.path.UTF8String;
		return(!path.empty());
	}
}
