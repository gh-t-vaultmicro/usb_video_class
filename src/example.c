#include "libuvc/libuvc.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include <string.h>
#include <jpeglib.h>

void bgr_to_rgb(uint8_t *data, int width, int height) {
  for (int i = 0; i < width * height * 3; i += 3) {
    uint8_t temp = data[i];      // Blue
    data[i] = data[i + 2];       // Red
    data[i + 2] = temp;          // Blue
  }
}

/* Helper function to save BGR data as a JPEG file */
void save_bgr_to_jpeg(uint8_t *bgr_data, int width, int height, const char *filename) {
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;

  FILE *outfile;  /* target file */
  JSAMPROW row_pointer[1];  /* pointer to a single row */
  int row_stride;  /* physical row width in BGR buffer */

  /* Step 1: Allocate and initialize JPEG compression object */
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);

  /* Step 2: Specify the destination for the compressed data */
  if ((outfile = fopen(filename, "wb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    return;
  }
  jpeg_stdio_dest(&cinfo, outfile);

  /* Step 3: Set parameters for compression */
  cinfo.image_width = width;      /* image width and height, in pixels */
  cinfo.image_height = height;
  cinfo.input_components = 3;     /* # of color components per pixel */
  cinfo.in_color_space = JCS_RGB; /* colorspace of input image */

  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, 85, TRUE); /* set the quality [0..100] */

  /* Step 4: Start compressor */
  jpeg_start_compress(&cinfo, TRUE);

  /* Step 5: while (scan lines remain to be written) */
  row_stride = width * 3; /* JSAMPLEs per row in image_buffer */
  while (cinfo.next_scanline < cinfo.image_height) {
    row_pointer[0] = &bgr_data[cinfo.next_scanline * row_stride];
    jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  /* Step 6: Finish compression */
  jpeg_finish_compress(&cinfo);

  /* Step 7: Release JPEG compression object */
  fclose(outfile);
  jpeg_destroy_compress(&cinfo);
}


/* Helper function to save RGB data as a JPEG file */
void save_rgb_to_jpeg(uint8_t *rgb_data, int width, int height, const char *filename) {
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;

  FILE *outfile;  /* target file */
  JSAMPROW row_pointer[1];  /* pointer to a single row */
  int row_stride;  /* physical row width in RGB buffer */

  /* Step 1: Allocate and initialize JPEG compression object */
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);

  /* Step 2: Specify the destination for the compressed data */
  if ((outfile = fopen(filename, "wb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    return;
  }
  jpeg_stdio_dest(&cinfo, outfile);

  /* Step 3: Set parameters for compression */
  cinfo.image_width = width;      /* image width and height, in pixels */
  cinfo.image_height = height;
  cinfo.input_components = 3;     /* # of color components per pixel */
  cinfo.in_color_space = JCS_RGB; /* colorspace of input image */

  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, 85, TRUE); /* set the quality [0..100] */

  /* Step 4: Start compressor */
  jpeg_start_compress(&cinfo, TRUE);

  /* Step 5: while (scan lines remain to be written) */
  row_stride = width * 3; /* JSAMPLEs per row in image_buffer */
  while (cinfo.next_scanline < cinfo.image_height) {
    row_pointer[0] = &rgb_data[cinfo.next_scanline * row_stride];
    jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  /* Step 6: Finish compression */
  jpeg_finish_compress(&cinfo);

  /* Step 7: Release JPEG compression object */
  fclose(outfile);
  jpeg_destroy_compress(&cinfo);
}

const char* get_frame_format_string(enum uvc_frame_format format) {
    switch (format) {
        case UVC_FRAME_FORMAT_UNKNOWN:
            return "UNKNOWN";
        case UVC_FRAME_FORMAT_UNCOMPRESSED:
            return "UNCOMPRESSED";
        case UVC_FRAME_FORMAT_COMPRESSED:
            return "COMPRESSED";
        case UVC_FRAME_FORMAT_YUYV:
            return "YUYV";
        case UVC_FRAME_FORMAT_UYVY:
            return "UYVY";
        case UVC_FRAME_FORMAT_RGB:
            return "RGB";
        case UVC_FRAME_FORMAT_BGR:
            return "BGR";
        case UVC_FRAME_FORMAT_MJPEG:
            return "MJPEG";
        case UVC_FRAME_FORMAT_H264:
            return "H264";
        case UVC_FRAME_FORMAT_GRAY8:
            return "GRAY8";
        case UVC_FRAME_FORMAT_GRAY16:
            return "GRAY16";
        case UVC_FRAME_FORMAT_BY8:
            return "BY8";
        case UVC_FRAME_FORMAT_BA81:
            return "BA81";
        case UVC_FRAME_FORMAT_SGRBG8:
            return "SGRBG8";
        case UVC_FRAME_FORMAT_SGBRG8:
            return "SGBRG8";
        case UVC_FRAME_FORMAT_SRGGB8:
            return "SRGGB8";
        case UVC_FRAME_FORMAT_SBGGR8:
            return "SBGGR8";
        case UVC_FRAME_FORMAT_NV12:
            return "NV12";
        case UVC_FRAME_FORMAT_P010:
            return "P010";
        default:
            return "UNKNOWN";
    }
}

const char* get_payload_error_string(payload_error_t error_code) {
    switch (error_code) {
        case PAYLOAD_ERROR_NONE:
            return "NONE";
        case PAYLOAD_ERROR_SMALL_HEADER_LENGTH:
            return "SMALL_HEADER_LENGTH";
        case PAYLOAD_ERROR_BIG_HEADER_LENGTH:
            return "BIG_HEADER_LENGTH";
        case PAYLOAD_ERROR_INVALID_HEADER_LENGTH:
            return "INVALID_HEADER_LENGTH";
        case PAYLOAD_ERROR_RESERVED_BIT_SET:
            return "RESERVED_BIT_SET";
        case PAYLOAD_ERROR_ERROR_BIT_SET:
            return "ERROR_BIT_SET";
        case PAYLOAD_ERROR_WRONG_END_OF_PACKET:
            return "WRONG_END_OF_PACKET";
        case PAYLOAD_ERROR_OVERFLOW:
            return "OVERFLOW";
        case PAYLOAD_ERROR_NO_ENDOFHEADER:
            return "NO_ENDOFHEADER";
        case PAYLOAD_ERROR_FRAME_ID_FLIPPED:
            return "FRAME_ID_FLIPPED";
        case PAYLOAD_ERROR_UNKNOWN:
            return "UNKNOWN";
        default:
            return "INVALID_ERROR_CODE";
    }
}

/* This callback function runs once per frame. Use it to perform any
 * quick processing you need, or have it put the frame into your application's
 * input queue. If this function takes too long, you'll start losing frames. */
void cb(uvc_frame_t *frame, void *ptr) {
  uvc_frame_t *bgr;
  uvc_error_t ret;

  static struct timeval start_time;
  if (start_time.tv_sec == 0 && start_time.tv_usec == 0) {
    gettimeofday(&start_time, NULL);
  }

  static int jpeg_count = 0;  // JPEG count

  /* FILE *fp;
   * static int jpeg_count = 0;
   * static const char *H264_FILE = "iOSDevLog.h264";
   * static const char *MJPEG_FILE = ".jpeg";
   * char filename[16]; */


  /* We'll convert the image from YUV/JPEG to BGR, so allocate space */
  bgr = uvc_allocate_frame(frame->width * frame->height * 3);
  if (!bgr) {
    printf("unable to allocate bgr frame!\n");
    return;
  }

  if (frame->error_code != PAYLOAD_ERROR_NONE){
    printf("cb: frame_format = %s, %d x %d, length = %lu, count = %d, frame_error = %s", //ptr = %p
      get_frame_format_string(frame->frame_format), 
      frame->width, frame->height, frame->data_bytes, 
      frame->sequence, 
      get_payload_error_string(frame->error_code));
      //, ptr

    switch (frame->frame_format) {
    case UVC_FRAME_FORMAT_H264:
      /* use `ffplay H264_FILE` to play */
      /* fp = fopen(H264_FILE, "a");
      * fwrite(frame->data, 1, frame->data_bytes, fp);
      * fclose(fp); */
      break;
    case UVC_COLOR_FORMAT_MJPEG:
      // sprintf(filename, "%d%s", jpeg_count++, MJPEG_FILE);
      // fp = fopen(filename, "w");
      // fwrite(frame->data, 1, frame->data_bytes, fp);
      // fclose(fp);
      // break;
    case UVC_COLOR_FORMAT_YUYV:
      /* Do the BGR conversion */

      ret = uvc_any2bgr(frame, bgr);
      if (ret) {
        uvc_perror(ret, "uvc_any2bgr");
        uvc_free_frame(bgr);
        return;
      }

      bgr_to_rgb(bgr->data, bgr->width, bgr->height);
      break;
    default:
      break;
    } 
  }

  {
    if (frame->error_code != PAYLOAD_ERROR_NONE) {
        char filename[64];

        // MJPEG 형식일 때 처리
        if (frame->frame_format == UVC_COLOR_FORMAT_MJPEG) {
            snprintf(filename, sizeof(filename), "frame_%d.jpeg", jpeg_count);
            FILE *fp = fopen(filename, "w");
            if (fp) {
                fwrite(frame->data, 1, frame->data_bytes, fp);  // MJPEG 데이터를 바로 저장
                fclose(fp);
                printf("MJPEG frame saved as: %s\n", filename);
            } else {
                printf("Error opening file: %s\n", filename);
            }
        }
        // YUYV 형식일 때 처리
        else if (frame->frame_format == UVC_COLOR_FORMAT_YUYV) {
            snprintf(filename, sizeof(filename), "frame_%d.jpeg", jpeg_count);
            save_bgr_to_jpeg(bgr->data, bgr->width, bgr->height, filename);  // YUYV 변환 후 저장
            printf("YUYV frame saved as: %s\n", filename);
        }

        jpeg_count++;
    }
  }

  printf("\n");

  // printf("frame error code : %d \n", frame->error_code);
  // // printf("packet time stamp : %d", frame->packet_timestamp);
  // frame->error_code = 0;

  if (frame->sequence % 30 == 0) {
    printf(" * got image %u\n",  frame->sequence);
    // Calculate and display the elapsed time in milliseconds
    struct timeval curr_time;
    gettimeofday(&curr_time, NULL);
    long elapsed_ms = (curr_time.tv_sec - start_time.tv_sec) * 1000;
    elapsed_ms += (curr_time.tv_usec - start_time.tv_usec) / 1000;

    printf(" * elapsed time: %ld ms\n", elapsed_ms);

  }
  frame->error_code = PAYLOAD_ERROR_NONE;

  uvc_free_frame(bgr);
}

int main(int argc, char **argv) {
  uvc_context_t *ctx;
  uvc_device_t *dev;
  uvc_device_handle_t *devh;
  uvc_stream_ctrl_t ctrl;
  uvc_error_t res;

  /* Initialize a UVC service context. Libuvc will set up its own libusb
   * context. Replace NULL with a libusb_context pointer to run libuvc
   * from an existing libusb context. */
  res = uvc_init(&ctx, NULL);

  if (res < 0) {
    uvc_perror(res, "uvc_init");
    return res;
  }

  puts("UVC initialized");

  /* Locates the first attached UVC device, stores in dev */
  res = uvc_find_device(
      ctx, &dev,
      0, 0, NULL); /* filter devices: vendor_id, product_id, "serial_num" */

  if (res < 0) {
    uvc_perror(res, "uvc_find_device"); /* no devices found */
  } else {
    puts("Device found");

    /* Try to open the device: requires exclusive access */
    res = uvc_open(dev, &devh);

    if (res < 0) {
      uvc_perror(res, "uvc_open"); /* unable to open device */
    } else {
      puts("Device opened");

      /* Print out a message containing all the information that libuvc
       * knows about the device */
      uvc_print_diag(devh, stderr);

      const uvc_format_desc_t *format_desc = uvc_get_format_descs(devh);
      const uvc_frame_desc_t *frame_desc = format_desc->frame_descs;
      enum uvc_frame_format frame_format = UVC_VS_FORMAT_MJPEG;
      int width = 1280;
      int height = 720;
      int fps = 30;

      switch (format_desc->bDescriptorSubtype) {
      case UVC_VS_FORMAT_MJPEG:
        frame_format = UVC_COLOR_FORMAT_MJPEG;
        break;
      case UVC_VS_FORMAT_FRAME_BASED:
        frame_format = UVC_FRAME_FORMAT_H264;
        break;
      default:
        frame_format = UVC_FRAME_FORMAT_YUYV;
        break;
      }

      // if (frame_desc) {
      //   width = frame_desc->wWidth;
      //   height = frame_desc->wHeight;
      //   fps = 10000000 / frame_desc->dwDefaultFrameInterval;
      // }

      printf("\nFirst format: (%4s) %dx%d %dfps\n", format_desc->fourccFormat, width, height, fps);

      /* Try to negotiate first stream profile */
      res = uvc_get_stream_ctrl_format_size(
          devh, &ctrl, /* result stored in ctrl */
          frame_format,
          width, height, fps /* width, height, fps */
      );

      // /* Print out the result */
      // uvc_print_stream_ctrl(&ctrl, stderr);

      if (res < 0) {
        uvc_perror(res, "get_mode"); /* device doesn't provide a matching stream */
      } else {
        /* Start the video stream. The library will call user function cb:
         *   cb(frame, (void *) 12345)
         */
        res = uvc_start_streaming(devh, &ctrl, cb, (void *) 12345, 0);

        if (res < 0) {
          uvc_perror(res, "start_streaming"); /* unable to start stream */
        } else {
          puts("Streaming...");

          /* enable auto exposure - see uvc_set_ae_mode documentation */
          puts("Enabling auto exposure ...");
          const uint8_t UVC_AUTO_EXPOSURE_MODE_AUTO = 2;
          res = uvc_set_ae_mode(devh, UVC_AUTO_EXPOSURE_MODE_AUTO);
          if (res == UVC_SUCCESS) {
            puts(" ... enabled auto exposure");
          } else if (res == UVC_ERROR_PIPE) {
            /* this error indicates that the camera does not support the full AE mode;
             * try again, using aperture priority mode (fixed aperture, variable exposure time) */
            puts(" ... full AE not supported, trying aperture priority mode");
            const uint8_t UVC_AUTO_EXPOSURE_MODE_APERTURE_PRIORITY = 8;
            res = uvc_set_ae_mode(devh, UVC_AUTO_EXPOSURE_MODE_APERTURE_PRIORITY);
            if (res < 0) {
              uvc_perror(res, " ... uvc_set_ae_mode failed to enable aperture priority mode");
            } else {
              puts(" ... enabled aperture priority auto exposure mode");
            }
          } else {
            uvc_perror(res, " ... uvc_set_ae_mode failed to enable auto exposure mode");
          }

          sleep(10); /* stream for 10 seconds */

          /* End the stream. Blocks until last callback is serviced */
          uvc_stop_streaming(devh);
          puts("Done streaming.");
        }
      }

      /* Release our handle on the device */
      uvc_close(devh);
      puts("Device closed");
    }

    /* Release the device descriptor */
    uvc_unref_device(dev);
  }

  /* Close the UVC context. This closes and cleans up any existing device handles,
   * and it closes the libusb context if one was not provided. */
  uvc_exit(ctx);
  puts("UVC exited");

  return 0;
}

