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

/*************************************************************************************
 **   C O N F I D E N T I A L --- W E S T W O O D    S T U D I O S                  **
 *************************************************************************************
 *                                                                                   *
 *                 Project Name : Command & Conquer - Red Alert                      *
 *                                                                                   *
 *                    File Name : SENDFILE.CPP                                       *
 *                                                                                   *
 *                   Programmer : Steve Tall                                         *
 *                                                                                   *
 *                   Start Date : Audust 20th, 1996                                  *
 *                                                                                   *
 *                  Last Update : August 20th, 1996 [ST]                             *
 *                                                                                   *
 *-----------------------------------------------------------------------------------*
 * Overview:                                                                         *
 *                                                                                   *
 *  Functions for scenario file transfer between machines                            *
 *                                                                                   *
 *-----------------------------------------------------------------------------------*
 * Functions:                                                                        *
 *                                                                                   *
 *                                                                                   *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "conquer.h"
#include "dbgprint.h"
#include "gamedirs.h"
#include "globals.h"
#include "ini.h"
#include "ipxmgr.h"
#include "msgloop.h"
#include "progress.h"
#include "session.h"
#include "stimer.h"

#include <algorithm>

bool Receive_Remote_File ( char *file_name, unsigned int file_length, bool show_progress);
bool Send_Remote_File ( char const *file_name );

#define RESPONSE_TIMEOUT	TIMER_MINUTE


/***********************************************************************************************
 * Get_Scenario_File_From_Host -- Initiates download of scenario file from game host           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    ptr to buffer to copy file name into                                              *
 *                                                                                             *
 * OUTPUT:   true if file sucessfully downloaded                                               *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    8/22/96 3:06PM ST : Created                                                              *
 *=============================================================================================*/
bool Get_File_From_Host(char *return_name, bool show_progress)
{
	//DebugString ("RA95 - In Get_Scenario_From_Host\n");

	unsigned int file_length = 0;

	GlobalPacketType	net_send_packet;
	GlobalPacketType	net_receive_packet;
	unsigned short		product_id;

	IPXAddressClass	sender_address;

	DebugString("Getting file from host\n");

	CDTimerClass<SystemTimerClass> response_timer;		// timeout timer for waiting for responses

	DebugString("Requesting file download\n");

	response_timer = RESPONSE_TIMEOUT;

	/*
	**	Send the scenario request using guaranteed delivery.
	*/
	memset ((void*)&net_send_packet, 0, sizeof (net_send_packet));
	net_send_packet.Command = NET_REQ_SCENARIO;
	Ipx.Send_Global_Message (&net_send_packet, sizeof (net_send_packet),
		1, &(Session.HostAddress) );
	while (Ipx.Global_Num_Send() > 0 && response_timer) {
		Call_Back();
	}

	//DebugString ("RA95 - Waiting for response from host\n");

	if (!response_timer) {
		DebugString("Timeout waiting for REQ_SCENARIO packet send\n");
		return(false);
	}

	/*
	**	Wait for host to respond with a file info packet
	*/
	response_timer = RESPONSE_TIMEOUT;
	do {
		Call_Back();
		int receive_packet_length = sizeof (net_receive_packet);
		if (Ipx.Get_Global_Message (&net_receive_packet, &receive_packet_length,
			&sender_address, &product_id)){

			//DebugString ("RA95 - Got packet from host\n");
			if (net_receive_packet.Command == NET_FILE_INFO && sender_address == Session.HostAddress) {
				strcpy (return_name, net_receive_packet.ScenarioInfo.ShortFileName);
				file_length = net_receive_packet.ScenarioInfo.FileLength;
				DebugString("Host responded with file info\n");
				DebugString("File name is %s\n", return_name);
				break;
			}
		}
	} while ( response_timer );


	/*
	**	If we timed out then something horrible has happened to the other player so just
	**	return failure.
	*/
	if (!response_timer) {
		DebugString("Timeout waiting for host to respond to file info request\n");
		return(false);
	}

	DebugString("Sending file info received ack\n");

	response_timer = RESPONSE_TIMEOUT;
	memset ((void*)&net_send_packet, 0, sizeof (net_send_packet));
	net_send_packet.Command = NET_FILE_INFO_ACK;
	Ipx.Send_Global_Message (&net_send_packet, sizeof (net_send_packet),
		1, &(Session.HostAddress) );
	while (Ipx.Global_Num_Send() > 0 && response_timer) {
		Call_Back();
	}

	if (!response_timer) {
		DebugString("Timeout waiting for FILE_INFO_ACK send\n");
		return(false);
	}

//	DebugString( "about to download '%s'\n", return_name );

	/*
	**	Receive the file from the host
	*/
	bool res = Receive_Remote_File ( return_name, file_length, show_progress);
	DebugString("Receive_Remote_File returned %s\n", res != 0 ? "true" : "false");
	return(res);
}


/***********************************************************************************************
 * Receive_Remote_File -- Handles incoming file download packets from the game host            *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    file name to save as                                                              *
 *           length of file to expect                                                          *
 *                                                                                             *
 * OUTPUT:   true if file downloaded was completed                                             *
 *                                                                                             *
 * WARNINGS: This fuction can modify the file name passed in                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/22/96 3:07PM ST : Created                                                               *
 *=============================================================================================*/
bool Receive_Remote_File ( char *file_name, unsigned int file_length, bool show_progress)
{
	CDTimerClass<SystemTimerClass> response_timer;		// timeout timer for waiting for responses
	unsigned short		product_id;
	IPXAddressClass	sender_address;
	bool			return_code = 0;
	int 				received_count; /// number of blocks received so far
	int 				progress;      /// running progress accumulator (100 per block)

	RemoteFileTransferType * receive_packet = new RemoteFileTransferType;

	DebugString("Receiving download of file %s\n", (const char *)file_name);

	std::string const save_path = User_File_Write_Name(file_name);
	RawFileClass save_file (save_path.c_str());

	/*
	**	If the file already exists then delete it and re-create it.
	*/
	if (save_file.Is_Available()) save_file.Delete();

	/*
	**	Open the file for write
	*/
	save_file.Open ( FileClass::WRITE );

	/*
	 * Work out how many blocks make up the whole file and allocate a flag array so we can
	 * keep track of which blocks have already arrived (blocks may arrive out of order).
	 */
	int total_blocks = file_length / (MAX_SEND_FILE_PACKET_SIZE);
	if (file_length % (MAX_SEND_FILE_PACKET_SIZE)) {
		total_blocks++;
	}

	char *block_received = new char [total_blocks];
	memset (block_received, 0, total_blocks);
	received_count = 0;

	/*
	 * Allocate a buffer big enough to hold the whole file. Blocks are reassembled into this
	 * buffer and the entire buffer is written to disk once the transfer is complete.
	 */
	char *file_buffer = new char [file_length];

	if (show_progress) {
		Progress.Initialize(100, 1, true);		// Max is 100%
		Progress.Set_Graphic_Data("PROGBAR2.SHP");
		Progress.Display_Progress();
		Progress.Set_Progress_Percent(0, 0);			// Current is 0%
	}

	/*
	**	Wait for all the blocks to arrive
	*/
	response_timer = RESPONSE_TIMEOUT;
	progress = 0;
	while ( true ) {

		Windows_Message_Handler();
		Call_Back();

		int receive_packet_length = sizeof (RemoteFileTransferType);
		if (Ipx.Get_Global_Message (receive_packet, &receive_packet_length,
			&sender_address, &product_id)) {

			if (receive_packet->Command == NET_FILE_CHUNK && sender_address == Session.HostAddress){

				char *flag = &block_received[receive_packet->BlockNumber];
				if (!block_received[receive_packet->BlockNumber]) {
					*flag = true;
					received_count++;
					progress += 100;
					response_timer = RESPONSE_TIMEOUT/2;
					DebugString("Received file chunk %d\n", receive_packet->BlockNumber);
					memcpy (file_buffer + (MAX_SEND_FILE_PACKET_SIZE) * receive_packet->BlockNumber,
						receive_packet->RawData, receive_packet->BlockLength);

					if (show_progress){
						Progress.Set_Progress_Percent(0, progress / total_blocks);
					}

					if (received_count == total_blocks) {
						return_code = true;
						if (show_progress){
							Progress.Set_Progress_Percent(0, 100);
						}
						break;
					}
				}
			}
		}
		if (!response_timer) {
			break;
		}
	}

	/*
	 * The transfer is finished (or timed out). Write the whole reassembled buffer out to the
	 * file in one go and free up the working buffers.
	 */
	save_file.Write ( file_buffer, file_length );
	delete [] file_buffer;
	delete [] block_received;

	DebugString("File download completed OK\n");

	save_file.Close();
	DebugString("Download file closed\n");

	if (show_progress) {
		Progress.End_Dialog();
	}

	delete receive_packet;

	return(return_code);
}


/***********************************************************************************************
 * Send_Remote_File -- Sends a file to game clients                                            *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    File name                                                                         *
 *                                                                                             *
 * OUTPUT:   true if file transfer was successfully completed                                  *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    8/22/96 3:09PM ST : Created                                                              *
 *=============================================================================================*/
bool Send_Remote_File ( char const *file_name, bool send_to_all, bool show_progress )
{
	CDTimerClass<SystemTimerClass> response_timer;		// timeout timer for waiting for responses
	bool			return_code = 0;

	DebugString("Send file %s to clients\n", file_name);

	int			update_time = 0;


	int file_length;
	int block_number;
	int max_chunk_size;
	int total_blocks;
	int bytes_left;

	unsigned short product_id;

	void *read_ptr;

	/// Sized to one header plus one MAX_SEND_FILE_PACKET_SIZE chunk. The full
	/// RemoteFileTransferType would overrun a packet, so the buffer stays raw
	/// and is written through a reference.
	char						send_packet[200];
	GlobalPacketType			net_file_info;

	GlobalPacketType			net_receive_packet;

	CCFileClass send_file (file_name);

	if ( !send_file.Is_Available() ){
		//DebugString ("RA95 - Error - could not find file to send to client\n");
//		DebugString("RA95 - Error - could not find file to send to client\n");
		return(false);
	}
	file_length = send_file.Size();

	response_timer = RESPONSE_TIMEOUT;

	DebugString("Sending file info to clients\n");

	/*
	**	Send the file info to the remote machine(s)
	*/
	net_file_info.Command = NET_FILE_INFO;
	strcpy (net_file_info.ScenarioInfo.ShortFileName, file_name);
//		DebugString( "Uploading '%s'\n", file_name );
//		DebugString( "ShortFileName is '%s'\n", net_file_info.ScenarioInfo.ShortFileName );
	net_file_info.ScenarioInfo.FileLength = file_length;

	if (send_to_all) {
		for (int i = 1; i < Session.Players.Count(); i++) {
			do {
				Call_Back();
			} while (Ipx.Send_Global_Message (&net_file_info, sizeof (GlobalPacketType),
				1, &(Session.Players[i]->Address)) == 0 && response_timer);
		}
	} else {
		for (int i = 0; i < Session.RequestCount; i++) {
			do {
				Call_Back();
				DebugString("Sending file info packet to player %s\n", Session.Players[Session.ScenarioRequests[i]]->Name);
			} while (Ipx.Send_Global_Message (&net_file_info, sizeof (GlobalPacketType),
				1, &(Session.Players[Session.ScenarioRequests[i]]->Address)) == 0 && response_timer);
		}
	}

	while (Ipx.Global_Num_Send() > 0 && response_timer) {
		Call_Back();
	}

	if (response_timer == 0) {
		DebugString("File info send timed out\n");
		return(false);
	}

	IPXAddressClass	sender_address;

	int count;
	int acks = 0;
	if (send_to_all) {
		count = Session.Players.Count() - 1;
	} else {
		count = Session.RequestCount;
	}

	response_timer = RESPONSE_TIMEOUT;

	int net_packetlen;
	do {
		Call_Back();
		net_packetlen = sizeof (net_receive_packet);
		if (Ipx.Get_Global_Message (&net_receive_packet, &net_packetlen, &sender_address, &product_id)) {
			if (net_receive_packet.Command == NET_FILE_INFO_ACK) {
				acks++;
			}
		}
	} while (response_timer != 0 && acks < count);

	max_chunk_size = MAX_SEND_FILE_PACKET_SIZE;
	total_blocks = (file_length + max_chunk_size-1) / max_chunk_size;
	bytes_left = file_length;

	send_file.Open ( FileClass::READ );

	block_number = 0;

	if (show_progress) {
		Progress.Initialize(100, 1, true);		// Max is 100%
		Progress.Set_Graphic_Data("PROGBAR2.SHP");
		Progress.Display_Progress();
		Progress.Set_Progress_Percent(0, 0);			// Current is 0%
	}

	response_timer = RESPONSE_TIMEOUT;
	while ( response_timer ){

		Windows_Message_Handler();
		Call_Back();
		if (block_number < total_blocks){

			if ( Ipx.Global_Num_Send() < 10 ){

				((RemoteFileTransferType &)send_packet).Command = NET_FILE_CHUNK;
				((RemoteFileTransferType &)send_packet).BlockNumber = block_number;
				((RemoteFileTransferType &)send_packet).BlockLength = std::min(file_length, max_chunk_size);

				file_length -= ((RemoteFileTransferType &)send_packet).BlockLength;

				read_ptr = &((RemoteFileTransferType &)send_packet).RawData[0];

				if (send_file.Read (read_ptr , ((RemoteFileTransferType &)send_packet).BlockLength) == ((RemoteFileTransferType &)send_packet).BlockLength){
					DebugString("Sending file chunk %d\n", block_number);

					if (send_to_all) {
						for (int i=1 ; i<Session.Players.Count() ; i++){
							int ret;
							do {
								ret = Ipx.Send_Global_Message (&send_packet, sizeof (send_packet),
								1, &(Session.Players[i]->Address) );
								Call_Back();
							} while (ret == 0);
						}
					} else {
						for (int i=0 ; i<Session.RequestCount ; i++){
							int ret;
							do {
								ret = Ipx.Send_Global_Message (&send_packet, sizeof (send_packet),
								1, &(Session.Players[Session.ScenarioRequests[i]]->Address) );
								Call_Back();
							} while (ret == 0);
						}
					}
				}

				block_number++;

				if (show_progress){
					Progress.Set_Progress_Percent(0, (block_number*100) / total_blocks);
				}

			}
		}else{
			if (Ipx.Global_Num_Send() == 0) {
				response_timer = TIMER_SECOND;
				while (response_timer != 0) {
					Windows_Message_Handler();
					Call_Back();
				}
			} else {
				continue;
			}

			if (Ipx.Global_Num_Send() == 0){
				return_code = true;
				if (show_progress){
					Progress.Set_Progress_Percent(0, 100);
				}
				break;
			}
		}
	}

	DebugString("File upload completed\n");

	if (show_progress) {
		Progress.End_Dialog();
	}

	return(return_code);
}


/***********************************************************************************************
 * Find_Local_Scenario -- finds the file name of the scenario with matching attributes         *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    ptr to Scenario description                                                       *
 *           ptr to Scenario filename to fix up                                                *
 *           length of file for trivial rejection of scenario files                            *
 *           ptr to digest. Digests must match.                                                *
 *                                                                                             *
 *                                                                                             *
 * OUTPUT:   true if scenario is available locally                                             *
 *                                                                                             *
 * WARNINGS: We need to reject files that don't match exactly because scenarios with the same  *
 *           description can exist on both machines but have different contents. For example   *
 *           there will be lots of scenarios called 'my map' and 'crap' and 'aaaaaa'.          *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    8/23/96 12:36PM ST : Created                                                             *
 *=============================================================================================*/
bool Find_Local_Scenario (char *filename, unsigned int length, char *digest, bool official)
{
//FILE *fp;
//fp = fopen("findscen.txt","wt");
//DebugString("looking for local scenario: description = %s, name=%s, length=%d, digest=%s, official=%d\n", description, filename, length, digest, official);

	if (strcmpi(filename, "RandMap.Sed") != 0) {
		char digest_buffer[32];
		/*
		**	Scan through the scenario list looking for scenarios with matching descriptions.
		*/
		for (int index = 0; index < Session.Scenarios.Count(); index++) {

	//DebugString( "Checking against scenario: %s\n", Session.Scenarios[index]->Description());
			if (!strcmp (Session.Scenarios[index]->Get_Filename(), filename)) {
	//DebugString("found matching description.\n");
				CCFileClass file (Session.Scenarios[index]->Get_Filename());

				/*
				**	Possible rejection on the basis of availability.
				*/
				if (file.Is_Available()) {
	//DebugString("file is available.\n");
					/*
					**	Possible rejection on the basis of size.
					*/
					if ((unsigned)file.Size() == length) {
	//DebugString("length matches.\n");
						/*
						**	We don't know the digest for 'official' scenarios so assume its correct
						*/
						//if (!official) {
	//DebugString("!official.\n");
							/*
							**	Possible rejection on the basis of digest
							*/
							INIClass ini;
							ini.Load(file);
							ini.Get_String ("Digest", "1", "No digest here mate. Nope.", digest_buffer, sizeof (digest_buffer) );
						//}
	//DebugString("digest = %s, digest_buffer = %s.\n", digest, digest_buffer);

						/*
						**	This must be the same scenario. Copy the name and return true.
						*/
						//if (official || !strcmp (digest, digest_buffer)) {
						if (digest == NULL || *digest == '\0' || !strcmp (digest, digest_buffer)) {
			//DebugString("a match!\n");
							strcpy (filename, Session.Scenarios[index]->Get_Filename());
							return(true);
						}

					}
				}
	//			else
	//				DebugString("file not available '%s'.\n", Session.Scenarios[index]->Get_Filename());

			}
		}
	}
//DebugString("failed match.\n");
	/*
	**	Couldnt find the scenario locally. Return failure.
	*/
	return(false);
}
