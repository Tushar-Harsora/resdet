#include "image.h"

#include <mjpegtools/yuv4mpeg.h>

static unsigned char* read_y4m(FILE* f, size_t* width, size_t* height, size_t* nimages) {
	unsigned char* image = NULL,* discard = NULL;
	int fd = fileno(f);
	y4m_stream_info_t st;
	y4m_frame_info_t frame;
	y4m_init_stream_info(&st);
	y4m_init_frame_info(&frame);
	if(y4m_read_stream_header(fd,&st) != Y4M_OK)
		goto end;
	*width = y4m_si_get_width(&st);
	*height = y4m_si_get_height(&st);
	int frame_length = y4m_si_get_framelength(&st);
	if(!(*width && *height) || (*width > PIXEL_MAX / *height) || frame_length < 0 || (size_t)frame_length < *width * *height)
		goto end;
	frame_length -= *width * *height; // u/v plane skip
	discard = malloc(frame_length);
	while(y4m_read_frame_header(fd,&st,&frame) == Y4M_OK) {
		(*nimages)++;
		unsigned char* tmp;
		if((*width * *height > PIXEL_MAX / *nimages) ||
		   !(tmp = realloc(image,*width * *height * *nimages))) {
			free(image); image = NULL; break;
		}

		image = tmp;
		if(y4m_read(fd,image + *width * *height * (*nimages - 1),*width * *height) < 0 ||
		   read(fd,discard,frame_length) != frame_length) {
   			free(image); image = NULL; break;
		}
	}

end:
	free(discard);
	y4m_fini_frame_info(&frame);
	y4m_fini_stream_info(&st);
	return image;
}


struct image_reader resdet_image_reader_mjpegtools = {
	.read = read_y4m
};