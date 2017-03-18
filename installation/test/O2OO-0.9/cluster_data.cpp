#include <algorithm>
#include <iostream>
#include <vector>

static bool split_value_list_sorter(double a, double b)
{
	return a < b;
}

void count_values_in_range(std::vector<double> *values, double mi, double ma, int *n, double *avg)
{
	double total = 0;
	
	*n =0;
	for(unsigned int index=0; index<values -> size(); index++)
	{
		if (values -> at(index) >= mi && values -> at(index) < ma)
		{
			total += values -> at(index);
			(*n)++;
		}
	}
	
	*avg = total / double(*n);
}

void split_value_list_do(std::vector<double> *values, std::vector<double> *markers, double mi, double ma)
{
	int n = 0;
	double avg = 0.0;
	
	count_values_in_range(values, mi, ma, &n, &avg);

	markers -> push_back(avg);
	
	std::sort(markers -> begin(), markers -> end(), split_value_list_sorter);
}

std::vector<double> * split_value_list(std::vector<double> *values_in, int n)
{
	std::vector<double> values = *values_in;
	std::vector<double> *markers = new std::vector<double>();

	std::sort(values.begin(), values.end(), split_value_list_sorter);

	markers -> push_back(values.at(0));
	markers -> push_back(values.at(values.size() - 1));

	for(int nr=0; nr<n; nr++)
	{
		int max_n = 0, chosen_interval = -1;
		
		// find interval with greates number of elements
		for(unsigned int interval=0; interval<(markers -> size() - 1); interval++)
		{
			int cur_n = 0;
			double avg = 0.0;
			
			count_values_in_range(&values, markers -> at(interval), markers -> at(interval + 1), &cur_n, &avg);
			
			if (cur_n > max_n)
			{
				max_n = cur_n;
				chosen_interval = interval;
			}
		}

		if (chosen_interval != -1)
			split_value_list_do(&values, markers, markers -> at(chosen_interval), markers -> at(chosen_interval + 1));
	}

	return markers;
}
