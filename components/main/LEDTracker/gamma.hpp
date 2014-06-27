extern uint8_t gamma_compress_lookup_table[255];

float gamma_compress(float p, int input_image_scale);
void init_gamma_compress_lookup_table();
void gamma_compress_in_place(cv::Mat &im); // If you're using OpenCV, this is what you want
