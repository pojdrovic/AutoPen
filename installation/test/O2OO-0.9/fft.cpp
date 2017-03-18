#include <math.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <vector>
#include <fftw3.h>
#include <hpdf.h>

#include "error.h"
#include "main.h"
#include "main-evaluate.h"
#include "fft.h"

class fft
{
private:
	double *pin;
	fftw_complex *pout;
	fftw_plan plan;
	int sample_rate;

public:
	fft(int sample_rate_in, double *data_in);
	~fft();

	void do_fft(double *o);
};

fft::fft(int sample_rate_in, double *data)
{
	sample_rate = sample_rate_in;
	pin  = data;
	pout = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * sample_rate_in + 1);

	/* init fftw */
	plan = fftw_plan_dft_r2c_1d(sample_rate_in, pin, pout, FFTW_ESTIMATE);
	if (!plan)
		error_exit("failed calculating plan for fft");
}

fft::~fft()
{
	free(pin);
	fftw_free(pout);
}

void fft::do_fft(double *o)
{
	/* calc fft */
	fftw_execute(plan);

	for(int loop=0; loop<(sample_rate / 2) + 1; loop++)
	{
		double real = pout[loop][0];
		double img = pout[loop][1];
		double mag = sqrt(pow(real, 2.0) + pow(img, 2.0));

		o[loop] = mag;
	}
}

void add_sensors_spectrum_heat_diagram(HPDF_Doc pdf, HPDF_Image logo_img, HPDF_Font fnt_title, HPDF_Font fnt_text, double font_height, std::string title, double *v, int n_v, int *page_nr, std::vector<eval_t> *page_index, bool portrait)
{
	// create the heat diagram
	double offset_x = -1.0, offset_y = -1.0, w = -1.0, h = -1.0;
	HPDF_Page page = create_page(pdf, logo_img, title + " spectrum heat diagram", fnt_title, fnt_text, &offset_x, &offset_y, &w, &h, page_nr, page_index, true);

	stored_colors_t old_colors = push_colors(page);

	HPDF_Page_BeginText(page);
	HPDF_Page_SetFontAndSize(page, fnt_text, font_height);
	std::string text = "values (and so the colors) are normalized";
	double tw = HPDF_Page_TextWidth(page, text.c_str());
	HPDF_Page_TextOut(page, offset_x + w / 2.0 - tw / 2.0, offset_y + h - font_height, text.c_str());
	HPDF_Page_EndText(page);

	int h_rate = (FFT_SAMPLE_RATE / 2) + 1;
	int n_fft_out = (n_v + FFT_SAMPLE_RATE - 1) / FFT_SAMPLE_RATE;
	int n_bytes = sizeof(double) * h_rate * n_fft_out;
	double *fft_out = (double *)malloc(n_bytes);
	memset(fft_out, 0x00, n_bytes);

	int f_in_bytes = FFT_SAMPLE_RATE * sizeof(double);
	double *f_in = (double *)malloc(f_in_bytes);

	double block_w = w / double(n_fft_out);
	double block_h = (w * GRAPH_ASPECT_RATIO) / double(h_rate + 1);

	double diagram_x = offset_x;
	double diagram_y = offset_y + (h - w * GRAPH_ASPECT_RATIO) / 2.0;

	fft f(FFT_SAMPLE_RATE, f_in);

	int o_offset = 0;
	for(int index=0; index<n_v; index += FFT_SAMPLE_RATE, o_offset += h_rate)
	{
		memset(f_in, 0x00, f_in_bytes);
		memcpy(f_in, &v[index], std::min(FFT_SAMPLE_RATE, n_v - index) * sizeof(double));

		f.do_fft(&fft_out[o_offset]);
	}

	double mi = 999999999999.0, ma = -mi;
	for(int index=0; index<o_offset; index++)
	{
		if ((index % h_rate) == 0)
			continue;

		if (fft_out[index] < mi)
			mi = fft_out[index];

		if (fft_out[index] > ma)
			ma = fft_out[index];
	}

	for(int index=0; index<o_offset; index += h_rate)
	{
		double n_index = double(index) / double(h_rate);
		double cur_x  = diagram_x + block_w * n_index;

		for(int field=0; field<h_rate - 1; field++)
		{
			double cur_y  = diagram_y + block_h * (double(field) + 0.5);

			double value = (fft_out[index + field + 1] - mi) / (ma - mi);

			double start_r, start_g, start_b;
			double end_r, end_g, end_b;
			double sub;
			if (value < 0.25) // blue
			{
				start_r = start_g = start_b = 0.0;
				end_r = end_g = 0.0;
				end_b = 1.0;
				sub = 0.0;
			}
			else if (value < 0.50) // green
			{
				start_r = start_g = 0.0;
				start_b = 1.0;
				end_r = end_b = 0.0;
				end_g = 1.0;
				sub = 0.25;
			}
			else if (value < 0.75) // yellow
			{
				start_r = start_b = 0.0;
				start_g = 1.0;
				end_r = end_g = 1.0;
				end_b = 0.0;
				sub = 0.5;
			}
			else	// red
			{
				start_r = start_g = 1.0;
				start_b = 0.0;
				end_r = 1.0;
				end_g = end_b = 0.0;
				sub = 0.75;
			}

			double norm_val = (value - sub) * (1.0 / 0.25);

			double cur_r = start_r * (1.0 - norm_val) + end_r * norm_val;
			double cur_g = start_g * (1.0 - norm_val) + end_g * norm_val;
			double cur_b = start_b * (1.0 - norm_val) + end_b * norm_val;

			HPDF_Page_SetLineWidth(page, 1);
			HPDF_Page_SetRGBStroke(page, cur_r, cur_g, cur_b);
			HPDF_Page_SetRGBFill(page, cur_r, cur_g, cur_b);
			HPDF_Page_Rectangle(page, cur_x, cur_y, block_w, block_h);
			HPDF_Page_FillStroke(page);
		}
	}

	pop_colors(page, old_colors);
}
