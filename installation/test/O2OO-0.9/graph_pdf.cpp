// SVN: $Revision: 442 $
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <hpdf.h>

#include "error.h"
#include "utils.h"
#include "graph_pdf.h"

graph_pdf::graph_pdf()
{
}

graph_pdf::~graph_pdf()
{
}

void graph_pdf::calc_text_width(HPDF_Page page, HPDF_Font font, double font_height, std::string str, double *width, double *height)
{
	HPDF_Page_SetFontAndSize(page, font, font_height);

	*width = HPDF_Page_TextWidth(page, str.c_str());
	*height = font_height;
}

void graph_pdf::draw_text(HPDF_Page page, HPDF_Font font, double font_height, double r, double g, double b, std::string str, double x, double y)
{
	HPDF_Page_BeginText(page);

	HPDF_Page_SetRGBFill(page, r, g, b);

	HPDF_Page_SetFontAndSize(page, font, font_height);

	HPDF_Page_TextOut(page, x, y, str.c_str());

	HPDF_Page_EndText(page);
}

void graph_pdf::reset_pdf(HPDF_Page page)
{
	HPDF_Page_BeginText(page);
	HPDF_Page_SetRGBFill(page, 0, 0, 0);
	HPDF_Page_EndText(page);

	HPDF_Page_SetRGBStroke(page, 0, 0, 0);
}

void graph_pdf::draw_line(HPDF_Page page, double x1, double y1, double x2, double y2, double r, double g, double b)
{
        if (HPDF_Page_SetLineWidth(page, 1))
		error_exit("HPDF_Page_SetLineWidth failed");

	if (HPDF_Page_SetRGBStroke(page, r, g, b))
		error_exit("HPDF_Page_SetRGBStroke(%f, %f, %f) failed", r, g, b);

	if (HPDF_Page_MoveTo(page, x1, y1))
		error_exit("HPDF_Page_MoveTo(%f, %f) failed", x1, y1);

	if (HPDF_Page_LineTo(page, x2, y2))
		error_exit("HPDF_Page_LineTo(%f, %f) failed", x2, y2);

        if (HPDF_Page_Stroke(page))
		error_exit("HPDF_Page_Stroke failed");
}

void graph_pdf::draw_dot(HPDF_Page page, double x, double y, double radius, double r, double g, double b)
{
	if (HPDF_Page_SetRGBStroke(page, r, g, b))
		error_exit("HPDF_Page_SetRGBStroke(%f, %f, %f) failed", r, g, b);

	if (HPDF_Page_Circle(page, x, y, radius))
		error_exit("HPDF_Page_Circle(%f, %f) failed", x, y);

        if (HPDF_Page_Stroke(page))
		error_exit("HPDF_Page_Stroke failed");
}

void graph_pdf::do_draw(HPDF_Page page, HPDF_Font font, double x_off, double y_off, double width, double height, long int *ts, double *values, int n_values)
{
	double y_top = y_off + height;

	double yAxisTop = 12.0;
	double yAxisBottom = height - 25.0;
	int yTicks = 10;
	int xTicks = -1;
	double xAxisLeft = -1.0;
	double xAxisRight = width - 5.0;
	double font_height = 10.0;

	double black_r = 0.0, black_g = 0.0, black_b = 0.0;
	double gray_r = 0.5, gray_g = 0.5, gray_b = 0.5;
	// double white_r = 1.0, white_g = 1.0, white_b = 1.0;
	double red_r = 1.0, red_g = 0.0, red_b = 0.0;
	double green_r = 0.0, green_g = 1.0, green_b = 0.0;
	double yellow_r = 1.0, yellow_g = 1.0, yellow_b = 0.0;
	double magenta_r = 1.0, magenta_g = 0.0, magenta_b = 1.0;

	// determine center of date string
	double dateWidth = -1, dummy;
	calc_text_width(page, font, 10.0, "8888/88/88", &dateWidth, &dummy);
	double timeWidth = -1;
	calc_text_width(page, font, 10.0, "88:88:88", &timeWidth, &dummy);

	double dataMin = 99999999999.9;
	double dataMax = -99999999999.9;
	long int tMin = 99999999999;
	long int tMax = -99999999999;
	double avg = 0.0, sd = 0.0;
	double SUMx = 0, SUMy = 0, SUMxy = 0, SUMxx = 0; // for least squares
	for(int index=0; index<n_values; index++)
	{
		if (ts[index] < tMin) tMin = ts[index];
		if (ts[index] > tMax) tMax = ts[index];
		if (values[index] < dataMin) dataMin = values[index];
		if (values[index] > dataMax) dataMax = values[index];

		avg += values[index];
		sd += pow(values[index], 2.0);
	}
	for(int index=0; index<n_values; index++)
	{
		double t = double(ts[index] - tMin);
		SUMx += t;
		SUMy += values[index];
		SUMxx += t * t;
		SUMxy += t * values[index];
	}
	avg /= double(n_values);
	sd /= double(n_values);

	double slope = (SUMx * SUMy - double(n_values) * SUMxy) / (pow(SUMx, 2.0) - double(n_values) * SUMxx);
	double y_intercept = (SUMy - slope * SUMx) / double(n_values);

	// determine x-position of y-axis
	std::string use_width = "9999W";
	if (dataMin < 0)
		use_width = "-9999W";
	calc_text_width(page, font, font_height, use_width, &xAxisLeft, &dummy);
	xAxisLeft++; // 1 pixel extra space between text and lines

	double metaWidth = -1;
	std::string metaStr = "red: data, green: average, yellow: standard deviation, magenta: least squares";
	calc_text_width(page, font, 8.0, metaStr, &metaWidth, &dummy);
	double metaX = width / 2.0 - metaWidth / 2;
	draw_text(page, font, 8.0, black_r, black_g, black_b, metaStr, x_off + metaX, y_top - 8.0);

	xTicks = (width - xAxisLeft) / dateWidth;

	double scaleX = (xAxisRight - xAxisLeft) / double(tMax - tMin);
	double scaleY = (yAxisBottom - yAxisTop) / (dataMax - dataMin);
	double scaleT = double(tMax - tMin) / (double)xTicks;

	draw_line(page, x_off + xAxisLeft, y_top - yAxisTop, x_off + xAxisLeft, y_top - yAxisBottom, black_r, black_g, black_b);
	draw_line(page, x_off + xAxisLeft, y_top - yAxisBottom, x_off + xAxisRight, y_top - yAxisBottom, black_r, black_g, black_b);

	// draw ticks horizonal
	for(int xti=0; xti<=xTicks; xti++)
	{
		double x = (double(xAxisRight - xAxisLeft) * double(xti)) / double(xTicks) + xAxisLeft;

		time_t epoch = tMin + scaleT * double(xti);
		struct tm *tm = localtime(&epoch);

		char buffer[128];
		strftime(buffer, sizeof buffer, "%Y/%m/%d", tm);
		std::string strDate = std::string(buffer);
		strftime(buffer, sizeof buffer, "%H:%M:%S", tm);
		std::string strTime = std::string(buffer);

		if (xti > 0)
			draw_line(page, x_off + x, y_top - (yAxisTop + 1), x_off + x, y_top - yAxisBottom, gray_r, gray_g, gray_b);

		bool date = true;
		double xPosDate = -1.0, xPosTime = -1.0;
		if (xti == 0)
			xPosTime = xPosDate = mymax(0.0, x - dateWidth / 2.0);
		else if (xti == xTicks)
		{
			xPosDate = width - dateWidth;
			xPosTime = width - timeWidth;
		}
		else if (xti == xTicks - 1)
		{
			xPosTime = xPosDate = x - (dateWidth * 5.0) / 8.0;
			date = false;
		}
		else
		{
			xPosTime = xPosDate = x - dateWidth / 2.0;
		}

		draw_text(page, font, font_height, black_r, black_g, black_b, strTime, x_off + xPosTime, y_top - (yAxisBottom + 14.0));
		if (date)
			draw_text(page, font, font_height, black_r, black_g, black_b, strDate, x_off + xPosDate, y_top - (yAxisBottom + 24.0));

		draw_line(page, x_off + x, y_top - yAxisBottom, x_off + x, y_top - (yAxisBottom + 2.0), black_r, black_g, black_b);
	}

	// draw ticks vertical
	for(int yti=0; yti<=yTicks; yti++)
	{
		double y = (double(yAxisBottom - yAxisTop) * double(yti)) / double(yTicks) + yAxisTop;
		draw_line(page, x_off + xAxisLeft - 2, y_top - y, x_off + xAxisLeft, y_top - y, black_r, black_g, black_b);

		double value = (((dataMax - dataMin) / double(yTicks)) * double(yTicks - yti) + dataMin);

		std::string str = shorten(value);

		draw_line(page, x_off + xAxisLeft + 1, y_top - y, x_off + xAxisRight, y_top - y, gray_r, gray_g, gray_b);

		draw_text(page, font, font_height, black_r, black_g, black_b, str, x_off + 1.0, y_top - (y == yAxisTop ? y + 6.0 : y + 3.0));
	}

	// draw data
	if (n_values > 1 && dataMax - dataMin > 0.001)
	{
		bool first = true;
		double yPrev = -1.0, xPrev = -1.0;
		for(int index=0; index<n_values; index++)
		{
			double x = xAxisLeft + scaleX * double(ts[index] - tMin);
			double y = yAxisBottom - scaleY * (values[index] - dataMin);

			if (first)
			{
				xPrev = x;
				yPrev = y;
				first = false;
			}
			else
			{
				draw_line(page, x_off + xPrev, y_top - yPrev, x_off + x, y_top - y, red_r, red_g, red_b);
				xPrev = x;
				yPrev = y;
			}
		}

		first = true;
		double yLSPrev = -1.0, xLSPrev = -1.0;
		for(int index=0; index<n_values; index++)
		{
			double useT = ts[index] - tMin;
			double x = xAxisLeft + scaleX * double(useT);

			double LSVal = slope * double(useT) + y_intercept;
			double yLS = yAxisBottom - scaleY * double(LSVal - dataMin);

			if (LSVal >= dataMin && LSVal < dataMax)
			{
				if (first)
				{
					xLSPrev = x;
					yLSPrev = yLS;

					first = false;
				}
				else
				{
					draw_line(page, x_off + xLSPrev, y_top - yLSPrev, x_off + x, y_top - yLS, magenta_r, magenta_g, magenta_b);

					xLSPrev = x;
					yLSPrev = yLS;
				}
			}
		}

		double yAvg = yAxisBottom - scaleY * (avg - dataMin);
		draw_line(page, x_off + xAxisLeft + 1, y_top - yAvg, x_off + xAxisRight, y_top - yAvg, green_r, green_g, green_b);

		double stddev = sqrt(sd - pow(avg, 2.0));
		if (avg - stddev >= dataMin)
		{
			double ySD = yAxisBottom - scaleY * ((avg - stddev) - dataMin);
			draw_line(page, x_off + xAxisLeft + 1, y_top - ySD, x_off + xAxisRight, y_top - ySD, yellow_r, yellow_g, yellow_b);
		}

		if (avg + stddev < dataMax)
		{
			double ySD = yAxisBottom - scaleY * ((avg + stddev) - dataMin);
			draw_line(page, x_off + xAxisLeft + 1, y_top - ySD, x_off + xAxisRight, y_top - ySD, yellow_r, yellow_g, yellow_b);
		}
	}
	else
	{
		draw_text(page, font, font_height * 1.5, red_r, red_g, red_b, "No data or data too constant", x_off + xAxisLeft + 5.0, y_top - (height / 2.0 + yAxisTop / 2.0));
	}

	reset_pdf(page);
}

void graph_pdf::do_draw_generic(HPDF_Page page, HPDF_Font font, double x_off, double y_off, double width, double height, double *val_x, value_t *val_y, bool use_mima, int n_values, bool draw_avg, bool draw_sd, bool draw_ls)
{
	if (use_mima && draw_sd)
		error_exit("mima and sd do not mix");

	double y_top = y_off + height;

	double yAxisTop = 12.0;
	double yAxisBottom = height - 12.5;
	int yTicks = 10;
	int xTicks = -1;
	double xAxisLeft = -1.0;
	double xAxisRight = width - 5.0;
	double font_height = 10.0;

	double black_r = 0.0, black_g = 0.0, black_b = 0.0;
	double gray_r = 0.5, gray_g = 0.5, gray_b = 0.5;
	// double white_r = 1.0, white_g = 1.0, white_b = 1.0;
	double red_r = 1.0, red_g = 0.0, red_b = 0.0;
	double green_r = 0.0, green_g = 1.0, green_b = 0.0;
	double yellow_r = 1.0, yellow_g = 1.0, yellow_b = 0.0;
	double dark_yellow_r = 0.75, dark_yellow_g = 0.75, dark_yellow_b = 0.0;
	double magenta_r = 1.0, magenta_g = 0.0, magenta_b = 1.0;

	double xMin = 99999999999.9;
	double xMax = -99999999999.9;
	double yMin = 99999999999.9;
	double yMax = -99999999999.9;
	double avg = 0.0, sd = 0.0;
	double SUMx = 0, SUMy = 0, SUMxy = 0, SUMxx = 0; // for least squares
	for(int index=0; index<n_values; index++)
	{
		if (val_x[index] < xMin) xMin = val_x[index];
		if (val_x[index] > xMax) xMax = val_x[index];

		if (use_mima)
		{
			if (val_y[index].mi < yMin) yMin = val_y[index].mi;
			if (val_y[index].ma > yMax) yMax = val_y[index].ma;
		}
		else
		{
			if (val_y[index].av < yMin) yMin = val_y[index].av;
			if (val_y[index].av > yMax) yMax = val_y[index].av;
		}

		avg += val_y[index].av;
		sd += pow(val_y[index].av, 2.0);
	}
	avg /= double(n_values);
	sd /= double(n_values);

	double slope = -1.0, y_intercept = -1.0;
	if (draw_ls)
	{
		for(int index=0; index<n_values; index++)
		{
			double vx = val_x[index];
			double vy = val_y[index].av;
			SUMx += vx;
			SUMy += vy;
			SUMxx += vx * vx;
			SUMxy += vx * vy;
		}

		slope = (SUMx * SUMy - double(n_values) * SUMxy) / (pow(SUMx, 2.0) - double(n_values) * SUMxx);
		y_intercept = (SUMy - slope * SUMx) / double(n_values);
	}

	// determine x-position of y-axis
	std::string use_width = "9999W";
	if (yMin < 0)
		use_width = "-9999W";
	double valWidth = -1.0, dummy;
	calc_text_width(page, font, font_height, use_width, &valWidth, &dummy);
	xAxisLeft = valWidth + 1; // 1 pixel extra space between text and lines

	double metaWidth = -1;
	std::string metaStr = "red: data";
	if (draw_avg)
		metaStr += ", green: average";
	if (use_mima)
		metaStr += ", yellow: min/max";
	else if (draw_sd)
		metaStr += ", yellow: standard deviation";
	if (draw_ls)
		metaStr += ", magenta: least squares";
	calc_text_width(page, font, 8.0, metaStr, &metaWidth, &dummy);
	double metaX = width / 2.0 - metaWidth / 2;
	draw_text(page, font, 8.0, black_r, black_g, black_b, metaStr, x_off + metaX, y_top - 8.0);

	xTicks = (width - xAxisLeft) / valWidth;

	double scaleX = (xAxisRight - xAxisLeft) / (xMax - xMin);
	double scaleY = (yAxisBottom - yAxisTop) / (yMax - yMin);
	double scaleX2 = (xMax - xMin) / double(xTicks);
	// double scaleY2 = (yMax - yMin) / double(yTicks);

	draw_line(page, x_off + xAxisLeft, y_top - yAxisTop,    x_off + xAxisLeft,  y_top - yAxisBottom, black_r, black_g, black_b);
	draw_line(page, x_off + xAxisLeft, y_top - yAxisBottom, x_off + xAxisRight, y_top - yAxisBottom, black_r, black_g, black_b);

	// draw ticks horizonal
	for(int xti=0; xti<=xTicks; xti++)
	{
		double x = (xAxisRight - xAxisLeft) * double(xti) / double(xTicks) + xAxisLeft;

		if (xti > 0)
			draw_line(page, x_off + x, y_top - (yAxisTop + 1), x_off + x, y_top - yAxisBottom, gray_r, gray_g, gray_b);

		double xPosValue = -1.0;
		if (xti == 0)
			xPosValue = mymax(0.0, x - valWidth / 2.0);
		else if (xti == xTicks)
			xPosValue = width - valWidth;
		else if (xti == xTicks - 1)
			xPosValue = x - (valWidth * 5.0) / 8.0;
		else
			xPosValue = x - valWidth / 2.0;

		std::string str = shorten(xMin + double(xti) * scaleX2);

		draw_text(page, font, font_height, black_r, black_g, black_b, str, x_off + xPosValue, y_top - (yAxisBottom + 12.5));

		draw_line(page, x_off + x, y_top - yAxisBottom, x_off + x, y_top - (yAxisBottom + 2.0), black_r, black_g, black_b);
	}

	// draw ticks vertical
	for(int yti=0; yti<=yTicks; yti++)
	{
		double y = (yAxisBottom - yAxisTop) * double(yti) / double(yTicks) + yAxisTop;
		draw_line(page, x_off + xAxisLeft - 2.0, y_top - y, x_off + xAxisLeft, y_top - y, black_r, black_g, black_b);

		double value = (yMax - yMin) / double(yTicks) * double(yTicks - yti) + yMin;

		std::string str = shorten(value);

		draw_line(page, x_off + xAxisLeft + 1.0, y_top - y, x_off + xAxisRight, y_top - y, gray_r, gray_g, gray_b);

		draw_text(page, font, font_height, black_r, black_g, black_b, str, x_off + 1.0, y_top - (y == yAxisTop ? y + 6.0 : y + 3.0));
	}

	// draw data
	if (n_values > 1 && xMax != xMin)
	{
		bool first = true;
		double yPrev = -1.0, xPrev = -1.0;
		for(int index=0; index<n_values; index++)
		{
			double vx = val_x[index];
			double vy = val_y[index].av;
			double x = xAxisLeft   + scaleX * (vx - xMin);
			double y = yAxisBottom - scaleY * (vy - yMin);

			if (first)
			{
				xPrev = x;
				yPrev = y;
				first = false;
			}
			else
			{
				draw_line(page, x_off + xPrev, y_top - yPrev, x_off + x, y_top - y, red_r, red_g, red_b);
				xPrev = x;
				yPrev = y;
			}
		}

		if (use_mima)
		{
			first = true;
			for(int index=0; index<n_values; index++)
			{
				double vx = val_x[index];
				double vy = val_y[index].mi;
				double x = xAxisLeft   + scaleX * (vx - xMin);
				double y = yAxisBottom - scaleY * (vy - yMin);

				if (first)
				{
					xPrev = x;
					yPrev = y;
					first = false;
				}
				else
				{
					draw_line(page, x_off + xPrev, y_top - yPrev, x_off + x, y_top - y, yellow_r, yellow_g, yellow_b);
					xPrev = x;
					yPrev = y;
				}
			}
			first = true;
			for(int index=0; index<n_values; index++)
			{
				double vx = val_x[index];
				double vy = val_y[index].ma;
				double x = xAxisLeft   + scaleX * (vx - xMin);
				double y = yAxisBottom - scaleY * (vy - yMin);

				if (first)
				{
					xPrev = x;
					yPrev = y;
					first = false;
				}
				else
				{
					draw_line(page, x_off + xPrev, y_top - yPrev, x_off + x, y_top - y, dark_yellow_r, dark_yellow_g, dark_yellow_b);
					xPrev = x;
					yPrev = y;
				}
			}
		}

		if (draw_ls)
		{
			first = true;
			double yLSPrev = -1.0, xLSPrev = -1.0;
			for(int index=0; index<n_values; index++)
			{
				double vx = val_x[index];
				double x = xAxisLeft + scaleX * (vx - xMin);

				double vy = val_y[index].av;
				double LSVal = slope * vy + y_intercept;
				double yLS = yAxisBottom - scaleY * LSVal;

				if (LSVal >= yMin && LSVal < yMax)
				{
					if (first)
					{
						xLSPrev = x;
						yLSPrev = yLS;

						first = false;
					}
					else
					{
						draw_line(page, x_off + xLSPrev, y_top - yLSPrev, x_off + x, y_top - yLS, magenta_r, magenta_g, magenta_b);

						xLSPrev = x;
						yLSPrev = yLS;
					}
				}
			}
		}

		if (draw_avg)
		{
			double yAvg = yAxisBottom - scaleY * (avg - yMin);
			draw_line(page, x_off + xAxisLeft + 1, y_top - yAvg, x_off + xAxisRight, y_top - yAvg, green_r, green_g, green_b);
		}

		if (draw_sd)
		{
			double stddev = sqrt(sd - pow(avg, 2.0));
			if (avg - stddev >= yMin)
			{
				double ySD = yAxisBottom - scaleY * ((avg - stddev) - yMin);
				draw_line(page, x_off + xAxisLeft + 1, y_top - ySD, x_off + xAxisRight, y_top - ySD, yellow_r, yellow_g, yellow_b);
			}

			if (avg + stddev < yMax)
			{
				double ySD = yAxisBottom - scaleY * ((avg + stddev) - yMin);
				draw_line(page, x_off + xAxisLeft + 1, y_top - ySD, x_off + xAxisRight, y_top - ySD, yellow_r, yellow_g, yellow_b);
			}
		}
	}
	else
	{
		draw_text(page, font, font_height * 1.5, red_r, red_g, red_b, "No data or data too constant", x_off + xAxisLeft + 5.0, y_top - (height / 2.0 + yAxisTop / 2.0));
	}

	reset_pdf(page);
}

void graph_pdf::do_draw_point_cloud(HPDF_Page page, HPDF_Font font, double x_off, double y_off, double width, double height, double *val_x, double *val_y, int n_values, bool draw_avg, bool draw_sd, bool draw_ls)
{
	double y_top = y_off + height;

	double yAxisTop = 12.0;
	double yAxisBottom = height - 12.5;
	int yTicks = 10;
	int xTicks = -1;
	double xAxisLeft = -1.0;
	double xAxisRight = width - 5.0;
	double font_height = 10.0;

	double black_r = 0.0, black_g = 0.0, black_b = 0.0;
	double gray_r = 0.5, gray_g = 0.5, gray_b = 0.5;
	// double white_r = 1.0, white_g = 1.0, white_b = 1.0;
	double red_r = 1.0, red_g = 0.0, red_b = 0.0;
	double green_r = 0.0, green_g = 1.0, green_b = 0.0;
	double yellow_r = 1.0, yellow_g = 1.0, yellow_b = 0.0;
	// double dark_yellow_r = 0.75, dark_yellow_g = 0.75, dark_yellow_b = 0.0;
	double magenta_r = 1.0, magenta_g = 0.0, magenta_b = 1.0;

	double xMin = 99999999999.9;
	double xMax = -99999999999.9;
	double yMin = 99999999999.9;
	double yMax = -99999999999.9;
	double avg = 0.0, sd = 0.0;
	double SUMx = 0, SUMy = 0, SUMxy = 0, SUMxx = 0; // for least squares
	for(int index=0; index<n_values; index++)
	{
		if (val_x[index] < xMin) xMin = val_x[index];
		if (val_x[index] > xMax) xMax = val_x[index];

		if (val_y[index] < yMin) yMin = val_y[index];
		if (val_y[index] > yMax) yMax = val_y[index];

		avg += val_y[index];
		sd += pow(val_y[index], 2.0);
	}
	avg /= double(n_values);
	sd /= double(n_values);

	double slope = -1.0, y_intercept = -1.0;
	if (draw_ls)
	{
		for(int index=0; index<n_values; index++)
		{
			double vx = val_x[index];
			double vy = val_y[index];
			SUMx += vx;
			SUMy += vy;
			SUMxx += vx * vx;
			SUMxy += vx * vy;
		}

		slope = (SUMx * SUMy - double(n_values) * SUMxy) / (pow(SUMx, 2.0) - double(n_values) * SUMxx);
		y_intercept = (SUMy - slope * SUMx) / double(n_values);
	}

	// determine x-position of y-axis
	std::string use_width = "9999W";
	if (yMin < 0)
		use_width = "-9999W";
	double valWidth = -1.0, dummy;
	calc_text_width(page, font, font_height, use_width, &valWidth, &dummy);
	xAxisLeft = valWidth + 1; // 1 pixel extra space between text and lines

	double metaWidth = -1;
	std::string metaStr = "red: data";
	if (draw_avg)
		metaStr += ", green: average";
	if (draw_sd)
		metaStr += ", yellow: standard deviation";
	if (draw_ls)
		metaStr += ", magenta: least squares";
	calc_text_width(page, font, 8.0, metaStr, &metaWidth, &dummy);
	double metaX = width / 2.0 - metaWidth / 2;
	draw_text(page, font, 8.0, black_r, black_g, black_b, metaStr, x_off + metaX, y_top - 8.0);

	xTicks = (width - xAxisLeft) / valWidth;

	double scaleX = (xAxisRight - xAxisLeft) / (xMax - xMin);
	double scaleY = (yAxisBottom - yAxisTop) / (yMax - yMin);
	double scaleX2 = (xMax - xMin) / double(xTicks);
	// double scaleY2 = (yMax - yMin) / double(yTicks);

	draw_line(page, x_off + xAxisLeft, y_top - yAxisTop,    x_off + xAxisLeft,  y_top - yAxisBottom, black_r, black_g, black_b);
	draw_line(page, x_off + xAxisLeft, y_top - yAxisBottom, x_off + xAxisRight, y_top - yAxisBottom, black_r, black_g, black_b);

	// draw ticks horizonal
	for(int xti=0; xti<=xTicks; xti++)
	{
		double x = (xAxisRight - xAxisLeft) * double(xti) / double(xTicks) + xAxisLeft;

		if (xti > 0)
			draw_line(page, x_off + x, y_top - (yAxisTop + 1), x_off + x, y_top - yAxisBottom, gray_r, gray_g, gray_b);

		double xPosValue = -1.0;
		if (xti == 0)
			xPosValue = mymax(0.0, x - valWidth / 2.0);
		else if (xti == xTicks)
			xPosValue = width - valWidth;
		else if (xti == xTicks - 1)
			xPosValue = x - (valWidth * 5.0) / 8.0;
		else
			xPosValue = x - valWidth / 2.0;

		std::string str = shorten(xMin + double(xti) * scaleX2);

		draw_text(page, font, font_height, black_r, black_g, black_b, str, x_off + xPosValue, y_top - (yAxisBottom + 12.5));

		draw_line(page, x_off + x, y_top - yAxisBottom, x_off + x, y_top - (yAxisBottom + 2.0), black_r, black_g, black_b);
	}

	// draw ticks vertical
	for(int yti=0; yti<=yTicks; yti++)
	{
		double y = (yAxisBottom - yAxisTop) * double(yti) / double(yTicks) + yAxisTop;
		draw_line(page, x_off + xAxisLeft - 2.0, y_top - y, x_off + xAxisLeft, y_top - y, black_r, black_g, black_b);

		double value = (yMax - yMin) / double(yTicks) * double(yTicks - yti) + yMin;

		std::string str = shorten(value);

		draw_line(page, x_off + xAxisLeft + 1.0, y_top - y, x_off + xAxisRight, y_top - y, gray_r, gray_g, gray_b);

		draw_text(page, font, font_height, black_r, black_g, black_b, str, x_off + 1.0, y_top - (y == yAxisTop ? y + 6.0 : y + 3.0));
	}

	// draw data
	if (n_values > 0)
	{
		for(int index=0; index<n_values; index++)
		{
			double vx = val_x[index];
			double vy = val_y[index];
			double x = xAxisLeft   + scaleX * (vx - xMin);
			double y = yAxisBottom - scaleY * (vy - yMin);

			draw_dot(page, x_off + x, y_top - y, 1.0, red_r, red_g, red_b);
		}

		if (draw_ls)
		{
			bool first = true;
			double yLSPrev = -1.0, xLSPrev = -1.0;
			for(int index=0; index<n_values; index++)
			{
				double vx = val_x[index];
				double x = xAxisLeft + scaleX * (vx - xMin);

				double vy = val_y[index];
				double LSVal = slope * vy + y_intercept;
				double yLS = yAxisBottom - scaleY * LSVal;

				if (LSVal >= yMin && LSVal < yMax)
				{
					if (first)
					{
						xLSPrev = x;
						yLSPrev = yLS;

						first = false;
					}
					else
					{
						draw_line(page, x_off + xLSPrev, y_top - yLSPrev, x_off + x, y_top - yLS, magenta_r, magenta_g, magenta_b);

						xLSPrev = x;
						yLSPrev = yLS;
					}
				}
			}
		}

		if (draw_avg)
		{
			double yAvg = yAxisBottom - scaleY * (avg - yMin);
			draw_line(page, x_off + xAxisLeft + 1, y_top - yAvg, x_off + xAxisRight, y_top - yAvg, green_r, green_g, green_b);
		}

		if (draw_sd)
		{
			double stddev = sqrt(sd - pow(avg, 2.0));
			if (avg - stddev >= yMin)
			{
				double ySD = yAxisBottom - scaleY * ((avg - stddev) - yMin);
				draw_line(page, x_off + xAxisLeft + 1, y_top - ySD, x_off + xAxisRight, y_top - ySD, yellow_r, yellow_g, yellow_b);
			}

			if (avg + stddev < yMax)
			{
				double ySD = yAxisBottom - scaleY * ((avg + stddev) - yMin);
				draw_line(page, x_off + xAxisLeft + 1, y_top - ySD, x_off + xAxisRight, y_top - ySD, yellow_r, yellow_g, yellow_b);
			}
		}
	}
	else
	{
		draw_text(page, font, font_height * 1.5, red_r, red_g, red_b, "No data", x_off + xAxisLeft + 5.0, y_top - (height / 2.0 + yAxisTop / 2.0));
	}

	reset_pdf(page);
}

void graph_pdf::do_draw_histogram(HPDF_Page page, HPDF_Font font, double x_off, double y_off, double width, double height, std::vector<std::string> *labels_x, double *val_y, int n_values, std::string metaStr)
{
	double y_top = y_off + height;

	double yAxisTop = 12.0;
	double yAxisBottom = height - 12.5;
	int yTicks = 10;
	double xAxisLeft = -1.0;
	double xAxisRight = width - 5.0;
	double font_height = 10.0;

	double black_r = 0.0, black_g = 0.0, black_b = 0.0;
	double gray_r = 0.5, gray_g = 0.5, gray_b = 0.5;
	// double white_r = 1.0, white_g = 1.0, white_b = 1.0;
	double red_r = 1.0, red_g = 0.0, red_b = 0.0;
	// double green_r = 0.0, green_g = 1.0, green_b = 0.0;
	// double yellow_r = 1.0, yellow_g = 1.0, yellow_b = 0.0;
	// double dark_yellow_r = 0.75, dark_yellow_g = 0.75, dark_yellow_b = 0.0;
	// double magenta_r = 1.0, magenta_g = 0.0, magenta_b = 1.0;

	double yMin = 99999999999.9;
	double yMax = -99999999999.9;
	for(int index=0; index<n_values; index++)
	{
		if (val_y[index] < yMin) yMin = val_y[index];
		if (val_y[index] > yMax) yMax = val_y[index];
	}

	// determine x-position of y-axis
	std::string use_width = "9999W";
	if (yMin < 0)
		use_width = "-9999W";
	double valWidth = -1.0, dummy;
	calc_text_width(page, font, font_height, use_width, &valWidth, &dummy);
	xAxisLeft = valWidth + 1; // 1 pixel extra space between text and lines

	double hist_w = (xAxisRight - xAxisLeft) / double(n_values);

	double metaWidth = -1;
	calc_text_width(page, font, 8.0, metaStr, &metaWidth, &dummy);
	double metaX = width / 2.0 - metaWidth / 2;
	draw_text(page, font, 8.0, black_r, black_g, black_b, metaStr, x_off + metaX, y_top - 8.0);

	double scaleY = (yAxisBottom - yAxisTop) / (yMax - yMin);

	draw_line(page, x_off + xAxisLeft, y_top - yAxisTop,    x_off + xAxisLeft,  y_top - yAxisBottom, black_r, black_g, black_b);
	draw_line(page, x_off + xAxisLeft, y_top - yAxisBottom, x_off + xAxisRight, y_top - yAxisBottom, black_r, black_g, black_b);

	// draw ticks horizonal
	for(int index=0; index<n_values; index++)
	{
		double x = hist_w * double(index);

		std::string str = labels_x -> at(index);
		double str_w = -1.0, str_h = -1.0;
		calc_text_width(page, font, font_height, str, &str_w, &str_h);

		double plot_x = x_off + x + hist_w + str_w / 2.0;

		draw_text(page, font, font_height, black_r, black_g, black_b, str, plot_x, y_top - (yAxisBottom + 12.5));
	}

	// draw ticks vertical
	for(int yti=0; yti<=yTicks; yti++)
	{
		double y = (yAxisBottom - yAxisTop) * double(yti) / double(yTicks) + yAxisTop;
		draw_line(page, x_off + xAxisLeft - 2.0, y_top - y, x_off + xAxisLeft, y_top - y, black_r, black_g, black_b);

		double value = (yMax - yMin) / double(yTicks) * double(yTicks - yti) + yMin;

		std::string str = shorten(value);

		draw_line(page, x_off + xAxisLeft + 1.0, y_top - y, x_off + xAxisRight, y_top - y, gray_r, gray_g, gray_b);

		draw_text(page, font, font_height, black_r, black_g, black_b, str, x_off + 1.0, y_top - (y == yAxisTop ? y + 6.0 : y + 3.0));
	}

	// draw data
	if (n_values > 0)
	{
		for(int index=0; index<n_values; index++)
		{
			double x = hist_w * double(index);
			double y = scaleY * (val_y[index] - yMin);

			double x_cur_l = xAxisLeft + x_off + x;
			double x_cur_r = xAxisLeft + x_off + x + hist_w;
			double y_cur_top = y_top - (yAxisBottom - y);
			double y_cur_bot = y_top - (yAxisBottom - 0.0);

			draw_line(page, x_cur_l, y_cur_bot, x_cur_l, y_cur_top, red_r, red_g, red_b);
			draw_line(page, x_cur_l, y_cur_top, x_cur_r, y_cur_top, red_r, red_g, red_b);
			draw_line(page, x_cur_r, y_cur_top, x_cur_r, y_cur_bot, red_r, red_g, red_b);
		}
	}
	else
	{
		draw_text(page, font, font_height * 1.5, red_r, red_g, red_b, "No data", x_off + xAxisLeft + 5.0, y_top - (height / 2.0 + yAxisTop / 2.0));
	}

	reset_pdf(page);
}

void graph_pdf::do_draw(HPDF_Page page, HPDF_Font font, double x_off, double y_off, double width, double height, long int *ts, double *values1, double *values2, int n_values, std::string metaStr)
{
	double y_top = y_off + height;

	double yAxisTop = 12.0;
	double yAxisBottom = height - 25.0;
	int yTicks = 10;
	int xTicks = -1;
	double xAxisLeft = -1.0;
	double xAxisRight = -1.0;
	double font_height = 10.0;

	double black_r = 0.0, black_g = 0.0, black_b = 0.0;
	double gray_r = 0.5, gray_g = 0.5, gray_b = 0.5;
	// double white_r = 1.0, white_g = 1.0, white_b = 1.0;
	double red_r = 1.0, red_g = 0.0, red_b = 0.0;
	double magenta_r = 1.0, magenta_g = 0.0, magenta_b = 1.0;

	// determine center of date string
	double dateWidth = -1, dummy;
	calc_text_width(page, font, 10.0, "8888/88/88", &dateWidth, &dummy);
	double timeWidth = -1;
	calc_text_width(page, font, 10.0, "88:88:88", &timeWidth, &dummy);

	double dataMin1 = 99999999999.9, dataMin2 = dataMin1;
	double dataMax1 = -99999999999.9, dataMax2 = dataMax1;
	long int tMin = 99999999999;
	long int tMax = -99999999999;
	for(int index=0; index<n_values; index++)
	{
		if (ts[index] < tMin) tMin = ts[index];
		if (ts[index] > tMax) tMax = ts[index];
		if (values1[index] < dataMin1) dataMin1 = values1[index];
		if (values1[index] > dataMax1) dataMax1 = values1[index];
		if (values2[index] < dataMin2) dataMin2 = values2[index];
		if (values2[index] > dataMax2) dataMax2 = values2[index];
	}

	// determine x-position of y-axis
	std::string use_width1 = "9999W";
	if (dataMin1 < 0)
		use_width1 = "-9999W";
	calc_text_width(page, font, font_height, use_width1, &xAxisLeft, &dummy);
	xAxisLeft++; // 1 pixel extra space between text and lines
	std::string use_width2 = "9999W";
	if (dataMin2 < 0)
		use_width2 = "-9999W";
	double wdummy = -1;
	calc_text_width(page, font, font_height, use_width2, &wdummy, &dummy);
	xAxisRight = width - (wdummy + 1);

	double metaWidth = -1;
	calc_text_width(page, font, 8.0, metaStr, &metaWidth, &dummy);
	double metaX = width / 2.0 - metaWidth / 2;
	draw_text(page, font, 8.0, black_r, black_g, black_b, metaStr, x_off + metaX, y_top - 8.0);

	xTicks = (width - xAxisLeft) / dateWidth;

	double scaleX = (xAxisRight - xAxisLeft) / double(tMax - tMin);
	double scaleY1 = (yAxisBottom - yAxisTop) / (dataMax1 - dataMin1);
	double scaleY2 = (yAxisBottom - yAxisTop) / (dataMax2 - dataMin2);
	double scaleT = double(tMax - tMin) / (double)xTicks;

	draw_line(page, x_off + xAxisLeft, y_top - yAxisTop, x_off + xAxisLeft, y_top - yAxisBottom, black_r, black_g, black_b);
	draw_line(page, x_off + xAxisLeft, y_top - yAxisBottom, x_off + xAxisRight, y_top - yAxisBottom, black_r, black_g, black_b);

	// draw ticks horizonal
	for(int xti=0; xti<=xTicks; xti++)
	{
		double x = (double(xAxisRight - xAxisLeft) * double(xti)) / double(xTicks) + xAxisLeft;

		time_t epoch = tMin + scaleT * double(xti);
		struct tm *tm = localtime(&epoch);

		char buffer[128];
		strftime(buffer, sizeof buffer, "%Y/%m/%d", tm);
		std::string strDate = std::string(buffer);
		strftime(buffer, sizeof buffer, "%H:%M:%S", tm);
		std::string strTime = std::string(buffer);

		if (xti > 0)
			draw_line(page, x_off + x, y_top - (yAxisTop + 1), x_off + x, y_top - yAxisBottom, gray_r, gray_g, gray_b);

		bool date = true;
		double xPosDate = -1.0, xPosTime = -1.0;
		if (xti == 0)
			xPosTime = xPosDate = mymax(0.0, x - dateWidth / 2.0);
		else if (xti == xTicks)
		{
			xPosDate = width - dateWidth;
			xPosTime = width - timeWidth;
		}
		else if (xti == xTicks - 1)
		{
			xPosTime = xPosDate = x - (dateWidth * 5.0) / 8.0;
			date = false;
		}
		else
		{
			xPosTime = xPosDate = x - dateWidth / 2.0;
		}

		draw_text(page, font, font_height, black_r, black_g, black_b, strTime, x_off + xPosTime, y_top - (yAxisBottom + 14.0));
		if (date)
			draw_text(page, font, font_height, black_r, black_g, black_b, strDate, x_off + xPosDate, y_top - (yAxisBottom + 24.0));

		draw_line(page, x_off + x, y_top - yAxisBottom, x_off + x, y_top - (yAxisBottom + 2.0), black_r, black_g, black_b);
	}

	// draw ticks vertical
	for(int yti=0; yti<=yTicks; yti++)
	{
		double y = (double(yAxisBottom - yAxisTop) * double(yti)) / double(yTicks) + yAxisTop;
		draw_line(page, x_off + xAxisLeft - 2, y_top - y, x_off + xAxisLeft, y_top - y, black_r, black_g, black_b);

		double value1 = (((dataMax1 - dataMin1) / double(yTicks)) * double(yTicks - yti) + dataMin1);
		double value2 = (((dataMax2 - dataMin2) / double(yTicks)) * double(yTicks - yti) + dataMin2);

		std::string str1 = shorten(value1);
		std::string str2 = shorten(value2);

		draw_line(page, x_off + xAxisLeft + 1, y_top - y, x_off + xAxisRight, y_top - y, gray_r, gray_g, gray_b);

		draw_text(page, font, font_height, black_r, black_g, black_b, str1, x_off + 1.0, y_top - (y == yAxisTop ? y + 6.0 : y + 3.0));
		draw_text(page, font, font_height, black_r, black_g, black_b, str2, x_off + xAxisRight + 1.0, y_top - (y == yAxisTop ? y + 6.0 : y + 3.0));
	}

	// draw data
	if (n_values > 1 && (dataMax1 - dataMin1 > 0.001 || dataMax2 - dataMin2 > 0.001))
	{
		bool first = true;
		double yPrev = -1.0, xPrev = -1.0;
		for(int index=0; index<n_values; index++)
		{
			double x = xAxisLeft + scaleX * double(ts[index] - tMin);
			double y = yAxisBottom - scaleY1 * (values1[index] - dataMin1);

			if (first)
			{
				xPrev = x;
				yPrev = y;
				first = false;
			}
			else
			{
				draw_line(page, x_off + xPrev, y_top - yPrev, x_off + x, y_top - y, red_r, red_g, red_b);
				xPrev = x;
				yPrev = y;
			}
		}

		first = true;
		yPrev = xPrev = -1.0;
		for(int index=0; index<n_values; index++)
		{
			double x = xAxisLeft + scaleX * double(ts[index] - tMin);
			double y = yAxisBottom - scaleY2 * (values2[index] - dataMin2);

			if (first)
			{
				xPrev = x;
				yPrev = y;
				first = false;
			}
			else
			{
				draw_line(page, x_off + xPrev, y_top - yPrev, x_off + x, y_top - y, magenta_r, magenta_g, magenta_b);
				xPrev = x;
				yPrev = y;
			}
		}
	}
	else
	{
		draw_text(page, font, font_height * 1.5, red_r, red_g, red_b, "No data or data too constant", x_off + xAxisLeft + 5.0, y_top - (height / 2.0 + yAxisTop / 2.0));
	}

	reset_pdf(page);
}
