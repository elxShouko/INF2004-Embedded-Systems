/** @file drv_s3_codec.h
 * @brief Byte-defined S3 queue encoding; independent of C structure layout.
 */

#ifndef DRV_S3_CODEC_H
#define DRV_S3_CODEC_H

#include "member3_data.h"

#define S3_INPUT_WIRE_MAX 79U
#define S3_OUTPUT_WIRE_MAX 19U

/* Each field is least-significant-bit first, starting at byte zero bit zero.
 * Only this module owns the queue format. Rebuild every caller with headers.
 * Use nav_update_parse() at a text boundary; no text enters the owner queue.
 * Reject wrong length, reserved payloads and nonzero trailing padding.
 * Input tag 7 is the explicit negative-reply marker for invalid caller tags.
 */
/** @brief Encode a request; zero means invalid pointer or unencodable data. */
uint32_t s3_input_encode (const s3_input_t * p_input, uint8_t * p_bytes);
/** @brief Decode a request into private storage; error preserves output. */
bool s3_input_decode (const uint8_t * p_bytes, uint32_t size,
                      s3_input_t * p_input);
/** @brief Encode a validated owner output with its kind-specific payload. */
uint32_t s3_output_encode (const s3_output_t * p_output, uint8_t * p_bytes);
/** @brief Decode a copied owner output; error preserves caller storage. */
bool s3_output_decode (const uint8_t * p_bytes, uint32_t size,
                       s3_output_t * p_output);

#endif

/*** End of file. ***/

