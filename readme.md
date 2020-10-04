# Versatile Sketch

### File Descriptions

- `vs.h/cpp` contains the implementation of our Versatile Sketch.
- `gs.h/cpp` contains the implementation of generated sketch from Versatile Sketch.
- `cm.h/cpp` contains the implementation of CM Sketch.
- `cu.h/cpp` contains the implementation of CU Sketch.
- `as.h/cpp` contains the implementation of A-Sketch.
- `ss.h/cpp` contains the implementation of Space Saving.
- `minheap.h/cpp` contains a simple min-heap, which is used for CM and CU Sketch to record top-k items.
- `BOBHash32.h/cpp` contains the implementation of Bob Hash.
- `main.cpp` is the entrance of all benchmarks.
- `realtime_tasks` folder contains the codes of cardinality, frequency distribution and entropy estimation.

### Usages

- Modify the code of function `main()` in `main.cpp` to evoke test functions for corresponding tasks.
- Type `make` to build benchmark program and `./benchmark` to run it.


### Notes

- All sketch classes are derived from `class Sketch` so that you can push any sketch to the vector `sk`, which is sent to the test function. 
- Before evoking test functions, you can change the size of sketches.
- Datasets CAIDA and Web Page can be found in the reference of the paper.