// SVN: $Revision: 469 $
#include <errno.h>
#include <iostream>
#include <map>
#include <math.h>
#include <pwd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <hpdf.h>
#include <sqlite3.h>
#include <tinyxml2.h>
#include <curl/curl.h>

#include "error.h"
#include "utils.h"
#include "utils-sp.h"
#include "logo.h"
#include "graph_pdf.h"
#include "main.h"
#include "main-evaluate.h"
#include "fft.h"

#define EVAL_OK_STR "all fine"

#ifdef __GNUG__
	#define COMPILER_BRAND "GNU C++"
#else
	#define COMPILER_BRAND ""
#endif

std::string sensor_maf_air_flow_rate = "sensor_maf_air_flow_rate";
std::string sensor_vehicle_speed = "sensor_vehicle_speed";
std::string sensor_barometric_pressure = "sensor_barometric_pressure";
std::string sensor_ambient_air_temperature = "sensor_ambient_air_temperature";
std::string sensor_engine_rpm = "sensor_engine_rpm";
std::string sensor_throttle = "sensor_throttle";
std::string sensor_intake_air_temperature = "sensor_intake_air_temperature";
std::string sensor_short_term_fuel_bank1 = "sensor_short_term_fuel_bank1";
std::string sensor_short_term_fuel_bank2 = "sensor_short_term_fuel_bank2";
std::string sensor_engine_coolant_temperature = "sensor_engine_coolant_temperature";
std::string gps_location = "gps_location";
//
std::string location_lookup_table = "gps_location_lookup";

bool verbose = false;

void push_back(std::vector<eval_t> *v, std::string key, std::string value, elevel_t el)
{
	eval_t e;
	e.key = key;
	e.value = value;
	e.el = el;

	v -> push_back(e);
}

std::vector<std::vector<std::string> > * retrieve_sensor_data(sqlite3 *db, std::string sensor)
{
	std::string query = std::string("SELECT ts, value FROM ") + sensor + " ORDER BY nr";

	std::vector<std::vector<std::string> > *results = new std::vector<std::vector<std::string> >();
	char *error = NULL;

	if (sqlite3_exec(db, query.c_str(), sl_callback, (void *)results, &error))
		error_exit("DB error: %s for query %s", error, query.c_str());

	return results;
}

HPDF_Font load_font(HPDF_Doc pdf, std::string font_file)
{
	const char *fnt_name = HPDF_LoadTTFontFromFile(pdf, font_file.c_str(), HPDF_TRUE);
	if (!fnt_name)
		error_exit("Failed to load font %s", font_file.c_str());

	HPDF_Font fnt = HPDF_GetFont(pdf, fnt_name, NULL);
	if (!fnt)
		error_exit("Failed to get font %s", fnt_name);

	return fnt;
}

HPDF_Page create_page(HPDF_Doc pdf, HPDF_Image logo_img, std::string title, HPDF_Font title_fnt, HPDF_Font text_fnt, double *offset_x, double *offset_y, double *w, double *h, int *page_nr, std::vector<eval_t> *page_index, bool portrait)
{
	if (verbose)
		std::cout << "Creating page " << title << " (" << *page_nr << ")" << std::endl;

	HPDF_Page page = HPDF_AddPage(pdf); 
	if (!page)
		error_exit("Failed to add new page to PDF file %04x", HPDF_GetError(pdf));

	if (HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, portrait ? HPDF_PAGE_PORTRAIT : HPDF_PAGE_LANDSCAPE)) 
		error_exit("Failed to set page size of PDF file");

	HPDF_REAL width = HPDF_Page_GetWidth(page);
	HPDF_REAL height = HPDF_Page_GetHeight(page);

	HPDF_Point img_size = HPDF_Image_GetSize(logo_img);
	img_size.x /= 4.0;
	img_size.y /= 4.0;

	*offset_x = 50.0;
	*offset_y = 50.0;

	*w = width - 100.0;

	double draw_w = *w < img_size.x ? *w : img_size.x;
	double w_factor = draw_w / img_size.x;
	double draw_h = img_size.y * w_factor;

	double logo_y = height - (draw_h + 50 /* 60.0? */);
	if (HPDF_Page_DrawImage(page, logo_img, 50.0, logo_y, draw_w, draw_h))
		error_exit("Failed adding logo to page");

	if (HPDF_Page_BeginText(page))
		error_exit("Failed HPDF_Page_BeginText");

	// draw title
	double title_font_height = 32.0;
	double title_w_margin = *w * 0.05;
	HPDF_REAL tw = -1.0;
	for(;;)
	{
		if (HPDF_Page_SetFontAndSize(page, title_fnt, title_font_height))
			error_exit("HPDF_Page_SetFontAndSize failed");

		tw = HPDF_Page_TextWidth(page, title.c_str());

		if (tw < *w - title_w_margin)
			break;

		title_font_height -= 0.1;
	}
	double title_pos_x = *offset_x + *w / 2.0 - tw / 2.0;
	double title_pos_y = height - (draw_h + 50.0 + title_font_height + 5.0 /* margin above font */);
	HPDF_Page_TextOut(page, title_pos_x, title_pos_y, title.c_str());
	// draw (c)
	double spam_height = img_size.y / 6.0, page_nr_height = spam_height;
	HPDF_Page_SetFontAndSize(page, text_fnt, spam_height);
	std::string copyright = "O2OO v" VERSION ", (C) folkert@vanheusden.com";
	HPDF_REAL cw = HPDF_Page_TextWidth(page, copyright.c_str());
	HPDF_Page_TextOut(page, *offset_x + *w - (cw + 0.5), logo_y + 3.0, copyright.c_str());

	std::string url = "http://www.vanheusden.com/O2OO/";
	HPDF_REAL uw = HPDF_Page_TextWidth(page, url.c_str());
	HPDF_Page_TextOut(page, *offset_x + *w / 2.0 - uw / 2.0, *offset_y - spam_height, url.c_str());

	std::string page_nr_str = format("%3d", *page_nr);
	HPDF_REAL pnw = HPDF_Page_TextWidth(page, page_nr_str.c_str());
	HPDF_Page_TextOut(page, *offset_x + *w - pnw, *offset_y - page_nr_height, page_nr_str.c_str());

	HPDF_Page_EndText(page);

	double text_h = height - (*offset_y + draw_h + 50.0); // 60.0?

	HPDF_Page_SetLineWidth(page, 1);
	HPDF_Page_Rectangle(page, *offset_x, *offset_y, *w, text_h);
	HPDF_Page_Stroke(page);

	*h = height - (*offset_y + draw_h + 50.0 + title_font_height + 5.0 * 4.0); // 60.0?

	if (page_index)
		push_back(page_index, page_nr_str, title, EL_NEUTRAL);

	(*page_nr)++;

	return page;
}

std::vector<std::pair<std::string, std::string> > map_to_vector(std::map<std::string, std::string> *table)
{
	std::vector<std::pair<std::string, std::string> > vmap;

	std::map<std::string, std::string>::iterator it = table -> begin();
        for(;it != table -> end(); it++)
		vmap.push_back(*it);

	return vmap;
}

void find_table_dimensions(HPDF_Page page, std::vector<eval_t> *table, double *w_key, double *w_value, int *n_lines)
{
	double max_key_width = -1, max_value_width = -1;

	*n_lines = 0;

	for(unsigned int index=0; index<table -> size(); index++)
	{
		std::string key = table -> at(index).key;
		std::string value = table -> at(index).value;

		std::vector<std::string> key_parts = split_string(key, "\n");

		for(unsigned int index=0; index<key_parts.size(); index++)
		{
			double kw = HPDF_Page_TextWidth(page, key_parts.at(index).c_str());
			if (kw > max_key_width)
				max_key_width = kw;
		}

		std::vector<std::string> value_parts = split_string(value, "\n");

		for(unsigned int index=0; index<value_parts.size(); index++)
		{
			double vw = HPDF_Page_TextWidth(page, value_parts.at(index).c_str());
			if (vw > max_value_width)
				max_value_width = vw;
		}

		int max_n_lines = std::max(key_parts.size(), value_parts.size());

		*n_lines += max_n_lines;
	}

	*w_key = max_key_width;
	*w_value = max_value_width;
}

stored_colors_t push_colors(HPDF_Page page)
{
	stored_colors_t sc = { HPDF_Page_GetRGBStroke(page), HPDF_Page_GetRGBFill(page) };

	return sc;
}

void pop_colors(HPDF_Page page, stored_colors_t sc)
{
	HPDF_Page_SetRGBStroke(page, sc.stroke.r, sc.stroke.g, sc.stroke.b);
	HPDF_Page_SetRGBFill(page, sc.fill.r, sc.fill.g, sc.fill.b);
}

void draw_elevel_color_box(HPDF_Page page, double x1, double y1, double x2, double y2, elevel_t el)
{
	if (el == EL_NEUTRAL)
		return;

	double r = -1.0, g = -1.0, b = -1.0;
	if (el == EL_OK)
	{
		r = b = double(0x83) / double(0xff);
		g = 1.0;
	}
	else if (el == EL_WARNING)
	{
		b = double(0x83) / double(0xff);
		r = g = 1.0;
	}
	else if (el == EL_ERROR)
	{
		g = b = double(0x83) / double(0xff);
		r = 1.0;
	}
	else if (el == EL_UNKNOWN)
	{
		g = double(0xf6) / double(0xff);
		r = b = 1.0;
	}

	stored_colors_t old_colors = push_colors(page);

	HPDF_Page_SetLineWidth(page, 1);
	HPDF_Page_SetRGBStroke(page, r, g, b);
	HPDF_Page_SetRGBFill(page, r, g, b);
	HPDF_Page_Rectangle(page, x1, y1, x2 - x1, y2 - y1);
	HPDF_Page_FillStroke(page);

	pop_colors(page, old_colors);
}

// draws downwards
double draw_table(HPDF_Doc pdf, HPDF_Page page, HPDF_Font text_fnt, double text_fnt_height, std::vector<eval_t> *table, double x_offset, double y_offset)
{
	// find maximum width of all keys
	int n_table_lines = -1;
	double max_key_width = -1, max_value_width = -1;
	find_table_dimensions(page, table, &max_key_width, &max_value_width, &n_table_lines);

	HPDF_Page_BeginText(page);
	HPDF_Page_SetFontAndSize(page, text_fnt, text_fnt_height);
	double divider_w = HPDF_Page_TextWidth(page, ": ");
	HPDF_Page_EndText(page);

	// draw
	int line_nr = 1;
	for(unsigned int index=0; index<table -> size(); index++)
	{
		std::string key = table -> at(index).key;
		std::string value = table -> at(index).value;

		std::vector<std::string> key_parts = split_string(key, "\n");
		std::vector<std::string> value_parts = split_string(value, "\n");

		int n_max_n_lines = std::max(key_parts.size(), value_parts.size());

		draw_elevel_color_box(page, x_offset, (y_offset - (line_nr - 1) * text_fnt_height) - 1, x_offset + max_key_width, (y_offset - (line_nr + key_parts.size() - 1) * text_fnt_height) - 1, table -> at(index).el);

		for(unsigned int cur_offset = 0; cur_offset < key_parts.size(); cur_offset++)
		{
			HPDF_Page_BeginText(page);
			HPDF_Page_SetFontAndSize(page, text_fnt, text_fnt_height);
			HPDF_Page_TextOut(page, x_offset, y_offset - (line_nr + cur_offset) * text_fnt_height, key_parts.at(cur_offset).c_str());
			HPDF_Page_EndText(page);
		}

		draw_elevel_color_box(page, x_offset + max_key_width, (y_offset - (line_nr  - 1) * text_fnt_height) - 1, x_offset + max_key_width + max_value_width + divider_w, (y_offset - (line_nr + value_parts.size() - 1) * text_fnt_height) - 1, table -> at(index).el);

		for(unsigned int cur_offset = 0; cur_offset < value_parts.size(); cur_offset++)
		{
			HPDF_Page_BeginText(page);
			HPDF_Page_SetFontAndSize(page, text_fnt, text_fnt_height);
			HPDF_Page_TextOut(page, x_offset + max_key_width, y_offset - (line_nr + cur_offset) * text_fnt_height, (": " + value_parts.at(cur_offset)).c_str());
			HPDF_Page_EndText(page);
		}

		line_nr += n_max_n_lines;
	}

	return double(line_nr) * text_fnt_height;
}

double draw_table(HPDF_Doc pdf, HPDF_Page page, HPDF_Font text_fnt, double text_fnt_height, std::vector<std::pair<std::string, std::string> > *table, double x_offset, double y_offset)
{
	std::vector<eval_t> temp;

	for(unsigned int index=0; index<table -> size(); index++)
	{
		eval_t e;
		e.key = table -> at(index).first;
		e.value = table -> at(index).second;
		e.el = EL_NEUTRAL;

		temp.push_back(e);
	}

	return draw_table(pdf, page, text_fnt, text_fnt_height, &temp, x_offset, y_offset);
}

// returns table height
double draw_table(HPDF_Doc pdf, HPDF_Page page, HPDF_Font text_fnt, double text_fnt_height, std::map<std::string, std::string> *table, double x_offset, double y_offset)
{
	std::vector<std::pair<std::string, std::string> > vmap = map_to_vector(table);

	return draw_table(pdf, page, text_fnt, text_fnt_height, &vmap, x_offset, y_offset);
}

typedef struct
{
	char *p;
	int len;
} curl_data_t;

size_t curl_write_data(void *ptr, size_t size, size_t nmemb, void *ctx)
{
	curl_data_t *pctx = (curl_data_t *)ctx;

	int n = size * nmemb;
	pctx -> p = (char *)realloc(pctx -> p, pctx -> len + n + 1);
	memcpy(&pctx -> p[pctx -> len], ptr, n);
	pctx -> len += n;

	if (verbose)
		std::cout << "HTTP: " << format("%06d", pctx -> len) << "\r";

	pctx -> p[pctx -> len] = 0x00;

	return n;
}

std::string get_data_from_url(std::string url)
{
	CURL *ch = curl_easy_init();

	char error[CURL_ERROR_SIZE] = "?";
	curl_easy_setopt(ch, CURLOPT_ERRORBUFFER, error);

	std::string useragent="O2OO v" VERSION ", (C) folkert@vanheusden.com";
	if (curl_easy_setopt(ch, CURLOPT_USERAGENT, useragent.c_str()))
		error_exit("curl_easy_setopt(CURLOPT_USERAGENT) failed: %s", error);

	if (curl_easy_setopt(ch, CURLOPT_URL, url.c_str()))
		error_exit("curl_easy_setopt(CURLOPT_URL) failed: %s", error);

	long timeout = 15;
	if (curl_easy_setopt(ch, CURLOPT_CONNECTTIMEOUT, timeout))
		error_exit("curl_easy_setopt(CURLOPT_CONNECTTIMEOUT) failed: %s", error);

	if (curl_easy_setopt(ch, CURLOPT_WRITEFUNCTION, curl_write_data))
		error_exit("curl_easy_setopt(CURLOPT_WRITEFUNCTION) failed: %s", error);

	curl_data_t data = { NULL, 0 };
	if (curl_easy_setopt(ch, CURLOPT_WRITEDATA, &data))
		error_exit("curl_easy_setopt(CURLOPT_WRITEDATA) failed: %s", error);

	if (curl_easy_perform(ch))
		error_exit("curl_easy_perform() failed: %s", error);

	curl_easy_cleanup(ch);

	std::string result = std::string(data.p);
	free(data.p);

	return result;
}

bool retrieve_google_maps(double cur_lo, double cur_la, std::string *result, double *lo1, double *la1, double *lo2, double *la2)
{
	tinyxml2::XMLDocument xd;

	std::string url = "http://maps.googleapis.com/maps/api/geocode/xml?latlng=" + format("%f", cur_la) + "," + format("%f", cur_lo) + "&sensor=true";
	std::string google_maps_data = get_data_from_url(url);

	xd.Parse(google_maps_data.c_str());

	tinyxml2::XMLNode* tree = xd.RootElement();
	if(tree)
	{
		tinyxml2::XMLElement* element_status = tree -> FirstChildElement("status");
		if (verbose && element_status)
			std::cerr << "Google Maps status: " << element_status -> GetText() << std::endl;

		for(tinyxml2::XMLNode* child = tree->FirstChild(); child; child = child->NextSibling())
		{
			if (strcmp(child -> Value(), "result") != 0)
				continue;

			tinyxml2::XMLElement* element_type = child -> FirstChildElement("type");
			if (!element_type)
				continue;

			if (strcmp(element_type -> GetText(), "street_address") != 0)
				continue;

			tinyxml2::XMLElement* element_addr = child -> FirstChildElement("formatted_address");
			std::string descr = element_addr -> GetText();

			tinyxml2::XMLElement* element_geo = child -> FirstChildElement("geometry");
			if (!element_geo)
				continue;

			tinyxml2::XMLElement* element_vp = element_geo -> FirstChildElement("viewport");
			if (!element_vp)
				continue;

			tinyxml2::XMLElement* element_sw = element_vp -> FirstChildElement("southwest");
			if (!element_sw)
				continue;

			tinyxml2::XMLElement* element_sw_la = element_sw -> FirstChildElement("lat");
			if (!element_sw_la)
				continue;
			double sw_la = atof(element_sw_la -> GetText());
			tinyxml2::XMLElement* element_sw_lo = element_sw -> FirstChildElement("lng");
			if (!element_sw_lo)
				continue;
			double sw_lo = atof(element_sw_lo -> GetText());

			tinyxml2::XMLElement* element_ne = element_vp -> FirstChildElement("northeast");
			if (!element_ne)
				continue;

			tinyxml2::XMLElement* element_ne_la = element_ne -> FirstChildElement("lat");
			if (!element_ne_la)
				continue;
			double ne_la = atof(element_ne_la -> GetText());
			tinyxml2::XMLElement* element_ne_lo = element_ne -> FirstChildElement("lng");
			if (!element_ne_lo)
				continue;
			double ne_lo = atof(element_ne_lo -> GetText());

			result -> assign(descr);

			*lo1 = std::min(sw_lo, ne_lo);
			*la1 = std::min(sw_la, ne_la);
			*lo2 = std::max(sw_lo, ne_lo);
			*la2 = std::max(sw_la, ne_la);

			return true;
		}
	}

	return false;
}

std::string retrieve_location(sqlite3 *db, double cur_lo, double cur_la)
{
	if (!check_table_exists(db, location_lookup_table))
	{
		std::string query = std::string("CREATE TABLE ") + location_lookup_table + "(lo1 DOUBLE NOT NULL, la1 DOUBLE NOT NULL, lo2 DOUBLE NOT NULL, la2 DOUBLE NOT NULL, description TEXT NOT NULL)";

		char *error = NULL;
		if (sqlite3_exec(db, query.c_str(), NULL, NULL, &error))
			error_exit("DB error: %s", error);
	}

	std::string cur_lo_str = format("%.8f", cur_lo);
	std::string cur_la_str = format("%.8f", cur_la);

	std::string query = std::string("SELECT description FROM ") + location_lookup_table + " WHERE " + cur_lo_str + " >= lo1 AND " + cur_lo_str + " <= lo2 AND " + cur_la_str + " >= la1 AND " + cur_la_str + " <= LA2";

	std::vector<std::vector<std::string> > results;
	char *error = NULL;

	if (sqlite3_exec(db, query.c_str(), sl_callback, (void *)&results, &error))
		error_exit("DB error: %s for query %s", error, query.c_str());

	std::string descr;
	if (results.empty()) // location not found, ask google
	{
		double lo1, la1, lo2, la2;
		if (retrieve_google_maps(cur_lo, cur_la, &descr, &lo1, &la1, &lo2, &la2))
		{
			std::string query_insert = std::string("INSERT INTO ") + location_lookup_table + "(lo1, la1, lo2, la2, description) VALUES(" + format("%f", lo1) + ", " + format("%f", la1) + ", " + format("%f", lo2) + ", " + format("%f", la2) + ", '" + descr + "')";

			char *error = NULL;
			if (sqlite3_exec(db, query_insert.c_str(), NULL, NULL, &error))
				error_exit("DB error: %s", error);
		}
		else if (verbose)
		{
			std::cerr << "Google Maps did not return a location for " << cur_la << "," << cur_lo << std::endl;
		}
	}
	else
	{
		descr = results.at(0).at(0);
	}

	return descr;
}

bool load_gps_map(sqlite3 *db)
{
	std::string query = "SELECT MIN(latitude), MIN(longitude), MAX(latitude), MAX(longitude) FROM " GPS_TABLE;

	std::vector<std::vector<std::string> > results;

	char *error = NULL;
	if (sqlite3_exec(db, query.c_str(), sl_callback, (void *)&results, &error))
		error_exit("DB error: %s", error);

	// request map data from openstreetmap
	std::vector<std::string> row = results.at(0);
	std::string url = format("http://overpass-api.de/api/interpreter?data=(node(%s,%s,%s,%s);<;);out;",
			row.at(0).c_str(), row.at(1).c_str(), row.at(2).c_str(), row.at(3).c_str());

	if (verbose)
		std::cout << "Requesting " << url << std::endl;

	std::string osm_data = get_data_from_url(url);

	if (verbose)
		std::cout << "Parsing response data..." << std::endl;

	tinyxml2::XMLDocument xd;
	xd.Parse(osm_data.c_str());

	tinyxml2::XMLNode* tree = xd.RootElement();
	if (!tree)
	{
		if (verbose)
			std::cout << "No OSM data" << std::endl;

		return false;
	}

	if (verbose)
		std::cout << "Inserting data into database..." << std::endl;

	start_transaction(db);

	// at this point we somewhat know for sure we got the data and that it is valid
	// so insert it into the dabase
	std::string query_make_nodes = "CREATE TABLE " GPS_MAP_NODES "(id INTEGER, longitude DOUBLE, latitude DOUBLE)";
	if (sqlite3_exec(db, query_make_nodes.c_str(), NULL, NULL, &error))
		error_exit("DB error: %s", error);
	std::string query_make_lines = "CREATE TABLE " GPS_MAP_LINES "(ref1 INTEGER, ref2 INTEGER)";
	if (sqlite3_exec(db, query_make_lines.c_str(), NULL, NULL, &error))
		error_exit("DB error: %s", error);
	std::string query_make_poly = "CREATE TABLE " GPS_MAP_POLY "(id INTEGER, type TEXT, ref INTEGER, idx INTEGER)";
	if (sqlite3_exec(db, query_make_poly.c_str(), NULL, NULL, &error))
		error_exit("DB error: %s", error);

	for(tinyxml2::XMLNode* child = tree->FirstChild(); child; child = child->NextSibling())
	{
		if (strcasecmp(child -> Value(), "node") == 0)
		{
			tinyxml2::XMLElement* element = child -> ToElement();

			std::string query_insert = std::string("INSERT INTO ") + GPS_MAP_NODES + format("(id, latitude, longitude) VALUES(%s, %s, %s)", element -> Attribute("id"), element -> Attribute("lat"), element -> Attribute("lon"));
			if (sqlite3_exec(db, query_insert.c_str(), NULL, NULL, &error))
				error_exit("DB error: %s", error);
		}
		else if (strcasecmp(child -> Value(), "way") == 0)
		{
			tinyxml2::XMLElement* element = child -> ToElement();
			const char *way_id = element -> Attribute("id");

			// filter to only ways with tag with key "highway"
			tinyxml2::XMLElement * tag = child -> FirstChildElement("tag");
			bool is_highway = false, is_polygon = false;
			std::string type;
			for(;tag;)
			{
				const char *cur_k = tag -> Attribute("k");

				if (cur_k != NULL)
				{
					std::string cur_v = tag -> Attribute("v");

					if (strcasecmp(cur_k, "highway") == 0)
					{
						is_highway = true;
						break;
					}

					if (strcasecmp(cur_k, "building") == 0 || strcasecmp(cur_k, "landuse") == 0)
					{
						if (strcasecmp(cur_k, "building") == 0)
							type = "building";
						else if (cur_v == "grass" || cur_v == "village_green")
							type = "green";
						else if (cur_v == "forest" || cur_v == "farm" || cur_v == "village_green" || cur_v == "orchard" || cur_v == "meadow")
							type = "forest";

						is_polygon = true;

						break;
					}
				}

				tag = tag -> NextSiblingElement();
			}

			// load location nodes (x/y data, lines)
			tinyxml2::XMLElement * nd = child -> FirstChildElement("nd");

			if (is_highway)
			{
				const char *prev_id = NULL;

				for(;nd;)
				{
					const char *cur_id = nd -> Attribute("ref");
					if (cur_id != NULL)
					{
						if (prev_id != NULL)
						{
							std::string query_insert = std::string("INSERT INTO ") + GPS_MAP_LINES + format("(ref1, ref2) VALUES(%s, %s)", prev_id, cur_id);
							if (sqlite3_exec(db, query_insert.c_str(), NULL, NULL, &error))
								error_exit("DB error: %s", error);
						}

						prev_id = cur_id;
					}

					nd = nd -> NextSiblingElement();
				}
			}
			else if (is_polygon)
			{
				int idx = 0;

				for(;nd;)
				{
					const char *cur_ref = nd -> Attribute("ref");
					if (cur_ref != NULL)
					{
						std::string query_insert = std::string("INSERT INTO ") + GPS_MAP_POLY + format("(id, type, ref, idx) VALUES(%s, '%s', %s, %d)", way_id, type.c_str(), cur_ref, idx++);
						if (sqlite3_exec(db, query_insert.c_str(), NULL, NULL, &error))
							error_exit("DB error: %s", error);
					}

					nd = nd -> NextSiblingElement();
				}
			}
		}
	}

	commit_transaction(db);

	if (verbose)
		std::cout << "Finished loading map data to database" << std::endl;

	return true;
}

void draw_map_gps_sensor(sqlite3 *db, HPDF_Doc pdf, HPDF_Page page, double offset_x, double offset_y, double w, double h, std::string sensor)
{
	std::string query_meta = "SELECT MIN(latitude), MIN(longitude), MAX(latitude), MAX(longitude) FROM (SELECT latitude, longitude FROM " GPS_TABLE " UNION SELECT latitude, longitude FROM " GPS_MAP_NODES ") AS tmp";
	std::vector<std::vector<std::string> > results_meta;
	char *error = NULL;
	if (sqlite3_exec(db, query_meta.c_str(), sl_callback, (void *)&results_meta, &error))
		error_exit("DB error: %s", error);

	double mi_lo = atof(results_meta.at(0).at(1).c_str());
	double ma_lo = atof(results_meta.at(0).at(3).c_str());
	double mi_la = atof(results_meta.at(0).at(0).c_str());
	double ma_la = atof(results_meta.at(0).at(2).c_str());

	// ** draw map **
	// draw roads
	std::string query_graph = "SELECT n1.longitude AS n1_lo, n1.latitude AS n1_la, n2.longitude AS n2_lo, n2.latitude AS n2_la FROM " GPS_MAP_NODES " AS n1, " GPS_MAP_NODES " AS n2, " GPS_MAP_LINES " WHERE " GPS_MAP_LINES ".ref1=n1.id AND " GPS_MAP_LINES ".ref2=n2.id";
	std::vector<std::vector<std::string> > results_graph;
	if (sqlite3_exec(db, query_graph.c_str(), sl_callback, (void *)&results_graph, &error))
		error_exit("DB error: %s", error);

        if (HPDF_Page_SetLineWidth(page, 0.1))
                error_exit("HPDF_Page_SetLineWidth failed");

	stored_colors_t old_colors = push_colors(page);

        if (HPDF_Page_SetRGBStroke(page, double(0x4f) / 255.0, double(0x4f) / 255.0, double(0x4f) / 255.0))
                error_exit("HPDF_Page_SetRGBStroke() failed");

	// need to have same scale for longitude as well as latitude
	double lo_div = ma_lo - mi_lo;
	double la_div = ma_la - mi_la;
	double div = std::max(lo_div, la_div) * 1.01;
	// then we also need to adjust it to the center
	double dummy_w = w * (ma_lo - mi_lo) / div;
	double extra_offset_x = (w - dummy_w) / 2.0;
	double dummy_h = h * (ma_la - mi_la) / div;
	double extra_offset_y = (h - dummy_h) / 2.0;

	for(unsigned int index=0; index<results_graph.size(); index++)
	{
		std::vector<std::string> *row = &results_graph.at(index);
		double lo1 = atof(row -> at(0).c_str());
		double la1 = atof(row -> at(1).c_str());
		double lo2 = atof(row -> at(2).c_str());
		double la2 = atof(row -> at(3).c_str());

		double x1 = w * (lo1 - mi_lo) / div + offset_x + extra_offset_x;
		double y1 = h * (la1 - mi_la) / div + offset_y + extra_offset_y;
		double x2 = w * (lo2 - mi_lo) / div + offset_x + extra_offset_x;
		double y2 = h * (la2 - mi_la) / div + offset_y + extra_offset_y;

		HPDF_Page_MoveTo(page, x1, y1);
		HPDF_Page_LineTo(page, x2, y2);
	}

	if (HPDF_Page_Stroke(page))
		error_exit("HPDF_Page_Stroke failed");

	// draw buildings / green
	std::string query_poly = "SELECT " GPS_MAP_POLY ".id, type, idx, longitude, latitude FROM " GPS_MAP_POLY ", " GPS_MAP_NODES " WHERE " GPS_MAP_NODES ".id=" GPS_MAP_POLY ".ref ORDER BY " GPS_MAP_POLY ".id, type, idx";
	std::vector<std::vector<std::string> > results_poly;
	if (sqlite3_exec(db, query_poly.c_str(), sl_callback, (void *)&results_poly, &error))
		error_exit("DB error: %s", error);

        if (HPDF_Page_SetLineWidth(page, 0.1))
                error_exit("HPDF_Page_SetLineWidth failed");

	std::string prev_id, prev_type;
	bool ignore = true;
	for(unsigned int index=0; index<results_poly.size(); index++)
	{
		std::vector<std::string> *row = &results_poly.at(index);

		bool first = false;
		if (row -> at(0) != prev_id)
		{
			// finish old polygon
			if (prev_id != "" && !ignore)
			{
				if (HPDF_Page_Fill(page))
					error_exit("HPDF_Page_Fill failed");
			}

			// start new polygon
			ignore = true;
			if (row -> at(1) == "green")
			{
				if (HPDF_Page_SetRGBFill(page, 150.0/255.0, 1.0, 150.0/255.0))
					error_exit("HPDF_Page_SetRGBFill() failed");
				ignore = false;
			}
			else if (row -> at(1) == "forest")
			{
				if (HPDF_Page_SetRGBFill(page, 1.0, 216.0 / 255.0, 169.0 / 255.0))
					error_exit("HPDF_Page_SetRGBFill() failed");
				ignore = false;
			}

			prev_id = row -> at(0);
			prev_type = row -> at(1);

			first = true;
		}

		if (!ignore)
		{
			double lo = atof(row -> at(3).c_str());
			double la = atof(row -> at(4).c_str());
			double x = w * (lo - mi_lo) / div + offset_x + extra_offset_x;
			double y = h * (la - mi_la) / div + offset_y + extra_offset_y;

			if (first)
			{
				HPDF_Page_MoveTo(page, x, y);
				first = false;
			}
			else
			{
				HPDF_Page_LineTo(page, x, y);
			}
		}
	}

	if (!ignore)
	{
		if (HPDF_Page_Fill(page))
			error_exit("HPDF_Page_Fill failed");
	}

	// ** draw route driven **
	double v_mi = 0.0, v_ma = 0.0;
	find_data_bw(db, sensor, &v_mi, &v_ma);

	std::string query_route = "SELECT " GPS_TABLE ".ts, longitude, latitude, value FROM " + sensor + ", " GPS_TABLE " WHERE " + sensor + ".ts = " GPS_TABLE ".ts ORDER BY " GPS_TABLE ".nr ASC";
	std::vector<std::vector<std::string> > results_route;
	if (sqlite3_exec(db, query_route.c_str(), sl_callback, (void *)&results_route, &error))
		error_exit("DB error: %s", error);

	bool first = true;
	double px = -1.0, py = -1.0;
	for(unsigned int index=0; index<results_route.size(); index++)
	{
		std::vector<std::string> *row = &results_route.at(index);
		double lo = atof(row -> at(1).c_str());
		double la = atof(row -> at(2).c_str());
		double val = atof(row -> at(3).c_str());

		double x = w * (lo - mi_lo) / div + offset_x + extra_offset_x;
		double y = h * (la - mi_la) / div + offset_y + extra_offset_y;

		double col = (val - v_mi) / (v_ma - v_mi);
		if (col < 0.0) col = 0.0;
		if (col > 1.0) col = 1.0;
		if (first)
		{
			first = false;
		}
		else
		{
			if (HPDF_Page_SetLineWidth(page, 1.5))
				error_exit("HPDF_Page_SetLineWidth failed %04x", HPDF_GetError(pdf));

			if (HPDF_Page_SetRGBStroke(page, col, 0, 1.0 - col))
				error_exit("HPDF_Page_SetRGBStroke(%f) failed", col);

			HPDF_Page_MoveTo(page, px, py);
			HPDF_Page_LineTo(page, x, y);

			if (HPDF_Page_Stroke(page))
				error_exit("HPDF_Page_Stroke failed");
		}

		px = x;
		py = y;
	}

	pop_colors(page, old_colors);
}

void add_map_gps_sensor(sqlite3 *db, HPDF_Doc pdf, HPDF_Image logo_img, HPDF_Font fnt_title, HPDF_Font fnt_text, double fnt_text_height, std::string title, std::string sensor, int *page_nr, std::vector<eval_t> *page_index, bool portrait)
{
	double offset_x = -1.0, offset_y = -1.0, w = -1.0, h = 1.0;
	HPDF_Page page = create_page(pdf, logo_img, title, fnt_title, fnt_text, &offset_x, &offset_y, &w, &h, page_nr, page_index, portrait);

	draw_map_gps_sensor(db, pdf, page, offset_x, offset_y, w, h, sensor);
}

void add_locations(sqlite3 *db, std::vector<eval_t> *data)
{
	if (check_table_exists(db, GPS_TABLE))
	{
		std::string query = "SELECT longitude, latitude FROM " GPS_TABLE " ORDER BY nr ASC LIMIT 1"; // FROM

		std::vector<std::vector<std::string> > results;

		char *error = NULL;
		if (sqlite3_exec(db, query.c_str(), sl_callback, (void *)&results, &error))
			error_exit("DB error: %s", error);

		if (!results.empty())
		{
			std::string from = retrieve_location(db, atof(results.at(0).at(0).c_str()), atof(results.at(0).at(1).c_str()));
			push_back(data, "from address", from, EL_NEUTRAL);
		}

		query = "SELECT longitude, latitude FROM " GPS_TABLE " ORDER BY nr DESC LIMIT 1"; // TO
		results.clear();
		if (sqlite3_exec(db, query.c_str(), sl_callback, (void *)&results, &error))
			error_exit("DB error: %s", error);

		if (!results.empty())
		{
			std::string to = retrieve_location(db, atof(results.at(0).at(0).c_str()), atof(results.at(0).at(1).c_str()));
			push_back(data, "to address", to, EL_NEUTRAL);
		}
	}
}

void draw_gps(sqlite3 *db, HPDF_Page page, HPDF_Font fnt_title, double offset_x, double offset_y, double w, double h)
{
	double title_font_height = 8.0;
	HPDF_Page_BeginText(page);
	HPDF_Page_SetFontAndSize(page, fnt_title, title_font_height);
	std::string title = "route driven";
	double tw = HPDF_Page_TextWidth(page, title.c_str());
	double title_x = offset_x + w / 2.0 - tw / 2.0;
	HPDF_Page_TextOut(page, title_x, offset_y + h, title.c_str());
	HPDF_Page_EndText(page);

	std::string query = "SELECT longitude, latitude FROM " GPS_TABLE;

	std::vector<std::vector<std::string> > results;

	char *error = NULL;
	if (sqlite3_exec(db, query.c_str(), sl_callback, (void *)&results, &error))
		error_exit("DB error: %s", error);

	double mi_lo = 361.0, ma_lo = -361.0;
	double mi_la = 361.0, ma_la = -361.0;
	for(unsigned int index=0; index<results.size(); index++)
	{
		double lo = atof(results.at(index).at(0).c_str());
		double la = atof(results.at(index).at(1).c_str());

		if (lo < mi_lo)
			mi_lo = lo;
		if (lo > ma_lo)
			ma_lo = lo;
		if (la < mi_la)
			mi_la = la;
		if (la > ma_la)
			ma_la = la;
	}

        if (HPDF_Page_SetLineWidth(page, 1))
                error_exit("HPDF_Page_SetLineWidth failed");

	stored_colors_t old_colors = push_colors(page);

        if (HPDF_Page_SetRGBStroke(page, double(0xb) / 255.0, double(0x9a) / 255.0, double(0x0a) / 255.0))
                error_exit("HPDF_Page_SetRGBStroke() failed");

	bool first = true;
	for(unsigned int index=0; index<results.size(); index++)
	{
		double lo = atof(results.at(index).at(0).c_str());
		double la = atof(results.at(index).at(1).c_str());

		double x = w * (lo - mi_lo) / (ma_lo - mi_lo) + offset_x;
		double y = h * (la - mi_la) / (ma_la - mi_la) + offset_y;

		if (first)
		{
			first = false;

			if (HPDF_Page_MoveTo(page, x, y))
				error_exit("HPDF_Page_MoveTo(%f, %f) failed", x, y);
		}
		else
		{
			if (HPDF_Page_LineTo(page, x, y))
				error_exit("HPDF_Page_LineTo(%f, %f) failed", x, y);
		}
	}

        if (HPDF_Page_Stroke(page))
                error_exit("HPDF_Page_Stroke failed");

	pop_colors(page, old_colors);
}

// VSS - vehicle speed in kilometers per hour
// MAF - mass air flow rate in 100 grams per second
// 14.7 grams of air to 1 gram of gasoline - ideal air/fuel ratio
// 6.17 pounds per gallon - density of gasoline
// 4.54 grams per pound - conversion
// 0.621371 miles per hour/kilometers per hour - conversion
// 3600 seconds per hour - conversion
// 100 - to correct MAF to give grams per second
// kmPl => kilometer per liter
double calc_kmPl(double interval, double MAF, double VSS)
{
	double MPG = (14.7 * 6.17 * 4.54 * VSS * 0.621371) / (interval * MAF / 100.0);
#if 0
	return MPG * 3.78541178 / // gallon to liter
		1.609344; // miles to kilometer
#endif
	return MPG * 0.425143707; // 1 mile per gallon = 0.425143707 kilometers per liter according to google
}

// column added: liter per km
std::vector<std::vector<std::string> > * calc_distance_travelled_calc_fuel_usage(sqlite3 *db, bool *data_valid, double *crow_flies, double *real_distance, bool *fuel_valid, double *total_fuel_usage)
{
	bool have_VSS = check_table_exists(db, sensor_vehicle_speed);

	std::string tables = GPS_TABLE;
	std::string query = "SELECT " GPS_TABLE ".ts, longitude, latitude";
	std::string where;

	*fuel_valid = false;

	if (check_table_exists(db, sensor_maf_air_flow_rate))
	{
		tables += ", " + sensor_maf_air_flow_rate;
		query += ", " + sensor_maf_air_flow_rate + ".value";
		where = " WHERE " GPS_TABLE ".ts=" + sensor_maf_air_flow_rate + ".ts";

		*fuel_valid = true;
	}
	// FIXME deze berekening klopt niet denk ik
	else if (check_table_exists(db, sensor_intake_air_temperature) && check_table_exists(db, sensor_barometric_pressure) && check_table_exists(db, sensor_engine_rpm))
	{
		tables += ", " + sensor_barometric_pressure + ", " + sensor_intake_air_temperature + ", " + sensor_engine_rpm;
		query += ", " + sensor_engine_rpm + ".value * (" + sensor_barometric_pressure + ".value / " + sensor_intake_air_temperature + ".value)";
		where = " WHERE " GPS_TABLE ".ts=" + sensor_barometric_pressure + ".ts";
		where += " AND " GPS_TABLE ".ts=" + sensor_intake_air_temperature + ".ts";
		where += " AND " GPS_TABLE ".ts=" + sensor_engine_rpm + ".ts";

		*fuel_valid = true;
	}

	if (have_VSS)
	{
		tables += ", " + sensor_vehicle_speed;
		query += ", " + sensor_vehicle_speed + ".value";
		if (where == "")
			where = " WHERE ";
		else
			where += " AND ";
		where += GPS_TABLE".ts=" + sensor_vehicle_speed + ".ts";
	}
	else
	{
		query += ", 0";
	}

	query += " FROM " + tables + where + " ORDER BY " GPS_TABLE ".ts";

	std::vector<std::vector<std::string> > *results = new std::vector<std::vector<std::string> >();

	char *error = NULL;
	if (sqlite3_exec(db, query.c_str(), sl_callback, (void *)results, &error))
		error_exit("DB error: %s", error);

	if (results -> size() < 2)
	{
		*data_valid = *fuel_valid = false;
		return NULL;
	}

	double start_lo = 1.0, start_la = -1.0, start_ts = -1.0;
	start_ts = atof(results -> at(0).at(0).c_str());
	start_lo = atof(results -> at(0).at(1).c_str());
	start_la = atof(results -> at(0).at(2).c_str());
	unsigned int n = results -> size();
	double end_lo = 1.0, end_la = -1.0;
	end_lo = atof(results -> at(n - 1).at(1).c_str());
	end_la = atof(results -> at(n - 1).at(2).c_str());

	results -> at(0).push_back("");

	*crow_flies = haversine_km(start_la, start_lo, end_la, end_lo);

	unsigned int n_used = 0;
	*real_distance = 0.0;
	*total_fuel_usage = 0.0;
	double prev_lo = start_lo, prev_la = start_la, prev_ts = start_ts;
	for(unsigned int index=1; index<n; index++)
	{
		std::vector<std::string> *row = &results -> at(index);

		bool skip = false;
		for(unsigned int col=0; col<5; col++)
		{
			if (row -> at(col) == "")
			{
				skip = true;
				break;
			}
		}
		if (skip)
			continue;

		double cur_lo = 1.0, cur_la = -1.0, cur_ts = -1.0;
		cur_ts = atof(row -> at(0).c_str());
		cur_lo = atof(row -> at(1).c_str());
		cur_la = atof(row -> at(2).c_str());

		double cur_distance = fabs(haversine_km(cur_la, cur_lo, prev_la, prev_lo));
		double cur_time_interval = cur_ts - prev_ts;

		*real_distance += cur_distance;

		if (*fuel_valid)
		{
			double MAF = atof(row -> at(3).c_str());

			double VSS = -1.0;
			if (have_VSS)
				VSS = atof(row -> at(4).c_str());
			else
				VSS = cur_distance / cur_time_interval;

			double km_per_l = calc_kmPl(cur_time_interval, MAF, VSS);
			double cur_fuel = km_per_l != 0.0 ? cur_distance / km_per_l: 0; // liter per kilometer
			row -> push_back(format("%f", cur_fuel));
			*total_fuel_usage += cur_fuel; // liter per kilometer

			n_used++;
		}
		else
		{
			row -> push_back("");
		}

		prev_lo = cur_lo;
		prev_la = cur_la;
		prev_ts = cur_ts;
	}

	if (n_used > 0)
		*data_valid = true;

	return results;
}

void add_frontpage(sqlite3 *db, HPDF_Doc pdf, HPDF_Image logo_img, HPDF_Font fnt_title, HPDF_Font fnt_text, double font_height, bool use_internet, int *page_nr, std::vector<eval_t> *page_index)
{
	std::string query = "SELECT key, value FROM " META_DATA " ORDER BY key";
	std::vector<std::vector<std::string> > results_meta_global;

	char *error = NULL;
	if (sqlite3_exec(db, query.c_str(), sl_callback, (void *)&results_meta_global, &error))
		error_exit("DB error: %s", error);

	std::vector<eval_t> data;

	if (use_internet)
		add_locations(db, &data);
	for(unsigned int index=0; index<results_meta_global.size(); index++)
	{
		std::vector<std::string> *row = &results_meta_global.at(index);

		push_back(&data, row -> at(0), row -> at(1), EL_NEUTRAL);
	}

	double offset_x = -1.0, offset_y = -1.0, w = -1.0, h = 1.0;
	HPDF_Page page = create_page(pdf, logo_img, "general", fnt_title, fnt_text, &offset_x, &offset_y, &w, &h, page_nr, page_index, true);

	push_back(&data, "report generated on", get_time_str(), EL_NEUTRAL);

	bool have_gps_data = check_table_exists(db, GPS_TABLE);
	if (have_gps_data)
	{
		double crow_flies = -1.0, real_distance = -1.0, total_fuel_usage = -1.0;
		bool data_valid = false, fuel_valid = false;
		std::vector<std::vector<std::string> > *journey_data = calc_distance_travelled_calc_fuel_usage(db, &data_valid, &crow_flies, &real_distance, &fuel_valid, &total_fuel_usage);

		if (data_valid)
		{
			push_back(&data, "as the crow flies", format("%fkm", crow_flies), EL_NEUTRAL);
			push_back(&data, "real distance travelled", format("%fkm", real_distance), EL_NEUTRAL);
		}

		if (fuel_valid)
		{
			push_back(&data, "total fuel usage", format("%fl", total_fuel_usage), EL_NEUTRAL);
			push_back(&data, "liter fuel per km", format("%fl", total_fuel_usage / real_distance), EL_NEUTRAL);
			// FIXME push_back(&data, "km per liter fuel", format("%fl", real_distance / total_fuel_usage), EL_NEUTRAL);
		}

		delete journey_data;
	}

	int n_table_lines = -1;
	double max_key_width = -1, max_value_width = -1;
	find_table_dimensions(page, &data, &max_key_width, &max_value_width, &n_table_lines);

	double text_h = h, gps_bottom = -1, gps_height = -1;
	double table_offset = offset_y + text_h / 2.0 + double(n_table_lines) / 2.0;

	if (have_gps_data)
	{
		text_h = h * (2.0 / 3.0);
		gps_height = h / 3.0;
		gps_bottom = h - gps_height;

		double text_height = (n_table_lines + 1) * font_height;
		table_offset = text_h / 2.0 + text_height / 2.0;

		if (text_height > text_h)
		{
			text_h = text_height; 
			table_offset = text_h;
			gps_height = h - text_height;
			gps_bottom = h - gps_height;
		}
	}

	draw_table(pdf, page, fnt_text, font_height, &data, offset_x + w / 2.0 - (max_key_width + max_value_width) / 2.0, offset_y + table_offset); // draw downwards

	if (have_gps_data)
		draw_gps(db, page, fnt_title, offset_x + w * 0.05, offset_y + gps_bottom, w * 0.90, gps_height * 0.95); // draw upwards
}

void add_graph_point_cloud(HPDF_Doc pdf, HPDF_Image logo_img, HPDF_Font fnt_title, HPDF_Font fnt_text, double fnt_text_height, std::string title, double *xv, double *yv, int n_values, bool draw_avg, bool draw_sd, bool draw_ls, std::string x_unit, std::string y_unit, int *page_nr, std::vector<eval_t> *page_index, bool portrait)
{
	double offset_x = -1.0, offset_y = -1.0, w = -1.0, h = -1.0;
	HPDF_Page page = create_page(pdf, logo_img, title, fnt_title, fnt_text, &offset_x, &offset_y, &w, &h, page_nr, page_index, portrait);

	graph_pdf g;
	double w_margin = w * 0.05;
	double graph_h =  (w - w_margin) * GRAPH_ASPECT_RATIO;
	if (graph_h > h - fnt_text_height)
		graph_h = h - fnt_text_height;
	double graph_bottom = offset_y + h - graph_h;
	g.do_draw_point_cloud(page, fnt_text, offset_x + w_margin / 2.0, graph_bottom, w - w_margin, graph_h, xv, yv, n_values, draw_avg, draw_sd, draw_ls);

	std::vector<eval_t> data;
	push_back(&data, "x units", x_unit, EL_NEUTRAL);
	push_back(&data, "y units", y_unit, EL_NEUTRAL);
	push_back(&data, "description", title, EL_NEUTRAL);
	push_back(&data, "data count", format("%d", n_values), EL_NEUTRAL);
	if (n_values > 0)
	{
		double x_mi = 99999999999.9;
		double x_ma = -99999999999.9;
		double y_mi = 99999999999.9;
		double y_ma = -99999999999.9;
		double avg_x = 0.0, sd_x = 0.0;
		double avg_y = 0.0, sd_y = 0.0;
		for(int index=0; index<n_values; index++)
		{
			if (xv[index] < x_mi) x_mi = xv[index];
			if (xv[index] > x_ma) x_ma = xv[index];
			if (yv[index] < y_mi) y_mi = yv[index];
			if (yv[index] > y_ma) y_ma = yv[index];

			avg_x += xv[index];
			avg_y += yv[index];
			sd_x += pow(xv[index], 2.0);
			sd_y += pow(yv[index], 2.0);
		}
		avg_x /= double(n_values);
		avg_y /= double(n_values);
		sd_x /= double(n_values);
		sd_y /= double(n_values);
		double stddev_x = sqrt(sd_x - pow(avg_x, 2.0));
		double stddev_y = sqrt(sd_y - pow(avg_y, 2.0));
		push_back(&data, "x min", format("%f", x_mi), EL_NEUTRAL);
		push_back(&data, "x max", format("%f", x_ma), EL_NEUTRAL);
		push_back(&data, "y min", format("%f", y_mi), EL_NEUTRAL);
		push_back(&data, "y max", format("%f", y_ma), EL_NEUTRAL);
		push_back(&data, "x average", format("%f", avg_x), EL_NEUTRAL);
		push_back(&data, "y average", format("%f", avg_y), EL_NEUTRAL);
		push_back(&data, "x standard deviation", format("%f", stddev_x), EL_NEUTRAL);
		push_back(&data, "y standard deviation", format("%f", stddev_y), EL_NEUTRAL);
	}

	double table_h = h - w * GRAPH_ASPECT_RATIO;
	double table_offset = offset_y + table_h;

	int n_table_lines = -1;
	double max_key_width = -1, max_value_width = -1;
	find_table_dimensions(page, &data, &max_key_width, &max_value_width, &n_table_lines);

	if (table_h < n_table_lines * fnt_text_height)
	{
		page = create_page(pdf, logo_img, title, fnt_title, fnt_text, &offset_x, &offset_y, &w, &h, page_nr, NULL, true);
		table_offset = offset_y + h - fnt_text_height;
	}

	draw_table(pdf, page, fnt_text, fnt_text_height, &data, offset_x + w_margin / 2.0, table_offset);
}

void add_graph_time(HPDF_Doc pdf, HPDF_Image logo_img, HPDF_Font fnt_title, HPDF_Font fnt_text, double fnt_text_height, long int *t, double *v, unsigned int n_values, std::string units, double dt_min, double dt_max, std::string title, int *page_nr, std::vector<eval_t> *page_index, bool portrait, std::vector<eval_t> *eval_results)
{
	double offset_x = -1.0, offset_y = -1.0, w = -1.0, h = -1.0;
	HPDF_Page page = create_page(pdf, logo_img, title, fnt_title, fnt_text, &offset_x, &offset_y, &w, &h, page_nr, page_index, portrait);

	double avg = 0.0, sd = 0.0;
	double ma = dt_min;
	double mi = dt_max;
	for(unsigned int index=0; index<n_values; index++)
	{
		avg += v[index];
		sd += pow(v[index], 2.0);

		if (v[index] < mi)
			mi = v[index];
		else if (v[index] > ma)
			ma = v[index];
	}
	avg /= double(n_values);
	sd /= double(n_values);
	double stddev = sqrt(sd - pow(avg, 2.0));

	graph_pdf g;
	double w_margin = w * 0.05;
	double graph_h =  (w - w_margin) * GRAPH_ASPECT_RATIO;
	if (graph_h > h - fnt_text_height)
		graph_h = h - fnt_text_height;
	double graph_bottom = offset_y + h - graph_h;
	g.do_draw(page, fnt_text, offset_x + w_margin / 2.0, graph_bottom, w - w_margin, graph_h, t, v, n_values);

	std::vector<eval_t> data;
	push_back(&data, "data type min", format("%f", dt_min), EL_NEUTRAL);
	push_back(&data, "data type max", format("%f", dt_max), EL_NEUTRAL);
	push_back(&data, "data units", units, EL_NEUTRAL);
	push_back(&data, "description", title, EL_NEUTRAL);
	push_back(&data, "measurement count", format("%d", n_values), EL_NEUTRAL);

	if (n_values > 0)
	{
		push_back(&data, "data min", format("%f", mi), EL_NEUTRAL);
		push_back(&data, "data max", format("%f", ma), EL_NEUTRAL);
		push_back(&data, "first measurement", time_to_str((time_t)t[0]), EL_NEUTRAL);
		push_back(&data, "last measurement", time_to_str((time_t)t[n_values - 1]), EL_NEUTRAL);
		long int took = t[n_values - 1] - t[0];
		push_back(&data, "measuring duration", format("%02ld:%02ld:%02ld", took / 3600, (took / 60) % 60, took % 60), EL_NEUTRAL);
		push_back(&data, "average", format("%f", avg), EL_NEUTRAL);
		push_back(&data, "standard deviation", format("%f", stddev), EL_NEUTRAL);
	}

	if (eval_results)
	{
		for(unsigned int index=0; index<eval_results -> size(); index++)
			data.push_back(eval_results -> at(index));
	}

	double table_h = h - w * GRAPH_ASPECT_RATIO;
	double table_offset = offset_y + table_h;

	int n_table_lines = -1;
	double max_key_width = -1, max_value_width = -1;
	find_table_dimensions(page, &data, &max_key_width, &max_value_width, &n_table_lines);

	if (table_h < n_table_lines * fnt_text_height)
	{
		page = create_page(pdf, logo_img, title, fnt_title, fnt_text, &offset_x, &offset_y, &w, &h, page_nr, NULL, true);
		table_offset = offset_y + h - fnt_text_height;
	}

	draw_table(pdf, page, fnt_text, fnt_text_height, &data, offset_x + w_margin / 2.0, table_offset);
}

typedef struct {
	// double mi, ma, avg;
	double avg_val;
	int n;
} bar_t;

void add_histogram_time(HPDF_Doc pdf, HPDF_Image logo_img, HPDF_Font fnt_title, HPDF_Font fnt_text, double fnt_text_height, long int *t, double *v, unsigned int n_values, std::string units, double dt_min, double dt_max, std::string title, int *page_nr, std::vector<eval_t> *page_index, bool portrait, bool meta)
{
	double offset_x = -1.0, offset_y = -1.0, w = -1.0, h = -1.0;
	HPDF_Page page = create_page(pdf, logo_img, title, fnt_title, fnt_text, &offset_x, &offset_y, &w, &h, page_nr, page_index, portrait);

	double avg = 0.0, sd = 0.0;
	double ma = dt_min;
	double mi = dt_max;
	for(unsigned int index=0; index<n_values; index++)
	{
		avg += v[index];
		sd += pow(v[index], 2.0);

		if (v[index] < mi)
			mi = v[index];
		else if (v[index] > ma)
			ma = v[index];
	}
	avg /= double(n_values);
	sd /= double(n_values);
	double stddev = sqrt(sd - pow(avg, 2.0));

	if (ma != mi)
	{
		graph_pdf g;
		double w_margin = w * 0.05;
		double graph_h =  (w - w_margin) * GRAPH_ASPECT_RATIO;
		if (graph_h > h - fnt_text_height)
			graph_h = h - fnt_text_height;
		double graph_bottom = offset_y + h - graph_h;

		// create labels & group data
		int n_bars = 24;

		double *va = (double *)calloc(n_bars, sizeof(double));
		bar_t *bars = (bar_t *)calloc(n_bars, sizeof(bar_t));

		for(unsigned int index=0; index<n_values; index++)
		{
			int cur_bar = ((v[index] - mi) / (ma - mi)) * double(n_bars - 1);

			// bars[cur_bar].avg += v[index];
			// bars[cur_bar].mi = std::min(bars[cur_bar].mi, v[index]);
			// bars[cur_bar].ma = std::max(bars[cur_bar].ma, v[index]);
			bars[cur_bar].avg_val += v[index];
			bars[cur_bar].n++;
		}

		std::vector<std::string> labels;
		int out_index = 0;
		for(int index=0; index<n_bars; index++)
		{
			int n = bars[index].n;

			if (n)
			{
				va[out_index++] = double(n * 100) / double(n_values);

				labels.push_back(shorten(bars[index].avg_val / double(n)));
			}
		}

		g.do_draw_histogram(page, fnt_text, offset_x + w_margin / 2.0, graph_bottom, w - w_margin, graph_h, &labels, va, out_index, "x: value (in " + units + "), y: % of measurements with that value");

		free(bars);
		free(va);

		if (meta)
		{
			std::vector<std::pair<std::string, std::string> > data;
			data.push_back(std::pair<std::string, std::string>("data type min", format("%f", dt_min)));
			data.push_back(std::pair<std::string, std::string>("data type max", format("%f", dt_max)));
			data.push_back(std::pair<std::string, std::string>("data units", units));
			data.push_back(std::pair<std::string, std::string>("description", title));
			data.push_back(std::pair<std::string, std::string>("measurement count", format("%d", n_values)));

			if (n_values > 0)
			{
				data.push_back(std::pair<std::string, std::string>("data min", format("%f", mi)));
				data.push_back(std::pair<std::string, std::string>("data max", format("%f", ma)));
				data.push_back(std::pair<std::string, std::string>("first measurement", time_to_str((time_t)t[0])));
				data.push_back(std::pair<std::string, std::string>("last measurement", time_to_str((time_t)t[n_values - 1])));
				long int took = t[n_values - 1] - t[0];
				data.push_back(std::pair<std::string, std::string>("measuring duration", format("%02ld:%02ld:%02ld", took / 3600, (took / 60) % 60, took % 60)));
				data.push_back(std::pair<std::string, std::string>("average", format("%f", avg)));
				data.push_back(std::pair<std::string, std::string>("standard deviation", format("%f", stddev)));
			}

			double table_offset = offset_y + h - w * GRAPH_ASPECT_RATIO;
			draw_table(pdf, page, fnt_text, fnt_text_height, &data, offset_x + w_margin / 2.0, table_offset);
		}
	}
}

void evaluate_fuel_trim(double *values, unsigned int n_values, std::vector<eval_t> *eval)
{
	int n_bigger_than_10p = 0;

	for(unsigned int index=0; index<n_values; index++)
	{
		if (fabs(values[index]) > 10.0)
			n_bigger_than_10p++;
	}

	eval_t e;
	e.key = "fuel trim > 10%";
	e.value = EVAL_OK_STR;
	e.el = EL_OK;

	double perc = double(n_bigger_than_10p * 100) / double(n_values);
	if (perc)
	{
		e.value = format("for %.2f%% of all measurements", perc);
		e.el = EL_WARNING;
	}

	eval -> push_back(e);
}

void evaluate_coolant_temperature(double *values, unsigned int n_values, std::vector<eval_t> *eval)
{
	int n_warmer_than_104 = 0;

	for(unsigned int index=0; index<n_values; index++)
	{
		if (values[index] > 104.0)
			n_warmer_than_104++;
	}

	eval_t e;
	e.key = "coolant warmer than 104C";
	e.value = EVAL_OK_STR;
	e.el = EL_OK;

	double perc = double(n_warmer_than_104 * 100) / double(n_values);
	if (perc)
	{
		e.value = format("for %.2f%% of all measurements", perc);
		e.el = EL_WARNING;
	}
}

void add_sensor_graphs(sqlite3 *db, HPDF_Doc pdf, HPDF_Image logo_img, HPDF_Font fnt_title, HPDF_Font fnt_text, double fnt_text_height, std::string ignore_list, int *page_nr, std::vector<eval_t> *page_index, bool portrait)
{
	std::string query = "SELECT table_name, units, min, max, descr FROM table_meta_data ORDER BY table_name";

	std::vector<std::vector<std::string> > results_meta;
	char *error = NULL;
	if (sqlite3_exec(db, query.c_str(), sl_callback, (void *)&results_meta, &error))
		error_exit("DB error: %s for query %s", error, query.c_str());

	// other graphs
	for(unsigned int meta_index=0; meta_index<results_meta.size(); meta_index++)
	{
		std::string sensor_table = results_meta.at(meta_index).at(0);

		if (ignore_list.find(sensor_table) != std::string::npos)
			continue;

		std::string units = results_meta.at(meta_index).at(1);
		double dt_min = atof(results_meta.at(meta_index).at(2).c_str());
		double dt_max = atof(results_meta.at(meta_index).at(3).c_str());
		std::string title = results_meta.at(meta_index).at(4);

		std::vector<std::vector<std::string> > *sensor_data = retrieve_sensor_data(db, sensor_table);

		unsigned int n_values_in = sensor_data -> size(), out_index = 0, n_values = -1;
		long int *t = (long int *)calloc(n_values_in, sizeof(long int));
		double *v = (double *)calloc(n_values_in, sizeof(double));

		double ma = dt_min, mi = dt_max;
		for(unsigned int index=0; index<n_values_in; index++)
		{
			std::vector<std::string> *row = &sensor_data -> at(index);

			if (row -> at(0) != "" && row -> at(1) != "")
			{
				t[out_index] = atol(row -> at(0).c_str());
				v[out_index] = atof(row -> at(1).c_str());
				mi = std::min(v[out_index], mi);
				ma = std::max(v[out_index], ma);
				out_index++;
			}
		}
		n_values = out_index;

		std::vector<eval_t> eval;
		if (sensor_table == sensor_short_term_fuel_bank1 || sensor_table == sensor_short_term_fuel_bank2)
			evaluate_fuel_trim(v, n_values, &eval);
		if (sensor_table == sensor_engine_coolant_temperature)
			evaluate_coolant_temperature(v, n_values, &eval);

		add_graph_time(pdf, logo_img, fnt_title, fnt_text, fnt_text_height, t, v, n_values, units, dt_min, dt_max, title, page_nr, page_index, portrait, &eval);

		if (mi != ma)
			add_histogram_time(pdf, logo_img, fnt_title, fnt_text, fnt_text_height, t, v, n_values, units, dt_min, dt_max, title +  " histogram", page_nr, NULL, portrait, false);

		if (check_table_exists(db, GPS_MAP_LINES) && check_table_exists(db, GPS_MAP_NODES))
			add_map_gps_sensor(db, pdf, logo_img, fnt_title, fnt_text, fnt_text_height, title, sensor_table, page_nr, NULL, portrait);

		// if (n_values >= 16 * FFT_SAMPLE_RATE)
			// add_sensors_spectrum_heat_diagram(pdf, logo_img, fnt_title, fnt_text, fnt_text_height, title, v, n_values, page_nr, NULL, portrait);

		free(v);
		free(t);
		delete sensor_data;
	}
}

void add_rpm_versus_speed(sqlite3 *db, HPDF_Doc pdf, HPDF_Image logo_img, HPDF_Font fnt_title, HPDF_Font fnt_text, double fnt_text_height, int *page_nr, std::vector<eval_t> *page_index, bool portrait)
{
	std::string title = "RPM versus speed";

	if (check_table_exists(db, sensor_engine_rpm) && check_table_exists(db, sensor_vehicle_speed))
	{
		std::string query = "SELECT "+sensor_vehicle_speed+".value AS speed, "+sensor_engine_rpm+".value AS rpm  FROM "+sensor_vehicle_speed+", "+sensor_engine_rpm+" WHERE "+sensor_vehicle_speed+".ts = "+sensor_engine_rpm+".ts GROUP BY "+sensor_vehicle_speed+".value";

		std::vector<std::vector<std::string> > results;
		char *error = NULL;

		if (sqlite3_exec(db, query.c_str(), sl_callback, (void *)&results, &error))
			error_exit("DB error: %s for query %s", error, query.c_str());

		unsigned int n_values = results.size();
		double *xv = (double *)malloc(sizeof(double) * n_values);
		double *yv = (double *)malloc(sizeof(double) * n_values);

		for(unsigned int index=0; index<n_values; index++)
		{
			std::vector<std::string> *row = &results.at(index);

			xv[index] = atof(row -> at(0).c_str());

			yv[index] = atof(row -> at(1).c_str());
		}

		add_graph_point_cloud(pdf, logo_img, fnt_title, fnt_text, fnt_text_height, title, xv, yv, n_values, true, true, true, "km/h", "RPM", page_nr, page_index, portrait);

		free(yv);
		free(xv);
	}
	else if (verbose)
	{
		std::cout << "Table \"" << title << "\" skipped, table " << sensor_engine_rpm << " and/or " << sensor_vehicle_speed << " not available" << std::endl;
	}
}

void add_vehicle_speed_versus_gps_speed(sqlite3 *db, HPDF_Doc pdf, HPDF_Image logo_img, HPDF_Font fnt_title, HPDF_Font fnt_text, double fnt_text_height, int *page_nr, std::vector<eval_t> *page_index, bool portrait)
{
	std::string title = "vehicle speed (y axis) versus GPS speed (x axis)";

	if (check_table_exists(db, gps_location) && check_table_exists(db, sensor_vehicle_speed))
	{
		std::string query = "SELECT "+gps_location+".ground_speed AS gps_speed, AVG("+sensor_vehicle_speed+".value) AS vehicle FROM "+sensor_vehicle_speed+", "+gps_location+" WHERE "+sensor_vehicle_speed+".ts = "+gps_location+".ts GROUP BY "+gps_location+".ground_speed";

		std::vector<std::vector<std::string> > results;
		char *error = NULL;

		if (sqlite3_exec(db, query.c_str(), sl_callback, (void *)&results, &error))
			error_exit("DB error: %s for query %s", error, query.c_str());

		unsigned int n_values = results.size();
		double *xv = (double *)malloc(sizeof(double) * n_values);
		double *yv = (double *)malloc(sizeof(double) * n_values);

		for(unsigned int index=0; index<n_values; index++)
		{
			std::vector<std::string> *row = &results.at(index);

			xv[index] = atof(row -> at(0).c_str());
			yv[index] = atof(row -> at(1).c_str());
		}

		add_graph_point_cloud(pdf, logo_img, fnt_title, fnt_text, fnt_text_height, title, xv, yv, n_values, false, false, false, "km/h (GPS)", "km/h (vehicle)", page_nr, page_index, portrait);

		free(yv);
		free(xv);
	}
	else if (verbose)
	{
		std::cout << "Table \"" << title << "\" skipped, table " << gps_location << " and/or " << sensor_vehicle_speed << " not available" << std::endl;
	}
}

void add_rpm_div_throttle(sqlite3 *db, HPDF_Doc pdf, HPDF_Image logo_img, HPDF_Font fnt_title, HPDF_Font fnt_text, double fnt_text_height, int *page_nr, std::vector<eval_t> *page_index, bool portrait)
{
	if (!check_table_exists(db, sensor_engine_rpm) || !check_table_exists(db, sensor_throttle))
	{
		if (verbose)
			std::cout << "RPM versus throttle graph skipped: RPM or throttle table not available" << std::endl;
	}
	else
	{
		std::string query = "SELECT " + sensor_engine_rpm + ".ts, " + sensor_engine_rpm + ".value / " + sensor_throttle + ".value FROM " + sensor_engine_rpm + ", " + sensor_throttle + " WHERE " + sensor_engine_rpm + ".ts=" + sensor_throttle + ".ts";

		std::vector<std::vector<std::string> > results;
		char *error = NULL;

		if (sqlite3_exec(db, query.c_str(), sl_callback, (void *)&results, &error))
			error_exit("DB error: %s for query %s", error, query.c_str());

		unsigned int n_values_in = results.size(), out_index = 0, n_values = -1;
		long int *t = (long int *)calloc(n_values_in, sizeof(long int));
		double *v = (double *)calloc(n_values_in, sizeof(double));

		for(unsigned int index=0; index<n_values_in; index++)
		{
			std::vector<std::string> *row = &results.at(index);

			if (row -> at(0) != "" && row -> at(1) != "")
			{
				t[out_index] = atol(row -> at(0).c_str());
				v[out_index] = atof(row -> at(1).c_str());
				out_index++;
			}
		}

		n_values = out_index;

		add_graph_time(pdf, logo_img, fnt_title, fnt_text, fnt_text_height, t, v, n_values, "RPM/throttle%", 0.0, 16383.74 * 2.55, "RPM divided by throttle percentage", page_nr, page_index, portrait, NULL);

		free(v);
		free(t);
	}
}

void add_sensors_heat_diagram(sqlite3 *db, HPDF_Doc pdf, HPDF_Image logo_img, HPDF_Font fnt_title, HPDF_Font fnt_text, double font_height, std::string ignore_list, int *page_nr, std::vector<eval_t> *page_index, bool portrait)
{
	std::string query = "SELECT table_name, descr FROM table_meta_data ORDER BY table_name";

	std::vector<std::vector<std::string> > results_meta;
	char *error = NULL;
	if (sqlite3_exec(db, query.c_str(), sl_callback, (void *)&results_meta, &error))
		error_exit("DB error: %s for query %s", error, query.c_str());
	unsigned int n_fields = results_meta.size();
	if (n_fields < 2)
	{
		if (verbose)
			std::cerr << "Not enough sensors for sensor heat diagram" << std::endl;
		return;
	}

	// create query from list of sensor tables
	std::string fields, tables, where;
	bool first = true;
	std::string compare_table;
	for(int index=n_fields - 1; index>=0; index--)
	{
		std::string table = results_meta.at(index).at(0);

		if (first)
		{
			compare_table = table;
			first = false;
		}
		else
		{
			if (where != "")
				where += " AND ";

			where += table + ".ts = " + compare_table + ".ts";
		}

		if (fields != "")
			fields += ", ";

		fields += "(" + table + ".value - (SELECT MIN(value) FROM " + table + ")) / ((SELECT MAX(value) FROM " + table + ") - (SELECT MIN(value) FROM " + table + format(")) AS v%d", index);

		if (tables != "")
			tables += ", ";

		tables += table;
	}

	query = "SELECT " + fields + " FROM " + tables + " WHERE " + where + " ORDER BY " + compare_table + ".nr ASC";

	// load result from query
	std::vector<std::vector<std::string> > results;
	if (sqlite3_exec(db, query.c_str(), sl_callback, (void *)&results, &error))
		error_exit("DB error: %s for query %s", error, query.c_str());
	unsigned int n_values = results.size();
	if (n_values < 2)
	{
		std::cerr << "Not enough sensor data for sensor heat diagram" << std::endl;
		return;
	}

	// create the heat diagram
	double offset_x = -1.0, offset_y = -1.0, w = -1.0, h = -1.0;
	HPDF_Page page = create_page(pdf, logo_img, "sensor heat diagram", fnt_title, fnt_text, &offset_x, &offset_y, &w, &h, page_nr, page_index, true);

	double w_margin = w * 0.05;
	// double graph_h = (w - w_margin) * GRAPH_ASPECT_RATIO;

	HPDF_Page_BeginText(page);
	HPDF_Page_SetFontAndSize(page, fnt_text, font_height);
	std::string str = format("%2d", n_fields);
	double nr_w = HPDF_Page_TextWidth(page, str.c_str());
	HPDF_Page_EndText(page);

	double diagram_x = offset_x + w_margin / 2.0 + nr_w;
	double diagram_w = w - w_margin - nr_w;
	double block_w = diagram_w / double(n_values);
	double block_h = font_height * 2.0 - 1.0;
	double diagram_h = block_h * double(n_fields);
	double diagram_y = offset_y + h - diagram_h;

	HPDF_Page_BeginText(page);
	HPDF_Page_SetFontAndSize(page, fnt_text, font_height);
	std::string text = "values (and so the colors) are normalized";
	double tw = HPDF_Page_TextWidth(page, text.c_str());
	HPDF_Page_TextOut(page, offset_x + w / 2.0 - tw / 2.0, offset_y + h - font_height, text.c_str());
	HPDF_Page_EndText(page);

	std::vector<eval_t> meta_table;

	stored_colors_t old_colors = push_colors(page);

	for(unsigned int field=0; field<n_fields; field++)
	{
		double cur_x = offset_x + w_margin / 2.0 - nr_w / 2.0;
		double cur_y = diagram_y + block_h * double(n_fields - field - 1.0) - font_height - 1.0; // 1.0: slightly above the base-line

		HPDF_Page_BeginText(page);
		HPDF_Page_SetFontAndSize(page, fnt_text, font_height);
		std::string index_str = format("%2d", field + 1);
		HPDF_Page_TextOut(page, cur_x, cur_y, index_str.c_str());
		HPDF_Page_EndText(page);

		push_back(&meta_table, index_str, results_meta.at(field).at(1), EL_NEUTRAL);
	}

	for(unsigned int index=1; index<n_values; index++)
	{
		double prev_x = diagram_x + block_w * double(index - 1);
		double cur_x  = diagram_x + block_w * double(index);

		std::vector<std::string> *row = &results.at(index);

		for(unsigned int field=0; field<n_fields; field++)
		{
			double prev_y = diagram_y + block_h * (double(field) - 1.0);
			double cur_y  = diagram_y + block_h * double(field);

			double value = atof(row -> at(field).c_str());

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
			HPDF_Page_Rectangle(page, prev_x, prev_y, cur_x - prev_x, cur_y - prev_y);
			HPDF_Page_FillStroke(page);
		}
	}

	pop_colors(page, old_colors);

	double max_table_h = h - (diagram_h + font_height * 2.0);

	int n_table_lines = -1;
	double max_key_width = -1, max_value_width = -1;
	find_table_dimensions(page, &meta_table, &max_key_width, &max_value_width, &n_table_lines);

	double table_h = font_height * double(n_table_lines);

	double table_y = offset_y + (max_table_h - table_h) / 2.0 + table_h;

	if (table_h > max_table_h)
	{
		double offset2_x = -1.0, offset2_y = -1.0, w2 = -1.0, h2 = -1.0;
		page = create_page(pdf, logo_img, "sensor heat diagram (2)", fnt_title, fnt_text, &offset2_x, &offset2_y, &w2, &h2, page_nr, NULL, true);

		table_y = offset_y + h2;

		max_table_h = h2;
	}

	(void)draw_table(pdf, page, fnt_text, font_height, &meta_table, diagram_x, table_y);
}

void draw_index_page(HPDF_Doc pdf, HPDF_Page index_page, std::vector<eval_t> *page_index, HPDF_Font fnt_text, double text_height, double offset_x, double offset_y, double w, double h)
{
	int n_table_lines = -1;
	double max_key_width = -1, max_value_width = -1;
	find_table_dimensions(index_page, page_index, &max_key_width, &max_value_width, &n_table_lines);

	double table_width = max_key_width + max_value_width;

	double table_x = (w - table_width) / 2.0 + offset_x;
	double table_y = h - text_height + offset_y;

	(void)draw_table(pdf, index_page, fnt_text, text_height, page_index, table_x, table_y);
}

void add_kmPl(sqlite3 *db, HPDF_Doc pdf, HPDF_Image logo_img, HPDF_Font fnt_title, HPDF_Font fnt_text, double fnt_text_height, int *page_nr, std::vector<eval_t> *page_index, bool portrait)
{

	bool have_gps_data = check_table_exists(db, GPS_TABLE);
	if (have_gps_data)
	{
		double crow_flies = -1.0, real_distance = -1.0, total_fuel_usage = -1.0;
		bool data_valid = false, fuel_valid = false;
		std::vector<std::vector<std::string> > *journey_data = calc_distance_travelled_calc_fuel_usage(db, &data_valid, &crow_flies, &real_distance, &fuel_valid, &total_fuel_usage);

		std::string units = "km/l";
		double dt_min = 0.0;
		double dt_max = 255.0;
		std::string title = "km/l";

		unsigned int n_values_in = journey_data -> size(), out_index = 0, n_values = -1;
		long int *t = (long int *)calloc(n_values_in, sizeof(long int));
		double *v = (double *)calloc(n_values_in, sizeof(double));

		double ma = dt_min, mi = dt_max;
		for(unsigned int index=0; index<n_values_in; index++)
		{
			std::vector<std::string> *row = &journey_data -> at(index);

			if (row -> at(0) != "" && row -> at(5) != "")
			{
				t[out_index] = atol(row -> at(0).c_str());
				v[out_index] = atof(row -> at(5).c_str());
				mi = std::min(v[out_index], mi);
				ma = std::max(v[out_index], ma);
				out_index++;
			}
		}

		n_values = out_index;
printf("%d\n", n_values);
		add_graph_time(pdf, logo_img, fnt_title, fnt_text, fnt_text_height, t, v, n_values, units, dt_min, dt_max, title, page_nr, page_index, portrait, NULL);

		if (mi != ma)
			add_histogram_time(pdf, logo_img, fnt_title, fnt_text, fnt_text_height, t, v, n_values, units, dt_min, dt_max, title +  " histogram", page_nr, NULL, portrait, false);

		free(v);
		free(t);

		delete journey_data;
	}
}

void add_graph_time_two_data(HPDF_Doc pdf, HPDF_Image logo_img, HPDF_Font fnt_title, HPDF_Font fnt_text, double fnt_text_height, long int *t, double *v1, double *v2, unsigned int n_values, std::string units1, std::string units2, double dt_min1, double dt_max1, double dt_min2, double dt_max2, std::string title, std::string metaStr, int *page_nr, std::vector<eval_t> *page_index, bool portrait, std::vector<eval_t> *eval_results)
{
	double offset_x = -1.0, offset_y = -1.0, w = -1.0, h = -1.0;
	HPDF_Page page = create_page(pdf, logo_img, title, fnt_title, fnt_text, &offset_x, &offset_y, &w, &h, page_nr, page_index, portrait);

	double avg1 = 0.0, sd1 = 0.0;
	double ma1 = dt_min1;
	double mi1 = dt_max1;
	double avg2 = 0.0, sd2 = 0.0;
	double ma2 = dt_min2;
	double mi2 = dt_max2;
	for(unsigned int index=0; index<n_values; index++)
	{
		avg1 += v1[index];
		sd1 += pow(v1[index], 2.0);

		if (v1[index] < mi1)
			mi1 = v1[index];
		else if (v1[index] > ma1)
			ma1 = v1[index];

		avg2 += v2[index];
		sd2 += pow(v2[index], 2.0);

		if (v2[index] < mi2)
			mi2 = v2[index];
		else if (v2[index] > ma2)
			ma2 = v2[index];
	}
	avg1 /= double(n_values);
	sd1 /= double(n_values);
	double stddev1 = sqrt(sd1 - pow(avg1, 2.0));
	avg2 /= double(n_values);
	sd2 /= double(n_values);
	double stddev2 = sqrt(sd2 - pow(avg2, 2.0));

	graph_pdf g;
	double w_margin = w * 0.05;
	double graph_h =  (w - w_margin) * GRAPH_ASPECT_RATIO;
	if (graph_h > h - fnt_text_height)
		graph_h = h - fnt_text_height;
	double graph_bottom = offset_y + h - graph_h;
	g.do_draw(page, fnt_text, offset_x + w_margin / 2.0, graph_bottom, w - w_margin, graph_h, t, v1, v2, n_values, metaStr);

	std::vector<eval_t> data;
	push_back(&data, "data type x axis min", format("%f", dt_min1), EL_NEUTRAL);
	push_back(&data, "data type x axis max", format("%f", dt_max1), EL_NEUTRAL);
	push_back(&data, "data type y axis min", format("%f", dt_min2), EL_NEUTRAL);
	push_back(&data, "data type y axis max", format("%f", dt_max2), EL_NEUTRAL);
	push_back(&data, "data units x axis", units1, EL_NEUTRAL);
	push_back(&data, "data units y axis", units2, EL_NEUTRAL);
	push_back(&data, "description", title, EL_NEUTRAL);
	push_back(&data, "measurement count", format("%d", n_values), EL_NEUTRAL);

	if (n_values > 0)
	{
		push_back(&data, "data min x axis", format("%f", mi1), EL_NEUTRAL);
		push_back(&data, "data max x axis", format("%f", ma1), EL_NEUTRAL);
		push_back(&data, "data min y axis", format("%f", mi2), EL_NEUTRAL);
		push_back(&data, "data max y axis", format("%f", ma2), EL_NEUTRAL);
		push_back(&data, "first measurement", time_to_str((time_t)t[0]), EL_NEUTRAL);
		push_back(&data, "last measurement", time_to_str((time_t)t[n_values - 1]), EL_NEUTRAL);
		long int took = t[n_values - 1] - t[0];
		push_back(&data, "measuring duration", format("%02ld:%02ld:%02ld", took / 3600, (took / 60) % 60, took % 60), EL_NEUTRAL);
		push_back(&data, "average x axis", format("%f", avg1), EL_NEUTRAL);
		push_back(&data, "average y axis", format("%f", avg2), EL_NEUTRAL);
		push_back(&data, "standard deviation x", format("%f", stddev1), EL_NEUTRAL);
		push_back(&data, "standard deviation y", format("%f", stddev2), EL_NEUTRAL);
	}

	if (eval_results)
	{
		for(unsigned int index=0; index<eval_results -> size(); index++)
			data.push_back(eval_results -> at(index));
	}

	double table_h = h - w * GRAPH_ASPECT_RATIO;
	double table_offset = offset_y + table_h;

	int n_table_lines = -1;
	double max_key_width = -1, max_value_width = -1;
	find_table_dimensions(page, &data, &max_key_width, &max_value_width, &n_table_lines);

	if (table_h < n_table_lines * fnt_text_height)
	{
		page = create_page(pdf, logo_img, title, fnt_title, fnt_text, &offset_x, &offset_y, &w, &h, page_nr, NULL, true);
		table_offset = offset_y + h - fnt_text_height;
	}

	draw_table(pdf, page, fnt_text, fnt_text_height, &data, offset_x + w_margin / 2.0, table_offset);
}

void add_graph_speed_versus_rpm(sqlite3 *db, HPDF_Doc pdf, HPDF_Image logo_img, HPDF_Font fnt_title, HPDF_Font fnt_text, double fnt_text_height, int *page_nr, std::vector<eval_t> *page_index, bool portrait)
{
	std::string query_meta = "SELECT MIN(" + sensor_engine_rpm + ".value) AS rpm_min, MAX(" + sensor_engine_rpm + ".value) AS rpm_max, MIN(" + sensor_vehicle_speed + ".value) AS speed_min, MAX(" + sensor_vehicle_speed + ".value) AS speed_max FROM " + sensor_engine_rpm + ", " + sensor_vehicle_speed + " WHERE " + sensor_engine_rpm + ".nr=" + sensor_vehicle_speed + ".nr";
	std::vector<std::vector<std::string> > results_meta;
	char *error = NULL;
	if (sqlite3_exec(db, query_meta.c_str(), sl_callback, (void *)&results_meta, &error))
		error_exit("DB error: %s", error);

	double rpm_min = atof(results_meta.at(0).at(0).c_str());
	double rpm_max = atof(results_meta.at(0).at(1).c_str());
	double speed_min = atof(results_meta.at(0).at(2).c_str());
	double speed_max = atof(results_meta.at(0).at(3).c_str());

	std::string query_data = "SELECT " + sensor_engine_rpm + ".ts AS ts, " + sensor_engine_rpm + ".value AS rpm, " + sensor_vehicle_speed + ".value AS speed FROM " + sensor_engine_rpm + ", " + sensor_vehicle_speed + " WHERE " + sensor_engine_rpm + ".nr=" + sensor_vehicle_speed + ".nr ORDER BY " + sensor_engine_rpm + ".nr";
	std::vector<std::vector<std::string> > results_data;
	if (sqlite3_exec(db, query_data.c_str(), sl_callback, (void *)&results_data, &error))
		error_exit("DB error: %s", error);

	int n_values = results_data.size();
	if (n_values == 0)
		return;

	long int *t = (long int *)malloc(sizeof(long int) * n_values);
	double *v1 = (double *)malloc(sizeof(double) * n_values);
	double *v2 = (double *)malloc(sizeof(double) * n_values);

	for(int index=0; index<n_values; index++)
	{
		t[index] = atol(results_data.at(index).at(0).c_str());
		v1[index] = atol(results_data.at(index).at(1).c_str());
		v2[index] = atol(results_data.at(index).at(2).c_str());
	}

	add_graph_time_two_data(pdf, logo_img, fnt_title, fnt_text, fnt_text_height, t, v1, v2, n_values, "RPM", "km/h", rpm_min, rpm_max, speed_min, speed_max, "RPM versus speed (in time)", "red: rpm, magenta: speed", page_nr, page_index, portrait, NULL);

	free(v2);
	free(v1);
	free(t);
}

void add_colofon(sqlite3 *db, HPDF_Doc pdf, HPDF_Image logo_img, HPDF_Font fnt_title, HPDF_Font fnt_text, double fnt_text_height, int *page_nr, std::vector<eval_t> *page_index, bool portrait)
{
	double offset_x = -1.0, offset_y = -1.0, w = -1.0, h = -1.0;
	HPDF_Page page = create_page(pdf, logo_img, "miscellaneous", fnt_title, fnt_text, &offset_x, &offset_y, &w, &h, page_nr, page_index, true);

	std::vector<std::pair<std::string, std::string> > data;
	data.push_back(std::pair<std::string, std::string>("O2OO version", VERSION));
	data.push_back(std::pair<std::string, std::string>("O2OO build date", __DATE__ " " __TIME__));
	data.push_back(std::pair<std::string, std::string>("revision", "$Revision: 469 $"));
	data.push_back(std::pair<std::string, std::string>("build by/at", BUILD_BY));
	data.push_back(std::pair<std::string, std::string>("O2OO website", "http://www.vanheusden.com/O2OO/"));
	data.push_back(std::pair<std::string, std::string>("e-mail address O2OO developer", "folkert@vanheusden.com"));
	data.push_back(std::pair<std::string, std::string>("HPDF", HPDF_GetVersion()));
	data.push_back(std::pair<std::string, std::string>("TinyXML2", format("%d.%d.%d", TIXML2_MAJOR_VERSION, TIXML2_MINOR_VERSION, TIXML2_PATCH_VERSION)));

	curl_version_info_data *cvid = curl_version_info(CURLVERSION_NOW);
	data.push_back(std::pair<std::string, std::string>("CURL", cvid -> version));
	data.push_back(std::pair<std::string, std::string>("CURL SSL", cvid -> ssl_version));
	data.push_back(std::pair<std::string, std::string>("CURL libz", cvid -> libz_version));
	data.push_back(std::pair<std::string, std::string>("SQLite3", SQLITE_VERSION));
	data.push_back(std::pair<std::string, std::string>("Compiler", COMPILER_BRAND " " __VERSION__));

	char buffer[4096];
	if (gethostname(buffer, sizeof buffer) == -1)
		error_exit("gethostname() failed");
	data.push_back(std::pair<std::string, std::string>("running on", buffer));

	uid_t user = getuid();
	struct passwd *pw = NULL;
	for(;;)
	{
		pw = getpwent();
		if (pw == NULL)
			break;
		if (user == pw -> pw_uid)
			break;
	}
	data.push_back(std::pair<std::string, std::string>("invoked by", pw ? pw -> pw_name : "?"));

	double table_offset = offset_y + h;
	draw_table(pdf, page, fnt_text, fnt_text_height, &data, offset_x + 25.0, table_offset);
}

void set_pdf_meta(HPDF_Doc pdf)
{
	HPDF_SetInfoAttr(pdf, HPDF_INFO_AUTHOR, "folkert@vanheusden.com");
	HPDF_SetInfoAttr(pdf, HPDF_INFO_CREATOR, "O2OO v" VERSION);
	HPDF_SetInfoAttr(pdf, HPDF_INFO_TITLE, "OBD2 data");
	HPDF_SetInfoAttr(pdf, HPDF_INFO_KEYWORDS, "OBD2, OBDII, vehicle, car, sensors");

	struct timeval tv;
	struct timezone tz;

	if (gettimeofday(&tv, &tz) == -1)
		error_exit("gettimeofday failed");

	struct tm *tm = gmtime(&tv.tv_sec);

	HPDF_Date data = { tm -> tm_year + 1900, tm -> tm_mon + 1, tm -> tm_mday, tm -> tm_hour, tm -> tm_min, tm -> tm_sec, ' ', 0, 0 };

	HPDF_SetInfoDateAttr(pdf, HPDF_INFO_CREATION_DATE, data);
}

void check_db_file_exists(std::string db_file)
{
	struct stat st;

	if (stat(db_file.c_str(), &st) == -1)
		error_exit("The database file you selected (%s) does not exist or cannot be opened (error: %s).", db_file.c_str(), strerror(errno));
}

void help(void)
{
	std::cerr << "-b x    database (SQLite) to read from" << std::endl;
	std::cerr << "-f x    pdf file to write to" << std::endl;
	std::cerr << "-F x,y  font files (TTF) to use: title,text" << std::endl;
	std::cerr << "-i x    sensors to filter (list them with O2OO-dump)" << std::endl;
	std::cerr << "-L      draw graphs in landscape mode" << std::endl;
	std::cerr << "-I      enable retrieval of data from internet (e.g. Google Maps)" << std::endl;
	std::cerr << "-v      enable verbose mode, show errors (e.g. Google Maps errors)" << std::endl;
	std::cerr << "-h      this help" << std::endl;
	std::cerr << std::endl;
}

int main(int argc, char *argv[])
{
	std::cerr << "O2OO-evaluate v" VERSION ", (C) folkert@vanheusden.com" << std::endl << std::endl;

	bool use_internet = false;
	sqlite3 *db = NULL;
	std::string pdf_file, font_file_text = FONT_TEXT, font_file_title = FONT_TITLE, ignore_list;
	bool portrait = true;

	int c;
	while((c = getopt(argc, argv, "b:f:F:i:LIvh")) != -1)
	{
		switch(c)
		{
			case 'b':
				check_db_file_exists(optarg);

				if (sqlite3_open(optarg, &db))
					error_exit("Problem opening database file %s", optarg);
				break;

			case 'f':
				pdf_file = optarg;
				break;

			case 'F':
				{
					std::vector<std::string> parts = split_string(optarg, ",");
					font_file_title = parts.at(0);
					if (parts.size() == 2)
						font_file_text = parts.at(1);
					else if (parts.size() != 1)
						error_exit("-F: invalid number of parameters, expecting title[,text]-font");
					break;
				}

			case 'i':
				ignore_list = optarg;
				break;

			case 'L':
				portrait = false;
				break;

			case 'I':
				use_internet = true;
				break;

			case 'v':
				verbose = true;
				break;

			case 'h':
				help();
				return 0;

			default:
				help();
				return 1;
		}
	}

	if (!db)
		error_exit("No database selected to work on");

	if (pdf_file == "")
		error_exit("No PDF file selected to write to");

	try
	{
		HPDF_Doc pdf = HPDF_New(NULL, NULL); 
		if (!pdf)
			error_exit("Problem initializing PDF library");

		if (HPDF_SetCompressionMode(pdf, HPDF_COMP_ALL))
			error_exit("Failed to set compression mode in PDF library");

		set_pdf_meta(pdf);

		HPDF_Font fnt_text = load_font(pdf, font_file_text);

		HPDF_Font fnt_title = fnt_text;
		if (font_file_text != font_file_title)
			fnt_title = load_font(pdf, font_file_title);

		HPDF_Image logo_img = HPDF_LoadPngImageFromMem(pdf, (const HPDF_BYTE *)logo, logo_size);
		if (!logo_img)
			error_exit("Error retrieving logo from memory");

		int page_nr = 1;
		std::vector<eval_t> page_index;

		double fnt_text_height = 10.0;

		add_frontpage(db, pdf, logo_img, fnt_title, fnt_text, fnt_text_height, use_internet, &page_nr, &page_index);

		double i_offset_x = -1.0, i_offset_y = -1.0, i_w = -1.0, i_h = -1.0;
		HPDF_Page index_page = create_page(pdf, logo_img, "index", fnt_title, fnt_text, &i_offset_x, &i_offset_y, &i_w, &i_h, &page_nr, &page_index, true);

		// add_kmPl(db, pdf, logo_img, fnt_title, fnt_text, fnt_text_height, &page_nr, &page_index, portrait);

		add_sensors_heat_diagram(db, pdf, logo_img, fnt_title, fnt_text, fnt_text_height, ignore_list, &page_nr, &page_index, portrait);

		add_rpm_versus_speed(db, pdf, logo_img, fnt_title, fnt_text, fnt_text_height, &page_nr, &page_index, portrait);

		add_graph_speed_versus_rpm(db, pdf, logo_img, fnt_title, fnt_text, fnt_text_height, &page_nr, &page_index, portrait);

		add_vehicle_speed_versus_gps_speed(db, pdf, logo_img, fnt_title, fnt_text, fnt_text_height, &page_nr, &page_index, portrait);

		// add_rpm_div_throttle(db, pdf, logo_img, fnt_title, fnt_text, fnt_text_height, &page_nr, &page_index, portrait);

		if (use_internet && (!check_table_exists(db, GPS_MAP_LINES) || !check_table_exists(db, GPS_MAP_NODES)))
			load_gps_map(db);

		add_sensor_graphs(db, pdf, logo_img, fnt_title, fnt_text, fnt_text_height, ignore_list, &page_nr, &page_index, portrait);

		add_colofon(db, pdf, logo_img, fnt_title, fnt_text, fnt_text_height, &page_nr, &page_index, portrait);

		draw_index_page(pdf, index_page, &page_index, fnt_text, fnt_text_height, i_offset_x, i_offset_y, i_w, i_h);

		if (HPDF_SaveToFile(pdf, pdf_file.c_str()))
			error_exit("Failed to write to PDF file %s", pdf_file.c_str());

		HPDF_Free(pdf);

		std::cout << "File " << pdf_file << " created" << std::endl;
		std::cout << std::endl;
	}
	catch(std::string & error_str)
	{
		error_exit(error_str.c_str());
	}

	sqlite3_close(db);

	return 1;
}
