// SVN: $Revision: 442 $
typedef struct
{
	double mi, av, ma;
} value_t;

class graph_pdf
{
protected:
	void calc_text_width(HPDF_Page page, HPDF_Font font, double font_height, std::string str, double *width, double *height);
	void draw_text(HPDF_Page page, HPDF_Font font, double font_height, double r, double g, double b, std::string str, double x, double y);
	void draw_line(HPDF_Page page, double x1, double y1, double x2, double y2, double r, double g, double b);
	void draw_dot(HPDF_Page page, double x, double y, double radius, double r, double g, double b);
	void reset_pdf(HPDF_Page page);

public:
	graph_pdf();
	virtual ~graph_pdf();

	void do_draw(HPDF_Page page, HPDF_Font font, double x, double y, double width, double height, long int *t, double *v, int n_values);
	void do_draw(HPDF_Page page, HPDF_Font font, double x_off, double y_off, double width, double height, long int *ts, double *values1, double *values2, int n_values, std::string metaStr);
	void do_draw_generic(HPDF_Page page, HPDF_Font font, double x_off, double y_off, double width, double height, double *val_x, value_t *val_y, bool use_mima, int n_values, bool draw_avg, bool draw_sd, bool draw_ls);
	void do_draw_point_cloud(HPDF_Page page, HPDF_Font font, double x_off, double y_off, double width, double height, double *val_x, double *val_y, int n_values, bool draw_avg, bool draw_sd, bool draw_ls);
	void do_draw_histogram(HPDF_Page page, HPDF_Font font, double x_off, double y_off, double width, double height, std::vector<std::string> *labels_x, double *val_y, int n_values, std::string meta);
};
