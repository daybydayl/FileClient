## 仿Xftp客户端

![image-20241125101142027](C:\Users\lzj99\AppData\Roaming\Typora\typora-user-images\image-20241125101142027.png)



### 1.菜单栏





### 2.工具栏





### 3.服务端地址栏(输入栏)



### 4.主业务组件1|主业务组件2



### 5.辅助组件







Window下编译：

因Qt5.12.12的msvc只有2017的，所以程序也要置顶msvc2017的版本

mkdir build

cd build

cmake ..或cmake .. -G "Visual Studio 17 2022" -A x64 -T v141

cmake --build . --config Debug



linux下编译：

Qt5.9.7版本，gcc4.8.5版本

mkdir build

cd build

cmake ..

make
