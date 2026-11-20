#include <bits/stdc++.h>
using namespace std;

struct Stream {
    int value,length,time,last;
    bool operator>(const Stream& other) const {return value>other.value;}
};
int main() {
    ios::sync_with_stdio(false);cin.tie(nullptr);
    {
        int n,m;cin>>n>>m;
        vector<int> arrival(n*n+1);
        for(int row=1;row<=n;++row) for(int col=1;col<=n;++col) {
            int value;cin>>value;
            arrival[value]=row==1 ? 2-col : row+col-1;
        }
        int duration=2*n-1;
        vector<vector<int>> cache(n+1,vector<int>(duration+1));
        auto ask=[&](int length,int time) {
            int& value=cache[length][time];
            if(!value) {
                cout<<"? "<<length<<' '<<time<<endl;
                if(!(cin>>value) || value<1) exit(0);
            }
            return value;
        };
        priority_queue<Stream,vector<Stream>,greater<Stream>> heap;
        for(int length=1;length<=n;++length) {
            for(int begin=1;begin<=duration;begin+=length) {
                int end=min(duration,begin+length-1);
                int low=begin,high=end+1;
                while(high-low>1) {
                    int mid=(low+high)/2;
                    if(arrival[ask(length,mid)]>begin) high=mid;
                    else low=mid;
                }
                heap.push({ask(length,low),length,low,begin});
                if(high<=end) heap.push({ask(length,high),length,high,end});
            }
        }
        vector<int> answer;
        while(int(answer.size())<m) {
            auto s=heap.top();heap.pop();
            answer.push_back(s.value);
            if(s.time!=s.last && int(answer.size())<m) {
                s.time+=s.time<s.last ? 1 : -1;
                s.value=ask(s.length,s.time);heap.push(s);
            }
        }
        cout<<"!";
        for(int v:answer) cout<<' '<<v;
        cout<<endl;
    }
}
