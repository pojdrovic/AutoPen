#define FFT_SAMPLE_RATE 120

void add_sensors_spectrum_heat_diagram(HPDF_Doc pdf, HPDF_Image logo_img, HPDF_Font fnt_title, HPDF_Font fnt_text, double font_height, std::string title, double *v, int n_v, int *page_nr, std::vector<eval_t> *page_index, bool portrait);
