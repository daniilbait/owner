#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <pwd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/inotify.h>
#include <iomanip>
#include <thread>
#include <future>
#include <string>
#include <vector>
#include <set>
using namespace std;


//VFS
void vfs_monitor(const std::string& vfs_path) {
    while (true) {
        DIR* dir = opendir(vfs_path.c_str());
        if (!dir) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            continue;
        }
        
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_type == DT_DIR && std::string(entry->d_name) != "." && std::string(entry->d_name) != "..") {
                std::string username = entry->d_name;
                std::string user_dir = vfs_path + "/" + username;
                
                // Проверяем, есть ли файл id (значит пользователь уже обработан)
                std::string id_file = user_dir + "/id";
                if (access(id_file.c_str(), F_OK) != 0) {
                    // Файла id нет - значит новый пользователь
                    std::string cmd = "useradd -m -s /bin/bash " + username + " 2>/dev/null";
                    system(cmd.c_str());
                    
                    // Создаём файлы VFS
                    std::ofstream(id_file) << "1000";
                    std::ofstream(user_dir + "/home") << "/home/" + username;
                    std::ofstream(user_dir + "/shell") << "/bin/bash";
                }
            }
        }
        closedir(dir);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
//VFS 


void executeCommand(const vector<string>& commands){
	pid_t pid=fork();
	if(pid == -1) {
        perror("fork failed");
        return;
    }
    else if(pid==0)
	{
		vector<char*> argums;
		for (const string & command : commands)
			argums.push_back(const_cast<char*>(command.c_str()));
		argums.push_back(nullptr);

		execvp(argums[0], argums.data());

		perror("execvp failed");
        exit(1);
	}
	else
	{
		int status;
		waitpid(pid, &status, 0);
	}
}

vector<string> parseCommand(string input) {
	vector<string> command;
	string current_str;
	int quote_type=0;//0 - no; 1 - '; 2 - "
	bool screened_before = false; //false - no \; true - was \ before
	for(int i=0;i<input.length();i++)
	{
		char c = input[i];
		if(!screened_before)
		{
			if(c=='\\')
			{
				screened_before=true;
				continue;
			}
			if(c=='\'')
			{
				if(quote_type==0)
				{
					quote_type=1;
					continue;
				}
				else if (quote_type==1)
				{
					quote_type=0;
				}
				else
				{
					current_str+=c;
				}
			}
			else if(c=='"')
			{
				if(quote_type==0)
				{
					quote_type=2;
					continue;
				}
				else if (quote_type==2)
				{
					quote_type=0;
				}
				else
				{
					current_str+=c;
				}
			}
			else if(c==' ')
			{
				if(quote_type==0)
				{
					if(current_str!="")
					{
						command.push_back(current_str);
						current_str.clear();
					}
					else
					{
						continue;
					}
				}
				else
				{
					current_str+=c;
				}
			}
			else
			{
				current_str+=c;
			}
		}
		else// screened_before==true
		{
			current_str+=c;
			screened_before=false;
		}
	}
	if(quote_type!=0 || screened_before)
	{
		cout<<"Wrong command format or arguments!\n";
		return /*new*/ vector<string>();
	}
	if(current_str!="") command.push_back(current_str);
	if(command.empty()) 
	{
		cout<<"Input is empty!\n";
		return vector<string>();
	}
	return command;
}

void signalHandler(int signal) {
    if (signal == SIGHUP) {
        const char* msg = "Configuration reloaded\n";
        // Игнорируем результат write, даже если ошибка
    	write(STDOUT_FILENO, msg, strlen(msg));
    }
	//(void)signal;
}

void diskInfo(){
	FILE* pipe = popen("lsblk -o NAME,SIZE,FSTYPE,MOUNTPOINT /dev/sda", "r");
    if (!pipe) {
        cout << "Error: failed to execute lsblk command" << endl;
        return;
    }
    
    char buffer[128];
    string result = "";
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        result += buffer;
    }
    pclose(pipe);
    
    vector<string> lines;
    stringstream ss(result);
    string line;
    while (getline(ss, line)) {
        lines.push_back(line);
    }
    
    if (lines.size() > 1) {
        cout << left << setw(10) << "NAME" << setw(10) << "SIZE" << setw(15) << "FSTYPE" << setw(15) << "MOUNTPOINT" << endl;
        cout << string(55, '-') << endl;
        
        for (size_t i = 1; i < lines.size(); i++) {
            stringstream line_ss(lines[i]);
            string name, size, fstype, mountpoint;
            line_ss >> name >> size >> fstype >> mountpoint;
            
            if (name.find("sda") != string::npos && name != "sda") {
                cout << left << setw(14) << name << setw(10) << size << setw(15) << (fstype.empty() ? "unknown" : fstype) << setw(15) << (mountpoint.empty() ? "not mounted" : mountpoint) << endl;
            }
        }
    } else {
        cout << "No disk information available" << endl;
    }
}

int main() {
	cerr << "DEBUG: kubsh started" << endl;
	//Catching SIGHUP
	struct sigaction sa;
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NODEFER;
    sigaction(SIGHUP, &sa, NULL);
	//Catching SIGHUP
	//VFS
	const char* vfs_path = "/opt/users";
    mkdir(vfs_path, 0755);

    // Открываем /etc/passwd для чтения пользователей
    ifstream passwd("/etc/passwd");
    string line;
    
    while (getline(passwd, line)) {
		// Разбиваем строку по :
        vector<string> parts;
        string part;
        stringstream ss(line);
        
        while (getline(ss, part, ':')) {
            parts.push_back(part);
        }
        
        // В /etc/passwd должно быть минимум 7 полей
        if (parts.size() >= 7) {
            string username = parts[0];
            string uid = parts[2];
            string home = parts[5];
            string shell = parts[6];
			// Фильтруем только пользователей с оболочкой, заканчивающейся на "sh"
			if (shell.length() < 2 || (shell.substr(shell.length() - 2) != "sh" && shell.back() != '\n')) {
				continue;
			}
            
            // Создаем каталог пользователя
            string user_dir = string(vfs_path) + "/" + username;
            mkdir(user_dir.c_str(), 0755);
            
            // Создаем файл id
            ofstream id_file(user_dir + "/id");
            id_file << uid;
            id_file.close();
            
            // Создаем файл home
            ofstream home_file(user_dir + "/home");
            home_file << home;
            home_file.close();
            
            // Создаем файл shell
            ofstream shell_file(user_dir + "/shell");
            shell_file << shell;
            shell_file.close();
        }
    }
    passwd.close();
	// Запускаем мониторинг VFS в отдельном потоке
	std::thread(vfs_monitor, vfs_path).detach();
	//VFS

	const char* start_dir = getenv("HOME");
	string file_path = string(start_dir)+"/.kubsh_history";

	const char* path_dir = getenv("PATH");
	string path_intermediate = string(path_dir);
	vector<string> path_paths;
	while(!path_intermediate.empty())
	{
		size_t ind = path_intermediate.find(':');
		if(ind == string::npos)
		{
			path_paths.push_back(path_intermediate);
			break;
		}
		path_paths.push_back(path_intermediate.substr(0,ind));
		path_intermediate=path_intermediate.substr(ind+1);
	}

	string input;
	ofstream output(file_path, ios::app);
	while(getline(cin, input))
		{
		if(input=="\\q")
		{
			output<<input<<"\n";
			output.close();
			return 0;
		}
		else if(input.substr(0,4)=="\\e $")
		{
			string temp = input.substr(4);
			if(temp.empty())
			{
				cout<<"You have to write Variable environment!\n";
				continue;
			}
			if(temp.find(' ') != string::npos || temp.find(':') != string::npos || temp.find('$') != string::npos || temp.find('/') != string::npos || temp.find('\\') != string::npos)
			{
				cout<<"You have forbidden symbol in Variable environment!\n";
				continue;
			}
			char* env_temp = getenv(temp.c_str());
			if(env_temp==nullptr || env_temp[0] == '\0')
			{
				cout<<"There is no such Variable environment!\n";
				continue;
			}
			temp = string(env_temp);
			if(temp.find(':')==string::npos)
			{
				cout<<temp<<'\n';
			}
			else
			{
				vector<string> temp_env;
				while(!temp.empty())
				{
					size_t ind = temp.find(':');
					if(ind == string::npos)
					{
						temp_env.push_back(temp);
						break;
					}
					temp_env.push_back(temp.substr(0,ind));
					temp=temp.substr(ind+1);
				}
				for(string s: temp_env)
					cout<<s<<'\n';
			}
		}
		else if (input.find("echo \"")==0 && input.find("\"")!=input.rfind("\"") && input.substr(input.length()-1,1)=="\"")
		{
			output<<input<<"\n";
			cout<<input.substr(input.find("\"")+1, input.length()-7)<<"\n";
		}
		else if (input.find("debug '")==0 && input.find("'")!=input.rfind("'") && input.substr(input.length()-1,1)=="'")
		{
			output<<input<<"\n";
			cout<<input.substr(input.find("'")+1, input.length()-8)<<"\n";
		}
		else if(input=="\\l /dev/sda")
		{
			diskInfo();
			continue;
		}
		else
		{
			vector<string> command = parseCommand(input);
			if(!command.empty())
			{
				if(command.at(0).find('/')!=string::npos)
				{
					if(!access(command[0].c_str(), F_OK))
					{
						if(!access(command[0].c_str(), X_OK))
						{
							executeCommand(command);
						}
						else cout<<"You don't have permission to execute this command!\n";
					}
					else cout<<"Command doen't exist!\n";
				}
				else
				{
					string ways;
    				bool command_found = false;
					for(string i : path_paths)
					{
						ways=i+'/'+command.at(0);
						if(!access(ways.c_str(), F_OK))
						{//file exists
            				command_found = true;
							if(!access(ways.c_str(), X_OK))
							{
								command[0]=ways;
								executeCommand(command);
							}
							else cout<<"You don't have permission to execute this command!\n";
							break;
						}
					}
					if(!command_found) cout << command[0] << ": command not found" << "\n";
				}
			}
			
		}
	}
	output<<"^D\n";
	output.close();
}
