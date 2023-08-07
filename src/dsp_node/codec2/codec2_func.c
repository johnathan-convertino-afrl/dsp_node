//******************************************************************************
/// @file     codec2_func.c
/// @author   Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date     2023.06.29
/// @brief    CODEC2 datac1 modulation/demodulation routines.
//******************************************************************************

// standard includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "codec2_func.h"
#include "kill_throbber.h"

// COMMON FUNCTIONS //

//Setup codec2 arg struct for mod/demod init callbacks
struct s_codec2_func_args *create_codec2_args(enum e_binary_type sample_type)
{
  struct s_codec2_func_args *p_temp = NULL;

  p_temp = malloc(sizeof(struct s_codec2_func_args));

  if(!p_temp) return NULL;

  switch(sample_type)
  {
    case DATA_S16:
    case DATA_CFLOAT:
      p_temp->sample_type = sample_type;
      break;
    default:
      p_temp->sample_type = DATA_CFLOAT;
      fprintf(stderr, "ERROR: Type specified for codec2 incorrect, must be DATA_S16 or DATA_CFLOAT. Defaulting to DATA_CFLOAT.\n");
      break;
  }

  return p_temp;
}

//Free args struct created from create file args
void free_codec2_args(struct s_codec2_func_args *p_init_args)
{
  free(p_init_args);
}

// THREAD MODULATE FUNCTIONS //

//Setup codec2 thread for modulation
int init_callback_codec2_mod(void *p_init_args, void *p_object)
{
  struct s_codec2_func_args *p_codec2_args = NULL;

  struct s_dsp_node *p_dsp_node = NULL;

  p_codec2_args = (struct s_codec2_func_args *)p_init_args;

  if(!p_codec2_args)
  {
    fprintf(stderr, "ERROR: Codec2 DATAC1 init args are NULL.\n");

    return ~0;
  }

  p_dsp_node = (struct s_dsp_node *)p_object;

  p_dsp_node->p_data = freedv_open(FREEDV_MODE_DATAC1);

  if(!p_dsp_node->p_data)
  {
    fprintf(stderr, "ERROR: Codec2 DATAC1 mod create failed.\n");

    return ~0;
  }

  p_dsp_node->input_type = DATA_U8;

  p_dsp_node->output_type = p_codec2_args->sample_type;

  return 0;
}

//Pthread function for threading modulation
void* pthread_function_codec2_mod(void *p_data)
{
  // variables are copy pasta, so names clash with original authors vs mine.
  size_t bytes_per_modem_frame = 0;
  size_t payload_bytes_per_modem_frame = 0;
  size_t n_mod_out = 0;

  uint8_t *p_bytes_in = NULL;
  uint8_t *p_mod_out = NULL;

  unsigned long int numRead = 0;
  int inter_burst_delay_ms = 200;
  size_t samples_delay = (size_t)(FREEDV_FS_8000*inter_burst_delay_ms/1000);

  struct s_dsp_node *p_dsp_node = NULL;

  p_dsp_node = (struct s_dsp_node *)p_data;

  if(!p_dsp_node)
  {
    fprintf(stderr, "ERROR: Data Struct is NULL.\n");
    return NULL;
  }

  // setup data sizes for bytes per frame, number modulated out and such.
  bytes_per_modem_frame = (size_t)freedv_get_bits_per_modem_frame((struct freedv *)p_dsp_node->p_data)/8;

  payload_bytes_per_modem_frame = bytes_per_modem_frame - 2;

  n_mod_out = (size_t)freedv_get_n_tx_modem_samples((struct freedv *)p_dsp_node->p_data);

  p_bytes_in = malloc(bytes_per_modem_frame);

  if(!p_bytes_in)
  {
    perror("ERROR: Could not allocate p_bytes_in buffer.");

    kill_thread = 1;

    return NULL;
  }

  // this example creates a buffer large enough for the preamble, data, postamble, and silence to be held.
  p_mod_out = malloc((n_mod_out * 3 * p_dsp_node->output_type_size) + (samples_delay * p_dsp_node->output_type_size));

  if(!p_mod_out)
  {
    perror("ERROR: Could not allocate p_mod_out buffer.");

    free(p_bytes_in);

    kill_thread = 1;

    return NULL;
  }

  p_dsp_node->total_bytes_processed = 0;

  do
  {
    uint16_t crc16 = 0;
    long unsigned int numElemRead = 0;
    long unsigned int numElemWrote  = 0;

    // clear buffers so old data does not get sent out.
    memset(p_bytes_in, 0, bytes_per_modem_frame);
    memset(p_mod_out, 0, (n_mod_out * 3 * p_dsp_node->output_type_size) + (samples_delay * p_dsp_node->output_type_size));

    // create preamble
    switch(p_dsp_node->output_type)
    {
      case(DATA_S16):
        numElemRead = (long unsigned int)freedv_rawdatapreambletx((struct freedv *)p_dsp_node->p_data, (short *)p_mod_out);
        break;
      case(DATA_CFLOAT):
      default:
        numElemRead = (long unsigned int)freedv_rawdatapreamblecomptx((struct freedv *)p_dsp_node->p_data, (COMP *)p_mod_out);
        break;
    }

    numRead = ringBufferBlockingRead(p_dsp_node->p_input_ring_buffer, p_bytes_in, payload_bytes_per_modem_frame, NULL);

    if(numRead <= 0) continue;

    // generate crc
    crc16 = freedv_gen_crc16(p_bytes_in, (int)payload_bytes_per_modem_frame);

    // append crc to byte buffer
    p_bytes_in[bytes_per_modem_frame-2] = (uint8_t)(crc16 >> 8);
    p_bytes_in[bytes_per_modem_frame-1] = (uint8_t)(crc16 & 0xff);

    // modulate
    switch(p_dsp_node->output_type)
    {
      case(DATA_S16):
        freedv_rawdatatx((struct freedv *)p_dsp_node->p_data, (short *)p_mod_out + numElemRead, p_bytes_in);
        break;
      case(DATA_CFLOAT):
      default:
        freedv_rawdatacomptx((struct freedv *)p_dsp_node->p_data, (COMP *)p_mod_out + numElemRead, p_bytes_in);
        break;
    }

    // numElemRead is the total bytes modulated that need to be written out.
    numElemRead += n_mod_out;

    // create postamble
    switch(p_dsp_node->output_type)
    {
      case(DATA_S16):
        numElemRead += (long unsigned int)freedv_rawdatapostambletx((struct freedv *)p_dsp_node->p_data, (short *)p_mod_out + numElemRead);
        break;
      case(DATA_CFLOAT):
      default:
        numElemRead += (long unsigned int)freedv_rawdatapostamblecomptx((struct freedv *)p_dsp_node->p_data, (COMP *)p_mod_out + numElemRead);
        break;
    }

    numElemRead += samples_delay;

    p_dsp_node->total_bytes_processed += numElemRead * p_dsp_node->output_type_size;

    do
    {
      numElemWrote += ringBufferBlockingWrite(p_dsp_node->p_output_ring_buffer, p_mod_out + (numElemWrote * p_dsp_node->output_type_size), numElemRead - numElemWrote, NULL);
    } while(numElemWrote < numElemRead);

  } while((numRead > 0) && !kill_thread);

  ringBufferEndBlocking(p_dsp_node->p_output_ring_buffer);
  ringBufferEndBlocking(p_dsp_node->p_input_ring_buffer);

  free(p_mod_out);

  free(p_bytes_in);

  return NULL;
}

//Clean up all allocations from init_callback modulation
int free_callback_codec2_mod(void *p_object)
{
  struct s_dsp_node *p_dsp_node = NULL;

  p_dsp_node = (struct s_dsp_node *)p_object;

  freedv_close((struct freedv *)p_dsp_node->p_data);

  return 0;
}

// THREAD DEMODULATE FUNCTIONS //

//Setup codec2 demod thread
int init_callback_codec2_demod(void *p_init_args, void *p_object)
{
  struct s_codec2_func_args *p_codec2_args = NULL;

  struct s_dsp_node *p_dsp_node = NULL;

  p_codec2_args = (struct s_codec2_func_args *)p_init_args;

  if(!p_codec2_args)
  {
    fprintf(stderr, "ERROR: Codec2 DATAC1 init args are NULL.\n");

    return ~0;
  }

  p_dsp_node = (struct s_dsp_node *)p_object;

  p_dsp_node->p_data = freedv_open(FREEDV_MODE_DATAC1);

  if(!p_dsp_node->p_data)
  {
    fprintf(stderr, "ERROR: Codec2 DATAC1 demod create failed.\n");

    return ~0;
  }

  freedv_set_frames_per_burst((struct freedv *)p_dsp_node->p_data, 1);

  p_dsp_node->output_type = DATA_U8;

  p_dsp_node->input_type = p_codec2_args->sample_type;

  return 0;
}

//Pthread function for threading demodulation
void* pthread_function_codec2_demod(void *p_data)
{
  // variables are copy pasta, so names clash with original authors vs mine.
  size_t bytes_per_modem_frame = 0;
  size_t max_modem_samples = 0;

  uint8_t *p_bytes_out  = NULL;
  uint8_t *p_demod_in   = NULL;

  unsigned long int numRead = 0;

  struct s_dsp_node *p_dsp_node = NULL;

  p_dsp_node = (struct s_dsp_node *)p_data;

  if(!p_dsp_node)
  {
    fprintf(stderr, "ERROR: Data Struct is NULL.\n");

    kill_thread = 1;

    return NULL;
  }

  // how many bytes does each from the modem contain?
  bytes_per_modem_frame = (size_t)freedv_get_bits_per_modem_frame((struct freedv *)p_dsp_node->p_data)/8;

  p_bytes_out = malloc(bytes_per_modem_frame);

  if(!p_bytes_out)
  {
    perror("ERROR: Could not allocate raw processor buffer.");

    kill_thread = 1;

    return NULL;
  }

  max_modem_samples = (size_t)freedv_get_n_max_modem_samples((struct freedv *)p_dsp_node->p_data);

  p_demod_in = malloc(max_modem_samples * p_dsp_node->input_type_size);

  if(!p_demod_in)
  {
    perror("ERROR: Could not allocate enc processor buffer.");

    free(p_bytes_out);

    kill_thread = 1;

    return NULL;
  }

  p_dsp_node->total_bytes_processed = 0;

  do
  {
    size_t nin = 0;
    size_t nbytes_out = 0;
    long unsigned int numElemWrote  = 0;

    // number of modulated samples, is this a constant?
    nin = (size_t)freedv_nin((struct freedv *)p_dsp_node->p_data);

    numRead = ringBufferBlockingRead(p_dsp_node->p_input_ring_buffer, p_demod_in, nin, NULL);

    // demodulate data
    switch(p_dsp_node->input_type)
    {
      case(DATA_S16):
        nbytes_out = (size_t)freedv_rawdatarx((struct freedv *)p_dsp_node->p_data, p_bytes_out, (short *)p_demod_in);
        break;
      case(DATA_CFLOAT):
      default:
        nbytes_out = (size_t)freedv_rawdatacomprx((struct freedv *)p_dsp_node->p_data, p_bytes_out, (COMP *)p_demod_in);
        break;
    }

    // if we don't have enouch or any data don't decrament nbytes_out!
    // ring buffer will take 0 and just exit, no need to do a continue here.
    if(nbytes_out >= 2) nbytes_out = nbytes_out - 2;

    p_dsp_node->total_bytes_processed += nbytes_out;

    do
    {
      // write out demod bytes for file writting
      numElemWrote += ringBufferBlockingWrite(p_dsp_node->p_output_ring_buffer, p_bytes_out + numElemWrote, nbytes_out - numElemWrote, NULL);
    } while(numElemWrote < nbytes_out);

  } while((numRead > 0) && !kill_thread);

  ringBufferEndBlocking(p_dsp_node->p_output_ring_buffer);
  ringBufferEndBlocking(p_dsp_node->p_input_ring_buffer);

  free(p_bytes_out);

  free(p_demod_in);

  return NULL;
}

//Clean up all allocations from init_callback for demodulation
int free_callback_codec2_demod(void *p_object)
{
  struct s_dsp_node *p_dsp_node = NULL;

  p_dsp_node = (struct s_dsp_node *)p_object;

  freedv_close((struct freedv *)p_dsp_node->p_data);

  return 0;
}
