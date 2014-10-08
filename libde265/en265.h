/*
 * H.265 video codec.
 * Copyright (c) 2013-2014 struktur AG, Dirk Farin <farin@struktur.de>
 *
 * Authors: Dirk Farin <farin@struktur.de>
 *
 * This file is part of libde265.
 *
 * libde265 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * libde265 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libde265.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EN265_H
#define EN265_H

#include "libde265/de265.h"


// ========== encoder context ==========

typedef void en265_encoder_context; // private structure

/* Get a new encoder context. Must be freed with en265_free_encoder(). */
LIBDE265_API en265_encoder_context* en265_new_encoder(void);

LIBDE265_API de265_error en265_start_worker_threads(en265_encoder_context*, int number_of_threads);

/* Free encoder context. May only be called once on a context. */
LIBDE265_API de265_error en265_free_encoder(en265_encoder_context*);

/* The user data pointer will be given to the release_buffer() function
   in de265_image_allocation. */
LIBDE265_API void en265_set_image_allocation_functions(en265_encoder_context*,
                                                       struct de265_image_allocation*,
                                                       void* alloc_userdata);


// ========== encoder parameters ==========

LIBDE265_API de265_error en265_set_parameter_bool(en265_encoder_context*,
                                                  const char* parametername,int value);
LIBDE265_API de265_error en265_set_parameter_int(en265_encoder_context*,
                                                 const char* parametername,int value);
LIBDE265_API de265_error en265_set_parameter_option(en265_encoder_context*,
                                                    const char* parametername,const char* value);

LIBDE265_API de265_error en265_parse_command_line_parameters(en265_encoder_context*,
                                                             int* argc, char** argv);


LIBDE265_API int  en265_list_parameters(en265_encoder_context*,
                                        const char** parametername, int maxParams);

enum en265_parameter_type {
  en265_parameter_bool,
  en265_parameter_int,
  en265_parameter_choice
};

LIBDE265_API enum en265_parameter_type en265_get_parameter_type(en265_encoder_context*,
                                                                const char* parametername);

// returns number of options
LIBDE265_API int  en265_list_parameter_options(en265_encoder_context*,
                                               const char* parametername,
                                               const char** out_options, int maxOptions);

#define EN265_PARAM_TB_IntraPredMode "TB-IntraPredMode"
#define EN265_PARAM_CB_IntraPartMode "CB-IntraPartMode"
#define EN265_PARAM_IntraPredMode_Fastbrute_estimator "IntraPredMode-FastBrute-estimator"


// ========== encoding loop ==========

LIBDE265_API struct de265_image* en265_allocate_image(en265_encoder_context*,
                                                      int width, int height, de265_chroma chroma,
                                                      de265_PTS pts, void* image_userdata);

// Request a specification of the image memory layout for an image of the specified dimensions.
LIBDE265_API void de265_get_image_spec(en265_encoder_context*,
                                       int width, int height, de265_chroma chroma,
                                       struct de265_image_spec* out_spec);

// Image memory layout specification for an image returned by en265_allocate_image().
LIBDE265_API void de265_get_image_spec_from_image(de265_image* img, struct de265_image_spec* spec);



LIBDE265_API de265_error en265_push_image(en265_encoder_context*,
                                          struct de265_image*); // non-blocking

LIBDE265_API de265_error en265_push_eof(en265_encoder_context*);

// block when there are more than max_input_images in the input queue
LIBDE265_API de265_error en265_block_on_input_queue_length(en265_encoder_context*,
                                                           int max_pending_images,
                                                           int timeout_ms);

LIBDE265_API de265_error en265_trim_input_queue(en265_encoder_context*, int max_pending_images);

LIBDE265_API int  en265_current_input_queue_length(en265_encoder_context*);


enum en265_encoder_state
{
  EN265_STATE_IDLE,
  EN265_STATE_WAITING_FOR_INPUT,
  EN265_STATE_WORKING,
  EN265_STATE_OUTPUT_QUEUE_FULL,
  EN265_STATE_EOS
};


LIBDE265_API enum en265_encoder_state en265_get_encoder_state(en265_encoder_context*);


enum en265_packet_content_type {
  EN265_PACKET_VPS,
  EN265_PACKET_SPS,
  EN265_PACKET_PPS,
  EN265_PACKET_SEI,
  EN265_PACKET_SLICE,
  EN265_PACKET_SKIPPED_IMAGE
};


struct en265_packet
{
  const uint8_t* data;
  int   length;

  enum en265_packet_content_type content_type;
  char complete_picture : 1;
  char final_slice      : 1;
  char dependent_slice  : 1;

  PTS   pts;
  void* user_data;

  de265_image* input_image;
  de265_image* reconstruction; // optional
};

LIBDE265_API struct en265_packet* en265_get_packet(en265_encoder_context*); // blocking
LIBDE265_API void en265_free_packet(en265_encoder_context*, struct en265_packet*);

#endif