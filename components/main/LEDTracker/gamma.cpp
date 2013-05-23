

uint8_t gamma_compress_lookup_table[255];

float gamma_compress(float p, int input_image_scale){
	float a = 0.055;
	float out = p;
	if (input_image_scale != 1){
		out = out / input_image_scale;
	}
	if(out <= 0.0031308){
		out = out * 12.92;
	}else{
		out = powf(out, 1/2.4)*(1+a)-a;
	}
	if (input_image_scale != 1){
		out = out * input_image_scale;
	}
	return out;
}

void init_gamma_compress_lookup_table()
{
	for(int i=0; i<256; i++)
	{
		gamma_compress_lookup_table[i] = gamma_compress(i,255);
	}
}
