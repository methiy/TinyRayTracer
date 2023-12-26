#include<bits/stdc++.h>
using namespace std;
typedef long long ll;
void s(){
	//交点：x=48/25 y=36/25
	//起始点：0,4 
	//终点：3,0
	//半径 24/5
	//圆心:2x 2y
	double x1,y1,x2,y2;cin>>x1>>y1>>x2>>y2;
	double rx,ry,r;cin>>rx>>ry>>r;
	double a=(y2-y1)/(x2-x1);
	double c=y1-a*x1;
	
	//a*x+c=y;
	//(rx-x)^2+(ry-y)^2=r^2
	
	//(rx-x)^2+(ry-a*x-c)^2=r^2
	//rx^2-2rx * x +x^2 +(ry-c)^2-2*a*(ry-c)*x+a*a*x*x=r^2;
	
	//求以下方程：
	//(a^2+1)x^2-2*(rx+a*ry-a*c)*x+(rx^2+(ry-c)^2-r*r)=0
	
	double A=a*a+1;
	double B=-2*(rx+a*ry-a*c);
	double C=rx*rx+(ry-c)*(ry-c)-r*r;
	
	double fac=B*B-4*A*C;
	double X,X1,X2;
	
	if(fac>0){
		cout<<"two x to solve\n";
		X1=(-B-sqrt(fac))/(2*A);
		X2=(-B+sqrt(fac))/(2*A);
		cout<<X1<<' '<<X2<<endl;
	}else if(fac==0){
		cout<<"ont x to solve\n";
		X=(-B)/(2*A);
		cout<<X<<endl;
		
	}else
	{
		cout<<"not x to solve\n";
		
	}

}
void solve(){
	s();
	
}
int main(){
	ios::sync_with_stdio(0);
	cin.tie(0);
	int t=1;
//	cin>>t;
	while(t--){
		solve();
	}
}


