#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
int main() {
  const char* start_dir = getenv("HOME");
  std::string file_path = std::string(start_dir)+"/kubsh_history";


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
			if(input=="echo") std::cout<<123<<"\n";
			break;
		}
		default:{std::cout<<input<<"\n";break;}
		}
	}
  output.close();
}
