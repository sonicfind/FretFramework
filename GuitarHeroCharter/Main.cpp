#include "Chart.h"
#include <iostream>
int main()
{
	std::string chartfile;
	std::getline(std::cin, chartfile);
	if (chartfile[0] == '\"')
		chartfile = chartfile.substr(1, chartfile.length() - 2);

	
	std::ifstream inFile(chartfile);
	if (inFile)
	{
		Chart ch(inFile);
		inFile.close();
	}
	return 0;
}