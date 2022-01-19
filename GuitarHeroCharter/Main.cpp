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
		bool v2 = chartfile.find(".chart2") != std::string::npos;
		Chart ch(inFile, v2);
		inFile.close();
		
		if (!v2)
		{
			std::ofstream outFile(chartfile + "2", std::ios_base::trunc);
			ch.write_chart(outFile, true);
			outFile.close();
		}
	}
	return 0;
}