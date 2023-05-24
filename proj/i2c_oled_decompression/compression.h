#ifndef COMPRESSION_H
#define COMPRESSION_H

#include <string.h>
#include <stdint.h>


/*
 * maintenance enables for the compression sections below
 * only enable to work on this file!
#define COMP_PACKBITS 1
#define COMP_UZLIB 1
#define COMP_HEATSHRINK 1
*/

#if COMP_PACKBITS==1
#include "packbits/packbits.h"
#endif

#if COMP_UZLIB==1
#define UZ_OUT_CHUNK_SIZE 1
#include "uzlib/src/uzlib.h"
#endif

#if COMP_HEATSHRINK==1
#include "heatshrink_config.h"
#include "heatshrink/heatshrink_common.h"
#include "heatshrink/heatshrink_encoder.h"
#include "heatshrink/heatshrink_decoder.h"

#endif

#if LOG_LVL>0
#define LOG(x) printf(x)
#else
#define LOG(x)
#endif



static inline int compress_packbits(uint8_t* input_data, uint16_t input_length, uint8_t* output_data, uint16_t* output_length);
static inline int decompress_packbits(uint8_t* input_data, uint16_t input_length, uint8_t* output_data, uint16_t* output_length);
static inline int decompress_packbits_window(uint8_t* input_data, uint16_t input_length, uint8_t* output_data, uint16_t output_start_byte, uint16_t* output_length);

static inline int compress_heatshrink(uint8_t* input_data, uint16_t input_length, uint8_t* output_data, uint16_t* output_length);
static inline int decompress_heatshrink(uint8_t* input_data, uint16_t input_length, uint8_t* output_data, uint16_t* output_length);
static inline int decompress_heatshrink_window(uint8_t* input_data, uint16_t input_length, uint8_t* output_data, uint16_t output_start_byte, uint16_t* output_length);



#if COMP_PACKBITS==1
static inline int compress_packbits(uint8_t* input_data, uint16_t input_length, uint8_t* output_data, uint16_t* output_length) {
	*output_length = packbits(input_data, output_data, input_length, *output_length);
	return 0;
}

static inline int decompress_packbits(uint8_t* input_data, uint16_t input_length, uint8_t* output_data, uint16_t* output_length) {
	*output_length = unpackbits(input_data, output_data, input_length, *output_length);
	return 0;
}

/*
* decompress a specific window of output data
* approach:	packbits decompression is sequential; everything before your desired bytes has to be decompressed byte by byte
* 		when your desired bytes have been decompressed, this function will end
* example:	input decompresses to 4096 bytes, you only need bytes 512 to 520
* 		set output_start_byte to 512, provide an output_data buffer of 8 bytes and set output_length to 8
*/
static inline int decompress_packbits_window(uint8_t* input_data, uint16_t input_length, uint8_t* output_data, uint16_t output_start_byte, uint16_t* output_length) {
	*output_length = unpackbits_window(input_data, output_data, input_length, output_start_byte, *output_length);
	return 0;
}
#endif



#if COMP_UZLIB==1
// only provided as a starting point, won't ever fit into 16k flash and 2k ram, only for "big boy" microcontrollers.
int compress_uzlib(uint8_t* input_data, uint16_t input_length, uint8_t* output_data, uint16_t* output_length, uint8_t enable_raw) {
	// Initialize compression structure
	struct uzlib_comp comp = {0};
	comp.dict_size = 32768;
	comp.hash_bits = 12;
	//comp.outlen = output_max_size - 4;

	size_t hash_size = sizeof(uzlib_hash_entry_t) * (1 << comp.hash_bits);
	comp.hash_table = malloc(hash_size);
	memset(comp.hash_table, 0, hash_size);

	// Perform compression
	zlib_start_block(&comp);
	uzlib_compress(&comp, input_data, input_length);
	zlib_finish_block(&comp);

	size_t output_size = comp.outlen;
	if (enable_raw != 1) {
		output_size += 4; // add 4 bytes because of header
	}

	uint32_t output_write_position = 0;

	if (enable_raw != 1) {
		//construct header
		memcpy(output_data + output_write_position, "\x1F", 1);
		output_write_position += 1;
		memcpy(output_data + output_write_position, "\x8B", 1);
		output_write_position += 1;
		memcpy(output_data + output_write_position, "\x08", 1);
		output_write_position += 1;
		memcpy(output_data + output_write_position, "\x00", 1);
		output_write_position += 1;
		int mtime = 0;
		memcpy(output_data + output_write_position, &mtime, sizeof(mtime));
		output_write_position += sizeof(mtime);
		memcpy(output_data + output_write_position, "\x04", 1); // XFL
		output_write_position += 1;
		memcpy(output_data + output_write_position, "\x03", 1); // OS
		output_write_position += 1;
	}

	memcpy(output_data + output_write_position, comp.outbuf, comp.outlen);
	output_write_position += comp.outlen;

	if (enable_raw != 1) {
		// construct footer
		unsigned crc = ~uzlib_crc32(output_data, input_length, ~0);
		memcpy(output_data + output_write_position, &crc, sizeof(crc));
		output_write_position += sizeof(crc);
		memcpy(output_data + output_write_position, &input_length, sizeof(input_length));
		output_write_position += sizeof(input_length);
	}

	*output_length = output_write_position;

	return 0;
}

int decompress_uzlib(uint8_t* input_data, uint16_t input_length, uint8_t* output_data, uint16_t* output_length, uint8_t enable_raw) {
	uzlib_init();

	uint32_t output_data_length;
	if (enable_raw != 1) {
		output_data_length = *(uint32_t*)(&input_data[input_length - sizeof(uint32_t)]);
		*output_length = output_data_length;
	}
	else {
		output_data_length = *output_length;
	}

	/* there can be mismatch between length in the trailer and actual
	   data stream; to avoid buffer overruns on overlong streams, reserve
	   one extra byte */
	output_data_length++;

	struct uzlib_uncomp d_stream;
	
	uzlib_uncompress_init(&d_stream, NULL, 0);

	d_stream.source = input_data;
	d_stream.source_limit = input_data + input_length;
	if (enable_raw != 1) {
		d_stream.source_limit -= 4;
	}
	d_stream.source_read_cb = NULL;
	d_stream.dest_start = d_stream.dest = output_data;

	int res;
	if (enable_raw != 1) {
		res = uzlib_gzip_parse_header(&d_stream);
		if (res != TINF_OK) {
			printf("Error parsing header: %d\n", res);
			exit(1);
		}
	}

	while (output_data_length) {
		unsigned int chunk_len = output_data_length < UZ_OUT_CHUNK_SIZE ? output_data_length : UZ_OUT_CHUNK_SIZE;
		d_stream.dest_limit = d_stream.dest + chunk_len;
		if (enable_raw != 1) {
			res = uzlib_uncompress_chksum(&d_stream);
		}
		else if (enable_raw == 1) {
			res = uzlib_uncompress(&d_stream);
		}
		output_data_length -= chunk_len;
		if (res != TINF_OK) {
			break;
		}
	}
/*
	if (res != TINF_DONE) {
		printf("Error during decompression: %d\n", res);
		exit(-res);
	}
*/
	*output_length = d_stream.dest - output_data;

	return 0;
}
#endif




#if COMP_HEATSHRINK==1
static inline int compress_heatshrink(uint8_t* input_data, uint16_t input_length, uint8_t* output_data, uint16_t* output_length) {

	// initialize encoder
	heatshrink_encoder hse;
	heatshrink_encoder_reset(&hse);

	// track progress
	size_t input_index = 0;
	size_t output_index = 0;

	// track encoder status
	HSE_sink_res sink_res;
	HSE_poll_res poll_res;

	// sink input data into the encoder
	while (input_index < input_length) {
		size_t input_consumed;
		// push in more data
		sink_res= heatshrink_encoder_sink(&hse, &input_data[input_index], input_length - input_index, &input_consumed);
		input_index += input_consumed;
		if (sink_res < 0) {
			return 1;
		}
		// pull out as much data as can be at this moment (actual compression)
		do {
			size_t output_produced;
			poll_res = heatshrink_encoder_poll(&hse, &output_data[output_index], *output_length - output_index, &output_produced);
			output_index += output_produced;
			if (poll_res < 0) {
				return 1;
			}
		} while (poll_res == HSER_POLL_MORE);
	}

	// track encoder status
	HSE_finish_res finish_res;
	// pull out the remaining data
	do {
		finish_res = heatshrink_encoder_finish(&hse);
		if (finish_res < 0) {
			return 1;
		}

		size_t output_produced;
		poll_res = heatshrink_encoder_poll(&hse, output_data, *output_length - output_index, &output_produced);
		output_index += output_produced;
	} while (finish_res == HSER_FINISH_MORE);

	// return size of data after compression
	*output_length = output_index;

	// success
	return 0;
}



static inline int decompress_heatshrink(uint8_t* input_data, uint16_t input_length, uint8_t* output_data, uint16_t* output_length) {

	// initialize decoder
	heatshrink_decoder hsd;
	heatshrink_decoder_reset(&hsd);

	// track progress
	size_t input_index = 0;
	size_t output_index = 0;

	// track decoder status
	HSD_sink_res sink_res;
	HSD_poll_res poll_res;

	// sink input data into the decoder
	while (input_index < input_length) {
		size_t input_consumed;
		// push in more data
		sink_res = heatshrink_decoder_sink(&hsd, &input_data[input_index], input_length - input_index, &input_consumed);
		input_index += input_consumed;
		if (sink_res < 0) {
			return 1;
		}
		// pull out as much data as can be at this moment (actual decompression)
		do {
			size_t output_produced;
			poll_res = heatshrink_decoder_poll(&hsd, &output_data[output_index], 1, &output_produced);
			output_index += output_produced;
			if (poll_res < 0) {
				return 1;
			}
		} while (poll_res == HSDR_POLL_MORE);
	}
	// track decoder status
	HSD_finish_res finish_res;
	// pull out the remaining data
	do {
		finish_res = heatshrink_decoder_finish(&hsd);
		if (finish_res < 0) {
			return 1;
		}
	} while (finish_res == HSDR_FINISH_MORE);

	// return size of data after decompression
	*output_length = output_index;

	// success
	return 0;
}

/*
* decompress a specific window of output data
* approach:	heatshrink decompression uses a streaming decoder; everything before your desired bytes has to be decompressed byte by byte
* 		when your desired bytes have been decompressed, this function will end
* example:	input decompresses to 4096 bytes, you only need bytes 512 to 520
* 		set output_start_byte to 512, provide an output_data buffer of 8 bytes and set output_length to 8
*/
static inline int decompress_heatshrink_window(uint8_t* input_data, uint16_t input_length, uint8_t* output_data, uint16_t output_start_byte, uint16_t* output_length) {

	// initialize decoder
	heatshrink_decoder hsd;
	heatshrink_decoder_reset(&hsd);

	// track progress
	size_t input_index = 0;
	size_t output_index = 0;

	// track decoder status
	HSD_sink_res sink_res;
	HSD_poll_res poll_res;

	// sink input data into the decoder
	while (input_index < input_length) {
		size_t input_consumed;
		// push in more data
		sink_res = heatshrink_decoder_sink(&hsd, &input_data[input_index], input_length - input_index, &input_consumed);
		input_index += input_consumed;
		if (sink_res < 0) {
			return 1;
		}
		// pull out as much data as can be at this moment (actual decompression)
		do {
			size_t output_produced;
			uint8_t decoded_byte;
			poll_res = heatshrink_decoder_poll(&hsd, &decoded_byte, 1, &output_produced);
			if (output_produced > 0) {
				// only write decoded bytes if within the output window
				if (output_index >= output_start_byte && output_index < output_start_byte + *output_length) {
					output_data[output_index - output_start_byte] = decoded_byte;
				}
				output_index += output_produced;

				// break the loop when output window has been filled
				if (output_index >= output_start_byte + *output_length) {
					break;
				}
			}
			if (poll_res < 0) {
				return 1;
			}
		} while (poll_res == HSDR_POLL_MORE);
		if (output_index >= output_start_byte + *output_length) {
			break;
		}
	}
	// return size of data after decompression
	*output_length = output_index - output_start_byte;

	// success
	return 0;
}
#endif

#endif // COMPRESSION_H
