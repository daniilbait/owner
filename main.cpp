#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <sstream>
int main() {
  const char* start_dir = std::getenv("HOME");
  std::string start_path = std::string(start_dir)+"/kubsh_history";
  const char* PATH_dir = std::getenv("PATH");
  
 
  
  std::string input;
  std::ofstream output(start_path, std::ios::app);
  while(std::getline(std::cin, input))
	{
	output<<input<<"\n";
	char letter=input.at(0);
	switch(letter)
		{
		case '/':
		{
			if(input=="/q") {output.close(); return 0;}
			else if (input=="/e $PATH") 
			{
				std::string path_str(PATH_dir);
				std::stringstream ss(path_str);
				std::string directory;
				std::cout<<"Directories in PATH:\n";
				int count=1;
				while(std::getline(ss, directory, ':'))
				{
					std::cout<<count<<". "<<directory<<"\n";
					count++;
				}
			}
			break;
		}
		case 'e':
		{
			if(input=="echo") std::cout<<123<<"\n";
			break;
		}
		default:{std::cout<<"There is no such command!\n";break;}
		}
	}
  output.close();
}
