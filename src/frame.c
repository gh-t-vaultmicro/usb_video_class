
/**
 * @defgroup frame Frame processing
 * @brief Tools for managing frame buffers and converting between image formats
 */
#include "libuvc/libuvc.h"
#include "libuvc/libuvc_internal.h"

/** @internal */
uvc_error_t uvc_ensure_frame_size(uvc_frame_t *frame, size_t need_bytes) {
  if (frame->library_owns_data) {
    if (!frame->data || frame->data_bytes != need_bytes) {
      frame->data_bytes = need_bytes;
      frame->data = realloc(frame->data, frame->data_bytes);
    }
    if (!frame->data)
      return UVC_ERROR_NO_MEM;
    return UVC_SUCCESS;
  } else {
    if (!frame->data || frame->data_bytes < need_bytes)
      return UVC_ERROR_NO_MEM;
    return UVC_SUCCESS;
  }
}

/** @brief Allocate a frame structure
 * @ingroup frame
 *
 * @param data_bytes Number of bytes to allocate, or zero
 * @return New frame, or NULL on error
 */
uvc_frame_t *uvc_allocate_frame(size_t data_bytes) {
  uvc_frame_t *frame = malloc(sizeof(*frame));

  if (!frame)
    return NULL;

  memset(frame, 0, sizeof(*frame));
  frame->library_owns_data = 1;

  if (data_bytes > 0) {
    frame->data_bytes = data_bytes;
    frame->data = malloc(data_bytes);

    if (!frame->data) {
      free(frame);
      return NULL;
    }
  }

  return frame;
}

/** @brief Free a frame structure
 * @ingroup frame
 *
 * @param frame Frame to destroy
 */
void uvc_free_frame(uvc_frame_t *frame) {
  if (frame->library_owns_data)
  {
    if (frame->data_bytes > 0)
      free(frame->data);
    if (frame->metadata_bytes > 0)
      free(frame->metadata);
    if (frame->time_stamp)
      free(frame->time_stamp);
  }
  free(frame);
}

static inline unsigned char sat(int i) {
  return (unsigned char)( i >= 255 ? 255 : (i < 0 ? 0 : i));
}

/** @brief Duplicate a frame, preserving color format
 * @ingroup frame
 *
 * @param in Original frame
 * @param out Duplicate frame
 */
uvc_error_t uvc_duplicate_frame(uvc_frame_t *in, uvc_frame_t *out) {
  if (uvc_ensure_frame_size(out, in->data_bytes) < 0)
    return UVC_ERROR_NO_MEM;

  out->width = in->width;
  out->height = in->height;
  out->frame_format = in->frame_format;
  out->step = in->step;
  out->sequence = in->sequence;
  out->capture_time = in->capture_time;
  out->capture_time_finished = in->capture_time_finished;
  out->source = in->source;

  memcpy(out->data, in->data, in->data_bytes);

  if (in->metadata && in->metadata_bytes > 0)
  {
      if (out->metadata_bytes < in->metadata_bytes)
      {
          out->metadata = realloc(out->metadata, in->metadata_bytes);
      }
      out->metadata_bytes = in->metadata_bytes;
      memcpy(out->metadata, in->metadata, in->metadata_bytes);
  }

  return UVC_SUCCESS;
}

#define YUYV2RGB_2(pyuv, prgb) { \
    float r = 1.402f * ((pyuv)[3]-128); \
    float g = -0.34414f * ((pyuv)[1]-128) - 0.71414f * ((pyuv)[3]-128); \
    float b = 1.772f * ((pyuv)[1]-128); \
    (prgb)[0] = sat(pyuv[0] + r); \
    (prgb)[1] = sat(pyuv[0] + g); \
    (prgb)[2] = sat(pyuv[0] + b); \
    (prgb)[3] = sat(pyuv[2] + r); \
    (prgb)[4] = sat(pyuv[2] + g); \
    (prgb)[5] = sat(pyuv[2] + b); \
    }
#define IYUYV2RGB_2(pyuv, prgb) { \
    int r = (22987 * ((pyuv)[3] - 128)) >> 14; \
    int g = (-5636 * ((pyuv)[1] - 128) - 11698 * ((pyuv)[3] - 128)) >> 14; \
    int b = (29049 * ((pyuv)[1] - 128)) >> 14; \
    (prgb)[0] = sat(*(pyuv) + r); \
    (prgb)[1] = sat(*(pyuv) + g); \
    (prgb)[2] = sat(*(pyuv) + b); \
    (prgb)[3] = sat((pyuv)[2] + r); \
    (prgb)[4] = sat((pyuv)[2] + g); \
    (prgb)[5] = sat((pyuv)[2] + b); \
    }
#define IYUYV2RGB_16(pyuv, prgb) IYUYV2RGB_8(pyuv, prgb); IYUYV2RGB_8(pyuv + 16, prgb + 24);
#define IYUYV2RGB_8(pyuv, prgb) IYUYV2RGB_4(pyuv, prgb); IYUYV2RGB_4(pyuv + 8, prgb + 12);
#define IYUYV2RGB_4(pyuv, prgb) IYUYV2RGB_2(pyuv, prgb); IYUYV2RGB_2(pyuv + 4, prgb + 6);

/** @brief Convert a frame from YUYV to RGB
 * @ingroup frame
 *
 * @param in YUYV frame
 * @param out RGB frame
 */
uvc_error_t uvc_yuyv2rgb(uvc_frame_t *in, uvc_frame_t *out) {
  if (in->frame_format != UVC_FRAME_FORMAT_YUYV)
    return UVC_ERROR_INVALID_PARAM;

  if (uvc_ensure_frame_size(out, in->width * in->height * 3) < 0)
    return UVC_ERROR_NO_MEM;

  out->width = in->width;
  out->height = in->height;
  out->frame_format = UVC_FRAME_FORMAT_RGB;
  out->step = in->width * 3;
  out->sequence = in->sequence;
  out->capture_time = in->capture_time;
  out->capture_time_finished = in->capture_time_finished;
  out->source = in->source;

  uint8_t *pyuv = in->data;
  uint8_t *prgb = out->data;
  uint8_t *prgb_end = prgb + out->data_bytes;

  while (prgb < prgb_end) {
    IYUYV2RGB_8(pyuv, prgb);

    prgb += 3 * 8;
    pyuv += 2 * 8;
  }

  return UVC_SUCCESS;
}

#define IYUYV2BGR_2(pyuv, pbgr) { \
    int r = (22987 * ((pyuv)[3] - 128)) >> 14; \
    int g = (-5636 * ((pyuv)[1] - 128) - 11698 * ((pyuv)[3] - 128)) >> 14; \
    int b = (29049 * ((pyuv)[1] - 128)) >> 14; \
    (pbgr)[0] = sat(*(pyuv) + b); \
    (pbgr)[1] = sat(*(pyuv) + g); \
    (pbgr)[2] = sat(*(pyuv) + r); \
    (pbgr)[3] = sat((pyuv)[2] + b); \
    (pbgr)[4] = sat((pyuv)[2] + g); \
    (pbgr)[5] = sat((pyuv)[2] + r); \
    }
#define IYUYV2BGR_16(pyuv, pbgr) IYUYV2BGR_8(pyuv, pbgr); IYUYV2BGR_8(pyuv + 16, pbgr + 24);
#define IYUYV2BGR_8(pyuv, pbgr) IYUYV2BGR_4(pyuv, pbgr); IYUYV2BGR_4(pyuv + 8, pbgr + 12);
#define IYUYV2BGR_4(pyuv, pbgr) IYUYV2BGR_2(pyuv, pbgr); IYUYV2BGR_2(pyuv + 4, pbgr + 6);

/** @brief Convert a frame from YUYV to BGR
 * @ingroup frame
 *
 * @param in YUYV frame
 * @param out BGR frame
  */
// uvc_error_t uvc_yuyv2bgr(uvc_frame_t *in, uvc_frame_t *out) {
//   if (in->frame_format != UVC_FRAME_FORMAT_YUYV)
//     return UVC_ERROR_INVALID_PARAM;

//   if (uvc_ensure_frame_size(out, in->width * in->height * 3) < 0)
//     return UVC_ERROR_NO_MEM;

//   // size_t expected_in_size = in->width * in->height * 2; // YUYV 형식은 픽셀당 2바이트 사용
//   // if (in->data_bytes < expected_in_size){
//   //   printf("Error: The size of the input frame is smaller than expected\n");
//   //   return UVC_ERROR_NO_MEM;
//   // }


//   out->width = in->width;
//   out->height = in->height;
//   out->frame_format = UVC_FRAME_FORMAT_BGR;
//   out->step = in->width * 3;
//   out->sequence = in->sequence;
//   out->capture_time = in->capture_time;
//   out->capture_time_finished = in->capture_time_finished;
//   out->source = in->source;

//   uint8_t *pyuv = in->data;
//   uint8_t *pbgr = out->data;
//   uint8_t *pbgr_end = pbgr + out->data_bytes;

//   size_t total_pixels = in->width * in->height;
//   size_t num_blocks = total_pixels / 8;
//   size_t remaining_pixels = total_pixels % 8;

//   // // 처리할 픽셀 수가 8의 배수가 아닐 경우 오류 반환
//   // if (remaining_pixels != 0){
//   //   printf("Error: The number of pixels is not a multiple of 8\n");
//   //   return UVC_ERROR_NO_MEM;
//   // }

//   while (pbgr < pbgr_end) {
//     IYUYV2BGR_8(pyuv, pbgr);

//     pbgr += 3 * 8;
//     pyuv += 2 * 8;
//   }

//   return UVC_SUCCESS;
// }

uvc_error_t uvc_yuyv2bgr(uvc_frame_t *in, uvc_frame_t *out) {
  if (in->frame_format != UVC_FRAME_FORMAT_YUYV)
    return UVC_ERROR_INVALID_PARAM;

  size_t expected_out_size = in->width * in->height * 3;
  if (uvc_ensure_frame_size(out, expected_out_size) < 0)
    return UVC_ERROR_NO_MEM;

  // size_t expected_in_size = in->width * in->height * 2; // YUYV 형식은 픽셀당 2바이트 사용

  // // 입력 데이터가 예상 크기보다 작을 경우, 부족한 부분을 0으로 채운 버퍼 생성
  // uint8_t *pyuv_full;
  // if (in->data_bytes < expected_in_size) {
  //   // 새로운 버퍼 할당
  //   pyuv_full = (uint8_t *)malloc(expected_in_size);
  //   if (!pyuv_full)
  //     return UVC_ERROR_NO_MEM;

  //   // 기존 데이터를 복사하고 나머지 부분을 0으로 채움
  //   memcpy(pyuv_full, in->data, in->data_bytes);
  //   memset(pyuv_full + in->data_bytes, 0, expected_in_size - in->data_bytes);
  // } else {
  //   pyuv_full = in->data;
  // }

  out->width = in->width;
  out->height = in->height;
  out->frame_format = UVC_FRAME_FORMAT_BGR;
  out->step = in->width * 3;
  out->sequence = in->sequence;
  out->capture_time = in->capture_time;
  out->capture_time_finished = in->capture_time_finished;
  out->source = in->source;

  uint8_t *pyuv = in->data;
  uint8_t *pbgr = out->data;
  uint8_t *pbgr_end = pbgr + out->data_bytes;

  size_t total_pixels = in->width * in->height;
  size_t num_blocks = total_pixels / 8;
  size_t remaining_pixels = total_pixels % 8;

  // // 메인 루프: 8픽셀씩 처리
  // for (size_t i = 0; i < num_blocks; ++i) {
  //   IYUYV2BGR_8(pyuv, pbgr);
  //   pbgr += 3 * 8;
  //   pyuv += 2 * 8;
  // }

  // // 남은 픽셀이 있을 경우 처리
  // if (remaining_pixels > 0) {
  //   // 남은 픽셀 수만큼 처리하는 함수 사용
  //   IYUYV2BGR(pyuv, pbgr, remaining_pixels);
  //   pbgr += 3 * remaining_pixels;
  //   pyuv += 2 * remaining_pixels;
  // }

  // // 추가로 할당한 버퍼가 있으면 해제
  // if (in->data_bytes < expected_in_size) {
  //   free(pyuv_full);
  // }

  return UVC_SUCCESS;
}


#define IYUYV2Y(pyuv, py) { \
    (py)[0] = (pyuv[0]); \
    }

/** @brief Convert a frame from YUYV to Y (GRAY8)
 * @ingroup frame
 *
 * @param in YUYV frame
 * @param out GRAY8 frame
 */
uvc_error_t uvc_yuyv2y(uvc_frame_t *in, uvc_frame_t *out) {
  if (in->frame_format != UVC_FRAME_FORMAT_YUYV)
    return UVC_ERROR_INVALID_PARAM;

  if (uvc_ensure_frame_size(out, in->width * in->height) < 0)
    return UVC_ERROR_NO_MEM;

  out->width = in->width;
  out->height = in->height;
  out->frame_format = UVC_FRAME_FORMAT_GRAY8;
  out->step = in->width;
  out->sequence = in->sequence;
  out->capture_time = in->capture_time;
  out->capture_time_finished = in->capture_time_finished;
  out->source = in->source;

  uint8_t *pyuv = in->data;
  uint8_t *py = out->data;
  uint8_t *py_end = py + out->data_bytes;

  while (py < py_end) {
    IYUYV2Y(pyuv, py);

    py += 1;
    pyuv += 2;
  }

  return UVC_SUCCESS;
}

#define IYUYV2UV(pyuv, puv) { \
    (puv)[0] = (pyuv[1]); \
    }

/** @brief Convert a frame from YUYV to UV (GRAY8)
 * @ingroup frame
 *
 * @param in YUYV frame
 * @param out GRAY8 frame
 */
uvc_error_t uvc_yuyv2uv(uvc_frame_t *in, uvc_frame_t *out) {
  if (in->frame_format != UVC_FRAME_FORMAT_YUYV)
    return UVC_ERROR_INVALID_PARAM;

  if (uvc_ensure_frame_size(out, in->width * in->height) < 0)
    return UVC_ERROR_NO_MEM;

  out->width = in->width;
  out->height = in->height;
  out->frame_format = UVC_FRAME_FORMAT_GRAY8;
  out->step = in->width;
  out->sequence = in->sequence;
  out->capture_time = in->capture_time;
  out->capture_time_finished = in->capture_time_finished;
  out->source = in->source;

  uint8_t *pyuv = in->data;
  uint8_t *puv = out->data;
  uint8_t *puv_end = puv + out->data_bytes;

  while (puv < puv_end) {
    IYUYV2UV(pyuv, puv);

    puv += 1;
    pyuv += 2;
  }

  return UVC_SUCCESS;
}

#define IUYVY2RGB_2(pyuv, prgb) { \
    int r = (22987 * ((pyuv)[2] - 128)) >> 14; \
    int g = (-5636 * ((pyuv)[0] - 128) - 11698 * ((pyuv)[2] - 128)) >> 14; \
    int b = (29049 * ((pyuv)[0] - 128)) >> 14; \
    (prgb)[0] = sat((pyuv)[1] + r); \
    (prgb)[1] = sat((pyuv)[1] + g); \
    (prgb)[2] = sat((pyuv)[1] + b); \
    (prgb)[3] = sat((pyuv)[3] + r); \
    (prgb)[4] = sat((pyuv)[3] + g); \
    (prgb)[5] = sat((pyuv)[3] + b); \
    }
#define IUYVY2RGB_16(pyuv, prgb) IUYVY2RGB_8(pyuv, prgb); IUYVY2RGB_8(pyuv + 16, prgb + 24);
#define IUYVY2RGB_8(pyuv, prgb) IUYVY2RGB_4(pyuv, prgb); IUYVY2RGB_4(pyuv + 8, prgb + 12);
#define IUYVY2RGB_4(pyuv, prgb) IUYVY2RGB_2(pyuv, prgb); IUYVY2RGB_2(pyuv + 4, prgb + 6);

/** @brief Convert a frame from UYVY to RGB
 * @ingroup frame
 * @param ini UYVY frame
 * @param out RGB frame
 */
uvc_error_t uvc_uyvy2rgb(uvc_frame_t *in, uvc_frame_t *out) {
  if (in->frame_format != UVC_FRAME_FORMAT_UYVY)
    return UVC_ERROR_INVALID_PARAM;

  if (uvc_ensure_frame_size(out, in->width * in->height * 3) < 0)
    return UVC_ERROR_NO_MEM;

  out->width = in->width;
  out->height = in->height;
  out->frame_format = UVC_FRAME_FORMAT_RGB;
  out->step = in->width *3;
  out->sequence = in->sequence;
  out->capture_time = in->capture_time;
  out->capture_time_finished = in->capture_time_finished;
  out->source = in->source;

  uint8_t *pyuv = in->data;
  uint8_t *prgb = out->data;
  uint8_t *prgb_end = prgb + out->data_bytes;

  while (prgb < prgb_end) {
    IUYVY2RGB_8(pyuv, prgb);

    prgb += 3 * 8;
    pyuv += 2 * 8;
  }

  return UVC_SUCCESS;
}

#define IUYVY2BGR_2(pyuv, pbgr) { \
    int r = (22987 * ((pyuv)[2] - 128)) >> 14; \
    int g = (-5636 * ((pyuv)[0] - 128) - 11698 * ((pyuv)[2] - 128)) >> 14; \
    int b = (29049 * ((pyuv)[0] - 128)) >> 14; \
    (pbgr)[0] = sat((pyuv)[1] + b); \
    (pbgr)[1] = sat((pyuv)[1] + g); \
    (pbgr)[2] = sat((pyuv)[1] + r); \
    (pbgr)[3] = sat((pyuv)[3] + b); \
    (pbgr)[4] = sat((pyuv)[3] + g); \
    (pbgr)[5] = sat((pyuv)[3] + r); \
    }
#define IUYVY2BGR_16(pyuv, pbgr) IUYVY2BGR_8(pyuv, pbgr); IUYVY2BGR_8(pyuv + 16, pbgr + 24);
#define IUYVY2BGR_8(pyuv, pbgr) IUYVY2BGR_4(pyuv, pbgr); IUYVY2BGR_4(pyuv + 8, pbgr + 12);
#define IUYVY2BGR_4(pyuv, pbgr) IUYVY2BGR_2(pyuv, pbgr); IUYVY2BGR_2(pyuv + 4, pbgr + 6);

/** @brief Convert a frame from UYVY to BGR
 * @ingroup frame
 * @param ini UYVY frame
 * @param out BGR frame
 */
uvc_error_t uvc_uyvy2bgr(uvc_frame_t *in, uvc_frame_t *out) {
  if (in->frame_format != UVC_FRAME_FORMAT_UYVY)
    return UVC_ERROR_INVALID_PARAM;

  if (uvc_ensure_frame_size(out, in->width * in->height * 3) < 0)
    return UVC_ERROR_NO_MEM;

  out->width = in->width;
  out->height = in->height;
  out->frame_format = UVC_FRAME_FORMAT_BGR;
  out->step = in->width *3;
  out->sequence = in->sequence;
  out->capture_time = in->capture_time;
  out->capture_time_finished = in->capture_time_finished;
  out->source = in->source;

  uint8_t *pyuv = in->data;
  uint8_t *pbgr = out->data;
  uint8_t *pbgr_end = pbgr + out->data_bytes;

  while (pbgr < pbgr_end) {
    IUYVY2BGR_8(pyuv, pbgr);

    pbgr += 3 * 8;
    pyuv += 2 * 8;
  }

  return UVC_SUCCESS;
}

/** @brief Convert a frame to RGB
 * @ingroup frame
 *
 * @param in non-RGB frame
 * @param out RGB frame
 */
uvc_error_t uvc_any2rgb(uvc_frame_t *in, uvc_frame_t *out) {
  switch (in->frame_format) {
#ifdef LIBUVC_HAS_JPEG
    case UVC_FRAME_FORMAT_MJPEG:
      return uvc_mjpeg2rgb(in, out);
#endif
    case UVC_FRAME_FORMAT_YUYV:
      return uvc_yuyv2rgb(in, out);
    case UVC_FRAME_FORMAT_UYVY:
      return uvc_uyvy2rgb(in, out);
    case UVC_FRAME_FORMAT_RGB:
      return uvc_duplicate_frame(in, out);
    default:
      return UVC_ERROR_NOT_SUPPORTED;
  }
}

/** @brief Convert a frame to BGR
 * @ingroup frame
 *
 * @param in non-BGR frame
 * @param out BGR frame
 */
uvc_error_t uvc_any2bgr(uvc_frame_t *in, uvc_frame_t *out) {
  switch (in->frame_format) {
    case UVC_FRAME_FORMAT_YUYV:
      return uvc_yuyv2bgr(in, out);
    case UVC_FRAME_FORMAT_UYVY:
      return uvc_uyvy2bgr(in, out);
    case UVC_FRAME_FORMAT_BGR:
      return uvc_duplicate_frame(in, out);
    default:
      return UVC_ERROR_NOT_SUPPORTED;
  }
}
