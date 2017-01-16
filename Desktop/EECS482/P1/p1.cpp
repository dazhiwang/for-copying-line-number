#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <queue>
#include <algorithm>
#include <iomanip>
#include <limits>
#include <cmath>
#include "thread.h"

using namespace std;

mutex mutex1;
//mutex mutex2;
cv cv1;
cv cv2;

int track = 0;
int totalnum=0;
int max_disk_queue;
vector<thread> t2;
vector<pair<int, int>> diskQueue;
vector<queue<int>> disks;

void requester(void* a){
	intptr_t num= (intptr_t) a;
	//cout<<"222  "<<num<<endl;
	//mutex1.lock();
    //mutex1.unlock();
    mutex1.lock();
    //cout<<"diskQueue size: "<<diskQueue.size()<<endl;
    //cout<<"totalnum: "<<totalnum<<endl;
    while(diskQueue.size()==max_disk_queue || totalnum==0){
    	cv1.wait(mutex1);
    }
    if(!disks[num].empty()){
    	cout<<"requester "<< num <<" track "<<disks[num].front()<<endl;
    	diskQueue.push_back(make_pair(num, disks[num].front()));
    	disks[num].pop();
    	totalnum--;
    }
    cv2.signal();
    mutex1.unlock();
}

void service_request(void* a){
	//cout<<"111\n";
    mutex1.lock();
    for(int i=0; i<disks.size(); ++i){
    	thread t2 ((thread_startfunc_t) requester, (void *)i);
    }
    mutex1.unlock();
    mutex1.lock();
    while(diskQueue.size()<max_disk_queue && totalnum>0){
    	cv2.wait(mutex1);
    }
    int min=abs(track-diskQueue[0].second);
    int n=0;
    for(int i=1; i<diskQueue.size(); ++i){
    	if(abs(track-diskQueue[i].second) < min){
    		min=abs(track-diskQueue[i].second);
    		n=i;
    	}
    }
    track=diskQueue[n].second;
    cout<<"service requester "<<diskQueue[n].first<<" track "<<diskQueue[n].second<<endl;
    diskQueue.erase(diskQueue.begin()+n);
    cv1.signal();
    mutex1.unlock();
}

int main(int argc, char * argv[]){
	max_disk_queue = atoi(argv[1]);
	for(int i=2; i<argc; ++i){
		ifstream fs;
		fs.open(argv[i]);
		queue<int> tmp;
		int t;
		while(fs >> t){
			tmp.push(t);
			totalnum++;
		}
		fs.close();
		disks.push_back(tmp);
		//cout<<disks.back().size()<<endl;
	}

	cpu::boot((thread_startfunc_t) service_request, (void *) 0, 0);
}