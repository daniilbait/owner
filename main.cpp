#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
int main() {
  const char* start_dir = getenv("HOME");
  std::string file_path = std::string(start_dir)+"/.kubsh_history";


  std::string input;
  std::ofstream output(file_path, std::ios::app);
  while(std::getline(std::cin, input))
	{
	output<<input<<"\n";
	char letter=input.at(0);
	switch(letter)
	{
		case '/':
		{
			if(input=="/q") {output.close(); return 0;}
			else std::cout<<input<<"\n";
			break;
		}
		case 'e':
		{
			if(input.find("echo \"")==0 && input.find("\"")!=input.rfind("\"") && input.substr(input.length()-1,1)=="\"")
			{
				std::cout<<input.substr(input.find("\"")+1, input.length()-7)<<"\n";
			}
			else std::cout<<input<<"\n";
			break;
		}
		default:{std::cout<<input<<"\n";break;}
	}
  }
  output.close();
}
