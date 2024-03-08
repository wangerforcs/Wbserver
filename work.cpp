#include<iostream>
#include<string>
using namespace std;

int solve1(string a, string b){
    for(auto& i: a){
        if(i>='A' && i<='Z') i = i-'A'+'a';
    } 
    for(auto& i: b){
        if(i>='A' && i<='Z') i = i-'A'+'a';
    } 
    auto pos = a.find(b);
    return pos;
}

int main(){
    cout << solve1("abcdefg","Ab");
}