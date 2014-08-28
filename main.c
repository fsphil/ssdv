
/* SSDV - Slow Scan Digital Video                                        */
/*=======================================================================*/
/* Copyright 2011 Philip Heron <phil@sanslogic.co.uk                     */
/*                                                                       */
/* This program is free software: you can redistribute it and/or modify  */
/* it under the terms of the GNU General Public License as published by  */
/* the Free Software Foundation, either version 3 of the License, or     */
/* (at your option) any later version.                                   */
/*                                                                       */
/* This program is distributed in the hope that it will be useful,       */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of        */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         */
/* GNU General Public License for more details.                          */
/*                                                                       */
/* You should have received a copy of the GNU General Public License     */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "ssdv.h"

void exit_usage()
{
	fprintf(stderr,
		"Usage: ssdv [-e|-d] [-n] [-t <percentage>] [-c <callsign>] [-i <id>] [<in file>] [<out file>]\n"
		"\n"
		"  -e Encode JPEG to SSDV packets.\n"
		"  -d Decode SSDV packets to JPEG.\n"
		"\n"
		"  -n Encode or decode packets with no FEC.\n"
		"  -t For testing, drops the specified percentage of packets while decoding.\n"
		"  -c Set the callign. Accepts A-Z 0-9 and space, up to 6 characters.\n"
		"  -i Set the image ID (0-255).\n"
		"\n");
	exit(-1);
}

int main(int argc, char *argv[])
{
	int c, i;
	FILE *fin = stdin;
	FILE *fout = stdout;
	char encode = -1;
	char type = SSDV_TYPE_NORMAL;
	int droptest = 0;
	char callsign[7];
	uint8_t image_id = 0;
	ssdv_t ssdv;
	
	uint8_t pkt[SSDV_PKT_SIZE], b[128], *jpeg;
	size_t jpeg_length;
	
	callsign[0] = '\0';
	
	opterr = 0;
	while((c = getopt(argc, argv, "ednc:i:t:")) != -1)
	{
		switch(c)
		{
		case 'e': encode = 1; break;
		case 'd': encode = 0; break;
		case 'n': type = SSDV_TYPE_NOFEC; break;
		case 'c':
			if(strlen(optarg) > 6)
				fprintf(stderr, "Warning: callsign is longer than 6 characters.\n");
			strncpy(callsign, optarg, 7);
			break;
		case 'i': image_id = atoi(optarg); break;
		case 't': droptest = atoi(optarg); break;
		case '?': exit_usage();
		}
	}
	
	c = argc - optind;
	if(c > 2) exit_usage();
	
	for(i = 0; i < c; i++)
	{
		if(!strcmp(argv[optind + i], "-")) continue;
		
		switch(i)
		{
		case 0:
			fin = fopen(argv[optind + i], "rb");
			if(!fin)
			{
				fprintf(stderr, "Error opening '%s' for input:\n", argv[optind + i]);
				perror("fopen");
				return(-1);
			}
			break;
		
		case 1:
			fout = fopen(argv[optind + i], "wb");
			if(!fout)         
			{                 
				fprintf(stderr, "Error opening '%s' for output:\n", argv[optind + i]);
				perror("fopen");        
				return(-1);
			}
			break;
		}
	}
	
	switch(encode)
	{
	case 0: /* Decode */
		if(droptest > 0) fprintf(stderr, "*** NOTE: Drop test enabled: %i ***\n", droptest);
		
		ssdv_dec_init(&ssdv, type);
		
		jpeg_length = 1024 * 1024 * 4;
		jpeg = malloc(jpeg_length);
		ssdv_dec_set_buffer(&ssdv, jpeg, jpeg_length);
		
		i = 0;
		while(fread(pkt, 1, SSDV_PKT_SIZE, fin) > 0)
		{
			/* Drop % of packets */
			if(droptest && (rand() / (RAND_MAX / 100) < droptest)) continue;
			
			/* Test the packet is valid */
			if(ssdv_dec_is_packet(pkt, NULL, type) != 0) continue;
			
			/* Feed it to the decoder */
			ssdv_dec_feed(&ssdv, pkt);
			i++;
		}
		
		ssdv_dec_get_jpeg(&ssdv, &jpeg, &jpeg_length);
		fwrite(jpeg, 1, jpeg_length, fout);
		free(jpeg);
		
		fprintf(stderr, "Read %i packets\n", i);
		
		break;
	
	case 1: /* Encode */
		ssdv_enc_init(&ssdv, type, callsign, image_id);
		ssdv_enc_set_buffer(&ssdv, pkt);
		
		i = 0;
		
		while(1)
		{
			while((c = ssdv_enc_get_packet(&ssdv)) == SSDV_FEED_ME)
			{
				size_t r = fread(b, 1, 128, fin);
				
				if(r <= 0)
				{
					fprintf(stderr, "Premature end of file\n");
					break;
				}
				ssdv_enc_feed(&ssdv, b, r);
			}
			
			if(c == SSDV_EOI)
			{
				fprintf(stderr, "ssdv_enc_get_packet said EOI\n");
				break;
			}
			else if(c != SSDV_OK)
			{
				fprintf(stderr, "ssdv_enc_get_packet failed: %i\n", c);
				return(-1);
			}
			
			fwrite(pkt, 1, SSDV_PKT_SIZE, fout);
			i++;
		}
		
		fprintf(stderr, "Wrote %i packets\n", i);
		
		break;
	
	default:
		fprintf(stderr, "No mode specified.\n");
		break;
	}
	
	if(fin != stdin) fclose(fin);
	if(fout != stdout) fclose(fout);
	
	return(0);
}

