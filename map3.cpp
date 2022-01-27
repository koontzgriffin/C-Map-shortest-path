#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>

using namespace std;

double inf = -1;

struct road{
	string name, code;
	int a, b;
	double len;

	road(string n, string c, int a_, int b_, double l){
		name = n;
		code = c;
		a = a_; 
		b = b_;
		len = l;
	}
	void print(){
		cout<<name<<" "<<code<<" "<<a<<" "<<b<<" "<<len<<endl;
	}
	int from(int p){
		if(a == p) return b;
		else return a;
	}
};

class node{
private:
	int pos;
	double lon, lat, dist;
	string state, place;
	vector<road *> exits;

public:
	double eta;
	bool visited; 
	node(int i, double l, double la, double d, string s, string p){
                pos = i;
		lon = l;
                lat = la;
                dist = d;
                state = s;
                place = p;
		eta = inf;
		visited = false;
        }
	void add_exit(road * r){
		exits.push_back(r);
	}
	void print(){
		cout<<"location "<<pos<<", "<<dist<<" miles from "<<place<<",  "<<state<<". \n";
		cout<<"roads leading away:\n";
		for(int i = 0; i < exits.size(); i++){
			cout<<i+1<<" - "<<exits[i]->name<<", "<<exits[i]->len<<" miles to location ";
			if(exits[i]->a == pos) cout<<exits[i]->b<<".\n";
			else cout<<exits[i]->a<<".\n";
		}
	}
	road * get_exit(int i){
		return exits[i];
	}
	int num_exits(){
		return exits.size();
	}
	int get_pos(){
		return pos;
	}
	string name(){
		return place;
	}
	double getlat(){return lat;}
	double getlon(){return lon;}
};

struct priorityQ{
	vector<node *> heap;
	int L;

	priorityQ(){
		heap.push_back(NULL);
		L = 0;
	}
	void swap(int i, int j){
		node * temp = heap[i];
		heap[i] = heap[j];
		heap[j] = temp;
	}
	int least(node * a, int apos, node * b, int bpos){
		if(a->eta < b->eta) return apos;
		else return bpos;
	}
	void adjust_up(int poi){
		while(true){
			if(poi == 1)break;
			if(heap[poi]->eta < heap[poi/2]->eta){
				swap(poi, poi/2);
				poi = poi/2;
				if(poi == 1)break;
			}
			else break;
		}
	}
	void adjust_down(int poi){
		while(true){
			int left = poi*2, right = left+1;
			if(left > L)break;
			
			if(left == L){
				if(heap[left]->eta < heap[poi]->eta){
					swap(left, poi);
				}
				break;
			}
			int smallest = least(heap[left],left, heap[right], right);
			if(heap[smallest]->eta < heap[poi]->eta){
				swap(smallest, poi);
				poi = smallest;
			}
			else break;
		}
	}
	void add(node * n){
		heap.push_back(n);
		L++;
		if(L == 1) return;
		int poi = L;
		adjust_up(poi);
	}
	void update(node * n, double e){
		for(int i = 1; i <= L; i++){
			if(heap[i] == n){
				if(heap[i]->eta > e){
					heap[i]->eta = e;
					adjust_up(i);
					return;
				}
				else {
					heap[i]->eta = e;
					adjust_down(i);
					return;
				}
			}
		}
	}
	node * rm(){
		node * item = heap[1];
		int poi = 1;
		swap(1, L);
		heap.pop_back();
		L--;
		adjust_down(poi);
		return item;
	}
	bool isempty(){
		if(L == 0) return true;
		else return false;
	}
	int getL(){return L;}
};

string dir(double lon1, double lat1, double lon2, double lat2){
	string direction;
	if(lat1 > lat2)direction += 'S';
	else if(lat1 < lat2)direction += 'N';
	if(lon1 < lon2)direction += 'E';
	else if(lon1 > lon2)direction += 'W';

	return direction;
}

class map{
private:	
	vector<node *> nodes;
public:	
	map(){};
	void add(node * n){
		nodes.push_back(n);
	}
	node * get_node(int i){
		return nodes[i];
	}
	node * operator[](int i){
		return nodes[i];
	}
	void print_node(int i){
		cout<<"node: "<<i<<" ";
		nodes[i]->print();
	}
	void reset(){
		for(int i = 0; i < nodes.size(); i++){
			nodes[i]->eta = inf;
			nodes[i]->visited = false;
		}
	}
	void navigate(){
		int start;
		cout<<"Choose a starting location.";
		cin>>start;
		if(start > 0 && start < nodes.size()){
			cout<<"\nstarting location: "<<start<<endl;
			navigate(start);
		}
		else {cout<<"invalid start location."<<endl; navigate();}
	}
	void navigate(int start){
		int choice;
		nodes[start]->print();
		cout<<"Take which road?";
		cin>>choice;
		cout<<"\n";
		if(nodes[start]->get_exit(choice - 1)->a == nodes[start]->get_pos()){navigate(nodes[start]->get_exit(choice-1)->b);}
		else navigate(nodes[start]->get_exit(choice-1)->a);
	}
	double eta(node * start, node * destination){
		start->eta = 0;
		priorityQ Q;
		Q.add(start);
		while(true){
			node * here = Q.rm();
			if(here == NULL){cout<<"Path not possible.\n";break;}
			here->visited = true;
			if(here == destination)break;
			for(int i = 0; i < here->num_exits(); i++){	
				road * r = here->get_exit(i);
				node * there = nodes[r->from(here->get_pos())];//returns node at end of road
				if(there->visited){continue;}
				double timethere = here->eta + r->len;
				if(there->eta == inf){
					there->eta = timethere;
					Q.add(there);
				}
				else if(there->eta > timethere){
					Q.update(there, timethere);
				}
			}
		}
		return destination->eta;
	}
	double eta(int start, int end){
		return eta(nodes[start], nodes[end]);
	}
	vector<road *> get_path(node * start, node * destination){
		vector<road *> path;
		double c_eta = destination->eta;
		node * ptr = destination;
		while(c_eta > 0){
			for(int i = 0; i < ptr->num_exits(); i++){
				road * r  = ptr->get_exit(i);
				node * there = nodes[r->from(ptr->get_pos())];
				double dif = abs(c_eta - r->len - there->eta);
				if(dif < .1){	//checks if current eta - road length = the eta of other place
					path.push_back(r); 		// push back the new road
					ptr = there;			//ptr set to new place
					c_eta = there->eta;			
				}
			}
		}
		return path;
	}
	void directions(int start, int end){
		cout<<"getting eta...\n";
		eta(start, end);
		cout<<"getting path...\n";
		vector<road *> path = get_path(nodes[start], nodes[end]);
		node * curr = nodes[start];
		cout<<"The directions are: \n";
		for(int i = path.size()-1; i >= 0; i--){
			cout<<"from intersection "<<curr->get_pos()<<" take "<<path[i]->name<<" "<<path[i]->len<<" "<<dir(curr->getlon(),curr->getlat(),nodes[path[i]->from(curr->get_pos())]->getlon(),nodes[path[i]->from(curr->get_pos())]->getlat())<<" miles to intersection "<<path[i]->from(curr->get_pos())<<endl;
			curr = nodes[path[i]->from(curr->get_pos())];
		}
		cout<<"done.\n";
	}
};

string despace(string s){
	for(int i = 0; i < s.length(); i++){
		if(s[i] == ' '){
			s.erase(i, 1);
			i--;
		}
	}
	return s;
}

vector<string> parse(string s){
	vector<string> data;
	string item;
	for(int i = 0; i < s.length(); i++){
		if(s[i] != ' '){
			item+=s[i];
		}
		else if(!item.empty()){
			data.push_back(item);
			item = "";
		}
	}
	if(!item.empty()){data.push_back(item);}
	return data;		
}

map map_from_files(string intersections, string roads){
	map m;
	
	//reading in all intersections
	ifstream fin(intersections);
	if(fin.fail()){cout<<"failed.\n";}	
	else{
		int pos = 0;
		while(true){
			string lat, lon, dist, state, name;	
			string line;
			getline(fin, line);
			if(line.empty()){break;}

			lon.append(line, 0, 9);
			lon = despace(lon);
			lat.append(line, 9, 10);
  			lat = despace(lat);
			dist.append(line, 19, 8);
			dist = despace(dist);
			state.append(line, 28, 2);
			name = line.erase(0, 31);
			
			m.add(new node(pos, stod(lon), stod(lat), stod(dist), state, name));
			pos++;
		}
		fin.close();
	}
	
	//reading in all roads
	ifstream fi(roads);
	if(fi.fail()){cout<<"failed.\n";}
	else{
		while(true){
			string line;
			getline(fi, line);
			if(line.empty())break;
			vector<string> parsed = parse(line);//vector of the road data
			road * r = new road(parsed[0], parsed[1], stoi(parsed[2]), stoi(parsed[3]), stod(parsed[4]));
			m[stoi(parsed[2])]->add_exit(r);
			m[stoi(parsed[3])]->add_exit(r);
		}
		fi.close();
	}	

	return m;
}

int main(){
	map m = map_from_files("/home/www/class/een318/intersections.txt", "/home/www/class/een318/connections.txt");
	int a, b;
	cout<<"from?";
	cin>>a;
	cout<<"too?";
	cin>>b;
	m.directions(a, b);
	//m.navigate();
	return 0;
}

