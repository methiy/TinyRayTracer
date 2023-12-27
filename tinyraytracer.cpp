#include<iostream>
#include<fstream>
#include<vector>
#include<cmath>

struct vec3 {
    float x=0, y=0, z=0;
    float& operator[](const int i)       { return i==0 ? x : (1==i ? y : z); }
    const float& operator[](const int i) const { return i==0 ? x : (1==i ? y : z); }
    vec3  operator*(const float v) const { return {x*v, y*v, z*v};       }
    float operator*(const vec3& v) const { return x*v.x + y*v.y + z*v.z; }
    vec3  operator+(const vec3& v) const { return {x+v.x, y+v.y, z+v.z}; }
    vec3  operator-(const vec3& v) const { return {x-v.x, y-v.y, z-v.z}; }
    vec3  operator-()              const { return {-x, -y, -z};          }
    float norm() const { return std::sqrt(x*x+y*y+z*z); }
    vec3 normalized() const { return (*this)*(1.f/norm()); }
};
int main(){

    const int width = 1024;
    const int height=768;
    std::vector<vec3> framebuffer(width*height);

    for(size_t  j=0;j<height;j++)
        for(size_t i=0;i<width;i++){
            framebuffer[i+j*width]=vec3{j/float(height),i/float(width),0};
        }

    std::ofstream ofs;
    ofs.open("./out.ppm",std::ios::binary);

    ofs<<"P6\n"<<width<<" "<<height<<"\n255\n";

    for(size_t i=0;i<width*height;i++){
        for(size_t j=0;j<3;j++){
            ofs<<(char)(framebuffer[i][j]*255);
        }
    }
    ofs.close();
}