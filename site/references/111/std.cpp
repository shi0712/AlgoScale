#include <bits/stdc++.h>
using namespace std;

// Finite decision tree for the six published strategies. --certificate also tests smaller horizons.
int horizon;
int policy[21][64][3][11];
int winning_mask(int turn,int previous,int move) {
    int strategy[6]={0,1,2,turn%3,turn?previous:0,turn?(previous+1)%3:0};
    int mask=0;
    for(int i=0;i<6;++i)if((strategy[i]+1)%3==move)mask|=1<<i;
    return mask;
}
bool feasible(int t,int mask,int prev,int streak) {
    if(streak==10)return true;
    if(streak+horizon-t<10)return false;
    int &choice=policy[t][mask][prev][streak];
    if(choice!=-2)return choice>=0;
    for(int move=0;move<3;++move) {
        int won=mask & winning_mask(t,prev,move);
        if(won && !feasible(t+1,won,move,streak+1))continue;
        if(mask!=won && !feasible(t+1,mask^won,move,0))continue;
        choice=move;return true;
    }
    choice=-1;return false;
}
int main(int argc,char**) {
    for(horizon=10;horizon<=20;++horizon) {
        for(auto& by_mask:policy)for(auto& by_prev:by_mask)
            for(auto& row:by_prev)fill(begin(row),end(row),-2);
        bool ok=feasible(0,63,0,0);
        if(argc>1)cout<<horizon<<' '<<ok<<'\n';
        if(ok)break;
    }
    if(argc>1)return 0;
    int mask=63,prev=0,streak=0;
    for(int t=0;streak<10;++t) {
        int move=policy[t][mask][prev][streak];
        if(move<0)return 0;
        cout<<"RPS"[move]<<endl;
        string reply;if(!(cin>>reply))return 0;
        int won=mask & winning_mask(t,prev,move);
        if(reply=="player") {mask=won;++streak;}
        else if(reply=="ai") {mask^=won;streak=0;}
        else return 0;
        if(!mask)return 0;
        prev=move;
    }
}

