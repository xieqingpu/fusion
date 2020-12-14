/***************************************************************************************
 *
 *  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
 *
 *  By downloading, copying, installing or using the software you agree to this license.
 *  If you do not agree to this license, do not download, install, 
 *  copy or use the software.
 *
 *  Copyright (C) 2014-2019, Happytimesoft Corporation, all rights reserved.
 *
 *  Redistribution and use in binary forms, with or without modification, are permitted.
 *
 *  Unless required by applicable law or agreed to in writing, software distributed 
 *  under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 *  CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
 *  language governing permissions and limitations under the License.
 *
****************************************************************************************/

#include "sys_inc.h"
#include "base64.h"


const char *BASE64_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64_encode_triple(uint8 triple[3], char result[4])
{
    int tripleValue, i;

    tripleValue = triple[0];
    tripleValue *= 256;
    tripleValue += triple[1];
    tripleValue *= 256;
    tripleValue += triple[2];

    for (i=0; i<4; i++)
    {
		result[3-i] = BASE64_CHARS[tripleValue % 64];
		tripleValue /= 64;
    }
}

/**
 * encode an array of bytes using Base64 (RFC 3548)
 *
 * @param source the source buffer
 * @param sourcelen the length of the source buffer
 * @param target the target buffer
 * @param targetlen the length of the target buffer
 * @return 1 on success, 0 otherwise
 */
int base64_encode(uint8 *source, uint32 sourcelen, char *target, uint32 targetlen)
{
    /* check if the result will fit in the target buffer */
    if ((sourcelen+2)/3*4 > targetlen-1)
    {
		return 0;
	}
	
    /* encode all full triples */
    while (sourcelen >= 3)
    {
		base64_encode_triple(source, target);
		sourcelen -= 3;
		source += 3;
		target += 4;
    }

    /* encode the last one or two characters */
    if (sourcelen > 0)
    {
		uint8 temp[3];
		memset(temp, 0, sizeof(temp));
		memcpy(temp, source, sourcelen);
		base64_encode_triple(temp, target);
		target[3] = '=';

		if (sourcelen == 1)
		{
		    target[2] = '=';
		}
		
		target += 4;
    }

    /* terminate the string */
    target[0] = 0;

    return 1;
}

/**
 * decode base64 encoded data
 *
 * @param source the encoded data (zero terminated)
 * @param target pointer to the target buffer
 * @param targetlen length of the target buffer
 * @return length of converted data on success, -1 otherwise
 */
int base64_decode(const char *source, uint8 *target, uint32 targetlen) 
{
    const char *cur;
    uint8 *dest, *max_dest;
    int d, dlast, phase;
    uint8 c;
    static int table[256] = {
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 00-0F */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 10-1F */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,  /* 20-2F */
        52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,  /* 30-3F */
        -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,  /* 40-4F */
        15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,  /* 50-5F */
        -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,  /* 60-6F */
        41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,  /* 70-7F */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 80-8F */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 90-9F */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* A0-AF */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* B0-BF */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* C0-CF */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* D0-DF */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* E0-EF */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1   /* F0-FF */
    };

    d = dlast = phase = 0;
    dest = target;
    max_dest = dest+targetlen;

    for (cur = source; *cur != '\0' && dest<max_dest; ++cur) 
    {
        d = table[(int)*cur];
        if (d != -1) 
        {
            switch(phase) 
            {
			case 0:
			    ++phase;
			    break;
			case 1:
			    c = (uint8)(((dlast << 2) | ((d & 0x30) >> 4)));
			    *dest++ = c;
			    ++phase;
			    break;
			case 2:
			    c = (((dlast & 0xf) << 4) | ((d & 0x3c) >> 2));
			    *dest++ = c;
			    ++phase;
			    break;
			case 3:
			    c = (uint8)((((dlast & 0x03 ) << 6) | d));
			    *dest++ = c;
			    phase = 0;
			    break;
		    }
		    
            dlast = d;
        }
    }

    /* we decoded the whole buffer */
    if (*cur == '\0') 
    {
		return (int)(dest-target);
    }

    /* we did not convert the whole data, buffer was to small */
    return -1;
}





