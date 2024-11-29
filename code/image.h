#ifndef IMAGE_H
#define IMAGE_H

#include "zf_common_headfile.h"

#define CAMERA 
#define IMAGE_WIDTH  MT9V03X_W
#define IMAGE_HEIGHT MT9V03X_H

extern char image_raw[IMAGE_HEIGHT][IMAGE_WIDTH];
extern char image_compressed[IMAGE_HEIGHT / 4][IMAGE_WIDTH / 4];
extern char image_binary[IMAGE_HEIGHT / 4][IMAGE_WIDTH / 4];
extern char image_processed[IMAGE_HEIGHT / 4][IMAGE_WIDTH / 4];
extern char track[IMAGE_HEIGHT / 4][IMAGE_WIDTH / 4];

void cam_init(void);
void cam_get_image(void);
void cam_compress_image(char image_raw[IMAGE_HEIGHT][IMAGE_WIDTH], char image_compressed[IMAGE_HEIGHT / 4][IMAGE_WIDTH / 4]);
void cam_bin_image(char image_compressed[IMAGE_HEIGHT / 4][IMAGE_WIDTH / 4], char image_binary[IMAGE_HEIGHT / 4][IMAGE_WIDTH / 4]);
int bin_map_trace_ccd(char image_processed[IMAGE_HEIGHT / 4][IMAGE_WIDTH / 4]);
int ccd_trace(void);

#endif // IMAGE_H
