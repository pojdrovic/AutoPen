typedef enum
{
	EL_NEUTRAL,
	EL_OK,
	EL_WARNING,
	EL_ERROR,
	EL_UNKNOWN
} elevel_t;

typedef struct
{
	std::string key, value;
	elevel_t el;
} eval_t;

typedef struct
{
	HPDF_RGBColor stroke, fill;
} stored_colors_t;

#define GRAPH_ASPECT_RATIO  (1.0 / (1.0 + 1.0 / 3.0))

HPDF_Page create_page(HPDF_Doc pdf, HPDF_Image logo_img, std::string title, HPDF_Font title_fnt, HPDF_Font text_fnt, double *offset_x, double *offset_y, double *w, double *h, int *page_nr, std::vector<eval_t> *page_index, bool portrait);
stored_colors_t push_colors(HPDF_Page page);
void pop_colors(HPDF_Page page, stored_colors_t sc);
