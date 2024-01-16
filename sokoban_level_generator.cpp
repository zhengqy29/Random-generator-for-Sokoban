/*
Developed by Kevin29

Readme:

1. Copy the data of level you want to level.txt. manually
   The level.txt is in the same document of sokoban_level_generator.exe
   
2. Adjust the parameters
   seed: 0 (depend on the current time); other unsigned integer (to generate specific levels)
   push_depth: Number of times to pull the box
   number: Number of levels generated
*/

#include<bits/stdc++.h>
#include<wchar.h>
#include<cwchar>
#include<Windows.h>
#include<vector>
#include<queue>

using namespace std;

// These data should be constant after loading
int height = 0; // Height of level
int width = 0; // Width of level
int box_count = 0; // Number of boxes
char** level_data; // Data of level, in XSB format
int language=0;
// End

int* box_h;
int* box_w;

int global_count = 1;
int min_move; 

char** temp_level_data; // Data we deal with actually
string** path; // Record the path to points
bool** reachable; // Record whether we can reach the point

string Title = "Untitled";
string Author = "Unknown";

void load_level();
void print_level();
void get_height_and_width();
void get_box_location();

bool if_initiate = false;
void initiate();
void eliminate();

void generate_level(int index);
void search();
void find_player(int& h, int& w);
void print_reachable_area();
int select_a_box();
string random_walk();
void put_player_randomly();
void pull(vector<string> map, char direction, int h, int w, string** pull_path, bool** pull_here, int& location_count);

string pull_box(int index);
string reverse(string forward);
string pure_reverse(string forward);
string a_to_b(int h1, int w1, int h2, int w2, vector<string> map);

string UTF8ToGB(const char* str);

/* XSB format:
	# Wall	 
	@ Player
	+ Player on goal
	$ Box
	* Box on goal
	. Goal
	- _ or (space) Floor
*/

int seed = 1; // Random seed, for test only
int push_depth = 1000; // The larger push_depth, the harder level you may get
int number = 10; // Number of levels

int main(){
	
	cout << "\n0: 中文; 1: English\n";
	cin >> language;
	cout << endl;
	/*
	while(true){*/
		if(language){
			cout << "Copy the level you want to level.txt\n"
		   		 << "level.txt and sokoban_level_generator.exe should be in the same document\n\n"
		   		 << "Input the seed: 0 (depend on the current time, i.e. srand(time(0)));\n"
				 <<	"                positive integer (used to generate specific levels)\n";
			cin >> seed;
			cout << endl;
			
			cout << "Input the pull depth: The number of pulling box (The larger, the harder we may obtain)\n";
			cin >> push_depth;
			cout << endl;
			
			cout << "Input the number of levels: \n";
			cin >> number;
			cout << endl;
			
			cout << "Input the minimal moves of levels: \n";
			cin >> min_move;
			cout << endl;
		}
		else{
			cout << "\n复制你想要的关卡到level.txt\n"
		   		 << "level.txt要和sokoban_level_generator.exe位于同一个文件夹\n\n"
		   		 << "输入种子: 0 (取决于当前时间, 即srand(time(0))); 正整数 (用于生成特定关卡)\n";
			cin >> seed;
			cout << endl;
			
			cout << "输入逆推深度: 拉箱子的次数 (数值越大, 生成的关卡有概率越难)\n";
			cin >> push_depth;
			cout << endl;
			
			cout << "输入生成关卡的数量: \n";
			cin >> number;
			cout << endl;
			
			cout << "输入生成关卡的最小步数: \n";
			cin >> min_move;
			cout << endl;
		}
		
		// random seed
		if(seed){
			srand(seed);
		}
		else{
			srand(time(0));
		}
		
		get_height_and_width();
		load_level();
		initiate();
		get_box_location();
		// print_level();
		
		for(int i=0; i<number; ++i){
			generate_level(i);
		}
		
		for(int i=0; i<height; ++i){
			delete[] level_data[i];
		}
		delete[] level_data;
		
		eliminate();
		
		delete[] box_h;
		delete[] box_w;
		/*
		if(language){
			cout << "\nExit? y:yes n:no\n";
		}
		else{
			cout << "\n退出? y:是 n:否\n";
		}
		char exit;
		cin >> exit;
		if(exit == 'y') break;
	}*/
	
	char sleep;
	if(language){
		cout << "\nEnter anything to exit.\n";
	}
	else{
		cout << "\n输入任意键退出.\n";
	}
	cin >> sleep;

	return 0;
}
	
void get_height_and_width(){
	height = 0;
	width = 0;
	ifstream in("level.txt");
	string line = "";
	while(getline(in, line)){
		if(line[0] == ' ' || line[0] == '_' || line[0] == '-' || line[0] == '#'){
			++height;
			width = line.length();
		}
	}
	in.close();
}

void put_player_randomly(){
	vector<pair<int, int>> vpint;
	int count = 0;
	for(int i=1; i<=height-2; ++i){
		for(int j=1; j<=width-2; ++j){
			char temp = temp_level_data[i][j];
			if(temp == '-' || temp == '.'){
				vpint.push_back({i,j});
				++count;
			}
		}
	}
	int randnum = rand() % count;
	int h=vpint[randnum].first;
	int w=vpint[randnum].second;
	if(temp_level_data[h][w] == '.') temp_level_data[h][w] = '+';
	else if(temp_level_data[h][w] == '-') temp_level_data[h][w] = '@';
}

void initiate(){
	eliminate();
	if_initiate = true;
	
	temp_level_data = new char* [height];
	path = new string* [height];
	reachable = new bool* [height];
	
	for(int i=0; i<height; ++i){
		temp_level_data[i] = new char [width];
		path[i] = new string [width];
		reachable[i] = new bool [width];
		for(int j=0; j<width; ++j){
			temp_level_data[i][j] = level_data[i][j];
			path[i][j] = "";
			reachable[i][j] = false;
		}
	}
	
	put_player_randomly();
}

void eliminate(){
	if(if_initiate){
		for(int i=0; i<height; ++i){
			delete[] temp_level_data[i];
			delete[] path[i];
			delete[] reachable[i];
		}
		delete[] temp_level_data;
		delete[] path;
		delete[] reachable;
		if_initiate = false;
	}
}
	
void load_level(){
	ifstream in("level.txt");
	string line = "";
	box_count = 0;
	level_data = new char* [height];
	for(int i=0; i<height; ++i){
		level_data[i] = new char [width];
		getline(in, line);
		for(int j=0; j<width; ++j){
			if(line[j] == '+'){
				level_data[i][j] = '*';
			}
			else if(line[j] == '$'){
				++box_count;
				level_data[i][j] = '-';
			}
			else if(line[j] == '.'){
				level_data[i][j] = '*';
			}
			else if(line[j] == '*'){
				++box_count;
				level_data[i][j] = '*';
			}
			else if(line[j] == '@'){
				level_data[i][j] = '-';
			}
			else{
				level_data[i][j] = line[j];
			}
		}
	}
	while(getline(in, line)){
		if(line[0] == 'T' || line[0] == 't'){
			Title = "";
			Title = line.substr(7);
		}
		else if(line[0] == 'A' || line[0] == 'a'){
			Author = "";
			Author = line.substr(8);
		}
	}
	in.close();
	if(language){
		Title += " Remodel Randomly";
		Author += " + Level generator by Kevin29";
	}
}

void get_box_location(){
	if(box_h != NULL){
		delete[] box_h;
		delete[] box_w;
	}
	int temp = 0;
	box_h = new int [box_count];
	box_w = new int [box_count];
	for(int i=1; i<height; ++i){
		for(int j=1; j<width; ++j){
			if(temp_level_data[i][j] == '*' || temp_level_data[i][j] == '$'){
				box_h[temp] = i;
				box_w[temp] = j;
				++temp;
			}
		}
	}
	/*
	for(int i=0; i<box_count; ++i){
		cout << "box " << i << ": (" << box_h[i] << "," << box_w[i] << ") ";
	}
	cout << endl;*/
}
	
void print_level(){
	cout << "height: " << height << endl;
	cout << "width: " << width << endl;
	cout << "number of boxes: " << box_count << endl;
	for(int i=0; i<height; ++i){
		for(int j=0; j<width; ++j){
			cout << temp_level_data[i][j];
		}
		cout << endl;
	}
	cout << endl;
	for(int i=0; i<box_count; ++i){
		cout << "box " << i+1 << ": (" << box_h[i] << "," << box_w[i] << ") ";
	}
	cout << endl;
	cout << "Title: " << UTF8ToGB(Title.c_str()) << endl;
	cout << "Author: " << UTF8ToGB(Author.c_str()) << endl;
}

void print_reachable_area(){
	for(int i=0; i<height; ++i){
		bool cl = false;
		for(int j=0; j<width; ++j){
			if(reachable[i][j]){
				cl = true;
				cout << "i: " << i << ",j: " << j << ",path: " << path[i][j] << ". ";
			}
		}
		if(cl){
			cout << endl;
		}
	}
}

string UTF8ToGB(const char* str){
	string result;
	WCHAR* strSrc;
	LPSTR szRes;

	int i = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
	strSrc = new WCHAR[i + 1];
	MultiByteToWideChar(CP_UTF8, 0, str, -1, strSrc, i);

	i = WideCharToMultiByte(CP_ACP, 0, strSrc, -1, NULL, 0, NULL, NULL);
	szRes = new CHAR[i + 1];
	WideCharToMultiByte(CP_ACP, 0, strSrc, -1, szRes, i, NULL, NULL);

	result = szRes;
	delete[] strSrc;
	delete[] szRes;

	return result;
}

string reverse(string forward){
	int len = forward.length()-1;
	int index = 0;
	while(index <= len){
		if(forward[index] == 'L' || forward[index] == 'U' || forward[index] == 'R' || forward[index] == 'D') break;
		++index;
	}
	string backward = "";
	for(int i=len; i>=index; --i){
		if(forward[i] == 'l') backward += "r";
		else if(forward[i] == 'L') backward += "R";
		else if(forward[i] == 'r') backward += "l";
		else if(forward[i] == 'R') backward += "L";
		else if(forward[i] == 'u') backward += "d";
		else if(forward[i] == 'U') backward += "D";
		else if(forward[i] == 'd') backward += "u";
		else if(forward[i] == 'D') backward += "U";
	}
	return backward;
}

string pure_reverse(string forward){
	int len = forward.length()-1;
	string backward = "";
	for(int i=len; i>=0; --i){
		if(forward[i] == 'l') backward += "r";
		else if(forward[i] == 'L') backward += "R";
		else if(forward[i] == 'r') backward += "l";
		else if(forward[i] == 'R') backward += "L";
		else if(forward[i] == 'u') backward += "d";
		else if(forward[i] == 'U') backward += "D";
		else if(forward[i] == 'd') backward += "u";
		else if(forward[i] == 'D') backward += "U";
	}
	return backward;
}

string a_to_b(int h1, int w1, int h2, int w2, vector<string> map){
	string** pull_path = new string* [height];
	bool** pull_here = new bool* [height];
	for(int i=0; i<height; ++i){
		pull_path[i] = new string [width];
		pull_here[i] = new bool [width];
		for(int j=0; j<width; ++j){
			pull_path[i][j] = "";
			pull_here[i][j] = false;
		}
	}
	pull_here[h1][w1] = true;
	
	queue<pair<int, int>> q;
    q.push({h1, w1});
	while(q.empty() == false){
        auto t = q.front();
        q.pop();
        int h=t.first, w=t.second;
        if(pull_here[h-1][w] == false){
        	if(map[h-1][w] == '-' || map[h-1][w] == '.' || map[h-1][w] == ' ' || map[h-1][w] == '_'){
        		pull_here[h-1][w] = true;
        		pull_path[h-1][w] = pull_path[h][w] + "u";
        		q.push({h-1, w});
			}
		}
        if(pull_here[h+1][w] == false){
        	if(map[h+1][w] == '-' || map[h+1][w] == '.' || map[h+1][w] == ' ' || map[h+1][w] == '_'){
        		pull_here[h+1][w] = true;
        		pull_path[h+1][w] = pull_path[h][w] + "d";
        		q.push({h+1, w});
			}
		}
        if(pull_here[h][w-1] == false){
        	if(map[h][w-1] == '-' || map[h][w-1] == '.' || map[h][w-1] == ' ' || map[h][w-1] == '_'){
        		pull_here[h][w-1] = true;
        		pull_path[h][w-1] = pull_path[h][w] + "l";
        		q.push({h, w-1});
			}
		}
        if(pull_here[h][w+1] == false){
        	if(map[h][w+1] == '-' || map[h][w+1] == '.' || map[h][w+1] == ' ' || map[h][w+1] == '_'){
        		pull_here[h][w+1] = true;
        		pull_path[h][w+1] = pull_path[h][w] + "r";
        		q.push({h, w+1});
			}
		}
    }
    
    string res = pull_path[h2][w2];
    
	for(int i=0; i<height; ++i){
		delete[] pull_path[i];
		delete[] pull_here[i];
	}
	delete[] pull_path;
	delete[] pull_here;
	
	return res;
}

string pull_box(int index){
	const int h=box_h[index], w=box_w[index];
	string** pull_path = new string* [height];
	bool** pull_here = new bool* [height];
	int location_count = 0;
	for(int i=0; i<height; ++i){
		pull_path[i] = new string [width];
		pull_here[i] = new bool [width];
		for(int j=0; j<width; ++j){
			pull_path[i][j] = "";
			pull_here[i][j] = false;
		}
	}
	pull_here[h][w] = true;
	vector<string> map(height, "");
	for(int i=0; i<height; ++i){
		for(int j=0; j<width; ++j){
			map[i].push_back(temp_level_data[i][j]);
		}
	}
	
	int ph=0,pw=0;
	find_player(ph,pw);
	if(map[ph][pw] == '@') map[ph][pw] = '-';
	else if(map[ph][pw] == '+') map[ph][pw] = '.';
	
	for(int i=1; i<height; ++i){
		if(map[h-i][w] == '#' || map[h-i][w] == '$' || map[h-i][w] == '*') break;
		if(pull_here[h-i][w]==false && reachable[h-i][w] && reachable[h-i-1][w]){
			pull_here[h-i][w] = true;
			++location_count;
			pull_path[h-i][w] = path[h-1][w];
			for(int j=0; j<i; ++j){
				pull_path[h-i][w] += "U";
			}
			vector<string> tempmap(map);
			if(tempmap[h][w] == '$') tempmap[h][w] = '-';
			else if(tempmap[h][w] == '*') tempmap[h][w] = '.';
			
			if(tempmap[h-i][w] == '.') tempmap[h-i][w] = '*';
			else tempmap[h-i][w] = '$';
			
			if(tempmap[h-i-1][w] == '.') tempmap[h-i-1][w] = '+';
			else tempmap[h-i-1][w] = '@';
			
			pull(tempmap, 'u', h-i, w, pull_path, pull_here, location_count);
		}
	}
	
	for(int i=1; i<height; ++i){
		if(map[h+i][w] == '#' || map[h+i][w] == '$' || map[h+i][w] == '*') break;
		if(pull_here[h+i][w]==false && reachable[h+i][w] && reachable[h+i+1][w]){
			pull_here[h+i][w] = true;
			++location_count;
			pull_path[h+i][w] = path[h+1][w];
			for(int j=0; j<i; ++j){
				pull_path[h+i][w] += "D";
			}
			vector<string> tempmap(map);
			if(tempmap[h][w] == '$') tempmap[h][w] = '-';
			else if(tempmap[h][w] == '*') tempmap[h][w] = '.';
			
			if(tempmap[h+i][w] == '.') tempmap[h+i][w] = '*';
			else tempmap[h+i][w] = '$';
			
			if(tempmap[h+i+1][w] == '.') tempmap[h+i+1][w] = '+';
			else tempmap[h+i+1][w] = '@';
			
			pull(tempmap, 'd', h+i, w, pull_path, pull_here, location_count);
		}
	}
	
	for(int i=1; i<width; ++i){
		if(map[h][w-i] == '#' || map[h][w-i] == '$' || map[h][w-i] == '*') break;
		if(pull_here[h][w-i]==false && reachable[h][w-i] && reachable[h][w-i-1]){
			pull_here[h][w-i] = true;
			++location_count;
			pull_path[h][w-i] = path[h][w-1];
			for(int j=0; j<i; ++j){
				pull_path[h][w-i] += "L";
			}
			vector<string> tempmap(map);
			if(tempmap[h][w] == '$') tempmap[h][w] = '-';
			else if(tempmap[h][w] == '*') tempmap[h][w] = '.';
			
			if(tempmap[h][w-i] == '.') tempmap[h][w-i] = '*';
			else tempmap[h][w-i] = '$';
			
			if(tempmap[h][w-i-1] == '.') tempmap[h][w-i-1] = '+';
			else tempmap[h][w-i-1] = '@';
			
			pull(tempmap, 'l', h, w-i, pull_path, pull_here, location_count);
		}
	}
	
	for(int i=1; i<width; ++i){
		if(map[h][w+i] == '#' || map[h][w+i] == '$' || map[h][w+i] == '*') break;
		if(pull_here[h][w+i]==false && reachable[h][w+i] && reachable[h][w+i+1]){
			pull_here[h][w+i] = true;
			++location_count;
			pull_path[h][w+i] = path[h][w+1];
			for(int j=0; j<i; ++j){
				pull_path[h][w+i] += "R";
			}
			vector<string> tempmap(map);
			if(tempmap[h][w] == '$') tempmap[h][w] = '-';
			else if(tempmap[h][w] == '*') tempmap[h][w] = '.';
			
			if(tempmap[h][w+i] == '.') tempmap[h][w+i] = '*';
			else tempmap[h][w+i] = '$';
			
			if(tempmap[h][w+i+1] == '.') tempmap[h][w+i+1] = '+';
			else tempmap[h][w+i+1] = '@';
			
			pull(tempmap, 'r', h, w+i, pull_path, pull_here, location_count);
		}
	}
	
	int randnum = rand() % location_count;
	int res_h, res_w;
	for(int i=0; i<height; ++i){
		bool flag = false;
		for(int j=0; j<width; ++j){
			if(pull_here[i][j] == true && pull_path[i][j].length()>0){
				if(randnum > 0) --randnum;
				else{
					res_h = i;
					res_w = j;
					flag = true;
					break;
				}
			}
		}
		if(flag) break;
	}
	
	string res = pull_path[res_h][res_w];
	
	if(temp_level_data[ph][pw] == '@') temp_level_data[ph][pw] = '-';
	else if(temp_level_data[ph][pw] == '+') temp_level_data[ph][pw] = '.';
	
	if(temp_level_data[h][w] == '$') temp_level_data[h][w] = '-';
	else if(temp_level_data[h][w] == '*') temp_level_data[h][w] = '.';
	
	if(temp_level_data[res_h][res_w] == '.') temp_level_data[res_h][res_w] = '*';
	else temp_level_data[res_h][res_w] = '$';
	
	char last = res.back();
	if(last == 'U') --res_h;
	else if(last == 'D') ++res_h;
	else if(last == 'L') --res_w;
	else if(last == 'R') ++res_w;
	
	if(temp_level_data[res_h][res_w] == '.') temp_level_data[res_h][res_w] = '+';
	else temp_level_data[res_h][res_w] = '@';
	/*
	cout << "location_count: " << location_count << endl;
	for(int i=0; i<height; ++i){
		for(int j=0; j<width; ++j){
			cout << temp_level_data[i][j];
		}
		cout << endl;
	}
	cout << pure_reverse(res) << endl;
	int sleep;
	cin >> sleep;
	*/
	for(int i=0; i<height; ++i){
		delete[] pull_path[i];
		delete[] pull_here[i];
	}
	delete[] pull_path;
	delete[] pull_here;
	
	return res;
}

void pull(vector<string> map, char direction, int h, int w, string** pull_path, bool** pull_here, int& location_count){
	int ph=h, pw=w;
	if(direction == 'u') --ph;
	else if(direction == 'd') ++ph;
	else if(direction == 'l') --pw;
	else if(direction == 'r') ++pw;
	
	if(direction != 'u'){
		string pth = a_to_b(ph, pw, h-1, w, map);
		if(pth.length() > 0){
			for(int i=1; i<height; ++i){
				if(map[h-i][w] == '#' || map[h-i][w] == '$' || map[h-i][w] == '*') break;
				if(map[h-i-1][w] == '#' || map[h-i-1][w] == '$' || map[h-i-1][w] == '*') break;
				if(pull_here[h-i][w]==false){
					pull_here[h-i][w] = true;
					++location_count;
					pull_path[h-i][w] = pull_path[h][w] + pth;
					for(int j=0; j<i; ++j){
						pull_path[h-i][w] += "U";
					}
					vector<string> tempmap(map);
					if(tempmap[ph][pw] == '@') tempmap[ph][pw] = '-';
					else if(tempmap[ph][pw] == '+') tempmap[ph][pw] = '.';
					
					if(tempmap[h][w] == '$') tempmap[h][w] = '-';
					else if(tempmap[h][w] == '*') tempmap[h][w] = '.';
					
					if(tempmap[h-i][w] == '.') tempmap[h-i][w] = '*';
					else tempmap[h-i][w] = '$';
					
					if(tempmap[h-i-1][w] == '.') tempmap[h-i-1][w] = '+';
					else tempmap[h-i-1][w] = '@';
					
					pull(tempmap, 'u', h-i, w, pull_path, pull_here, location_count);
				}
			}
		}
	}
	
	if(direction != 'd'){
		string pth = a_to_b(ph, pw, h+1, w, map);
		if(pth.length() > 0){
			for(int i=1; i<height; ++i){
				if(map[h+i][w] == '#' || map[h+i][w] == '$' || map[h+i][w] == '*') break;
				if(map[h+i+1][w] == '#' || map[h+i+1][w] == '$' || map[h+i+1][w] == '*') break;
				if(pull_here[h+i][w]==false){
					pull_here[h+i][w] = true;
					++location_count;
					pull_path[h+i][w] = pull_path[h][w] + pth;
					for(int j=0; j<i; ++j){
						pull_path[h+i][w] += "D";
					}
					vector<string> tempmap(map);
					if(tempmap[ph][pw] == '@') tempmap[ph][pw] = '-';
					else if(tempmap[ph][pw] == '+') tempmap[ph][pw] = '.';
					
					if(tempmap[h][w] == '$') tempmap[h][w] = '-';
					else if(tempmap[h][w] == '*') tempmap[h][w] = '.';
					
					if(tempmap[h+i][w] == '.') tempmap[h+i][w] = '*';
					else tempmap[h+i][w] = '$';
					
					if(tempmap[h+i+1][w] == '.') tempmap[h+i+1][w] = '+';
					else tempmap[h+i+1][w] = '@';
					
					pull(tempmap, 'd', h+i, w, pull_path, pull_here, location_count);
				}
			}
		}
	}
	
	if(direction != 'L'){
		string pth = a_to_b(ph, pw, h, w-1, map);
		if(pth.length() > 0){
			for(int i=1; i<width; ++i){
				if(map[h][w-i] == '#' || map[h][w-i] == '$' || map[h][w-i] == '*') break;
				if(map[h][w-i-1] == '#' || map[h][w-i-1] == '$' || map[h][w-i-1] == '*') break;
				if(pull_here[h][w-i]==false){
					pull_here[h][w-i] = true;
					++location_count;
					pull_path[h][w-i] = pull_path[h][w] + pth;
					for(int j=0; j<i; ++j){
						pull_path[h][w-i] += "L";
					}
					vector<string> tempmap(map);
					if(tempmap[ph][pw] == '@') tempmap[ph][pw] = '-';
					else if(tempmap[ph][pw] == '+') tempmap[ph][pw] = '.';
					
					if(tempmap[h][w] == '$') tempmap[h][w] = '-';
					else if(tempmap[h][w] == '*') tempmap[h][w] = '.';
					
					if(tempmap[h][w-i] == '.') tempmap[h][w-i] = '*';
					else tempmap[h][w-i] = '$';
					
					if(tempmap[h][w-i-1] == '.') tempmap[h][w-i-1] = '+';
					else tempmap[h][w-i-1] = '@';
					
					pull(tempmap, 'l', h, w-i, pull_path, pull_here, location_count);
				}
			}
		}
	}
	
	if(direction != 'R'){
		string pth = a_to_b(ph, pw, h, w+1, map);
		if(pth.length() > 0){
			for(int i=1; i<width; ++i){
				if(map[h][w+i] == '#' || map[h][w+i] == '$' || map[h][w+i] == '*') break;
				if(map[h][w+i+1] == '#' || map[h][w+i+1] == '$' || map[h][w+i+1] == '*') break;
				if(pull_here[h][w+i]==false){
					pull_here[h][w+i] = true;
					++location_count;
					pull_path[h][w+i] = pull_path[h][w] + pth;
					for(int j=0; j<i; ++j){
						pull_path[h][w+i] += "R";
					}
					vector<string> tempmap(map);
					if(tempmap[ph][pw] == '@') tempmap[ph][pw] = '-';
					else if(tempmap[ph][pw] == '+') tempmap[ph][pw] = '.';
					
					if(tempmap[h][w] == '$') tempmap[h][w] = '-';
					else if(tempmap[h][w] == '*') tempmap[h][w] = '.';
					
					if(tempmap[h][w+i] == '.') tempmap[h][w+i] = '*';
					else tempmap[h][w+i] = '$';
					
					if(tempmap[h][w+i+1] == '.') tempmap[h][w+i+1] = '+';
					else tempmap[h][w+i+1] = '@';
					
					pull(tempmap, 'r', h, w+i, pull_path, pull_here, location_count);
				}
			}
		}
	}
}

void generate_level(int index){
	string solution = "";
	initiate();
	get_box_location();
	for(int i=0; i<push_depth; ++i){
		search();
		get_box_location();
		int box_index = select_a_box();
		
		// cout << "box " << box_index << " selected.\n";
		
		if(box_index >= 0){
			solution += pull_box(box_index);
		}
		else break;
		/*
		for(int i=0; i<height; ++i){
			for(int j=0; j<width; ++j){
				cout << temp_level_data[i][j];
			}
			cout << endl;
		}
		cout << reverse(solution) << endl;
		*/
	}
	solution += random_walk();
	string forward_solution = reverse(solution);
	
	if(solution.length() >= min_move){
		/*char sl;
		cin >> sl;*/
		cout << "Level " << global_count << " is generated.\n";
		for(int i=0; i<height; ++i){
			for(int j=0; j<width; ++j){
				cout << temp_level_data[i][j];
			}
			cout << endl;
		}
		cout << reverse(solution) << endl;
	
		string filename = "Generated_levels.txt";
		ofstream out_level;
		out_level.open(filename, ios::app);
		if(out_level.is_open()){
			for(int i=0; i<height; ++i){
				for(int j=0; j<width; ++j){
					out_level << temp_level_data[i][j];
				}
				out_level << endl;
			}
			
			out_level << "Title: " << UTF8ToGB(Title.c_str()); 
			if(language){
				out_level << " / Level " << global_count << endl;
			}
			else{
				out_level << " 随机生成版 / 关卡" << global_count << endl;
			}
			
			out_level << "Author: " << UTF8ToGB(Author.c_str());
			if(language){
				out_level << endl;
			}
			else{
				out_level << " + Kevin29的随机生成器" << endl;
			}
			out_level << "Solution: \n" << forward_solution << endl;
		}
		out_level.close();
		++global_count;
	}
}

string random_walk(){
	int h,w;
	find_player(h,w);
	search();
	int count=0;
	for(int i=0; i<height; ++i){
		for(int j=0; j<width; ++j){
			if(reachable[i][j]) ++count;
		}
	}
	if(count == 0){
		return "";
	}
	int randnum = rand() % count;
	int res_h, res_w;
	for(int i=0; i<height; ++i){
		bool flag = false;
		for(int j=0; j<width; ++j){
			if(reachable[i][j]){
				if(randnum > 0) --randnum;
				else{
					res_h = i;
					res_w = j;
					flag = true;
					break;
				}
			}
		}
		if(flag) break;
	}
	
	if(temp_level_data[h][w] == '@') temp_level_data[h][w] = '-';
	else if(temp_level_data[h][w] == '+') temp_level_data[h][w] = '.';
	
	if(temp_level_data[res_h][res_w] == '.') temp_level_data[res_h][res_w] = '+';
	else temp_level_data[res_h][res_w] = '@';
	
	return path[res_h][res_w];
}

int select_a_box(){
	vector<int> vint;
	int movable_count = 0;
	for(int i=0; i<box_count; ++i){
		int h=box_h[i], w=box_w[i];
		if(reachable[h-1][w] && reachable[h-2][w]){
			vint.push_back(i);
			++movable_count;
		}
		else if(reachable[h+1][w] && reachable[h+2][w]){
			vint.push_back(i);
			++movable_count;
		}
		else if(reachable[h][w-1] && reachable[h][w-2]){
			vint.push_back(i);
			++movable_count;
		}
		else if(reachable[h][w+1] && reachable[h][w+2]){
			vint.push_back(i);
			++movable_count;
		}
	}
	/*
	cout << "movable_count: " << movable_count << endl;
	for(int i=0; i<movable_count; ++i){
		int box_id = vint[i];
		cout << "box " << box_id << ": (" << box_h[box_id] << "," << box_w[box_id] << ") ";
	}
	cout << endl;
	*/
	if(movable_count){
		int randnum = rand() % movable_count;
		//cout << "Box " << vint[randnum] << " selected\n";
		return vint[randnum];
	}
	else return -1;
}

void search(){
	int h,w;
	find_player(h,w);
	// cout << h << " " << w << endl;
	for(int i=0; i<height; ++i){
		for(int j=0; j<width; ++j){
			reachable[i][j] = false;
			path[i][j] = "";
		}
	}
	
	reachable[h][w] = true;
	
	queue<pair<int, int>> q;
    q.push({h,w});
	while(q.empty() == false){
        auto t = q.front();
        q.pop();
        int h=t.first, w=t.second;
        if(reachable[h-1][w] == false){
        	if(temp_level_data[h-1][w] == '-' || temp_level_data[h-1][w] == '.' || temp_level_data[h-1][w] == ' ' || temp_level_data[h-1][w] == '_'){
        		reachable[h-1][w] = true;
        		path[h-1][w] = path[h][w] + "u";
        		q.push({h-1, w});
			}
		}
        if(reachable[h+1][w] == false){
        	if(temp_level_data[h+1][w] == '-' || temp_level_data[h+1][w] == '.' || temp_level_data[h+1][w] == ' ' || temp_level_data[h+1][w] == '_'){
        		reachable[h+1][w] = true;
        		path[h+1][w] = path[h][w] + "d";
        		q.push({h+1, w});
			}
		}
        if(reachable[h][w-1] == false){
        	if(temp_level_data[h][w-1] == '-' || temp_level_data[h][w-1] == '.' || temp_level_data[h][w-1] == ' ' || temp_level_data[h][w-1] == '_'){
        		reachable[h][w-1] = true;
        		path[h][w-1] = path[h][w] + "l";
        		q.push({h, w-1});
			}
		}
        if(reachable[h][w+1] == false){
        	if(temp_level_data[h][w+1] == '-' || temp_level_data[h][w+1] == '.' || temp_level_data[h][w+1] == ' ' || temp_level_data[h][w+1] == '_'){
        		reachable[h][w+1] = true;
        		path[h][w+1] = path[h][w] + "r";
        		q.push({h, w+1});
			}
		}
    }
	
	// print_reachable_area();
}

void find_player(int& h, int& w){
	for(int i=1; i<height; ++i){
		for(int j=1; j<width; ++j){
			if(temp_level_data[i][j] == '@' || temp_level_data[i][j] == '+'){
				h = i;
				w = j;
				return;
			}
		}
	}
}
