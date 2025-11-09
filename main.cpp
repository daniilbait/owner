#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstdlib>
int main() {
  const char* start_dir = getenv("HOME");
  std::string file_path = std::string(start_dir)+"/.kubsh_history";

  const char* path_dir = getenv("PATH");
  std::string path_intermediate = std::string(path_dir);
  std::vector<std::string> path_paths;
  while(!path_intermediate.empty())
  {
	size_t ind = path_intermediate.find(':');
	if(ind == std::string::npos)
	{
		path_paths.push_back(path_intermediate);
		break;
	}
	path_paths.push_back(path_intermediate.substr(0,ind));
	path_intermediate=path_intermediate.substr(ind+1);
  }

  std::string input;
  std::ofstream output(file_path, std::ios::app);
  while(std::getline(std::cin, input))
	{
	char letter=input.at(0);
	switch(letter)
	{
		case '\\':
		{
			if(input=="\\q") 
			{
				output<<input<<"\n";
				output.close();
				return 0;
			}
			else if(input=="\\e $PATH")
			{
				output<<input<<"\n";
				for(std::string str : path_paths)
					std::cout<<str<<"\n";
			}
			else std::cout<<"There is no such command!\n";
			break;
		}
		case 'e':
		{
			if(input.find("echo \"")==0 && input.find("\"")!=input.rfind("\"") && input.substr(input.length()-1,1)=="\"")
			{
				output<<input<<"\n";
				std::cout<<input.substr(input.find("\"")+1, input.length()-7)<<"\n";
			}
			else std::cout<<"There is no such command!\n";
			break;
		}
		default:
		{
			std::cout<<"There is no such command!\n";
			break;
		}
	}
  }
  output<<"^D\n";
  output.close();
}
