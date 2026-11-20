#include <cstdlib>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>
using namespace std;
int main() {
    int n;
    if (!(cin>>n)) return 0;
    string inside(n,'0');inside[0]='1';
    vector<int> outside;for(int i=1;i<n;++i) outside.push_back(i);
    auto query=[](const string& set) {
        cout << "? " << set << endl;
        int k;if(!(cin>>k) || k<0) exit(0);
        return k;
    };
    while(!outside.empty()) {
        int neighbors=query(inside);
        if(!neighbors) {cout << "! 0" << endl;return 0;}
        vector<int> choices=outside;
        while(choices.size()>1) {
            size_t mid=choices.size()/2;
            string half(n,'0'),united=inside;
            for(size_t i=0;i<mid;++i) half[choices[i]]=united[choices[i]]='1';
            int nh=query(half),nu=query(united);
            if(neighbors+nh!=nu) choices.resize(mid);
            else choices.erase(choices.begin(),choices.begin()+mid);
        }
        int v=choices[0];inside[v]='1';
        for(size_t i=0;i<outside.size();++i) if(outside[i]==v) {
            outside[i]=outside.back();outside.pop_back();break;
        }
    }
    cout << "! 1" << endl;
}
