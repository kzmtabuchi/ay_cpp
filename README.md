テスト用フォーク

ay_cpp
==================
C++ libraries for robot programming, including geometry utility and optimization.  This package is compiled with ROS-build, but can also be compiled without ROS.


Author
==================
Akihiko Yamaguchi, http://akihikoy.net/


Acknowledgment
==================
CMA-ES (include/ay_cpp/3rdparty/cma_es) is implemented by Nikolaus Hansen.  Read include/ay_cpp/3rdparty/cma_es/README.md for more information.


Requirements
==================
- Boost, Eigen


Build
==================
Build `ay_cpp` with `rosmake`.

```
$ rosmake ay_cpp
```

After `rosmake`, you will find a shared object in `lib/` directory.
There will be some directories made by `rosmake`.


Usage
==================
See `ay_vision` and `ay_3dvision` for examples.


Troubles
==================
Send e-mails to the author.
