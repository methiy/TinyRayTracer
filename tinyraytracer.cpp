#include <tuple>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cmath>

struct vec3 {
    float x=0, y=0, z=0;

    // 重载下标 0:x 1:y 2:z
    //重载 operator[] 返回值需要 reference 和 const reference
    float& operator[](const int i)       { return i==0 ? x : (1==i ? y : z); }
    const float& operator[](const int i) const { return i==0 ? x : (1==i ? y : z); }

    vec3  operator*(const float v) const { return {x*v, y*v, z*v};       }//向量数乘
    float operator*(const vec3& v) const { return x*v.x + y*v.y + z*v.z; }//向量点乘
    vec3  operator+(const vec3& v) const { return {x+v.x, y+v.y, z+v.z}; }//向量相加
    vec3  operator-(const vec3& v) const { return {x-v.x, y-v.y, z-v.z}; }//向量相减
    vec3  operator-()              const { return {-x, -y, -z};          }

    float norm() const { return std::sqrt(x*x+y*y+z*z); }//点到原点的距离
    vec3 normalized() const { return (*this)*(1.f/norm()); }//求此向量方向上的单位向量
};

// vec3 cross(const vec3 v1, const vec3 v2) {
//     return { v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x };
// }


//材料
struct Material {
    float refractive_index = 1;//折射率

    //albedo 用于控制漫反射、高光、反射、折射产生的颜色强度
    //diffuse_light_intensity：漫反射光照强度
    //specular_light_intensity：镜面（高光）光照强度
    //reflect_color：反射光照强度
    //refract_color：折射光照强度
    float albedo[4] = {2,0,0,0};
    
    vec3 diffuse_color = {0,0,0};//默认颜色
    float specular_exponent = 0;//高光系数
};

//球体
struct Sphere {
    vec3 center;//球心
    float radius;//半径
    Material material;//材料
};

//设置了几种材料
constexpr Material      ivory = {1.0, {0.9,  0.5, 0.1, 0.0}, {0.4, 0.4, 0.3},   50.};
constexpr Material      glass = {1.5, {0.0,  0.9, 0.1, 0.8}, {0.6, 0.7, 0.8},  125.};
constexpr Material red_rubber = {1.0, {1.4,  0.3, 0.0, 0.0}, {0.3, 0.1, 0.1},   10.};
constexpr Material     mirror = {1.0, {0.0, 16.0, 0.8, 0.0}, {1.0, 1.0, 1.0}, 1425.};
//设置了几种球体
constexpr Sphere spheres[] = {
    {{-3,    0,   -16}, 2,      ivory},
    {{-1.0, -1.5, -12}, 2,      glass},
    {{ 1.5, -0.5, -18}, 3, red_rubber},
    {{ 7,    5,   -18}, 4,     mirror}
};
//设置了光照
constexpr vec3 lights[] = {
    {-20, 20,  20},
    { 30, 50, -25},
    { 30, 20,  30}
};

//根据入射光线求出反射光线
//I：入射光线
//N：单位法向量
vec3 reflect(const vec3 &I, const vec3 &N) {
    return I - N*2.f*(I*N);
}

//eta_t:光在物体中的折射率 -> n2
//eta_i:光在空气中的折射率 -> n1
//n_1 * sinθ1 =n_2 * sinθ2
//我们知道入射光线I、入射角θ1、n1和n2，求折射光线
//cos i 入射角和法线的夹角
vec3 refract(const vec3 &I, const vec3 &N, const float eta_t, const float eta_i=1.f) { // Snell's law
    float cosi = - std::max(-1.f, std::min(1.f, I*N));//判断入射方向 是从空气到物体 还是 物体到空气
    
    if (cosi<0) return refract(I, -N, eta_i, eta_t); //如果光从物体到空气，就翻转法向量，然后交换折射率

    float eta = eta_i / eta_t;//eta=n1/n2

    // float k = 1 - eta*eta*(1 - cosi*cosi);//k=cos θ2 * cos θ2 源
    float k = std::sqrt(1 - eta*eta*(1 - cosi*cosi));//k=cos θ2 改

    //k<0 折射角大于90度，发生全反射 
    //k>=0 
    // return k<0 ? vec3{1,0,0} : I*eta + N*(eta*cosi - std::sqrt(k));// 源
    return k<0 ? vec3{1,0,0} : I*eta + N*(eta*cosi - k);// 改
}

//返回 能否交 交点里orig多远
std::tuple<bool,float> ray_sphere_intersect(const vec3 &orig, const vec3 &dir, const Sphere &s) { // ret value is a pair [intersection found, distance]
    vec3 L = s.center - orig;
    float tca = L*dir;
    float d2 = L*L - tca*tca;//球心到射线的距离
    if (d2 > s.radius*s.radius) return {false, 0};
    float thc = std::sqrt(s.radius*s.radius - d2);
    float t0 = tca-thc, t1 = tca+thc;

    //射线的起点可能在球体里
    if (t0>.001) return {true, t0};  // offset the original point by .001 to avoid occlusion by the object itself
    if (t1>.001) return {true, t1};
    return {false, 0};
}

//返回存不存在遮盖 射线方向 圆心到交点的方向 射线射到的最近的球体的材料
std::tuple<bool,vec3,vec3,Material> scene_intersect(const vec3 &orig, const vec3 &dir) {
    vec3 pt, N;
    Material material;

    float nearest_dist = 1e10;

    //???
    if (std::abs(dir.y)>.001) { // intersect the ray with the checkerboard, avoid division by zero
        float d = -(orig.y+4)/dir.y; // the checkerboard plane has equation y = -4
        vec3 p = orig + dir*d;
        if (d>.001 && d<nearest_dist && std::abs(p.x)<10 && p.z<-10 && p.z>-30) {
            nearest_dist = d;
            pt = p;
            N = {0,1,0};
            material.diffuse_color = (int(.5*pt.x+1000) + int(.5*pt.z)) & 1 ? vec3{.3, .3, .3} : vec3{.3, .2, .1};
        }
    }

    //射球体 两个球会交叉 看哪个在表面
    for (const Sphere &s : spheres) { // intersect the ray with all spheres
        auto [intersection, d] = ray_sphere_intersect(orig, dir, s);
        if (!intersection || d > nearest_dist) continue;
        nearest_dist = d;
        //pt: 射线方向
        pt = orig + dir*nearest_dist;
        //圆心到交点的方向
        N = (pt - s.center).normalized();
        material = s.material;
    }
    return { nearest_dist<1000, pt, N, material };
}

//返回显示的颜色
vec3 cast_ray(const vec3 &orig, const vec3 &dir, const int depth=0) {
    auto [hit, point, N, material] = scene_intersect(orig, dir);
    if (depth>4 || !hit)
        return {0.2, 0.7, 0.8}; // background color

    vec3 reflect_dir = reflect(dir, N).normalized();
    vec3 refract_dir = refract(dir, N, material.refractive_index).normalized();
    vec3 reflect_color = cast_ray(point, reflect_dir, depth + 1);
    vec3 refract_color = cast_ray(point, refract_dir, depth + 1);

    float diffuse_light_intensity = 0, specular_light_intensity = 0;
    //diffuse_light_intensity 漫射光强度
    //specular_light_intensity 镜面光强度
    for (const vec3 &light : lights) { // checking if the point lies in the shadow of the light
        vec3 light_dir = (light - point).normalized();
        auto [hit, shadow_pt, trashnrm, trashmat] = scene_intersect(point, light_dir);//光线

        //打到其他物体 且在原物体和光源之间 那么就被遮盖了
        if (hit && (shadow_pt-point).norm() < (light-point).norm()) continue;

        diffuse_light_intensity  += std::max(0.f, light_dir*N);
        specular_light_intensity += std::pow(std::max(0.f, -reflect(-light_dir, N)*dir), material.specular_exponent);
    }

    //物体原色*反射光强度*物体系数 白色*镜面光强度*物体 反射和折射分别乘物体的这些系数=显示的颜色
    return material.diffuse_color * diffuse_light_intensity * material.albedo[0] + vec3{1., 1., 1.}*specular_light_intensity * material.albedo[1] + reflect_color*material.albedo[2] + refract_color*material.albedo[3];
}

int main() {
    constexpr int   width  = 1024;
    constexpr int   height = 768;
    constexpr float fov    = 1.05; // 60 degrees field of view in radians
    std::vector<vec3> framebuffer(width*height);
    
    //多线程运行下面的for
#pragma omp parallel for
    for (int pix = 0; pix<width*height; pix++) { // actual rendering loop
        float dir_x =  (pix%width + 0.5) -  width/2.;
        float dir_y = -(pix/width + 0.5) + height/2.; // this flips the image at the same time
        float dir_z = -height/(2.*tan(fov/2.));
        framebuffer[pix] = cast_ray(vec3{0,0,0}, vec3{dir_x, dir_y, dir_z}.normalized());
    }

    std::ofstream ofs("./out.ppm", std::ios::binary);
    ofs << "P6\n" << width << " " << height << "\n255\n";
    for (vec3 &color : framebuffer) {
        float max = std::max(1.f, std::max(color[0], std::max(color[1], color[2])));
        for (int chan : {0,1,2})
            ofs << (char)(255 *  color[chan]/max);
    }
    return 0;
}